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
#include "eventlog.h"
#include "sm.h"
#include "stopwatch.h"
#include "xct.h"
#include <vector>

bf_tree_cleaner::bf_tree_cleaner(bf_tree_m* bufferpool, const sm_options& options)
    : page_cleaner_base(bufferpool, options)
{
    num_candidates = options.get_int_option("sm_cleaner_num_candidates", 0);
    min_write_size = options.get_int_option("sm_cleaner_min_write_size", 1);
    min_write_ignore_freq = options.get_int_option("sm_cleaner_min_write_ignore_freq", 0);
    ignore_metadata = options.get_bool_option("sm_cleaner_ignore_metadata", false);

    string pstr = options.get_string_option("sm_cleaner_policy", "");
    policy = make_cleaner_policy(pstr);

    if (num_candidates > 0) {
        candidates.reserve(num_candidates);
    }
}

bf_tree_cleaner::~bf_tree_cleaner()
{
}

void bf_tree_cleaner::do_work()
{
    collect_candidates();
    clean_candidates();

    if (!ignore_metadata) {
        lsn_t dur_lsn = smlevel_0::log->durable_lsn();
        W_COERCE(smlevel_0::vol->get_alloc_cache()->write_dirty_pages(dur_lsn));
        W_COERCE(smlevel_0::vol->get_stnode_cache()->write_page(dur_lsn));
    }
}

void bf_tree_cleaner::clean_candidates()
{
    if (candidates.empty()) {
        return;
    }
    stopwatch_t timer;

    _clean_lsn = smlevel_0::log->curr_lsn();

    size_t i = 0;
    bool ignore_min_write = ignore_min_write_now();
    while (i < candidates.size()) {
        if (should_exit()) { break; }

        // Get size of current cluster
        size_t cluster_size = 1;
        for (size_t j = i + 1; j < candidates.size(); j++) {
            if (candidates[j].pid != candidates[i].pid + (j - i)) {
                break;
            }
            cluster_size++;
        }

        // Skip if current cluster is too small
        if (!ignore_min_write && cluster_size < min_write_size) {
            i++;
            continue;
        }

        ADD_TSTAT(cleaner_time_cpu, timer.time_us());

        // Copy pages in the cluster to the workspace
        if (cluster_size > _workspace_size) { cluster_size = _workspace_size; }
        for (size_t k = 0; k < cluster_size; k++) {
            PageID pid = candidates[i+k].pid;
            bf_idx idx = candidates[i+k].idx;

            if (!latch_and_copy(pid, idx, k)) {
                // If latch failed, cut down the current cluster
                cluster_size = k;
                break;
            }
        }

        ADD_TSTAT(cleaner_time_copy, timer.time_us());

        log_and_flush(cluster_size);
        i += cluster_size;

        ADD_TSTAT(cleaner_time_io, timer.time_us());
    }
}

void bf_tree_cleaner::log_and_flush(size_t wpos)
{
    if (wpos == 0) { return; }

    flush_workspace(0, wpos);

    PageID pid = _workspace[0].pid;
    sysevent::log_page_write(pid, _clean_lsn, wpos);

    _clean_lsn = smlevel_0::log->curr_lsn();
}

bool bf_tree_cleaner::latch_and_copy(PageID pid, bf_idx idx, size_t wpos)
{
    const generic_page* const page_buffer = _bufferpool->_buffer;
    bf_tree_cb_t &cb = _bufferpool->get_cb(idx);

    // CS TODO: policy option: wait for latch or just attempt conditionally
    rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, WAIT_IMMEDIATE);
    if (latch_rc.is_error()) {
        // Could not latch page in EX mode -- just skip it
        return false;
    }

    fixable_page_h page;
    page.fix_nonbufferpool_page(const_cast<generic_page*>(&page_buffer[idx]));
    if (page.pid() != pid) {
        // New page was loaded in the frame -- skip it
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
    // CS TODO: swizzling!
    // if the page contains a swizzled pointer, we need to convert
    // the data back to the original pointer.  we need to do this
    // before releasing SH latch because the pointer might be
    // unswizzled by other threads.
    _bufferpool->_convert_to_disk_page(&pdest);

    cb.latch().latch_release();

    _workspace[wpos].checksum = _workspace[wpos].calculate_checksum();
    _workspace_cb_indexes[wpos] = idx;

    return true;
}

policy_predicate_t bf_tree_cleaner::get_policy_predicate()
{
    return [this] (const cleaner_cb_info& a, const cleaner_cb_info& b)
    {
        switch (policy) {
            case cleaner_policy::highest_refcount:
                return a.ref_count < b.ref_count;
            case cleaner_policy::lowest_refcount:
                return a.ref_count > b.ref_count;
            case cleaner_policy::oldest_lsn: default:
                return a.clean_lsn > b.clean_lsn;
        }
    };
}

void bf_tree_cleaner::collect_candidates()
{
    stopwatch_t timer;
    candidates.clear();

    // Comparator to be used by the heap
    auto heap_cmp = get_policy_predicate();

    bf_idx block_cnt = _bufferpool->_block_cnt;

    for (bf_idx idx = 1; idx < block_cnt; ++idx) {
        bf_tree_cb_t &cb = _bufferpool->get_cb(idx);
        cb.pin();

        // If page is not dirty or not in use, no need to flush
        if (!cb.is_dirty() || !cb._used) {
            cb.unpin();
            continue;
        }

        candidates.emplace_back(idx, cb);
        if (num_candidates > 0) {
            std::push_heap(candidates.begin(), candidates.end(), heap_cmp);
            while (candidates.size() > num_candidates) {
                std::pop_heap(candidates.begin(), candidates.end(), heap_cmp);
                candidates.pop_back();
            }
        }
        cb.unpin();
    }

    // CS TODO: one policy could sort each sequence of adjacent pids by cluster size
    // Sort by PageID to exploit large sequential writes
    auto lt = [] (const cleaner_cb_info& a, const cleaner_cb_info& b)
    {
        return a.pid < b.pid;
    };

    std::sort(candidates.begin(), candidates.end(), lt);

    ADD_TSTAT(cleaner_time_cpu, timer.time_us());
}
