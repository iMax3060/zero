/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

#include "bf_tree_cleaner.h"
#include "w_gettimeofday.h"
#include "sm_int_0.h"
#include "sm_int_1.h"
#include "bf_tree_cb.h"
#include "bf_tree_vol.h"
#include "bf_tree.h"
#include "bf_fixed.h"
#include "w_autodel.h"
#include "generic_page.h"
#include "bf.h"
#include "page.h"
#include "sm_io.h"
#include "log.h"
#include "vol.h"
#include <string.h>
#include <vector>
#include <algorithm>
#include <stdlib.h>

#include "sm.h"
#include "xct.h"


typedef bf_tree_cleaner_slave_thread_t* slave_ptr;


bool _dirty_shutdown_happening() {
    return (ss_m::shutting_down && !ss_m::shutdown_clean);
}


bf_tree_cleaner::bf_tree_cleaner(bf_tree_m* bufferpool, uint32_t cleaner_threads,
    uint32_t cleaner_interval_millisec_min,
    uint32_t cleaner_interval_millisec_max,
    uint32_t cleaner_write_buffer_pages,
    bool initially_wakeup_workers) :
    _bufferpool(bufferpool),
    _requested_lsn(lsndata_null),
    _slave_threads(new slave_ptr[cleaner_threads + 1]), // +1 for id=0
    _slave_threads_size (cleaner_threads + 1), // +1 for id=0
    _cleaner_interval_millisec_min (cleaner_interval_millisec_min),
    _cleaner_interval_millisec_max (cleaner_interval_millisec_max),
    _cleaner_write_buffer_pages (cleaner_write_buffer_pages),
    _initially_wakeup_workers(initially_wakeup_workers)
{
    w_assert1(cleaner_threads >= 1);
    
    // assign volumes to workers in a round robin fashion.
    // this assignment is totally static.
    _volume_assignments[0] = 0;
    for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
        _volume_assignments[vol] = _volume_assignments[vol - 1] + 1;
        if (_volume_assignments[vol] >= _slave_threads_size) {
            _volume_assignments[vol] = 1;
        }
    }
    
    ::memset (_requested_volumes, 0, sizeof(bool) * MAX_VOL_COUNT);

    _slave_threads[0] = NULL;
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        _slave_threads[id] = new bf_tree_cleaner_slave_thread_t (this, id);
    }

   _error_happened = false;
}

bf_tree_cleaner::~bf_tree_cleaner()
{
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        delete _slave_threads[id];
        _slave_threads[id] = NULL;
    }
    delete[] _slave_threads;
    _slave_threads = NULL;
}

w_rc_t bf_tree_cleaner::start_cleaners()
{
    DBGOUT1(<<"bf_tree_cleaner: starting " << (_slave_threads_size - 1) << " cleaner threads.. _initially_wakeup_workers=" << _initially_wakeup_workers);
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        w_assert1(_slave_threads[id] != NULL);
        _slave_threads[id]->_start_requested = _initially_wakeup_workers;
        W_DO(_slave_threads[id]->fork());
    }
    DBGOUT1(<<"bf_tree_cleaner: started cleaner threads");
    return RCOK;
}

w_rc_t bf_tree_cleaner::wakeup_cleaners()
{
    DBGOUT2(<<"bf_tree_cleaner: waking up all cleaner threads");
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        W_DO(_wakeup_a_cleaner (id));
    }
    return RCOK;
}

w_rc_t bf_tree_cleaner::wakeup_cleaner_for_volume(volid_t vol)
{
    DBGOUT2(<<"bf_tree_cleaner: waking up the cleaner for volume:" << vol);
    bf_cleaner_slave_id_t id = _volume_assignments[vol];
    W_DO(_wakeup_a_cleaner (id));
    return RCOK;
}

