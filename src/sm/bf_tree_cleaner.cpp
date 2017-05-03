/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#include "bf_tree_cleaner.h"
#include "sm_base.h"
#include "bf_tree.h"
#include "generic_page.h"
#include "fixable_page_h.h"
#include "log_core.h"
#include "alloc_cache.h"
#include "stnode_page.h"
#include "vol.h"
#include "xct_logger.h"
#include "sm.h"
#include "stopwatch.h"
#include "xct.h"
#include <vector>

class candidate_collector_thread : public worker_thread_t
{
public:
    candidate_collector_thread(bf_tree_cleaner* cleaner)
        : worker_thread_t(-1), cleaner(cleaner)
    {};

    virtual ~candidate_collector_thread() {};

    virtual void do_work()
    {
        cleaner->collect_candidates();
    }

private:
    bf_tree_cleaner* cleaner;
};

bf_tree_cleaner::bf_tree_cleaner(bf_tree_m* bufferpool, const sm_options& options)
    : page_cleaner_base(bufferpool, options),
    next_candidates(new vector<cleaner_cb_info>()),
    curr_candidates(new vector<cleaner_cb_info>())
{
    num_candidates = options.get_int_option("sm_cleaner_num_candidates", 0);
    min_write_size = options.get_int_option("sm_cleaner_min_write_size", 1);
    min_write_ignore_freq = options.get_int_option("sm_cleaner_min_write_ignore_freq", 0);
    async_candidate_collection =
        options.get_bool_option("sm_cleaner_async_candidate_collection", false);

    string pstr = options.get_string_option("sm_cleaner_policy", "");
    policy = make_cleaner_policy(pstr);

    if (num_candidates > 0) {
        curr_candidates->reserve(num_candidates);
        next_candidates->reserve(num_candidates);
    }

    if (async_candidate_collection) {
        collector.reset(new candidate_collector_thread(this));
        collector->fork();
    }
}

bf_tree_cleaner::~bf_tree_cleaner()
{
    if (collector) { collector->stop(); }
}

void bf_tree_cleaner::do_work()
{
    // Only used in async mode
    unsigned long round = 0;

    // fill up list of next candidates
    next_candidates->clear();
    if (collector) {
        round = collector->get_rounds_completed();
        collector->wakeup();
    }
    else {
        collect_candidates();
        curr_candidates.swap(next_candidates);
    }

    // if there's something in the current list, clean it
    if (curr_candidates->size() > 0) {
        clean_candidates();
    }
    // synchronize with asynchronous collector
    if (collector) {
        collector->wait_for_round(round + 1);
        w_assert1(curr_candidates->empty());
        curr_candidates.swap(next_candidates);
    }
}

void bf_tree_cleaner::clean_candidates()
{
    if (curr_candidates->empty()) {
        return;
    }
    stopwatch_t timer;

    size_t i = 0;
    // bool ignore_min_write = ignore_min_write_now();

    // keeps track of cluster positions in the workspace, so that
    // they can be flush asynchronously below
    std::vector<size_t> clusters;

    while (i < curr_candidates->size()) {
        // Get size of current cluster
        // size_t cluster_size = 1;
        // for (size_t j = i + 1; j < curr_candidates->size(); j++) {
        //     if (curr_candidates->at(j).pid != curr_candidates->at(i).pid + (j - i)) {
        //         break;
        //     }
        //     cluster_size++;
        // }

        // // Skip if current cluster is too small
        // if (!ignore_min_write && cluster_size < min_write_size) {
        //     i++;
        //     continue;
        // }

        // ADD_TSTAT(cleaner_time_cpu, timer.time_us());

        _clean_lsn = smlevel_0::log->durable_lsn();

        // index of the current frame in the workspace
        size_t w_index = 0;

        // Copy pages in the cluster to the workspace
        size_t k = 0;
        PageID prev_pid = curr_candidates->at(i).pid;
        while (w_index < _workspace_size && i+k < curr_candidates->size()) {
            PageID pid = curr_candidates->at(i+k).pid;
            bf_idx idx = curr_candidates->at(i+k).idx;

            if (!latch_and_copy(pid, idx, w_index)) {
                k++;
                continue;
            }

            // std::cerr << "cleaning " << curr_candidates->at(i+k) << endl;

            if (pid > prev_pid + 1 && w_index > 0) {
                // Current cluster ends at index k (k is part of the next cluster)
                clusters.push_back(w_index);
            }

            k++;
            w_index++;
            prev_pid = pid;

            if (should_exit()) { break; }
        }

        clusters.push_back(w_index);

        ADD_TSTAT(cleaner_time_copy, timer.time_us());

        if (should_exit()) { break; }

        flush_clusters(clusters);
        ADD_TSTAT(cleaner_time_io, timer.time_us());

        clusters.clear();
        i += k;
    }

    curr_candidates->clear();
}

void bf_tree_cleaner::flush_clusters(const vector<size_t>& clusters)
{
    size_t i = 0;
    for (auto k : clusters) {
        w_assert1(k > i);
        write_pages(i, k);
        i = k;
    }

    smlevel_0::vol->sync();

    i = 0;
    for (auto k : clusters) {
        w_assert1(k > i);
        PageID pid = _workspace[i].pid;
        Logger::log_sys<page_write_log>(pid, _clean_lsn, k-i);
        mark_pages_clean(i, k);
        i = k;
    }
}

