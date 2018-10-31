/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

/*<std-header orig-src='shore'>

 $Id: smstats.cpp,v 1.22 2010/11/08 15:07:06 nhall Exp $

SHORE -- Scalable Heterogeneous Object REpository

Copyright (c) 1994-99 Computer Sciences Department, University of
                      Wisconsin -- Madison
All Rights Reserved.

Permission to use, copy, modify and distribute this software and its
documentation is hereby granted, provided that both the copyright
notice and this permission notice appear in all copies of the
software, derivative works or modified versions, and any portions
thereof, and that both notices appear in supporting documentation.

THE AUTHORS AND THE COMPUTER SCIENCES DEPARTMENT OF THE UNIVERSITY
OF WISCONSIN - MADISON ALLOW FREE USE OF THIS SOFTWARE IN ITS
"AS IS" CONDITION, AND THEY DISCLAIM ANY LIABILITY OF ANY KIND
FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.

This software was developed with support by the Advanced Research
Project Agency, ARPA order number 018 (formerly 8230), monitored by
the U.S. Army Research Laboratory under contract DAAB07-91-C-Q518.
Further funding for this work was provided by DARPA through
Rome Research Laboratory Contract No. F30602-97-2-0247.

*/

#include "w_defines.h"

/*  -- do not edit anything above this line --   </std-header>*/

#include "sm_base.h"
// smstats_info_t is the collected stats from various
// sm parts.  Each part is separately-generate from .dat files.
#include "smstats.h"

// the strings:
const char* get_stat_name(sm_stat_id s)
{
    switch (s) {
        case sm_stat_id::rwlock_r_wait: return "rwlock_r_wait";
        case sm_stat_id::rwlock_w_wait: return "rwlock_w_wait";
        case sm_stat_id::needs_latch_condl: return "needs_latch_condl";
        case sm_stat_id::needs_latch_uncondl: return "needs_latch_uncondl";
        case sm_stat_id::latch_condl_nowait: return "latch_condl_nowait";
        case sm_stat_id::latch_uncondl_nowait: return "latch_uncondl_nowait";
        case sm_stat_id::cleaned_pages: return "cleaned_pages";
        case sm_stat_id::cleaner_time_cpu: return "cleaner_time_cpu";
        case sm_stat_id::cleaner_time_io: return "cleaner_time_io";
        case sm_stat_id::cleaner_time_copy: return "cleaner_time_copy";
        case sm_stat_id::bf_eviction_attempts: return "bf_eviction_attempts";
        case sm_stat_id::bf_evict: return "bf_evict";
        case sm_stat_id::bf_evict_duration: return "bf_evict_duration";
        case sm_stat_id::bf_hit_cnt: return "bf_hit_cnt";
        case sm_stat_id::vol_reads: return "vol_reads";
        case sm_stat_id::vol_writes: return "vol_writes";
        case sm_stat_id::vol_blks_written: return "vol_blks_written";
        case sm_stat_id::log_dup_sync_cnt: return "log_dup_sync_cnt";
        case sm_stat_id::log_fsync_cnt: return "log_fsync_cnt";
        case sm_stat_id::log_chkpt_cnt: return "log_chkpt_cnt";
        case sm_stat_id::log_fetches: return "log_fetches";
        case sm_stat_id::log_buffer_hit: return "log_buffer_hit";
        case sm_stat_id::log_inserts: return "log_inserts";
        case sm_stat_id::log_bytes_generated: return "log_bytes_generated";
        case sm_stat_id::log_bytes_written: return "log_bytes_written";
        case sm_stat_id::log_short_flush: return "log_short_flush";
        case sm_stat_id::log_long_flush: return "log_long_flush";
        // case sm_stat_id::nonunique_fingerprints: return "nonunique_fingerprints";
        // case sm_stat_id::unique_fingerprints: return "unique_fingerprints";
        case sm_stat_id::bt_find_cnt: return "bt_find_cnt";
        case sm_stat_id::bt_insert_cnt: return "bt_insert_cnt";
        case sm_stat_id::bt_remove_cnt: return "bt_remove_cnt";
        case sm_stat_id::bt_traverse_cnt: return "bt_traverse_cnt";
        case sm_stat_id::bt_partial_traverse_cnt: return "bt_partial_traverse_cnt";
        case sm_stat_id::bt_grows: return "bt_grows";
        case sm_stat_id::bt_shrinks: return "bt_shrinks";
        case sm_stat_id::bt_links: return "bt_links";
        case sm_stat_id::bf_fix_cnt: return "bf_fix_cnt";
        case sm_stat_id::page_alloc_cnt: return "page_alloc_cnt";
        case sm_stat_id::page_dealloc_cnt: return "page_dealloc_cnt";
        case sm_stat_id::xct_log_flush: return "xct_log_flush";
        case sm_stat_id::begin_xct_cnt: return "begin_xct_cnt";
        case sm_stat_id::commit_xct_cnt: return "commit_xct_cnt";
        case sm_stat_id::abort_xct_cnt: return "abort_xct_cnt";
        case sm_stat_id::rollback_savept_cnt: return "rollback_savept_cnt";
        case sm_stat_id::internal_rollback_cnt: return "internal_rollback_cnt";
        case sm_stat_id::anchors: return "anchors";
        case sm_stat_id::compensate_in_log: return "compensate_in_log";
        // case sm_stat_id::compensate_in_xct: return "compensate_in_xct";
        case sm_stat_id::compensate_records: return "compensate_records";
        case sm_stat_id::compensate_skipped: return "compensate_skipped";
        case sm_stat_id::bf_fix_nonroot_count: return "bf_fix_nonroot_count";
        case sm_stat_id::bf_fix_nonroot_miss_count: return "bf_fix_nonroot_miss_count";
        case sm_stat_id::bf_fix_adjusted_parent: return "bf_fix_adjusted_parent";
        case sm_stat_id::bf_batch_wait_time: return "bf_batch_wait_time";
        case sm_stat_id::restart_log_analysis_time: return "restart_log_analysis_time";
        case sm_stat_id::restart_redo_time: return "restart_redo_time";
        case sm_stat_id::restart_dirty_pages: return "restart_dirty_pages";
        case sm_stat_id::restore_log_volume: return "restore_log_volume";
        case sm_stat_id::la_log_slow: return "la_log_slow";
        case sm_stat_id::la_activations: return "la_activations";
        case sm_stat_id::la_read_volume: return "la_read_volume";
        case sm_stat_id::la_read_count: return "la_read_count";
        case sm_stat_id::la_open_count: return "la_open_count";
        case sm_stat_id::la_read_time: return "la_read_time";
        case sm_stat_id::la_block_writes: return "la_block_writes";
        case sm_stat_id::la_img_compressed_bytes: return "la_img_compressed_bytes";
        case sm_stat_id::log_img_format_bytes: return "log_img_format_bytes";
        case sm_stat_id::la_skipped_bytes: return "la_skipped_bytes";
        case sm_stat_id::la_img_trimmed: return "la_img_trimmed";
        case sm_stat_id::la_wasted_read: return "la_wasted_read";
    }
    return "UNKNOWN_STAT";
}