w_rc_t bf_tree_cleaner::_wakeup_a_cleaner(bf_cleaner_slave_id_t id)
{
    DBGOUT3(<<"bf_tree_cleaner: waking up cleaner " << id);
    w_assert1(id > 0);
    w_assert1(id < _slave_threads_size);
    w_assert1(_slave_threads[id] != NULL);
    if (!_slave_threads[id]->_running) {
        DBGOUT1(<<"bf_tree_cleaner: cleaner " << id << " has already shut down. didn't wake it up");
        return RCOK;
    }
    if (_slave_threads[id]->_start_requested == false) {
        DBGOUT1(<<"bf_tree_cleaner: wakeup_cleaners: cleaner thread " << id << " hasn't been activated. it's now activated");
        _slave_threads[id]->_start_requested = true;
    }
    
    _slave_threads[id]->wakeup();
    return RCOK;
}

w_rc_t bf_tree_cleaner::request_stop_cleaners()
{
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        _slave_threads[id]->_stop_requested = true;
        if (_slave_threads[id]->_running) {
            W_DO(_wakeup_a_cleaner (id));
        }
    }
    return RCOK;
}


w_rc_t bf_tree_cleaner::join_cleaners(uint32_t max_wait_millisec)
{
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        if (_slave_threads[id]->_running) {
            if (max_wait_millisec == 0) {
                W_DO(_slave_threads[id]->join(sthread_base_t::WAIT_FOREVER));
            } else {
                W_DO(_slave_threads[id]->join(max_wait_millisec));
            }
        }
    }
    return RCOK;
}
/*
w_rc_t bf_tree_cleaner::kill_cleaners()
{
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        if (_slave_threads[id]->_running) {
            _slave_threads[id]->
        }
    }

}
*/

const uint32_t FORCE_SLEEP_MS_MIN = 10;
const uint32_t FORCE_SLEEP_MS_MAX = 1000;


w_rc_t bf_tree_cleaner::force_all()
{
    W_DO (io_m::flush_all_fixed_buffer());
    ::memset(_requested_volumes, 1, sizeof(bool) * MAX_VOL_COUNT);
    W_DO(wakeup_cleaners());
    uint32_t interval = FORCE_SLEEP_MS_MIN;
    while (!_dirty_shutdown_happening() && !_error_happened) {
        DBGOUT2(<< "waiting in force_all...");
        g_me()->sleep(interval);
        interval *= 2;
        if (interval >= FORCE_SLEEP_MS_MAX) {
            interval = FORCE_SLEEP_MS_MAX;
        }
        bool remains = false;
        for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
            if (_requested_volumes[vol]) {
                remains = true;
                break;
            }
        }
        if (!remains) {
            break;
        }
    }
    if (_dirty_shutdown_happening()) {
        DBGOUT1(<< "joining all cleaner threads up to 100ms...");
        W_DO(join_cleaners(100));
    }
    DBGOUT2(<< "done force_all!");
    return RCOK;
}

bool bf_tree_cleaner::_is_force_until_lsn_done(lsndata_t lsn) const
{
    for (bf_cleaner_slave_id_t id = 1; id < _slave_threads_size; ++id) {
        if (_slave_threads[id]->_completed_lsn < lsn) {
            return false;
        }
    }
    return true;
}

w_rc_t bf_tree_cleaner::force_until_lsn(lsndata_t lsn)
{
    W_DO (io_m::flush_all_fixed_buffer());
    if (_is_force_until_lsn_done(lsn)) {
        return RCOK;
    }
    _requested_lsn = lsn;
    W_DO(wakeup_cleaners());
    uint32_t interval = FORCE_SLEEP_MS_MIN;
    while (!_dirty_shutdown_happening() && !_error_happened) {
        DBGOUT2(<< "waiting in force_until_lsn...");
        if (lsn > _requested_lsn) {
            // make it sure because a concurrent thread might have overwritten it
            lsn = _requested_lsn;
        }
        g_me()->sleep(interval);
        interval *= 2;
        if (interval >= FORCE_SLEEP_MS_MAX) {
            interval = FORCE_SLEEP_MS_MAX;
        }
        if (_is_force_until_lsn_done(lsn)) {
            break;
        }
    }
    if (_dirty_shutdown_happening()) {
        DBGOUT1(<< "joining all cleaner threads up to 100ms...");
        W_DO(join_cleaners(100));
    }
    DBGOUT2(<< "done force_until_lsn!");
    return RCOK;
}

