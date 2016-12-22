/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

/*<std-header orig-src='shore' incl-file-exclusion='SMSTATS_H'>

 $Id: smstats.h,v 1.36 2010/09/21 14:26:20 nhall Exp $

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

#ifndef SMSTATS_H
#define SMSTATS_H

#include "w_defines.h"
#include "tid_t.h"
#include "latch.h"
#include "basics.h"

#include <iostream>

/*  -- do not edit anything above this line --   </std-header>*/

// This file is included in sm.h in the middle of the class ss_m
// declaration.  Member functions are defined in sm.cpp


/**\addtogroup SSMSTATS
 *
 * The storage manager API allows a server to gather statistics on
 * a per-transaction basis or on a global basis.
 * These counters are not segregated by the semantics of the counters. Rather,
 * they are segregated by the thread that performed the action being measured.
 * In other words, an action is attributed to a transaction if it was 
 * performed by a thread while attached to that transaction.
 * This means that some actions, such as writing pages to disk, might not
 * be attributable to a transaction even though they are in some 
 * sense logically
 * associated with that transaction.  If the write is performed by a
 * page writer (background thread), it will show up in the global statistics
 * but not in any per-transaction statistics. On the other hand, if a write
 * is performed by ss_m::set_store_property (which flushes to disk
 * all pages for the store thus changed) it will be attributed to the
 * transaction.
 *
 * All statistics are collected on a per-smthread_t basis 
 * (thus avoiding expensive atomic updates on a per-counter basis).  
 * Each smthread has its own 
 * local sm_stats_info_t structure for these statistics. 
 * Any time this structure is cleared,
 * its contents are added to a single global statistics structure 
 * (protected by a mutex) before it is cleared.
 * The clearing happens in two circumstances:
 * - when an smthread_t is destroyed (in its destructor)
 * - when an attached instrumented transaction collects the statistics from
 *   the thread (see below).
 *
 * Thus, the single global statistics structure's contents reflect the
 * activities of finished threads and of instrumented transactions' collected
 * statistics, and all other activities are reflected in per-thread
 * statistics structures.
 *
 * A value-added server may collect the global statistics with the
 * ss_m::gather_stats method.  This method first adds together all the
 * per-thread statistics, adds in the global statistics, and returns.
 * The global statistics cannot be reset, and, indeed, they survive
 * the storage manager so that they can be gathered after the
 * storage manager shuts down. This means that to determine incremental
 * statistics, the value-added server has to keep the prior copy of 
 * statistics and diff the current statistics from the prior statistics.
 * The sm_stats_info_t has a difference operator to make this easy.
 * \attention Gathering the per-thread statistics from running threads is
 * not atomic; in other words, if threads are updating their counters
 * while the gathering of their counters is going on, some counts may
 * be missed (become stale). (In any case, they will be stale soon
 * after the statistics are gathered.)
 *
 * A transaction must be instrumented to collect its statistics.
 *
 * Instrumenting a transaction
 * consists in allocating a structure in which to store the collected
 * statistics, and passing in that structure to the storage manager using
 * using the variants of begin_xct, commit_xct, etc that take 
 * an argument of this type.  
 *
 * When a transaction is detached from a thread,
 * the statistics it gathered up to that point by the thread are
 * added to the per-transaction statistics, and the thread statistics are
 * cleared so they don't get over-counted.
 *
 * A server may gather the per-transaction statistics for the
 * attached running transaction with
 * ss_m::gather_xct_stats.  
 *
 * A server may choose to reset the per-transaction statistics when it
 * gathers them; this facilitates gathering incremental statistics.
 * These counters aren't lost to the world, since their values were 
 * added to the global statistics before they were gathered in the first place. 
 *
 * \attention The per-transaction statistics structure is not 
 * protected against concurrently-attached threads, so
 * its values are best collected and reset when the server 
 * knows that only one thread is attached to the 
 * transaction when making the call. 
 */

 /**\brief Statistics (counters) for most of the storage manager.
  * \details
  * This structure holds most of the storage manager's statictics, 
  * those not specific to the buffer-manager's hash table.
  * Those counters are in bf_htab_stats_t.
  */
class sm_stats_t {
public:
    void    compute();
#include "sm_stats_t_struct_gen.h"
};

 /**\brief Statistics (counters) for the buffer-manager hash table.
  * \details
  * This structure holds counters
  * specific to the buffer-manager's hash table.
  * Although it is not necessary,
  * they are separated from the rest for ease of unit-testing.
  */
class bf_htab_stats_t {
public:
    void    compute();
#include "bf_htab_stats_t_struct_gen.h"
};

