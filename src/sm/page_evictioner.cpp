#include "page_evictioner.h"

#include "bf_tree.h"
#include "log_core.h"
#include "xct_logger.h"
#include "btree_page_h.h"

page_evictioner_base::page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options)
    :
    worker_thread_t(options.get_int_option("sm_eviction_interval", 100)),
    _bufferpool(bufferpool),
    _rnd_distr(1, _bufferpool->get_block_cnt() - 1)
{
    _swizzling_enabled = options.get_bool_option("sm_bufferpool_swizzle", false);
    _maintain_emlsn = options.get_bool_option("sm_bf_maintain_emlsn", false);
    _random_pick = options.get_bool_option("sm_evict_random", false);
    _use_clock = options.get_bool_option("sm_evict_use_clock", true);
    _log_evictions = options.get_bool_option("sm_log_page_evictions", false);
    _wakeup_cleaner_attempts = options.get_int_option("sm_evict_wakeup_cleaner_attempts", 0);
    _clean_only_attempts = options.get_int_option("sm_evict_clean_only_attempts", 0);
    _write_elision = options.get_bool_option("sm_write_elision", false);
    _no_db_mode = options.get_bool_option("sm_no_db", false);

    if (options.get_bool_option("sm_evict_dirty_pages", false)) {
        // this option overrides clean_only_attempts
        _clean_only_attempts = 1;
    }

    if (_use_clock) { _clock_ref_bits.resize(_bufferpool->get_block_cnt(), false); }

    _current_frame = 0;

    constexpr unsigned max_rounds = 1000;
    _max_attempts = max_rounds * _bufferpool->get_block_cnt();
}

page_evictioner_base::~page_evictioner_base() {}

void page_evictioner_base::do_work() {
    uint32_t preferred_count = EVICT_BATCH_RATIO * _bufferpool->_block_cnt + 1;

    // In principle, _freelist_len requires a fence, but here it should be OK
    // because we don't need to read a consistent value every time.
    while (_bufferpool->_freeList.get()->getCount() < preferred_count) {
        bf_idx victim = pick_victim();

        if (evict_one(victim)) {
            _bufferpool->_freeList->addFreeBufferpoolFrame(victim);
        }

        /* Rather than waiting for all the pages to be evicted, we notify
         * waiting threads every time a page is evicted. One of them is going to
         * be able to re-use the freed slot, the others will go back to waiting.
         */
        notify_one();

        if (should_exit()) { break; }
    }

    // cerr << "Eviction done; free frames: " << _bufferpool->_freelist_len << endl;
}

bool page_evictioner_base::evict_one(bf_idx victim) {
    bf_tree_cb_t &cb = _bufferpool->get_cb(victim);
    w_assert1(cb.latch().is_mine());

    if (!unswizzle_and_update_emlsn(victim)) {
        /* We were not able to unswizzle/update parent, therefore we cannot
         * proceed with this victim. We just jump to the next iteration and
         * hope for better luck next time. */
        cb.latch().latch_release();
        return false;
    }

    // Try to atomically set pin from 0 to -1; give up if it fails
    if (!cb.prepare_for_eviction()) {
        cb.latch().latch_release();
        return false;
    }

    // When media failure is detected, all pages currently in buffer pool should be
    // considered dirty. Since that's tricky to do without quiescing the system,
    // we just detect those frames here and add to the dirty page table, so that
    // single-page recovery takes care of bringing them to correct state after
    // restore.
    // bool media_failure = _bufferpool->is_media_failure() && !cb._check_recovery;
    lsn_t page_lsn = cb.get_page_lsn();
    bool media_failure = _bufferpool->is_media_failure(cb._pid);
    if (_no_db_mode || _write_elision || cb.is_dirty() || media_failure) {
        // CS TODO: apparently page LSN can be null for unallocated pages
        // (i.e., "holes" in extents)
        if (!page_lsn.is_null()) {
            smlevel_0::recovery->add_dirty_page(cb._pid, page_lsn);
        }
    }

    // We're passed the point of no return: eviction must happen no mather what

    // Check if page needs to be flushed
    bool was_dirty = cb.is_dirty();
    if (was_dirty && !_write_elision) { flush_dirty_page(cb); }
    w_assert1(cb.latch().is_mine());

    if (_log_evictions) {
        Logger::log_sys<evict_page_log>(cb._pid, was_dirty, page_lsn);
    }

    // remove it from hashtable.
    w_assert1(cb._pin_cnt < 0);
    w_assert1(!cb._used);
    _bufferpool->_hashtable->erase(cb._pid);

    DBG2(<< "EVICTED " << victim << " pid " << cb._pid
         << " log-tail " << smlevel_0::log->curr_lsn());

    cb.latch().latch_release();

//     if (_bufferpool->is_no_db_mode()) {
//         auto lsn = smlevel_0::recovery->get_dirty_page_emlsn(cb._pid);
//         w_assert0(!lsn.is_null());
//     }

    INC_TSTAT(bf_evict);
    return true;
}