w_rc_t bf_tree_cleaner::force_volume(volid_t vol)
{
    if (_bufferpool->_volumes[vol] == NULL) {
        DBGOUT2(<< "volume " << vol << " is not mounted");
        return RCOK;
    }
    W_DO (_bufferpool->_volumes[vol]->_volume->get_fixed_bf()->flush());
    _requested_volumes[vol] = true;
    W_DO(wakeup_cleaner_for_volume(vol));
    uint32_t interval = FORCE_SLEEP_MS_MIN;
    while (_requested_volumes[vol] && !_dirty_shutdown_happening() && !_error_happened) {
        DBGOUT2(<< "waiting in force_volume...");
        g_me()->sleep(interval);
        interval *= 2;
        if (interval >= FORCE_SLEEP_MS_MAX) {
            interval = FORCE_SLEEP_MS_MAX;
        }
    }
    if (_dirty_shutdown_happening()) {
        DBGOUT1(<< "joining all cleaner threads up to 100ms...");
        W_DO(join_cleaners(100));
    }
    DBGOUT2(<< "done force_volume!");
    return RCOK;
}


const int INITIAL_SORT_BUFFER_SIZE = 64;

bf_tree_cleaner_slave_thread_t::bf_tree_cleaner_slave_thread_t(bf_tree_cleaner* parent, bf_cleaner_slave_id_t id)
    : _parent(parent), _id(id), _start_requested(false), _running(false), _stop_requested(false), _wakeup_requested(false),
    _interval_millisec(parent->_cleaner_interval_millisec_min),  _completed_lsn(lsndata_null),
    _sort_buffer (new uint64_t[INITIAL_SORT_BUFFER_SIZE]), _sort_buffer_size(INITIAL_SORT_BUFFER_SIZE)
{
    ::pthread_mutex_init(&_interval_mutex, NULL);
    ::pthread_cond_init(&_interval_cond, NULL);
    for (volid_t vol = 0; vol < MAX_VOL_COUNT; ++vol) {
        _candidates_buffer.push_back (std::vector<bf_idx>());
    }
    w_assert1(_candidates_buffer.size() == MAX_VOL_COUNT);

    // use posix_memalign because the write buffer might be used for raw disk I/O
    void *buf = NULL;
    ::posix_memalign(&buf, SM_PAGESIZE, SM_PAGESIZE * (parent->_cleaner_write_buffer_pages + 1)); // +1 margin for switching to next batch
    w_assert0(buf != NULL);
    _write_buffer = reinterpret_cast<generic_page*>(buf);
    
    _write_buffer_indexes = new bf_idx[parent->_cleaner_write_buffer_pages + 1];
}

bf_tree_cleaner_slave_thread_t::~bf_tree_cleaner_slave_thread_t()
{
    ::pthread_mutex_destroy(&_interval_mutex);
    ::pthread_cond_destroy(&_interval_cond);
    delete[] _sort_buffer;

    void *buf = reinterpret_cast<void*>(_write_buffer);
    // note we use free(), not delete[], which corresponds to posix_memalign
    ::free (buf);
    
    delete[] _write_buffer_indexes;
}

void bf_tree_cleaner_slave_thread_t::_take_interval()
{
    if (_dirty_shutdown_happening()) {
        return;
    }
    bool timeouted = _cond_timedwait((uint64_t) _interval_millisec * 1000);
    if (timeouted) {
        // exponentially grow the interval
        _interval_millisec *= 2;
        if (_interval_millisec > _parent->_cleaner_interval_millisec_max) {
            _interval_millisec = _parent->_cleaner_interval_millisec_max;
        }
    } else {
        // restart the interval from minimal value
        _interval_millisec = _parent->_cleaner_interval_millisec_min;
    }
}