/**\brief Storage Manager Statistics 
 *
 * The storage manager is instrumented; it collects the statistics
 * (mostly counters) that are described in *_stats.dat files (input 
 * files to Perl scripts).
 * These statistics are incremented in per-thread structures, which
 * are gathered and available to the value-added server
 * under various circumstances, described in \ref SSMSTATS.
 */
class sm_stats_info_t {
public:
    bf_htab_stats_t  bfht;
    sm_stats_t       sm;
    void    compute() { 
        bfht.compute(); 
        sm.compute(); 
    }
    friend ostream& operator<<(ostream&, const sm_stats_info_t& s);
    sm_stats_info_t() {
        memset(this, '\0', sizeof (*this));
    }
};

extern sm_stats_info_t &operator+=(sm_stats_info_t &s, const sm_stats_info_t &t);
extern sm_stats_info_t &operator-=(sm_stats_info_t &s, const sm_stats_info_t &t);

/*!\class   sm_stats_logstats_t
 * \brief   Buffer Pool Log
 *
 * \details The storage manager is instrumented; it collects detailed logs about
 *          every refix, fix and unfix that happens in the buffer pool. It works
 *          independent from the database log and stores the log entries as text
 *          in a file specified with the option \c sm_fix_stats_file \c when it
 *          is activated with the option \c sm_fix_stats \c.
 *          This can be easily extended by other kinds of log records that log
 *          events and characteristics of those that happen in the Storage Manager
 *          during a transaction by just adding member functions like
 *          \link log_refix() and using them as in
 *          \verbatim me()->TL_stats_logstats()->log_refix \endverbatim.
 *
 * \warning The activation of this feature can cause some performance loss
 *          as it writes around 25kB of log records per avg. TPC-C transaction.
 * \author  Max Gilbert
 */
class sm_stats_logstats_t {
private:
    /*!\var ofstream* logstats
     * File stream used to append the log records to the log. It's open as long as
     * this instance of \link sm_stats_logstats_t exists.
     */
    std::ofstream*         logstats;
public:
    /*!\fn      sm_stats_logstats_t()
     * \brief   Constructor for \link sm_stats_logstats_t
     * \details It uses the the value of the static member variable
     *          \link sm_stats_logstats_t::filepath to specify its log file by
     *          opening the file stream \link logstats.
     */
    sm_stats_logstats_t() {
        logstats = new std::ofstream (sm_stats_logstats_t::filepath, std::ofstream::app);
        w_assert0(logstats->is_open());
    };
    /*!\fn      virtual ~sm_stats_logstats_t()
     * \brief   Destructor for \link sm_stats_logstats_t
     * \details It closes file stream \link logstats.
     */
    virtual ~sm_stats_logstats_t();
    