bool bf_tree_cleaner::latch_and_copy(PageID pid, bf_idx idx, size_t wpos)
{
    const generic_page* const page_buffer = _bufferpool->_buffer;
    bf_tree_cb_t &cb = _bufferpool->get_cb(idx);

    // CS TODO: policy option: wait for latch or just attempt conditionally
    W_COERCE(cb.latch().latch_acquire(LATCH_SH, timeout_t::WAIT_FOREVER));

    // No need to pin CB here because we hold EX latch (eviction cannot clear CB)

    fixable_page_h page;
    page.fix_nonbufferpool_page(const_cast<generic_page*>(&page_buffer[idx]));
    if (page.pid() != pid || !cb.is_in_use()) {
        // New page was loaded in the frame -- skip it
        cb.latch().latch_release();
        return false;
    }

    // CS TODO: get rid of this buggy and ugly deletion mechanism
    if (page.is_to_be_deleted()) {
        sys_xct_section_t sxs(true);
        W_COERCE (sxs.check_error_on_start());
        W_COERCE (smlevel_0::vol->deallocate_page(page_buffer[idx].pid));
        W_COERCE (sxs.end_sys_xct (RCOK));

        // drop the page from bufferpool too
        _bufferpool->_delete_block(idx);

        cb.latch().latch_release();
        return false;
    }

    // Copy page and update its page_lsn from what's on the cb
    generic_page& pdest = _workspace[wpos];
    ::memcpy(&pdest, page_buffer + idx, sizeof (generic_page));
    pdest.lsn = cb.get_page_lsn();

    // if the page contains a swizzled pointer, we need to convert
    // the data back to the original pointer.  we need to do this
    // before releasing SH latch because the pointer might be
    // unswizzled by other threads.
    _bufferpool->_convert_to_disk_page(&pdest);

    // Record the fact that we are taking a copy for flushing in the CB
    cb.mark_persisted_lsn();

    cb.latch().latch_release();

    _workspace[wpos].checksum = _workspace[wpos].calculate_checksum();
    _workspace_cb_indexes[wpos] = idx;

    return true;
}

policy_predicate_t bf_tree_cleaner::get_policy_predicate()
{
    // A less-than function makes pop_heap return the highest value, and a
    // greater-than function the lowest. Because the heap's top element should
    // be the lowest in a "highest" policy and vice-versa, less-than should be
    // used for "highest" policies and vice-versa. When testing if an element
    // should replace the current top of the heap, the inverse of the
    // comparison function should be used, e.g., in a "highest" policy, an
    // incoming element enters the heap if it is greater than the heap's
    // lowest.
    switch (policy) {
        case cleaner_policy::highest_refcount:
            return [this] (const cleaner_cb_info& a, const cleaner_cb_info& b)
            {
                return a.ref_count < b.ref_count;
            };
        case cleaner_policy::lowest_refcount:
            return [this] (const cleaner_cb_info& a, const cleaner_cb_info& b)
            {
                return a.ref_count > b.ref_count;
            };
        case cleaner_policy::oldest_lsn: default: // mixed also falls here
            return [this] (const cleaner_cb_info& a, const cleaner_cb_info& b)
            {
                // return a.clean_lsn > b.clean_lsn;
                return a.rec_lsn < b.rec_lsn;
            };
    }
}

void bf_tree_cleaner::collect_candidates()
{
    stopwatch_t timer;
    w_assert1(next_candidates->empty());

    // Comparator to be used by the heap
    auto heap_cmp = get_policy_predicate();

    bf_idx block_cnt = _bufferpool->_block_cnt;

    // mixed policy = ignore null clean LSNs every 2 rounds
    // bool ignore_empty_clean_lsn = false;
    // if (policy == cleaner_policy::mixed) {
    //     ignore_empty_clean_lsn = get_rounds_completed() % 4 != 0;
    // }

    for (bf_idx idx = 1; idx < block_cnt; ++idx) {
        auto& cb = _bufferpool->get_cb(idx);
        if (!cb.pin()) { continue; }

        // If page is not dirty or not in use, no need to flush
        if (!cb.is_dirty() || !cb._used || cb.get_rec_lsn().is_null()) {
            cb.unpin();
            continue;
        }

        // CS TODO: update or remove mixed policy
        // if (cb.get_clean_lsn() == lsn_t::null && ignore_empty_clean_lsn) {
        //     cb.unpin();
        //     continue;
        // }

        // add new element to the back of vector
        next_candidates->emplace_back(idx, cb);

        cb.unpin();

        // manage heap if we are limiting the number of candidates
        if (num_candidates > 0) {
            if (next_candidates->size() < num_candidates ||
                !heap_cmp(next_candidates->front(), next_candidates->back()))
            {
                // if it's among the top-k candidates, push it into the heap
                std::push_heap(next_candidates->begin(), next_candidates->end(), heap_cmp);
                while (next_candidates->size() > num_candidates) {
                    std::pop_heap(next_candidates->begin(), next_candidates->end(), heap_cmp);
                    // cerr << "popped "<< next_candidates->back() << endl;
                    next_candidates->pop_back();
                }
            }
            // otherwise just remove it
            else { next_candidates->pop_back(); }
        }
    }

    // CS TODO: one policy could sort each sequence of adjacent pids by cluster size
    // Sort by PageID to exploit large sequential writes
    auto lt = [] (const cleaner_cb_info& a, const cleaner_cb_info& b)
    {
        return a.pid < b.pid;
    };

    std::sort(next_candidates->begin(), next_candidates->end(), lt);

    ADD_TSTAT(cleaner_time_cpu, timer.time_us());
}

std::ostream& operator<<(std::ostream& out, const cleaner_cb_info& cb)
{
    out << "pid=" << cb.pid
        << " page=" << cb.page_lsn
        << " rec=" << cb.rec_lsn
        << std::endl;

    return out;
}