bool bf_tree_cleaner_slave_thread_t::_cond_timedwait (uint64_t timeout_microsec) {
    W_IFDEBUG1(int rc_mutex_lock = )
        ::pthread_mutex_lock (&_interval_mutex);
    w_assert1(rc_mutex_lock == 0);

    timespec   ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += timeout_microsec * 1000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec = ts.tv_nsec % 1000000000;    

#if W_DEBUG_LEVEL>=2
    timespec   ts_init;
    ::clock_gettime(CLOCK_REALTIME, &ts_init);
    DBGOUT2(<< "bf_tree_cleaner_slave_thread_t: waiting for " << timeout_microsec << " us..");
#endif // W_DEBUG_LEVEL>=2
    
    bool timeouted = false;
    while (!_wakeup_requested) {
        int rc_wait =
            ::pthread_cond_timedwait(&_interval_cond, &_interval_mutex, &ts);

#if W_DEBUG_LEVEL>=2
    timespec   ts_end;
    ::clock_gettime(CLOCK_REALTIME, &ts_end);
    double ts_diff = (ts_end.tv_sec - ts_init.tv_sec) * 1000000 + (ts_end.tv_nsec - ts_init.tv_nsec) / 1000.0;
    DBGOUT2(<< "bf_tree_cleaner_slave_thread_t: waked up after " << ts_diff << " usec. wait-ret="
        << rc_wait << (rc_wait == ETIMEDOUT ? "(ETIMEDOUT)" : ""));
#endif // W_DEBUG_LEVEL>=2

        if (rc_wait == ETIMEDOUT) {
#if W_DEBUG_LEVEL>=2
            timespec   ts_now;
            ::clock_gettime(CLOCK_REALTIME, &ts_now);
            if (ts_now.tv_sec > ts.tv_sec || (ts_now.tv_sec == ts.tv_sec && ts_now.tv_nsec >= ts.tv_nsec)) {
            } else {
                DBGOUT2(<< "Seems a fake ETIMEDOUT?? Trying again");
                // ah, CLOCK_MONOTONIC caused this. CLOCK_REALTIME doesn't have this issue.
            }
#endif // W_DEBUG_LEVEL>=2
            DBGOUT2(<<"timeouted");
            timeouted = true;
            break;
        }
    }

    W_IFDEBUG1(int rc_mutex_unlock = )
        ::pthread_mutex_unlock (&_interval_mutex);
    w_assert1(rc_mutex_unlock == 0);
    _wakeup_requested = false;
    return timeouted;
}

void bf_tree_cleaner_slave_thread_t::wakeup()
{
    _wakeup_requested = true;
    W_IFDEBUG1(int rc_mutex_lock = )
        ::pthread_mutex_lock (&_interval_mutex);
    w_assert1(rc_mutex_lock == 0);

    W_IFDEBUG1(int rc_broadcast = )
        ::pthread_cond_broadcast(&_interval_cond);
    w_assert1(rc_broadcast == 0);

    W_IFDEBUG1(int rc_mutex_unlock = )
        ::pthread_mutex_unlock (&_interval_mutex);
    w_assert1(rc_mutex_unlock == 0);
}

void bf_tree_cleaner_slave_thread_t::run()
{
    DBGOUT1(<<"bf_tree_cleaner_slave_thread_t: cleaner- " << _id << " has been created");
    _running = true;
    while (!_start_requested) {
        _take_interval();
    }

    DBGOUT1(<<"bf_tree_cleaner_slave_thread_t: cleaner- " << _id << " starts up");
    while (!_stop_requested && !_parent->_error_happened) {
        if (_dirty_shutdown_happening()) {
            ERROUT (<<"the system is going to shutdown dirtily. this cleaner will shutdown without flushing!");
            break;
        }
        w_rc_t rc = _do_work();
        if (rc.is_error()) {
            ERROUT (<<"cleaner thread error:" << rc);
            _parent->_error_happened = true;
            break;
        }
        if (!_stop_requested && !_exists_requested_work() && !_parent->_error_happened) {
            _take_interval();
        }
    }

    _running = false;
    DBGOUT1(<<"bf_tree_cleaner_slave_thread_t: cleaner- " << _id << " shuts down");
}