    /*!\var static bool activate
     * This static variable is set according to the value specified in the option
     * \c sm_fix_stats \c.
     */
    static bool activate;
    /*!\var static char* filepath
     * This static variable is set according to the value specified in the option
     * \c sm_fix_stats_file \c.
     */
    static char* filepath;
public:
    /*!\fn void log_fix_nonroot(tid_t tid, PageID page, PageID parent, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, u_long start, u_long finish)
     * \brief Creates a new log record for a fix_nonroot event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        The \c pid \c as in the function call of  \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param parent      The \link PageID of the \c parent \c as in the function call of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param mode        As in the function call of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param conditional As in the function call of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param virgin_page As in the function call of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param only_if_hit As in the function call of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) started.
     * \param finish      The time when the execution of \link bf_tree_m::fix_nonroot(generic_page*& page, generic_page *parent, PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, lsn_t emlsn) finished.
     */
    void log_fix_nonroot(tid_t tid, PageID page, PageID parent, latch_mode_t mode, bool conditional,
                         bool virgin_page, bool only_if_hit, u_long start, u_long finish);
    /*!\fn void log_fix_root(tid_t tid, PageID page, StoreID store, latch_mode_t mode, bool conditional, u_long start, u_long finish)
     * \brief Creates a new log record for a fix_root event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        The \c pid \c as in the function call of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) that caused this event.
     * \param store       As in the function call of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) that caused this event.
     * \param mode        As in the function call of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) that caused this event.
     * \param conditional As in the function call of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) started.
     * \param finish      The time when the execution of \link bf_tree_m::fix_root (generic_page*& page, StoreID store, latch_mode_t mode, bool conditional, bool virgin) finished.
     */
    void log_fix_root(tid_t tid, PageID page, StoreID store, latch_mode_t mode, bool conditional,
                      u_long start, u_long finish);
    /*!\fn void log_unfix_nonroot(tid_t tid, PageID page, PageID parent ,bool evict, u_long start, u_long finish)
     * \brief Creates a new log record for a unfix event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        As in the function call of \link bf_tree_m::unfix(const generic_page* page, bool evict) that caused this event.
     * \param parent      The \link PageID of the \c page \c.
     * \param evict       As in the function call of \link bf_tree_m::unfix(const generic_page* page, bool evict) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::unfix(const generic_page* page, bool evict) started.
     * \param finish      The time when the execution of \link bf_tree_m::unfix(const generic_page* page, bool evict) finished.
     */
    void log_unfix_nonroot(tid_t tid, PageID page, PageID parent ,bool evict, u_long start, u_long finish);
    /*!\fn void log_unfix_root(tid_t tid, PageID page, bool evict, u_long start, u_long finish)
     * \brief Creates a new log record for a unfix event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        As in the function call of \link bf_tree_m::unfix(const generic_page* page, bool evict) that caused this event.
     * \param evict       As in the function call of \link bf_tree_m::unfix(const generic_page* page, bool evict) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::unfix(const generic_page* page, bool evict) started.
     * \param finish      The time when the execution of \link bf_tree_m::unfix(const generic_page* page, bool evict) finished.
     */
    void log_unfix_root(tid_t tid, PageID page, bool evict, u_long start, u_long finish);
    /*!\fn void log_refix(tid_t tid, PageID page, latch_mode_t mode, bool conditional, u_long start, u_long finish)
     * \brief Creates a new log record for a refix event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        The \link PageID of the \c page \c after the execution of the function \link bf_tree_m::refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional) that caused this event.
     * \param mode        As in the function call of \link bf_tree_m::refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional) that caused this event.
     * \param conditional As in the function call of \link bf_tree_m::refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional) started.
     * \param finish      The time when the execution of \link bf_tree_m::refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional) finished.
     */
    void log_refix(tid_t tid, PageID page, latch_mode_t mode, bool conditional, u_long start,
                   u_long finish);
    /*!\fn log_pin(tid_t tid, PageID page, u_long start, u_long finish)
     * \brief Creates a new log record for a pin for refix event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        As in the function call of \link bf_tree_m::pin_for_refix(const generic_page* page) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::pin_for_refix(const generic_page* page) started.
     * \param finish      The time when the execution of \link bf_tree_m::pin_for_refix(const generic_page* page) finished.
     */
    void log_pin(tid_t tid, PageID page, u_long start, u_long finish);
    /*!\fn log_unpin(tid_t tid, PageID page, u_long start, u_long finish)
     * \brief Creates a new log record for a unpin for refix event
     * \param tid         The \link tid_t of the \link smthread_t that caused this event.
     * \param page        As in the function call of \link bf_tree_m::unpin_for_refix(bf_idx idx) that caused this event.
     * \param start       The time when the execution of \link bf_tree_m::unpin_for_refix(bf_idx idx) started.
     * \param finish      The time when the execution of \link bf_tree_m::unpin_for_refix(bf_idx idx) finished.
     */
    void log_unpin(tid_t tid, PageID page, u_long start, u_long finish);
};



/**\brief Configuration Information
 * \details
 * The method ss_m::config_info lets a server to pick up some useful
 * configuration information from the storage manager.  Several of
 * these data depend on the compile-time page size; some depend on
 * run-time options.
 */
struct sm_config_info_t {
    /**\brief compile-time constant. Settable in 
     * \code shore.def \endcode. 
     * Default is 8K.
     */
    u_long page_size;         // bytes in page, including all headers

    /**\brief Data space available on a page of a large record */
    //TODO: SHORE-KITS-API
    // shore-kits needs max_small_rec; shore-sm-6.0.1 initializes this field at 
    // several places. Make sure Zero similarly initializes max_small_rec
    u_long max_small_rec;      // maximum number of bytes in a "small"
                // (ie. on one page) record.  This is
                // align(header_len)+align(body_len).
    u_long lg_rec_page_space;    

    /**\brief Size in KB of buffer pool */
    u_long buffer_pool_size;    // buffer pool size in kilo-bytes

    /**\brief Largest permissible size in bytes of an index entry 
     * (key,value pair) */
    u_long max_btree_entry_size;

    /**\brief Number of extent links on an extent page */
    u_long exts_on_page;

    /**\brief Number of pages per extent (compile-time constant) 
     * \note The storage manager has not been tested with any value but 8.
     */
    u_long pages_per_ext; 

    /**\brief True if logging is on.
     * \note The multi-threaded storage manager has not been 
     * tested with logging turned off, so turning off logging is
     * not supported in this release.
     */
    bool   logging; 

    friend ostream& operator<<(ostream&, const sm_config_info_t& s);
};

/*<std-footer incl-file-exclusion='SMSTATS_H'>  -- do not edit anything below this line -- */

#endif          /*</std-footer>*/