void page_evictioner_base::flush_dirty_page(const bf_tree_cb_t& cb) {
    // WAL rule
    W_COERCE(smlevel_0::log->flush(cb.get_page_lsn()));

    // Straight-forward write -- no need to do it asynchronously or worry about
    // any race conditions. We hold EX latch and the entry hasn't been removed
    // from the buffer-pool hash table yet. Any thread attempting to fix the
    // page will be waiting on the EX latch, after which it will notice that the
    // CB pin count is -1, which means it must try the fix again.
    generic_page *page = _bufferpool->get_page(&cb);
    W_COERCE(smlevel_0::vol->write_page(cb._pid, page));
    smlevel_0::vol->sync();

    // Log the write operation so that log analysis sees the page as clean.
    // clean_lsn cannot be page_lsn, otherwise page is considered dirty, so
    // simply use any LSN above that (clean_lsn doesn't have to be of a valid
    // log record)
    lsn_t clean_lsn = page->lsn + 1;
    Logger::log_sys<page_write_log>(cb._pid, clean_lsn, 1);
}

void page_evictioner_base::hit_ref(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = true; }
}

void page_evictioner_base::unfix_ref(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = true; }
}

void page_evictioner_base::miss_ref(bf_idx b_idx, PageID pid) {
    if (_use_clock && !_clock_ref_bits[b_idx]) { _clock_ref_bits[b_idx] = true; }
}

void page_evictioner_base::used_ref(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = true; }
}

void page_evictioner_base::dirty_ref(bf_idx idx) {}

void page_evictioner_base::block_ref(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = true; }
}

void page_evictioner_base::swizzle_ref(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = true; }
}

void page_evictioner_base::unbuffered(bf_idx idx) {
    if (_use_clock && !_clock_ref_bits[idx]) { _clock_ref_bits[idx] = false; }
}

bf_idx page_evictioner_base::pick_victim() {
    auto next_idx = [this] {
        if (_current_frame > _bufferpool->_block_cnt) {
            // race condition here, but it's not a big deal
            _current_frame = 1;
        }
        return _random_pick ? get_random_idx() : _current_frame++;
    };

    unsigned attempts = 0;
    while (true) {

        if (should_exit()) return 0; // in bf_tree.h, 0 is never used, means null

        bf_idx idx = next_idx();
        if (idx >= _bufferpool->_block_cnt || idx == 0) { idx = 1; }

        attempts++;
        if (attempts >= _max_attempts) {
            W_FATAL_MSG(fcINTERNAL, << "Eviction got stuck!");
        } else if (_wakeup_cleaner_attempts > 0 && attempts % _wakeup_cleaner_attempts == 0) {
            _bufferpool->wakeup_cleaner();
        }

        auto &cb = _bufferpool->get_cb(idx);

        if (!cb._used) {
            continue;
        }

        // If I already hold the latch on this page (e.g., with latch
        // coupling), then the latch acquisition below will succeed, but the
        // page is obviously not available for eviction. This would not happen
        // if every fix would also pin the page, which I didn't wan't to do
        // because it seems like a waste.  Note that this is only a problem
        // with threads perform their own eviction (i.e., with the option
        // _async_eviction set to false in bf_tree_m), because otherwise the
        // evictioner thread never holds any latches other than when trying to
        // evict a page.  This bug cost me 2 days of work. Anyway, it should
        // work with the check below for now.
        if (cb.latch().held_by_me()) {
            // I (this thread) currently have the latch on this frame, so
            // obviously I should not evict it
            used_ref(idx);
            continue;
        }

        // Step 1: latch page in EX mode and check if eligible for eviction
        rc_t latch_rc;
        latch_rc = cb.latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error()) {
            used_ref(idx);
            DBG3(<< "Eviction failed on latch for " << idx);
            continue;
        }
        w_assert1(cb.latch().is_mine());

        // Only evict if clock referenced bit is not set
        if (_use_clock && _clock_ref_bits[idx]) {
            _clock_ref_bits[idx] = false;
            cb.latch().latch_release();
            continue;
        }

        // Only evict actually evictable pages (not required to stay in the buffer pool)
        if (!_bufferpool->isEvictable(idx, _flush_dirty)) {
            cb.latch().latch_release();
            continue;
        }

        // If we got here, we passed all tests and have a victim!
        w_assert1(_bufferpool->_is_active_idx(idx));
        w_assert0(idx != 0);
        ADD_TSTAT(bf_eviction_attempts, attempts);
        return idx;
    }
}