bool bf_tree_cleaner_slave_thread_t::_exists_requested_work()
{
    // the cleaner is requested to make a checkpoint. we need to immediately start it
    if (_parent->_requested_lsn > _completed_lsn) {
        return true;
    }
    
    // the cleaner is requested to flush out all dirty pages for assigned volume. let's do it immediately
    for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
        if (_parent->_volume_assignments[vol] == _id && _parent->_requested_volumes[vol]) {
            return true;
        }
    }
    
    // otherwise let's take a sleep
    return false;
}


w_rc_t bf_tree_cleaner_slave_thread_t::_clean_volume(
    volid_t vol, const std::vector<bf_idx> &candidates,
    bool requested_volume, lsndata_t requested_lsn)
{
    if (_dirty_shutdown_happening()) return RCOK;
    // TODO this method should separate dirty pages that have dependency and flush them after others.
    DBGOUT1(<<"_clean_volume(cleaner=" << _id << "): volume " << vol);
    if (_sort_buffer_size < candidates.size()) {
        size_t new_buffer_size = 2 * candidates.size();
        DBGOUT2(<<"_clean_volume(cleaner=" << _id << "): resize sort buffer " << _sort_buffer_size << "->" << new_buffer_size);
        uint64_t* new_sort_buffer = new uint64_t[new_buffer_size];
        if (new_sort_buffer == NULL) {
            return RC(smlevel_0::eOUTOFMEMORY);
        }
        delete[] _sort_buffer;
        _sort_buffer = new_sort_buffer;
        _sort_buffer_size = new_buffer_size;
    }

    // again, we don't take latch at this point because this sorting is just for efficiency.
    size_t sort_buf_used = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        bf_idx idx = candidates[i];
        bf_tree_cb_t &cb = _parent->_bufferpool->get_cb(idx);
        if (cb._pid_vol != vol) {
            DBGOUT1(<<"_clean_volume(cleaner=" << _id << "): volume " << vol << ". this candidate has changed its volume? idx=" << idx << ". current vol=" << cb._pid_vol);
            continue;
        }
        w_assert1(cb._pid_shpid >= _parent->_bufferpool->_volumes[vol]->_volume->first_data_pageid());
        w_assert1(_parent->_bufferpool->_buffer->lsn.valid());
        _sort_buffer[sort_buf_used] = (((uint64_t) cb._pid_shpid) << 32) + ((uint64_t) idx);
        ++sort_buf_used;
    }
    DBGOUT2(<<"_clean_volume(cleaner=" << _id << "): sorting " << sort_buf_used << " entries..");
    std::sort(_sort_buffer, _sort_buffer + sort_buf_used);
    DBGOUT2(<<"_clean_volume(cleaner=" << _id << "): done sorting");
    
    // now, write them out as sequentially as possible.
    // Note that
    // 1) at this point we need to take SH latch while copying the page and doing the real check
    // 2) there might be duplicates in _sort_buffer. it can be easily detected because it's sorted.
    size_t write_buffer_from = 0;
    size_t write_buffer_cur = 0;
    shpid_t prev_idx = 0; // to check duplicates
    shpid_t prev_shpid = 0; // to check if it's contiguous
    const generic_page* const page_buffer = _parent->_bufferpool->_buffer;
    int rounds = 0;
    while (true) {
        bool skipped_something = false;
        for (size_t i = 0; i < sort_buf_used; ++i) {
            if (write_buffer_cur == _parent->_cleaner_write_buffer_pages) {
                // now the buffer is full. flush it out and also reset the buffer
                W_DO(_flush_write_buffer (vol, write_buffer_from, write_buffer_cur - write_buffer_from));
                write_buffer_from = 0;
                write_buffer_cur = 0;
            }
            bf_idx idx = (bf_idx) (_sort_buffer[i] & 0xFFFFFFFF); // extract bf_idx
            w_assert1(idx > 0);
            w_assert1(idx < _parent->_bufferpool->_block_cnt);
            if (idx == prev_idx) {
                continue;
            }
            bf_tree_cb_t &cb = _parent->_bufferpool->get_cb(idx);
            if (!cb._dirty || !cb._used) {
                continue;
            }
            // just copy the page, and release the latch as soon as possible.
            // also tentatively skip an EX-latched page to avoid being interrepted by
            // a single EX latch for too long time.
            bool tobedeleted = false;
            w_rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, WAIT_IMMEDIATE);
            if (latch_rc.is_error()) {
                DBGOUT2 (<< "tentatively skipped an EX-latched page. " << i << "=" << page_buffer[idx].pid << ". rc=" << latch_rc);
                skipped_something = true;
                continue;
            }
            if ((page_buffer[idx].page_flags & t_tobedeleted) != 0) {
                tobedeleted = true;
            } else {
                ::memcpy(_write_buffer + write_buffer_cur, page_buffer + idx, sizeof (generic_page)); 
                // if the page contains a swizzled pointer, we need to convert the data back to the original pointer.
                // we need to do this before releasing SH latch because the pointer might be unswizzled by other threads.
                _parent->_bufferpool->_convert_to_disk_page(_write_buffer + write_buffer_cur);// convert swizzled data.
                w_assert1(_write_buffer[write_buffer_cur].pid.vol().vol > 0);
                w_assert1(_write_buffer[write_buffer_cur].pid.page > 0);
            }
            cb.latch().latch_release();
            
            // then, re-calculate the checksum.
            _write_buffer[write_buffer_cur].update_checksum();
            // also copy the new checksum to make the original (bufferpool) page consistent.
            // we can do this out of latch scope because it's never used in race condition.
            // this is mainly for testcases and such.
            page_buffer[idx].checksum = _write_buffer[write_buffer_cur].checksum;

            if (tobedeleted) {
                // as it's deleted, no one should be accessing it now. we can do everything without latch
                DBGOUT2(<< "physically delete a page by buffer pool:" << page_buffer[idx].pid);
                // this operation requires a xct for logging. we create a ssx for this reason.
                sys_xct_section_t sxs(true); // ssx to call free_page
                W_DO (sxs.check_error_on_start());
                W_DO (_parent->_bufferpool->_volumes[vol]->_volume->free_page(page_buffer[idx].pid));
                W_DO (sxs.end_sys_xct (RCOK));
                // drop the page from bufferpool too
                _parent->_bufferpool->_delete_block(idx);
            } else {
                if (_write_buffer[write_buffer_cur].pid.vol().vol != vol) {
                    DBGOUT1(<<"_clean_volume(cleaner=" << _id << "): oops, this page now has a different volume?? "
                        << _write_buffer[write_buffer_cur].pid << " isn't in volume " << vol);
                    continue;
                }
                shpid_t shpid = _write_buffer[write_buffer_cur].pid.page;
                _write_buffer_indexes[write_buffer_cur] = idx;
                
                // if next page is not consecutive, flush it out
                if (write_buffer_from < write_buffer_cur && shpid != prev_shpid + 1) {
                    // flush up to _previous_ entry (before incrementing write_buffer_cur)
                    W_DO(_flush_write_buffer (vol, write_buffer_from, write_buffer_cur - write_buffer_from));
                    write_buffer_from = write_buffer_cur;
                }
                ++write_buffer_cur;
                        
                prev_idx = idx;
                prev_shpid = shpid;
            }
        }
        if (!skipped_something) {
            break;
        }
        if (!requested_volume && requested_lsn == lsndata_null) {
            // then, page cleaning is just opportunistic. we can quit.
            break;
        } else {
            // umm, we really need to make sure all of these are flushed out.
            ++rounds;
            if (rounds > 2) {
                DBGOUT1(<<"some dirty page seems to have a persistent EX latch. waiting... rounds=" << rounds);
                if (rounds > 50) {
                    ERROUT(<<"FATAL! some dirty page keeps EX latch for long time! failed to flush out the bufferpool");
                    return RC(smlevel_0::eINTERNAL);
                }
                g_me()->sleep(rounds > 5 ? 100 : 20);
            }
        }
    }
    if (write_buffer_cur > write_buffer_from) {
        W_DO(_flush_write_buffer (vol, write_buffer_from, write_buffer_cur - write_buffer_from));
        write_buffer_from = 0; // not required, but to make sure
        write_buffer_cur = 0;
    }
    
    DBGOUT1(<<"_clean_volume(cleaner=" << _id << "): done volume " << vol);
    return RCOK;
}