const char* get_stat_expl(sm_stat_id s)
{
    switch (s) {
        case sm_stat_id::rwlock_r_wait: return "Number waits for read lock on srwlock";
        case sm_stat_id::rwlock_w_wait: return "Number waits for write lock on srwlock";
        case sm_stat_id::needs_latch_condl: return "Conditional latch requests";
        case sm_stat_id::needs_latch_uncondl: return "Unconditional latch requests";
        case sm_stat_id::latch_condl_nowait: return "Conditional requests satisfied immediately";
        case sm_stat_id::latch_uncondl_nowait: return "Unconditional requests satisfied immediately";
        case sm_stat_id::cleaned_pages: return "Number of pages cleaned by bf_cleaner thread";
        case sm_stat_id::cleaner_time_cpu: return "Time spent manipulating cleaner candidate lists";
        case sm_stat_id::cleaner_time_io: return "Time spent flushing the cleaner workspace";
        case sm_stat_id::cleaner_time_copy: return "Time spent latching and copy page images into workspace";
        case sm_stat_id::bf_eviction_attempts: return "Total number of frames inspected for eviction";
        case sm_stat_id::bf_evict: return "Evicted page from buffer pool";
        case sm_stat_id::bf_evict_duration: return "Duration of eviction calls in nanosecond";
        case sm_stat_id::bf_hit_cnt: return "Found page in buffer pool in find/grab";
        case sm_stat_id::vol_reads: return "Data volume read requests (from disk)";
        case sm_stat_id::vol_writes: return "Data volume write requests (to disk)";
        case sm_stat_id::vol_blks_written: return "Data volume pages written (to disk)";
        case sm_stat_id::log_dup_sync_cnt: return "Times the log was flushed superfluously";
        case sm_stat_id::log_fsync_cnt: return "Times the fsync system call was used";
        case sm_stat_id::log_chkpt_cnt: return "Checkpoints taken";
        case sm_stat_id::log_fetches: return "Log records fetched from log (read)";
        case sm_stat_id::log_buffer_hit: return "Log fetches that were served from in-memory fetch buffers";
        case sm_stat_id::log_inserts: return "Log records inserted into log (written)";
        case sm_stat_id::log_bytes_generated: return "Bytes of log records inserted ";
        case sm_stat_id::log_bytes_written: return "Bytes written to log including skip and padding";
        case sm_stat_id::log_short_flush: return "Log flushes <= 1 block";
        case sm_stat_id::log_long_flush: return "Log flushes > 1 block";
        // case sm_stat_id::nonunique_fingerprints: return "Smthreads created a non-unique fingerprint";
        // case sm_stat_id::unique_fingerprints: return "Smthreads created a unique fingerprint";
        case sm_stat_id::bt_find_cnt: return "Btree lookups (find_assoc())";
        case sm_stat_id::bt_insert_cnt: return "Btree inserts (create_assoc())";
        case sm_stat_id::bt_remove_cnt: return "Btree removes (destroy_assoc())";
        case sm_stat_id::bt_traverse_cnt: return "Btree traversals";
        case sm_stat_id::bt_partial_traverse_cnt: return "Btree traversals starting below root";
        case sm_stat_id::bt_grows: return "Btree grew a level";
        case sm_stat_id::bt_shrinks: return "Btree shrunk a level";
        case sm_stat_id::bt_links: return "Btree links followed";
        case sm_stat_id::bf_fix_cnt: return "Times bp fix called  (conditional or unconditional)";
        case sm_stat_id::page_alloc_cnt: return "Pages allocated";
        case sm_stat_id::page_dealloc_cnt: return "Pages deallocated";
        case sm_stat_id::xct_log_flush: return "Log flushes by xct for commit/prepare";
        case sm_stat_id::begin_xct_cnt: return "Transactions started";
        case sm_stat_id::commit_xct_cnt: return "Transactions committed";
        case sm_stat_id::abort_xct_cnt: return "Transactions aborted";
        case sm_stat_id::rollback_savept_cnt: return "Rollbacks to savepoints (not incl aborts)";
        case sm_stat_id::internal_rollback_cnt: return "Internal partial rollbacks ";
        case sm_stat_id::anchors: return "Log Anchors grabbed";
        case sm_stat_id::compensate_in_log: return "Compensations written in log buffer";
        // case sm_stat_id::compensate_in_xct: return "Compensations written in xct log buffer";
        case sm_stat_id::compensate_records: return "Compensations written as own log record ";
        case sm_stat_id::compensate_skipped: return "Compensations would be a no-op";
        case sm_stat_id::bf_fix_nonroot_count: return "Fix a non-root page";
        case sm_stat_id::bf_fix_nonroot_miss_count: return "Cache miss when fixing a non-root page";
        case sm_stat_id::bf_fix_adjusted_parent: return "Parent pointer adjusted in hash table while performing a fix";
        case sm_stat_id::bf_batch_wait_time: return "Time spent waiting for batch warmup when ficing pages (usec; nodb mode only)";
        case sm_stat_id::restart_log_analysis_time: return "Time spend with log analysis (usec)";
        case sm_stat_id::restart_redo_time: return "Time spend with non-concurrent REDO (usec)";
        case sm_stat_id::restart_dirty_pages: return "Number of dirty pages computed in restart log analysis";
        case sm_stat_id::restore_log_volume: return "Amount of log replayed during restore (bytes)";
        case sm_stat_id::la_log_slow: return "Log archiver activated with small window due to slow log growth";
        case sm_stat_id::la_activations: return "How often log archiver was activated";
        case sm_stat_id::la_read_volume: return "Number of bytes read during log archive scans";
        case sm_stat_id::la_read_count: return "Number of read operations performed on the log archive";
        case sm_stat_id::la_open_count: return "Number of open calls on run files of the log archive scanner";
        case sm_stat_id::la_read_time: return "Time spent reading blocks from log archive (usec)";
        case sm_stat_id::la_block_writes: return "Number of blocks appended to the log archive";
        case sm_stat_id::la_img_compressed_bytes: return "Bytes saved by applying page image compression";
        case sm_stat_id::log_img_format_bytes: return "Bytes added to transaction log by generating page images";
        case sm_stat_id::la_skipped_bytes: return "Bytes skipped in open method of archive index probes";
        case sm_stat_id::la_img_trimmed: return "Log archive lookups trimmed off thanks to page_img logrecs";
        case sm_stat_id::la_wasted_read: return "Wasted log archive reads, i.e., that didn't use any logrec";
    }
    return "UNKNOWN_STAT";
}