bool page_evictioner_base::unswizzle_and_update_emlsn(bf_idx idx) {
    if (!_maintain_emlsn && !_swizzling_enabled) { return true; }

    bf_tree_cb_t &cb = _bufferpool->get_cb(idx);
    w_assert1(cb.latch().is_mine());

    //==========================================================================
    // STEP 1: Look for parent.
    //==========================================================================
    PageID pid = _bufferpool->_buffer[idx].pid;
    bf_idx_pair idx_pair = *(_bufferpool->_hashtable->lookupPair(pid));
    w_assert1(idx_pair.first != 0);

    bf_idx parent_idx = idx_pair.second;
    w_assert1(idx_pair.first == 0 || idx == idx_pair.first);

    // If there is no parent, but write elision is off and the frame is not swizzled,
    // then it's OK to evict
    if (parent_idx == 0) {
        return !_bufferpool->_write_elision && !cb._swizzled;
    }

    bf_tree_cb_t &parent_cb = _bufferpool->get_cb(parent_idx);
    rc_t r = parent_cb.latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
    if (r.is_error()) {
        /* Just give up. If we try to latch it unconditionally, we may deadlock,
         * because other threads are also waiting on the eviction mutex. */
        return false;
    }
    w_assert1(parent_cb.latch().is_mine());

    /* Look for emlsn slot on parent (must be found because parent pointer is
     * kept consistent at all times). */
    w_assert1(_bufferpool->_is_active_idx(parent_idx));
    generic_page *parent = &_bufferpool->_buffer[parent_idx];
    btree_page_h parent_h;
    parent_h.fix_nonbufferpool_page(parent);

    general_recordid_t child_slotid;
    if (_swizzling_enabled && cb._swizzled) {
        // Search for swizzled address
        PageID swizzled_pid = idx | SWIZZLED_PID_BIT;
        child_slotid = _bufferpool->find_page_id_slot(parent, swizzled_pid);
    } else {
        child_slotid = _bufferpool->find_page_id_slot(parent, pid);
    }
    w_assert1(child_slotid != GeneralRecordIds::INVALID);

    //==========================================================================
    // STEP 2: Unswizzle pointer on parent before evicting.
    //==========================================================================
    if (_swizzling_enabled && cb._swizzled) {
        bool ret = _bufferpool->unswizzlePagePointer(parent, child_slotid);
        w_assert0(ret);
        w_assert1(!cb._swizzled);
    }

    //==========================================================================
    // STEP 3: Page will be evicted -- update EMLSN on parent.
    //==========================================================================
    lsn_t old = parent_h.get_emlsn_general(child_slotid);
    _bufferpool->_buffer[idx].lsn = cb.get_page_lsn();
    if (_maintain_emlsn && old < _bufferpool->_buffer[idx].lsn) {
        DBG3(<< "Updated EMLSN on page " << parent_h.pid()
             << " slot=" << child_slotid
             << " (child pid=" << pid << ")"
             << ", OldEMLSN=" << old
             << " NewEMLSN=" << _bufferpool->_buffer[idx].lsn);

        w_assert1(parent_cb.latch().is_mine());

        _bufferpool->_sx_update_child_emlsn(parent_h, child_slotid,
                                            _bufferpool->_buffer[idx].lsn);

        w_assert1(parent_h.get_emlsn_general(child_slotid)
                  == _bufferpool->_buffer[idx].lsn);
    }

    parent_cb.latch().latch_release();
    return true;
}