w_rc_t bf_tree_cleaner_slave_thread_t::_flush_write_buffer(volid_t vol, size_t from, size_t consecutive)
{
    if (_dirty_shutdown_happening()) return RCOK;
    if (consecutive == 0) {
        return RCOK;
    }
    DBGOUT2(<<"_flush_write_buffer(cleaner=" << _id << "): writing " << consecutive << " consecutive pages..");
    // we'll compute the highest lsn of the pages, and do one log flush to that lsn.
    lsndata_t max_page_lsn = lsndata_null; 
    for (size_t i = from; i < from + consecutive; ++i) {
        w_assert1(_write_buffer[i].pid.vol().vol == vol);
        if (i > from) {
            w_assert1(_write_buffer[i].pid.page == _write_buffer[i - 1].pid.page + 1);
        }
        if (_write_buffer[i].lsn.data() > max_page_lsn) {
            max_page_lsn = _write_buffer[i].lsn.data();
        }
    }


    // WAL: ensure that the log is durable to this lsn
    if (max_page_lsn != lsndata_null) {
        if (smlevel_0::log == NULL) {
            ERROUT (<<"cannot flush logs before flushing data. probably dirty shutdown?");
            return RC(smlevel_0::eINTERNAL);
        }
        W_COERCE( smlevel_0::log->flush(lsn_t(max_page_lsn)) );
    }

    // then, write them out in one batch
    W_COERCE(_parent->_bufferpool->_volumes[vol]->_volume->write_many_pages(_write_buffer[from].pid.page, _write_buffer + from, consecutive));

    // after writing them out, update rec_lsn in bufferpool
    for (size_t i = from; i < from + consecutive; ++i) {
        bf_idx idx = _write_buffer_indexes[i];
        bf_tree_cb_t &cb = _parent->_bufferpool->get_cb(idx);
        cb._dirty = false;
        --_parent->_bufferpool->_dirty_page_count_approximate;
        cb._rec_lsn = _write_buffer[i].lsn.data();
        cb._dependency_idx = 0;
        cb._dependency_lsn = 0;
        cb._dependency_shpid = 0;
    }

    return RCOK;
}

