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
#include "sm_stats_t_inc_gen.cpp"
#include "sm_stats_t_dec_gen.cpp"
#include "sm_stats_t_out_gen.cpp"
#include "bf_htab_stats_t_inc_gen.cpp"
#include "bf_htab_stats_t_dec_gen.cpp"
#include "bf_htab_stats_t_out_gen.cpp"

// the strings:
const char *sm_stats_t ::stat_names[] = {
#include "bf_htab_stats_t_msg_gen.h"
#include "sm_stats_t_msg_gen.h"
   ""
};

void bf_htab_stats_t::compute()
{
}

void sm_stats_t::compute()
{
    latch_uncondl_waits = need_latch_uncondl - latch_uncondl_nowaits;

    await_vol_lock_r = need_vol_lock_r - nowait_vol_lock_r;
    await_vol_lock_w = need_vol_lock_w - nowait_vol_lock_w;

    if(log_bytes_written > 0) {
        // skip-log and padding bytes -- actually,
        // anything flushed more than once, although inserted
        // bytes not yet flushed will tend to warp this number
        // if the log wasn't recently flushed.
        log_bytes_rewritten = log_bytes_written - log_bytes_generated;
    }
    if(log_bytes_generated_rb > 0) {
        // get the # bytes generated during forward processing.
        double x = log_bytes_generated - log_bytes_generated_rb;
        w_assert0(x >= 0.0);
        // should always be > 0, since the log_bytes_generated is 
        // the total of fwd and rollback bytes.
        if(x>0.0) {
            log_bytes_rbfwd_ratio = double(log_bytes_generated_rb) / x;
        }else {
            log_bytes_rbfwd_ratio = 0.0;
        }
    }
}

sm_stats_info_t &operator+=(sm_stats_info_t &s, const sm_stats_info_t &t)
{
    s.bfht += t.bfht;
    s.sm += t.sm;
    return s;
}

sm_stats_info_t &operator-=(sm_stats_info_t &s, const sm_stats_info_t &t)
{
    s.bfht -= t.bfht;
    s.sm -= t.sm;
    return s;
}


sm_stats_info_t &operator-=(sm_stats_info_t &s, const sm_stats_info_t &t);

/*
 * One static stats structure for collecting
 * statistics that might otherwise be lost:
 */
namespace local_ns {
    sm_stats_info_t _global_stats_;
    static queue_based_block_lock_t _global_stats_mutex;
}
void
smlevel_0::add_to_global_stats(const sm_stats_info_t &from)
{
    CRITICAL_SECTION(cs, local_ns::_global_stats_mutex);
    local_ns::_global_stats_ += from;
}
void
smlevel_0::add_from_global_stats(sm_stats_info_t &to)
{
    CRITICAL_SECTION(cs, local_ns::_global_stats_mutex);
    to += local_ns::_global_stats_;
}

bool sm_stats_logstats_t::activate = false;
char* sm_stats_logstats_t::filepath = "";

void sm_stats_logstats_t::log_fix_nonroot(tid_t tid, PageID page, PageID parent, latch_mode_t mode, bool conditional,
                                         bool virgin_page, bool only_if_hit, u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix_nonroot,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << mode << ","
              << conditional << ","
              << virgin_page << ","
              << only_if_hit << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_fix_root(tid_t tid, PageID page, StoreID store, latch_mode_t mode, bool conditional,
                                       u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix_root,"
              << tid.as_int64() << ","
              << page << ","
              << store << ","
              << mode << ","
              << conditional << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_fix(tid_t tid, PageID page, PageID parent, latch_mode_t mode, bool conditional,
                                  bool virgin_page, bool only_if_hit, bool hit, bool evict, u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << mode << ","
              << conditional << ","
              << virgin_page << ","
              << only_if_hit << ","
              << hit << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_unfix_nonroot(tid_t tid, PageID page, PageID parent, bool evict, u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unfix_nonroot,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_unfix_root(tid_t tid, PageID page, bool evict, u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unfix_root,"
              << tid.as_int64() << ","
              << page << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_refix(tid_t tid, PageID page, latch_mode_t mode, bool conditional, u_long start, u_long finish)
{
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "refix,"
              << tid.as_int64() << ","
              << page << ","
              << mode << ","
              << conditional << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_pin(tid_t tid, PageID page, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "pin,"
              << tid.as_int64() << ","
              << page << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_unpin(tid_t tid, PageID page, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unpin,"
              << tid.as_int64() << ","
              << page << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_miss_ref(tid_t tid, bf_idx b_idx, PageID page, u_int32_t p, u_int32_t b1_length,
                                       u_int32_t b2_length, bf_idx t1_length, bf_idx t2_length, bf_idx t1_index,
                                       bf_idx t2_index, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "miss_ref,"
              << tid.as_int64() << ","
              << b_idx << ","
              << page << ","
              << p  << ","
              << b1_length << ","
              << b2_length << ","
              << t1_length << ","
              << t2_length << ","
              << t1_index << ","
              << t2_index << ","
              << start << ","
              << finish << std::endl;
}

void
sm_stats_logstats_t::log_pick_victim(tid_t tid, bf_idx b_idx, u_int32_t iterations, u_int32_t p, u_int32_t b1_length,
                                     u_int32_t b2_length, bf_idx t1_length, bf_idx t2_length, bf_idx t1_index,
                                     bf_idx t2_index, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "pick_victim,"
              << tid.as_int64() << ","
              << b_idx << ","
              << iterations << ","
              << p  << ","
              << b1_length << ","
              << b2_length << ","
              << t1_length << ","
              << t2_length << ","
              << t1_index << ","
              << t2_index << ","
              << start << ","
              << finish << std::endl;
}

sm_stats_logstats_t::~sm_stats_logstats_t() {
    logstats->close();
    delete logstats;
}