void print_sm_stats(sm_stats_t& stats, std::ostream& out)
{
    for (size_t i = 0; i < stats.size(); i++) {
        out << get_stat_name(static_cast<sm_stat_id>(i)) << " "
            << stats[i]
            << std::endl;
    }
}

/*
 * One static stats structure for collecting
 * statistics that might otherwise be lost:
 */
namespace local_ns {
    sm_stats_t _global_stats_;
    static queue_based_block_lock_t _global_stats_mutex;
    static bool _global_stat_init = false;
}
void
smlevel_0::add_to_global_stats(const sm_stats_t &from)
{
    CRITICAL_SECTION(cs, local_ns::_global_stats_mutex);
    if (!local_ns::_global_stat_init) {
        local_ns::_global_stats_.fill(0);
        local_ns::_global_stat_init = true;
    }

    for (size_t i = 0; i < from.size(); i++) {
        local_ns::_global_stats_[i] += from[i];
    }
}
void
smlevel_0::add_from_global_stats(sm_stats_t &to)
{
    CRITICAL_SECTION(cs, local_ns::_global_stats_mutex);
    if (!local_ns::_global_stat_init) {
        local_ns::_global_stats_.fill(0);
        local_ns::_global_stat_init = true;
    }
    for (size_t i = 0; i < to.size(); i++) {
        to[i] += local_ns::_global_stats_[i];
    }
}