w_rc_t bf_tree_cleaner_slave_thread_t::_do_work()
{
    if (_dirty_shutdown_happening()) return RCOK;
    // copies parent's _requested_lsn and _requested_volumes first.
    lsndata_t requested_lsn = _parent->_requested_lsn; // this is an atomic copy
    if (requested_lsn <= _completed_lsn) {
        requested_lsn = lsndata_null;
    }
    bool requested_volumes[MAX_VOL_COUNT];
    ::memcpy (requested_volumes, _parent->_requested_volumes, sizeof(bool) * MAX_VOL_COUNT); // this does NOT have to be atomic.
    DBGOUT1(<<"_do_work(cleaner=" << _id << "): requested_lsn=" << lsn_t(requested_lsn));
    
    for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
        // I'm hoping this doesn't revoke the buffer, leaving the capacity for reuse.
        // but in some STL implementation this might make the capacity zero. even in that case it shouldn't be a big issue..
        _candidates_buffer[vol].clear();
    }
    
    // if the dirty page's lsn is same or smaller than durable lsn, 
    // we can write it out without log flush overheads.
    lsndata_t durable_lsn = smlevel_0::log == NULL ? lsndata_null : smlevel_0::log->durable_lsn().data();
    
    bf_idx block_cnt = _parent->_bufferpool->_block_cnt;

    // do we have lots of dirty pages? (note, these are approximate statistics)
    if (_parent->_bufferpool->_dirty_page_count_approximate < 0) {// so, even this can happen.
        _parent->_bufferpool->_dirty_page_count_approximate = 0;
    }
    bool in_hurry = _parent->_bufferpool->_dirty_page_count_approximate > (block_cnt / 3 * 2);
    bool in_real_hurry = _parent->_bufferpool->_dirty_page_count_approximate > (block_cnt / 4 * 3);

    // list up dirty pages
    generic_page* pages = _parent->_bufferpool->_buffer;
    for (bf_idx idx = 1; idx < block_cnt; ++idx) {
        bf_tree_cb_t &cb = _parent->_bufferpool->get_cb(idx);
        if (!cb._dirty || !cb._used) {
            continue;
        }
        // the following check is approximate (without latch).
        // we check for real later, so that's fine.
        volid_t vol = cb._pid_vol;
        if (vol >= MAX_VOL_COUNT || _parent->_volume_assignments[vol] != _id) {
            continue;
        }
        bool clean_it = false;
        if (cb._rec_lsn < requested_lsn) {
            clean_it = true;
        } else if (requested_volumes[vol]) {
            clean_it = true;
        } else if (in_real_hurry) {
            clean_it = true;
        } else if (in_hurry && pages[idx].lsn.data() <= durable_lsn) {
            clean_it = true;
        }
        if (clean_it) {
            _candidates_buffer[vol].push_back (idx);
            
            // also add dependent pages. note that this might cause a duplicate. we deal with duplicates in _clean_volume()
            bf_idx didx = cb._dependency_idx;
            if (didx != 0) {
                bf_tree_cb_t &dcb = _parent->_bufferpool->get_cb(didx);
                if (dcb._dirty && dcb._used && dcb._rec_lsn <= cb._dependency_lsn) {
                    DBGOUT2(<<"_do_work(cleaner=" << _id << "): added dependent dirty page: idx=" << didx << ": pid=" << dcb._pid_vol << "." << dcb._pid_shpid);
                    _candidates_buffer[vol].push_back (didx);
                }
            }
        }
    }
    for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
        if (!_candidates_buffer[vol].empty()) {
            W_DO(_clean_volume(vol, _candidates_buffer[vol], requested_volumes[vol], requested_lsn));
        }
    }
    
    
    if (requested_lsn != lsndata_null) {
        w_assert1(_completed_lsn < requested_lsn);
        _completed_lsn = requested_lsn;
        DBGOUT1(<<"_do_work(cleaner=" << _id << "): flushed until lsn=" << lsn_t(requested_lsn));
    }
    for (volid_t vol = 1; vol < MAX_VOL_COUNT; ++vol) {
        if (_parent->_volume_assignments[vol] == _id && requested_volumes[vol]) {
            _parent->_requested_volumes[vol] = false;
            if (_parent->_bufferpool->_volumes[vol] != NULL) {
                DBGOUT1(<<"_do_work(cleaner=" << _id << "): flushed volume " << vol);
            }
        }
    }
    DBGOUT1(<<"_do_work(cleaner=" << _id << "): done");
    return RCOK;
}
