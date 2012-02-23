/* -*- mode:C++; c-basic-offset:4 -*-
     Shore-MT -- Multi-threaded port of the SHORE storage manager
   
                       Copyright (c) 2007-2009
      Data Intensive Applications and Systems Labaratory (DIAS)
               Ecole Polytechnique Federale de Lausanne
   
                         All Rights Reserved.
   
   Permission to use, copy, modify and distribute this software and
   its documentation is hereby granted, provided that both the
   copyright notice and this permission notice appear in all copies of
   the software, derivative works or modified versions, and any
   portions thereof, and that both notices appear in supporting
   documentation.
   
   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS
   DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
   RESULTING FROM THE USE OF THIS SOFTWARE.
*/

/*<std-header orig-src='shore'>

 $Id: sm.cpp,v 1.501 2010/12/17 19:36:26 nhall Exp $

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

#define SM_SOURCE
#define SM_C

#ifdef __GNUG__
class prologue_rc_t;
#pragma implementation "sm.h"
#pragma implementation "prologue.h"
#pragma implementation "sm_base.h"
#endif

#include "w.h"
#include "option.h"
#include "sm_int_4.h"
#include "chkpt.h"
#include "sm.h"
#include "sm_vtable_enum.h"
#include "prologue.h"
#include "device.h"
#include "vol.h"
#include "crash.h"
#include "restart.h"

#ifdef EXPLICIT_TEMPLATE
template class w_auto_delete_t<SmStoreMetaStats*>;
#endif

bool        smlevel_0::shutdown_clean = true;
bool        smlevel_0::shutting_down = false;

smlevel_0::operating_mode_t 
            smlevel_0::operating_mode = smlevel_0::t_not_started;

            //controlled by AutoTurnOffLogging:
bool        smlevel_0::lock_caching_default = true;
bool        smlevel_0::logging_enabled = true;
bool        smlevel_0::do_prefetch = false;

#ifndef SM_LOG_WARN_EXCEED_PERCENT
#define SM_LOG_WARN_EXCEED_PERCENT 40
#endif
smlevel_0::fileoff_t smlevel_0::log_warn_trigger = 0;
int                  smlevel_0::log_warn_exceed_percent = 
                                    SM_LOG_WARN_EXCEED_PERCENT;
ss_m::LOG_WARN_CALLBACK_FUNC      
                     smlevel_0::log_warn_callback = 0;
ss_m::LOG_ARCHIVED_CALLBACK_FUNC 
                     smlevel_0::log_archived_callback = 0;

// these are set when the logsize option is set
smlevel_0::fileoff_t        smlevel_0::max_logsz = 0;
smlevel_0::fileoff_t        smlevel_0::chkpt_displacement = 0;

// Whenever a change is made to data structures stored on a volume,
// volume_format_version be incremented so that incompatibilities
// will be detected.
//
// Different ALIGNON values are NOT reflected in the version number,
// so it is still possible to create incompatible volumes by changing
// ALIGNON.
//
//  1 = original
//  2 = lid and lgrex indexes contain vid_t
//  3 = lid index no longer contains vid_t
//  4 = added store flags to pages
//  5 = large records no longer contain vid_t
//  6 = volume headers have lvid_t instead of vid_t
//  7 = removed vid_t from sinfo_s (stored in directory index)
//  8 = added special store for 1-page btrees
//  9 = changed prefix for reserved root index entries to SSM_RESERVED
//  10 = extent link changed shape.
//  11 = extent link changed, allowing concurrency in growing a store
//  12 = dir btree contents changed (removed store flag and property)
//  13 = Large volumes : changed size of snum_t and extnum_t
//  14 = Changed size of lsn_t, hence log record headers were rearranged
//       and page headers changed.  Small disk address
//  15 = Same as 14, but with large disk addresses.
//  16 = Align body of page to an eight byte boundary.  This should have 
//       occured in 14, but there are some people using it, so need seperate
//       numbers.
//  17 = Same as 16 but with large disk addresses.   
//  18 = Release 6.0 of the storage manager.  
//       Only large disk addresses, 8-byte alignment, added _hdr_pages to
//       volume header, logical IDs and 1page indexes are deprecated.
//       Assumes 64-bit architecture.
//       No support for older volume formats.

#define        VOLUME_FORMAT        18

uint32_t        smlevel_0::volume_format_version = VOLUME_FORMAT;


/*
 * _being_xct_mutex: Used to prevent xct creation during volume dismount.
 * Its sole purpose is to be sure that we don't have transactions 
 * running while we are  creating or destroying volumes or 
 * mounting/dismounting devices, which are generally 
 * start-up/shut-down operations for a server.
 */

typedef srwlock_t sm_vol_rwlock_t;
// Certain operations have to exclude xcts
static sm_vol_rwlock_t          _begin_xct_mutex;

device_m* smlevel_0::dev = 0;
io_m* smlevel_0::io = 0;
bf_m* smlevel_0::bf = 0;
log_m* smlevel_0::log = 0;
tid_t *smlevel_0::redo_tid = 0;

lock_m* smlevel_0::lm = 0;

ErrLog*            smlevel_0::errlog;


char smlevel_0::zero_page[page_sz];

chkpt_m* smlevel_1::chkpt = 0;

btree_m* smlevel_2::bt = 0;

lid_m* smlevel_4::lid = 0;

ss_m* smlevel_4::SSM = 0;

// option related statics
option_group_t* ss_m::_options = NULL;

#if defined(Sparc) && defined(SOLARIS2) && defined(SOLARIS2_PSETS)
#include <sys/types.h>
#include <sys/processor.h>
#include <sys/procset.h>
#include <sys/pset.h>
#endif
option_t* ss_m::_hugetlbfs_path = NULL;
option_t* ss_m::_reformat_log = NULL;
option_t* ss_m::_prefetch = NULL;
option_t* ss_m::_bufpoolsize = NULL;
option_t* ss_m::_locktablesize = NULL;
option_t* ss_m::_logdir = NULL;
option_t* smlevel_0::_backgroundflush = NULL;
option_t* ss_m::_logsize = NULL;
option_t* ss_m::_logbufsize = NULL;
option_t* ss_m::_error_log = NULL;
option_t* ss_m::_error_loglevel = NULL;
option_t* ss_m::_log_warn_percent = NULL;
option_t* ss_m::_num_page_writers = NULL;
option_t* ss_m::_logging = NULL;


/*
 *  Class ss_m code
 */

/*
 *  Order is important!!
 */
int ss_m::_instance_cnt = 0;
//ss_m::param_t ss_m::curr_param;

// sm_einfo.i defines the w_error_info_t smlevel_0::error_info[]
const
#include <e_einfo_gen.h>

const char *four_pages_min(int kb) {
    static char  buf[48];

    int four = 4 * SM_PAGESIZE/1024;

    sprintf(buf, "%d", ::max(four, kb));

    return   buf;
}

rc_t ss_m::setup_options(option_group_t* options)
{
    sthread_t::initialize_sthreads_package();

    W_DO(options->add_option("sm_reformat_log", "yes/no", "no",
            "yes will destroy your log",
            false, option_t::set_value_bool, _reformat_log));

    W_DO(options->add_option("sm_prefetch", "yes/no", "no",
            "no disables page prefetching on scans",
            false, option_t::set_value_bool, _prefetch));

    W_DO(options->add_option("sm_bufpoolsize", "#>=8192", NULL,
            "size of buffer pool in Kbytes",
            true, option_t::set_value_long, _bufpoolsize));

    W_DO(options->add_option("sm_locktablesize", "#>64", "64000",
            "size of lock manager hash table",
            false, option_t::set_value_long, _locktablesize));

    // Include this option in any case, so users don't have to remove
    // unknown options from their config files.
    W_DO(options->add_option("sm_hugetlbfs_path",  "absolute path",
            HUGETLBFS_PATH,
            "needed only if you configured --with-hugetlbfs, string NULL means do not use hugetlbfs",
            false, option_t::set_value_charstr, _hugetlbfs_path));

    W_DO(options->add_option("sm_logdir", "directory name", NULL,
            "directory for log files",
            true, option_t::set_value_charstr, _logdir));

    W_DO(options->add_option("sm_backgroundflush", "yes/no", "yes",
            "yes indicates background buffer pool flushing thread is enabled",
            false, option_t::set_value_bool, _backgroundflush));

    W_DO(options->add_option("sm_logbufsize", "(>=4 and <=128)*(page size)", 
                // Too bad we can't compute a string here:
             four_pages_min(128), // function above
            "size of log buffer Kbytes",
            false, option_t::set_value_long, _logbufsize));

// NOTE: CC's preprocessor can't handle putting the #if/#else/#endif
// inside the W_DO
#if SM_PAGESIZE < 8192
    W_DO(options->add_option("sm_logsize", 
            "#>8256", 
            "16448",
            "maximum size of the log in Kbytes",
            false, _set_option_logsize, _logsize));
#else
    W_DO(options->add_option("sm_logsize", 
            "#>8256", 
            "10000",
            "maximum size of the log in Kbytes",
            false, _set_option_logsize, _logsize));
#endif

    W_DO(options->add_option("sm_errlog", "string", "-",
            "- (stderr) or <filename>",
            false, option_t::set_value_charstr, _error_log));

    W_DO(options->add_option("sm_errlog_level", "string", "error",
            "none|emerg|fatal|alert|internal|error|warning|info|debug",
            false, option_t::set_value_charstr, _error_loglevel));

    W_DO(options->add_option("sm_log_warn", "0-100", "0",
            "% of log in use that triggers callback to server (0 means no trigger)",
            false, option_t::set_value_long, _log_warn_percent));

    W_DO(options->add_option("sm_num_page_writers", ">=0", "2",
            "the number of page writers in the bpool cleaner",
            false, option_t::set_value_long, _num_page_writers));

    W_DO(options->add_option("sm_logging", "yes/no", "yes",
            "no will turn off logging; Rollback, restart not possible.",
            false, option_t::set_value_bool, _logging));

    _options = options;
    return RCOK;
}

rc_t ss_m::_set_option_logsize(
        option_t* opt, 
        const char* value, 
        ostream* _err_stream
)
{
    ostream* err_stream = _err_stream;

    if (err_stream == NULL) {
        err_stream = &cerr;
    }

    // the logging system should not be running.  if it is
    // then don't set the option
    if (smlevel_0::log) return RCOK;

    w_assert3(opt == _logsize);

    w_rc_t        e;
    if (sizeof(fileoff_t) == 8)
        e = option_t::set_value_int8(opt, value, err_stream);
    else
        e = option_t::set_value_int4(opt, value, err_stream);
    W_DO(e);

    fileoff_t maxlogsize = fileoff_t(
// ARCH_LP64 and LARGEFILE_AWARE are determined by configure
// and set isn config/shore-config.h
#if defined(LARGEFILE_AWARE)  || defined(ARCH_LP64)
           w_base_t::strtoi8(_logsize->value())
#else
           atoi(_logsize->value())
#endif
        );

    // cerr << "User-option _logsize->value() " << maxlogsize << " KB" << endl;

    // The option is in units of KB; convert it to bytes.
    maxlogsize *= 1024;

    // cerr << "User-option _logsize->value() " << maxlogsize << " bytes" << endl;


    // maxlogsize is the user-defined maximum open-log size.
    // Compile-time constants determine the size of a segment,
    // and the open log size is smlevel_0::max_openlog segments,
    // so that means we determine the number of segments per
    // partition thus:
    // max partition size is user max / smlevel_0::max_openlog.
    // max partition size must be an integral multiple of segments
    // plus 1 block. The log manager computes this for us:
    fileoff_t psize = maxlogsize / smlevel_0::max_openlog;

    // cerr << "Resulting partition size " << psize << " bytes" << endl;

    // convert partition size to partition data size: (remove overhead)
    psize = log_m::partition_size(psize);

    // cerr << "Adjusted partition size " << psize << " bytes" << endl;

    /* Enforce the built-in shore limit that a log partition can only
       be as long as the file address in a lsn_t allows for...  
       This is really the limit of a LSN, since LSNs map 1-1 with disk
       addresses. 
       Also that it can't be larger than the os allows
   */

    if (psize > log_m::max_partition_size()) {
        // we might not be able to do this: 
        fileoff_t tmp = log_m::max_partition_size();
        tmp /= 1024;

        *err_stream << "Partition data size " << psize 
                << " exceeds limit (" << log_m::max_partition_size() << ") "
                << " imposed by the size of an lsn."
                <<endl;
        *err_stream << " Choose a smaller sm_logsize." <<endl;
        *err_stream << " Maximum is :" << tmp << endl;
        return RC(OPT_BadValue);
    }

    if (psize < log_m::min_partition_size()) {
        fileoff_t tmp = fileoff_t(log_m::min_partition_size());
        tmp *= smlevel_0::max_openlog;
        tmp /= 1024;
        *err_stream 
            << "Partition data size (" << psize 
            << ") is too small for " << endl
            << " a segment ("  
            << log_m::min_partition_size()   << ")" << endl
            << "Partition data size is computed from sm_logsize;"
            << " minimum sm_logsize is " << tmp << endl;
        return RC(OPT_BadValue);
    }


    // maximum size of all open log files together
    max_logsz = fileoff_t(psize * smlevel_0::max_openlog);

    // cerr << "Resulting max_logsz " << max_logsz << " bytes" << endl;

    // take check points every 3 log file segments.
    chkpt_displacement = log_m::segment_size() * 3;

    return RCOK;
}

/* 
 * NB: reverse function, _make_store_property
 * is defined in dir.cpp -- so far, used only there
 */
ss_m::store_flag_t
ss_m::_make_store_flag(store_property_t property)
{
    store_flag_t flag = st_bad;

    switch (property)  {
        case t_regular:
            flag = st_regular;
            break;
        case t_temporary:
            flag = st_tmp;
            break;
        case t_load_file:
            flag = st_load_file;
            break;
        case t_insert_file:
            flag = st_insert_file;
            break;
        case t_bad_storeproperty:
        default:
            W_FATAL_MSG(eINTERNAL, << "bad store property :" << property );
            break;
    }

    return flag;
}


static queue_based_block_lock_t ssm_once_mutex;

ss_m::ss_m(
    smlevel_0::LOG_WARN_CALLBACK_FUNC callbackwarn /* = NULL */,
    smlevel_0::LOG_ARCHIVED_CALLBACK_FUNC callbackget /* = NULL */
)
{
    FUNC(ss_m::ss_m);
    sthread_t::initialize_sthreads_package();

    // This looks like a candidate for pthread_once(), 
    // but then smsh would not be able to
    // do multiple startups and shutdowns in one process, alas. 
    CRITICAL_SECTION(cs, ssm_once_mutex);
    _construct_once(callbackwarn, callbackget);
}

void
ss_m::_construct_once(
    smlevel_0::LOG_WARN_CALLBACK_FUNC warn,
    smlevel_0::LOG_ARCHIVED_CALLBACK_FUNC get
)
{
    FUNC(ss_m::_construct_once);

    smlevel_0::log_warn_callback  = warn;
    smlevel_0::log_archived_callback  = get;

    // Clear out the fingerprint map for the smthreads.
    // All smthreads created after this will be compared against
    // this map for duplication.
    smthread_t::init_fingerprint_map();

    static bool initialized = false;
    if (! initialized)  {
        smlevel_0::init_errorcodes();
        initialized = true;
    }
    if (_instance_cnt++)  {
        // errlog might not be null since in this case there was another instance.
        if(errlog) {
            errlog->clog << fatal_prio 
            << "ss_m cannot be instantiated more than once"
             << flushl;
        }
        W_FATAL_MSG(eINTERNAL, << "instantiating sm twice");
    }

    /*
     *  Level 0
     */
    errlog = new ErrLog("ss_m", log_to_unix_file, _error_log->value());
    if(!errlog) {
        W_FATAL(eOUTOFMEMORY);
    }
    if(_error_loglevel // && _error_loglevel->is_set() is_set means by user
            && _error_loglevel->value()) {
        errlog->setloglevel(ErrLog::parse(_error_loglevel->value()));
    }
    ///////////////////////////////////////////////////////////////
    // Henceforth, all errors can go to ss_m::errlog thus:
    // ss_m::errlog->clog << XXX_prio << ... << flushl;
    // or
    // ss_m::errlog->log(log_XXX, "format...%s..%d..", s, n); NB: no newline
    ///////////////////////////////////////////////////////////////
#if W_DEBUG_LEVEL > 0
	// just to be sure errlog is working
	errlog->clog << debug_prio << "Errlog up and running." << flushl; 
#endif

    w_assert1(page_sz >= 1024);

    // make sure setup_options was called successfully
    w_assert1(_options);

    /*
     *  Reset flags
     */
    shutting_down = false;
    shutdown_clean = true;

   /*
    * buffer pool size
    */
    uint32_t  nbufpages = (strtoul(_bufpoolsize->value(), NULL, 0) * 1024 - 1) / page_sz + 1;
    if (nbufpages < 10)  {
        errlog->clog << fatal_prio << "ERROR: buffer size ("
             << _bufpoolsize->value() 
             << "-KB) is too small" << flushl;
        errlog->clog << fatal_prio << "       at least " << 32 * page_sz / 1024
             << "-KB is needed" << flushl;
        W_FATAL(eCRASH);
    }
    long  space_needed = bf_m::mem_needed(nbufpages);

    // number of page writers
    int32_t  npgwriters = int32_t(strtoul(_num_page_writers->value(), NULL, 0)); 
    if(npgwriters < 0) {
        errlog->clog << fatal_prio << "ERROR: num page writers must be positive : "
             << _num_page_writers->value() 
             << flushl;
        W_FATAL(eCRASH);
    }


    uint64_t logbufsize = (uint64_t) strtoul(_logbufsize->value(), NULL, 0) * 1024;
    // pretty big limit -- really, the limit is imposed by the OS's
    // ability to read/write
    if (uint64_t(logbufsize) < (uint64_t) 4 * ss_m::page_sz) {
        errlog->clog << fatal_prio 
        << "Log buf size (sm_logbufsize = " << (int)logbufsize
        << " ) is too small for pages of size " 
        << unsigned(ss_m::page_sz) << " bytes."
        << flushl; 
        errlog->clog << fatal_prio 
        << "Need to hold at least 4 pages ( " << 4 * ss_m::page_sz
        << ")"
        << flushl; 
        W_FATAL(OPT_BadValue);
    }
    if (uint64_t(logbufsize) > uint64_t(max_int4)) {
        errlog->clog << fatal_prio 
        << "Log buf size (sm_logbufsize = " << (int)logbufsize
        << " ) is too big: individual log files can't be large files yet."
        << flushl; 
        W_FATAL(OPT_BadValue);
    }
    DBG(<<"SHM Need " << space_needed << " for buffer pool" );

    /*
     * Allocate the buffer-pool memory
     */ 
    char        *shmbase;
    w_rc_t        e;
#ifdef HAVE_HUGETLBFS
    // fprintf(stderr, "setting path to  %s\n", _hugetlbfs_path->value());
     e = smthread_t::set_hugetlbfs_path(_hugetlbfs_path->value());
#else
     if(_hugetlbfs_path->is_set() /* by user, as opposed to default value */) {
         cerr << "Warning: sm_hugetlbfs_path option " <<
             _hugetlbfs_path->value() 
             << " ignored: not configured --with-hugetlbfs"
             << endl;
     }
#endif
    e = smthread_t::set_bufsize(space_needed, shmbase);
    if (e.is_error()) {
        W_COERCE(e);
    }
    w_assert1(is_aligned(shmbase));
    DBG(<<"SHM at address" << W_ADDR(shmbase));


    /*
     * Now we can create the buffer manager
     */ 

    bf = new bf_m(nbufpages, shmbase, npgwriters);
    if (! bf) {
        W_FATAL(eOUTOFMEMORY);
    }
    shmbase +=  bf_m::mem_needed(nbufpages);
    /* just hang onto this until we create thelog manager...*/

    bool badVal = false;

    lm = new lock_m(strtol(_locktablesize->value(), NULL, 0));
    if (! lm)  {
        W_FATAL(eOUTOFMEMORY);
    }

    dev = new device_m;
    if (! dev) {
        W_FATAL(eOUTOFMEMORY);
    }

    io = new io_m;
    if (! io) {
        W_FATAL(eOUTOFMEMORY);
    }

    /*
     *  Level 1
     */
    smlevel_0::logging_enabled = 
        option_t::str_to_bool(_logging->value(), badVal);
    if (logging_enabled)  
    {
        w_assert3(!badVal);

        bool reformat_log = 
            option_t::str_to_bool(_reformat_log->value(), badVal);
        w_assert3(!badVal);

        if(max_logsz < 8*int(logbufsize)) {
          errlog->clog << warning_prio << 
            "WARNING: Log buffer is bigger than 1/8 partition (probably safe to make it smaller)."
                   << flushl;
        }
        e = log_m::new_log_m(log, 
                     _logdir->value(), 
                     logbufsize, 
                     reformat_log);
        W_COERCE(e);

        int percent=0;
        if(_log_warn_percent // && _log_warn_percent->is_set() means by user; false if default value
                )
            percent = strtol(_log_warn_percent->value(), NULL, 0);

        // log_warn_exceed is %; now convert it to raw # bytes
        // that we must have left at all times. When the space available
        // in the log falls below this, it'll trigger the warning.
        if (percent > 0) {
            smlevel_0::log_warn_trigger  = (long) (
        // max_openlog is a compile-time constant
                log->limit() * max_openlog * 
                (100.0 - (double)smlevel_0::log_warn_exceed_percent) / 100.00);
        }

    } else {
        /* Run without logging at your own risk. */
        errlog->clog << warning_prio << 
        "WARNING: Running without logging! Do so at YOUR OWN RISK. " 
        << flushl;
    }
    DBG(<<"Level 2");
    
    /*
     *  Level 2
     */
    
    bt = new btree_m;
    if (! bt) {
        W_FATAL(eOUTOFMEMORY);
    }
    bt->construct_once();

    DBG(<<"Level 3");
    /*
     *  Level 3
     */
    chkpt = new chkpt_m;
    if (! chkpt)  {
        W_FATAL(eOUTOFMEMORY);
    }

    DBG(<<"Level 4");
    /*
     *  Level 4
     */
    SSM = this;

    lid = new lid_m();
    if (! lid) {
        W_FATAL(eOUTOFMEMORY);
    }

    me()->mark_pin_count();
 
    /*
     * Mount the volumes for recovery.  For now, we automatically
     * mount all volumes.  A better solution would be for restart_m
     * to tell us, after analysis, whether any volumes should be
     * mounted.  If not, we can skip the mount/dismount.
     */

    if (
            option_t::str_to_bool(_logging->value(), badVal)
    )  {
        w_assert3(!badVal);

        restart_m restart;
        smlevel_0::redo_tid = restart.redo_tid();
        restart.recover(log->master_lsn());

        {   // contain the scope of dname[]
            // record all the mounted volumes after recovery.
            int num_volumes_mounted = 0;
            int        i;
            char    **dname;
            dname = new char *[max_vols];
            if (!dname) {
                W_FATAL(fcOUTOFMEMORY);
            }
            for (i = 0; i < max_vols; i++) {
                dname[i] = new char[smlevel_0::max_devname+1];
                if (!dname[i]) {
                    W_FATAL(fcOUTOFMEMORY);
                }
            }
            vid_t    *vid = new vid_t[max_vols];
            if (!vid) {
                W_FATAL(fcOUTOFMEMORY);
            }

            W_COERCE( io->get_vols(0, max_vols, dname, vid, num_volumes_mounted) );

            DBG(<<"Dismount all volumes " << num_volumes_mounted);
            // now dismount all of them at the io level, the level where they
            // were mounted during recovery.
            W_COERCE( io->dismount_all(true/*flush*/) );

            // now mount all the volumes properly at the sm level.
            // then dismount them and free temp files only if there
            // are no locks held.
            for (i = 0; i < num_volumes_mounted; i++)  {
                uint vol_cnt;
                rc_t rc;
                DBG(<<"Remount volume " << dname[i]);
                rc =  _mount_dev(dname[i], vol_cnt, vid[i]) ;
                if(rc.is_error()) {
                    ss_m::errlog->clog  << warning_prio
                    << "Volume on device " << dname[i]
                    << " was only partially formatted; cannot be recovered."
                    << flushl;
                } else {
                    W_COERCE( _dismount_dev(dname[i]));
                }
            }
            delete [] vid;
            for (i = 0; i < max_vols; i++) {
                delete [] dname[i];
            }
            delete [] dname;    
        }

        smlevel_0::redo_tid = 0;

    }

    smlevel_0::operating_mode = t_forward_processing;

    // Have the log initialize its reservation accounting.
    if(log) log->activate_reservations();

    // Force the log after recovery.  The background flush threads exist
    // and might be working due to recovery activities.
    // But to avoid interference with their control structure, 
    // we will do this directly.  Take a checkpoint as well.
    if(log) {
        bf->force_until_lsn(log->curr_lsn());
        chkpt->wakeup_and_take();
    }    

    me()->check_pin_count(0);

    chkpt->spawn_chkpt_thread();

    do_prefetch = 
        option_t::str_to_bool(_prefetch->value(), badVal);
    w_assert3(!badVal);
    DBG(<<"constructor done");
}

ss_m::~ss_m()
{
    // This looks like a candidate for pthread_once(), but then smsh 
    // would not be able to
    // do multiple startups and shutdowns in one process, alas. 
    CRITICAL_SECTION(cs, ssm_once_mutex);
    _destruct_once();
}

void
ss_m::_destruct_once()
{
    FUNC(ss_m::~ss_m);

    --_instance_cnt;

    if (_instance_cnt)  {
        if(errlog) {
            errlog->clog << warning_prio << "ss_m::~ss_m() : \n"
             << "\twarning --- destructor called more than once\n"
             << "\tignored" << flushl;
        } else {
            cerr << "ss_m::~ss_m() : \n"
             << "\twarning --- destructor called more than once\n"
             << "\tignored" << endl;
        }
        return;
    }

    // Set shutting_down so that when we disable bg flushing, if the
    // log flush daemon is running, it won't just try to re-activate it.
    shutting_down = true;

    // We will flush if needed, serially -- not relying on b/g flushing
    W_COERCE(bf->disable_background_flushing());

    
    // get rid of all non-prepared transactions
    // First... disassociate me from any tx
    if(xct()) {
        me()->detach_xct(xct());
    }
    // now it's safe to do the clean_up
    int nprepared = xct_t::cleanup(false /* don't dispose of prepared xcts */);

    if (shutdown_clean) {
        // dismount all volumes which aren't locked by a prepared xct
        // We can't use normal dismounting for the prepared xcts because
        // they would be logged as dismounted. We need to dismount them
        // w/o logging turned on.
        // That happens below.

        W_COERCE( bf->force_all(true) );
        me()->check_actual_pin_count(0);

        // take a clean checkpoints with the volumes which need 
        // to be remounted and the prepared xcts
        // Note that this force_until_lsn will do a direct bpool scan
        // with serial writes since the background flushing has been
        // disabled
        if(log) bf->force_until_lsn(log->curr_lsn());
    chkpt->wakeup_and_take();

        // from now no more logging and checkpoints will be done
        chkpt->retire_chkpt_thread();

        W_COERCE( dev->dismount_all() );
    } else {
        /* still have to close the files, but don't log since not clean !!! */

        // from now no more logging and checkpoints will be done
        chkpt->retire_chkpt_thread();

        log_m* saved_log = log;
        log = 0;                // turn off logging

        W_COERCE( dev->dismount_all() );

        log = saved_log;            // turn on logging
    }
    nprepared = xct_t::cleanup(true /* now dispose of prepared xcts */);
    w_assert1(nprepared == 0);
    w_assert1(xct_t::num_active_xcts() == 0);

    lm->assert_empty(); // no locks should be left

    /*
     *  Level 4
     */
    delete lid; lid=0;

    /*
     *  Level 3
     */
    delete chkpt; chkpt = 0; // NOTE : not level 3 now, but
    // has been retired

    /*
     *  Level 2
     */
    bt->destruct_once();
    delete bt; bt = 0; // btree manager

    /*
     *  Level 1
     */


    // delete the lock manager
    delete lm; lm = 0; 

    if(log) {
        log->shutdown(); // log joins any subsidiary threads
        // We do not delete the log now; shutdown takes care of that. delete log;
    }
    log = 0;

    delete io; io = 0; // io manager
    delete dev; dev = 0; // device manager
    delete bf; bf = 0; // destroy buffer manager last because io/dev are flushing them!
    /*
     *  Level 0
     */
    if (errlog) {
        delete errlog; errlog = 0;
    }

    /*
     *  free buffer pool memory
     */
     w_rc_t        e;
     char        *unused;
     e = smthread_t::set_bufsize(0, unused);
     if (e.is_error())  {
        cerr << "ss_m: Warning: set_bufsize(0):" << endl << e << endl;
     }
}

void ss_m::set_shutdown_flag(bool clean)
{
    shutdown_clean = clean;
}

/*--------------------------------------------------------------*
 *  ss_m::begin_xct()                                *
 *
 *\details
 *
 * You cannot start a transaction while any thread is :
 * - mounting or unmounting a device, or
 * - creating or destroying a volume.
 *--------------------------------------------------------------*/
rc_t 
ss_m::begin_xct(
        sm_stats_info_t*             _stats, // allocated by caller
        timeout_in_ms timeout)
{
    SM_PROLOGUE_RC(ss_m::begin_xct, not_in_xct, read_only,  0);
    tid_t tid;
    W_DO(_begin_xct(_stats, tid, timeout));
    return RCOK;
}
rc_t 
ss_m::begin_xct(timeout_in_ms timeout)
{
    SM_PROLOGUE_RC(ss_m::begin_xct, not_in_xct, read_only,  0);
    tid_t tid;
    W_DO(_begin_xct(0, tid, timeout));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::begin_xct() - for Markos' tests                       *
 *--------------------------------------------------------------*/
rc_t
ss_m::begin_xct(tid_t& tid, timeout_in_ms timeout)
{
    SM_PROLOGUE_RC(ss_m::begin_xct, not_in_xct,  read_only, 0);
    W_DO(_begin_xct(0, tid, timeout));
    return RCOK;
}

rc_t ss_m::begin_sys_xct(bool single_log_sys_xct, bool deferred_ssx,
    sm_stats_info_t *stats, timeout_in_ms timeout)
{
    tid_t tid;
    W_DO (_begin_xct(stats, tid, timeout, true, single_log_sys_xct, deferred_ssx));
    return RCOK;
}


/*--------------------------------------------------------------*
 *  ss_m::commit_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::commit_xct(sm_stats_info_t*& _stats, bool lazy,
                 lsn_t* plastlsn)
{
    SM_PROLOGUE_RC(ss_m::commit_xct, commitable_xct, read_write, 0);

    W_DO(_commit_xct(_stats, lazy, plastlsn));
    prologue.no_longer_in_xct();

    return RCOK;
}

rc_t
ss_m::commit_sys_xct()
{
    sm_stats_info_t *_stats = NULL; 
    W_DO(_commit_xct(_stats, true, NULL)); // always lazy commit
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::commit_xct_group()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::commit_xct_group(xct_t *list[], int listlen)
{
    W_DO(_commit_xct_group(list, listlen));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::commit_xct()                                          *
 *--------------------------------------------------------------*/
rc_t
ss_m::commit_xct(bool lazy, lsn_t* plastlsn)
{
    SM_PROLOGUE_RC(ss_m::commit_xct, commitable_xct, read_write, 0);

    sm_stats_info_t*             _stats=0; 
    W_DO(_commit_xct(_stats,lazy,plastlsn));
    prologue.no_longer_in_xct();
    /*
     * throw away the _stats, since user isn't harvesting... 
     */
    delete _stats;

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::prepare_xct(vote_t &v)                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::prepare_xct(vote_t &v)
{
    sm_stats_info_t*             _stats=0;
    rc_t e = prepare_xct(_stats, v);
    /*
     * throw away the _stats, since user isn't harvesting... 
     */
    delete _stats;
    return e;
}
rc_t
ss_m::prepare_xct(sm_stats_info_t*&    _stats, vote_t &v)
{
    v = vote_bad;

    // NB:special-case checks here !! we use "abortable_xct"
    // because we want to allow this to be called mpl times
    //
    SM_PROLOGUE_RC(ss_m::prepare_xct, abortable_xct, read_write, 0);
    xct_t& x = *xct();
    if( x.is_extern2pc() && x.state()==xct_t::xct_prepared) {
        // already done
        v = (vote_t)x.vote();
        return RCOK;
    }

    // Special case:: ss_m::prepare_xct() is ONLY
    // for external 2pc transactions. That is enforced
    // in ss_m::_prepare_xct(...)

    w_rc_t rc = _prepare_xct(_stats, v);

    // TODO: not quite sure how to handle all the
    // error cases...
    if(rc.is_error() && !xct()) {
        // No xct() -- must do this
        prologue.no_longer_in_xct();
    } else switch(v) {
        // vote is one of :
        // commit -- ok
        // read-only (no commit necessary)
        // abort (already aborted)
        // bad (have no business calling prepare())
    case vote_abort:
    case vote_readonly:
        w_assert3(!xct());
        prologue.no_longer_in_xct();
        break;
    case vote_bad:
        break;
    case vote_commit:
        w_assert3(xct());
        break;
    }

    return rc;
}

/*--------------------------------------------------------------*
 *  ss_m::abort_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::abort_xct(sm_stats_info_t*&             _stats)
{
    SM_PROLOGUE_RC(ss_m::abort_xct, abortable_xct, read_write, 0);

    // Temp removed for debugging purposes only
    // want to see what happens if the abort proceeds (scripts/alloc.10)
    bool was_sys_xct = xct() && xct()->is_sys_xct();
    W_DO(_abort_xct(_stats));
    if (!was_sys_xct) { // system transaction might be nested
        prologue.no_longer_in_xct();
    }

    return RCOK;
}
rc_t
ss_m::abort_xct()
{
    SM_PROLOGUE_RC(ss_m::abort_xct, abortable_xct, read_write, 0);
    sm_stats_info_t*             _stats=0;

    W_DO(_abort_xct(_stats));
    /*
     * throw away _stats, since user is not harvesting them
     */
    delete _stats;
    prologue.no_longer_in_xct();

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::set_coordinator(...)                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::set_coordinator(const server_handle_t &h)
{
    SM_PROLOGUE_RC(ss_m::set_coordinator, in_xct, read_write, 0);
    return _set_coordinator(h);
}

/*--------------------------------------------------------------*
 *  ss_m::force_vote_readonly(...)                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::force_vote_readonly()
{
    SM_PROLOGUE_RC(ss_m::force_vote_readonly, in_xct, read_only, 0);

    W_DO(_force_vote_readonly());
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::enter_2pc(...)                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::enter_2pc(const gtid_t &gtid)
{
    SM_PROLOGUE_RC(ss_m::enter_2pc, in_xct, read_write, 0);

    W_DO(_enter_2pc(gtid));
    SSMTEST("enter.2pc.1");

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::recover_2pc(...)                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::recover_2pc(const gtid_t &gtid,
        bool                mayblock,
        tid_t                &tid // out
)
{
    SM_PROLOGUE_RC(ss_m::recover_2pc, not_in_xct, read_only,0);

    SSMTEST("recover.2pc.1");
    W_DO(_recover_2pc(gtid, mayblock, tid));
    SSMTEST("recover.2pc.2");

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::save_work()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::save_work(sm_save_point_t& sp)
{
    // For now, consider this a read/write operation since you
    // wouldn't be doing this unless you intended to write and
    // possibly roll back.
    SM_PROLOGUE_RC(ss_m::save_work, in_xct, read_write, 0);
    W_DO( _save_work(sp) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::rollback_work()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::rollback_work(const sm_save_point_t& sp)
{
    SM_PROLOGUE_RC(ss_m::rollback_work, in_xct, read_write, 0);
    W_DO( _rollback_work(sp) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::num_active_xcts()                            *
 *--------------------------------------------------------------*/
uint32_t 
ss_m::num_active_xcts()
{
    return xct_t::num_active_xcts();
}
/*--------------------------------------------------------------*
 *  ss_m::tid_to_xct()                                *
 *--------------------------------------------------------------*/
xct_t* ss_m::tid_to_xct(const tid_t& tid)
{
    return xct_t::look_up(tid);
}

/*--------------------------------------------------------------*
 *  ss_m::xct_to_tid()                                *
 *--------------------------------------------------------------*/
tid_t ss_m::xct_to_tid(const xct_t* x)
{
    w_assert0(x != NULL);
    return x->tid();
}

/*--------------------------------------------------------------*
 *  ss_m::dump_xcts()                                           *
 *--------------------------------------------------------------*/
rc_t ss_m::dump_xcts(ostream& o)
{
    xct_t::dump(o);
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::state_xct()                                *
 *--------------------------------------------------------------*/
ss_m::xct_state_t ss_m::state_xct(const xct_t* x)
{
    w_assert3(x != NULL);
    return x->state();
}

smlevel_0::fileoff_t ss_m::xct_log_space_needed()
{
    w_assert3(xct() != NULL);
    return xct()->get_log_space_used();
}

rc_t ss_m::xct_reserve_log_space(fileoff_t amt) {
    w_assert3(xct() != NULL);
    return xct()->wait_for_log_space(amt);
}

/*--------------------------------------------------------------*
 *  ss_m::chain_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::chain_xct( sm_stats_info_t*&  _stats, bool lazy)
{
    SM_PROLOGUE_RC(ss_m::chain_xct, commitable_xct, read_write, 0);
    W_DO( _chain_xct(_stats, lazy) );
    return RCOK;
}
rc_t
ss_m::chain_xct(bool lazy)
{
    SM_PROLOGUE_RC(ss_m::chain_xct, commitable_xct, read_write, 0);
    sm_stats_info_t        *_stats = 0;
    W_DO( _chain_xct(_stats, lazy) );
    /*
     * throw away the _stats, since user isn't harvesting... 
     */
    delete _stats;
    return RCOK;
}


/*--------------------------------------------------------------*
 *  ss_m::checkpoint()                                        
 *  For debugging, smsh
 *--------------------------------------------------------------*/
rc_t
ss_m::checkpoint()
{
    // Just kick the chkpt thread
    chkpt->wakeup_and_take();
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::force_buffers()
 *  For debugging, smsh
 *--------------------------------------------------------------*/
rc_t
ss_m::force_buffers(bool flush)
{
    W_DO( bf->force_all(flush) );
    W_DO (io_m::flush_all_fixed_buffer());
    return RCOK;
}

rc_t
ss_m::force_vol_hdr_buffers(const vid_t& vid)
{
    if (vid == vid_t::null) return RC(eBADVOL);
    
    // volume header is store 0
    stid_t stid(vid, 0);
    W_DO( bf->force_store(stid, true/*invalidate*/) );
    W_DO (io_m::flush_vol_fixed_buffer(vid));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::force_store_buffers(const stid_t& stid)                            *
 *  For debugging, smsh
 *--------------------------------------------------------------*/
rc_t
ss_m::force_store_buffers(const stid_t& stid, bool invalidate)
{
    W_DO( bf->force_store(stid, invalidate) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::dump_buffers()                            *
 *  For debugging, smsh
 *--------------------------------------------------------------*/
rc_t
ss_m::dump_buffers(ostream &o)
{
    bf->dump(o);
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::snapshot_buffers()                            *
 *  For debugging, smsh
 *--------------------------------------------------------------*/
rc_t
ss_m::snapshot_buffers(u_int& ndirty, 
                       u_int& nclean, 
                       u_int& nfree,
                       u_int& nfixed)
{
    bf_m::snapshot(ndirty, nclean, nfree, nfixed);
    return RCOK;
}


/*--------------------------------------------------------------*
 *  ss_m::config_info()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::config_info(sm_config_info_t& info)
{
    info.page_size = ss_m::page_sz;

    //however, page_p.space.acquire aligns() the whole mess (hdr + record)
    //which rounds up the space needed, so.... we have to figure that in
    //here: round up then subtract one aligned entity.
    // 
    // OK, now that _data is already aligned, we don't have to
    // lose those 4 bytes.
    info.lg_rec_page_space = page_s::data_sz;
    info.buffer_pool_size = bf_m::npages() * ss_m::page_sz / 1024;
    info.max_btree_entry_size  = btree_m::max_entry_size();
    info.exts_on_page  = 0;
    info.pages_per_ext = smlevel_0::ext_sz;

    info.logging  = (ss_m::log != 0);

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::set_disk_delay()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::set_disk_delay(u_int milli_sec)
{
    W_DO(io_m::set_disk_delay(milli_sec));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::start_log_corruption()                        *
 *--------------------------------------------------------------*/
rc_t
ss_m::start_log_corruption()
{
    SM_PROLOGUE_RC(ss_m::start_log_corruption, in_xct, read_write, 0);
    if(log) {
        // flush current log buffer since all future logs will be
        // corrupted.
        errlog->clog << emerg_prio << "Starting Log Corruption" << flushl;
        log->start_log_corruption();
    }
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::sync_log()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::sync_log(bool block)
{
    return log? log->flush_all(block) : RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::flush_until()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::flush_until(lsn_t& anlsn, bool block)
{
  return log->flush(anlsn, block);
}

/*--------------------------------------------------------------*
 *  ss_m::get_curr_lsn()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::get_curr_lsn(lsn_t& anlsn)
{
  anlsn = log->curr_lsn();
  return (RCOK);
}

/*--------------------------------------------------------------*
 *  ss_m::get_durable_lsn()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::get_durable_lsn(lsn_t& anlsn)
{
  anlsn = log->durable_lsn();
  return (RCOK);
}

/*--------------------------------------------------------------*
 *  DEVICE and VOLUME MANAGEMENT                        *
 *--------------------------------------------------------------*/

/*--------------------------------------------------------------*
 *  ss_m::format_dev()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::format_dev(const char* device, smksize_t size_in_KB, bool force)
{
     // SM_PROLOGUE_RC(ss_m::format_dev, not_in_xct, 0);
    FUNC(ss_m::format_dev);                             

    if(size_in_KB > sthread_t::max_os_file_size / 1024) {
        return RC(eDEVTOOLARGE);
    }
    {
        prologue_rc_t prologue(prologue_rc_t::not_in_xct,  
                                prologue_rc_t::read_write,0); 
        if (prologue.error_occurred()) return prologue.rc();

        bool result = dev->is_mounted(device);
        if(result) {
            return RC(eALREADYMOUNTED);
        }
        DBG( << "already mounted=" << result );

        W_DO(vol_t::format_dev(device, 
                /* XXX possible loss of bits */
                shpid_t(size_in_KB/(page_sz/1024)), force));
    }
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::mount_dev()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::mount_dev(const char* device, u_int& vol_cnt, devid_t& devid, vid_t local_vid)
{
    SM_PROLOGUE_RC(ss_m::mount_dev, not_in_xct, read_only, 0);

    spinlock_write_critical_section cs(&_begin_xct_mutex);

    // do the real work of the mount
    W_DO(_mount_dev(device, vol_cnt, local_vid));

    // this is a hack to get the device number.  _mount_dev()
    // should probably return it.
    devid = devid_t(device);
    w_assert3(devid != devid_t::null);
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::dismount_dev()                            *
 *                                                              *
 *  only allow this if there are no active XCTs                 *
 *--------------------------------------------------------------*/
rc_t
ss_m::dismount_dev(const char* device)
{
    SM_PROLOGUE_RC(ss_m::dismount_dev, not_in_xct, read_only, 0);

    spinlock_write_critical_section cs(&_begin_xct_mutex);

    if (xct_t::num_active_xcts())  {
        fprintf(stderr, "Active transactions: %d : cannot dismount %s\n", 
                xct_t::num_active_xcts(), device);
        return RC(eCANTWHILEACTIVEXCTS);
    }  else  {
        W_DO( _dismount_dev(device) );
    }

    // take a checkpoint to record the dismount
    chkpt->take();

    DBG(<<"dismount_dev ok");

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::dismount_all()                            *
 *                                                              *
 *  Only allow this if there are no active XCTs                 *
 *--------------------------------------------------------------*/
rc_t
ss_m::dismount_all()
{
    SM_PROLOGUE_RC(ss_m::dismount_all, not_in_xct, read_only, 0);
    
    spinlock_write_critical_section cs(&_begin_xct_mutex);

    // of course a transaction could start immediately after this...
    // we don't protect against that.
    if (xct_t::num_active_xcts())  {
        fprintf(stderr, 
        "Active transactions: %d : cannot dismount_all\n", 
        xct_t::num_active_xcts());
        return RC(eCANTWHILEACTIVEXCTS);
    }

    // take a checkpoint to record the dismounts
    chkpt->take();

    // dismount is protected by _begin_xct_mutex, actually....
    W_DO( io->dismount_all_dev() );

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::list_devices()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::list_devices(const char**& dev_list, devid_t*& devid_list, u_int& dev_cnt)
{
    SM_PROLOGUE_RC(ss_m::list_devices, not_in_xct,  read_only,0);
    W_DO(io->list_devices(dev_list, devid_list, dev_cnt));
    return RCOK;
}

rc_t
ss_m::list_volumes(const char* device, 
        lvid_t*& lvid_list, 
        u_int& lvid_cnt)
{
    SM_PROLOGUE_RC(ss_m::list_volumes, can_be_in_xct, read_only, 0);
    lvid_cnt = 0;
    lvid_list = NULL;

    // for now there is only on lvid possible, but later there will
    // be multiple volumes on a device
    lvid_t lvid;
    W_DO(io->get_lvid(device, lvid));
    if (lvid != lvid_t::null) {
        lvid_list = new lvid_t[1];
        lvid_list[0] = lvid;
        if (lvid_list == NULL) return RC(eOUTOFMEMORY);
        lvid_cnt = 1;
    }
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::get_device_quota()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::get_device_quota(const char* device, smksize_t& quota_KB, smksize_t& quota_used_KB)
{
    SM_PROLOGUE_RC(ss_m::get_device_quota, can_be_in_xct, read_only, 0);
    W_DO(io->get_device_quota(device, quota_KB, quota_used_KB));
    return RCOK;
}

rc_t
ss_m::generate_new_lvid(lvid_t& lvid)
{
    SM_PROLOGUE_RC(ss_m::generate_new_lvid, can_be_in_xct, read_only, 0);
    W_DO(lid->generate_new_volid(lvid));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::create_vol()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::create_vol(const char* dev_name, const lvid_t& lvid, 
                 smksize_t quota_KB, bool skip_raw_init, vid_t local_vid,
                 const bool apply_fake_io_latency, const int fake_disk_latency)
{
    SM_PROLOGUE_RC(ss_m::create_vol, not_in_xct, read_only, 0);

    spinlock_write_critical_section cs(&_begin_xct_mutex);

    // make sure device is already mounted
    if (!io->is_mounted(dev_name)) return RC(eDEVNOTMOUNTED);

    // make sure volume is not already mounted
    vid_t vid = io->get_vid(lvid);
    if (vid != vid_t::null) return RC(eVOLEXISTS);

    W_DO(_create_vol(dev_name, lvid, quota_KB, skip_raw_init, 
                     apply_fake_io_latency, fake_disk_latency));

    // remount the device so the volume becomes visible
    u_int vol_cnt;
    W_DO(_mount_dev(dev_name, vol_cnt, local_vid));
    w_assert3(vol_cnt > 0);
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::destroy_vol()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::destroy_vol(const lvid_t& lvid)
{
    SM_PROLOGUE_RC(ss_m::destroy_vol, not_in_xct, read_only, 0);

    spinlock_write_critical_section cs(&_begin_xct_mutex);

    if (xct_t::num_active_xcts())  {
        fprintf(stderr, 
            "Active transactions: %d : cannot destroy volume\n", 
            xct_t::num_active_xcts());
        return RC(eCANTWHILEACTIVEXCTS);
    }  else  {
        // find the device name
        vid_t vid = io->get_vid(lvid);

        if (vid == vid_t::null)
            return RC(eBADVOL);
        char *dev_name = new char[smlevel_0::max_devname+1];
        if (!dev_name)
            W_FATAL(fcOUTOFMEMORY);

        w_auto_delete_array_t<char> ad_dev_name(dev_name);
        const char* dev_name_ptr = io->dev_name(vid);
        w_assert1(dev_name_ptr != NULL);
        strncpy(dev_name, dev_name_ptr, smlevel_0::max_devname);
        w_assert3(io->is_mounted(dev_name));

        // remember quota on the device
        smksize_t quota_KB;
        W_DO(dev->quota(dev_name, quota_KB));
        
        // since only one volume on the device, we can destroy the
        // volume by reformatting the device
        // W_DO(_dismount_dev(dev_name));
        // GROT

        /* XXX possible loss of bits */
        W_DO(vol_t::format_dev(dev_name, shpid_t(quota_KB/(page_sz/1024)), true));
        // take a checkpoint to record the destroy (dismount)
        chkpt->take();

        // tell the system about the device again
        u_int vol_cnt;
        W_DO(_mount_dev(dev_name, vol_cnt, vid_t::null));
        w_assert3(vol_cnt == 0);
    }
    return RCOK;
}



/*--------------------------------------------------------------*
 *  ss_m::get_volume_quota()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::get_volume_quota(const lvid_t& lvid, smksize_t& quota_KB, smksize_t& quota_used_KB)
{
    SM_PROLOGUE_RC(ss_m::get_volume_quota, can_be_in_xct, read_only, 0);
    vid_t vid = io->get_vid(lvid);
    W_DO(io->get_volume_quota(vid, quota_KB, quota_used_KB));
    return RCOK;
}

rc_t ss_m::verify_volume(
    vid_t vid, int hash_bits, verify_volume_result &result)
{
    W_DO(btree_m::verify_volume(vid, hash_bits, result));
    return RCOK;
}

ostream& operator<<(ostream& o, const lpid_t& pid)
{
    return o << "p(" << pid.vol() << '.' << pid.store() << '.' << pid.page << ')';
}

istream& operator>>(istream& i, lpid_t& pid)
{
    char c[6];
    memset(c, 0, sizeof(c));
    i >> c[0] >> c[1] >> pid._stid.vol >> c[2] 
      >> pid._stid.store >> c[3] >> pid.page >> c[4];
    c[5] = '\0';
    if (i)  {
        if (strcmp(c, "p(..)")) {
            i.clear(ios::badbit|i.rdstate());  // error
        }
    }
    return i;
}

#if defined(__GNUC__) && __GNUC_MINOR__ > 6
ostream& operator<<(ostream& o, const smlevel_1::xct_state_t& xct_state)
{
// NOTE: these had better be kept up-to-date wrt the enumeration
// found in sm_int_1.h
    const char* names[] = {"xct_stale", 
                        "xct_active", 
                        "xct_prepared", 
                        "xct_aborting",
                        "xct_chaining", 
                        "xct_committing", 
                        "xct_freeing_space", 
                        "xct_ended"};
    
    o << names[xct_state];
    return o;
}
#endif


/*--------------------------------------------------------------*
 *  ss_m::dump_locks()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::dump_locks(ostream &o)
{
    lm->dump(o);
    return RCOK;
}

rc_t
ss_m::dump_locks() {
  return dump_locks(std::cout);
}



#ifdef SLI_HOOKS
/*--------------------------------------------------------------*
 *  Enable/Disable Shore-SM features                            *
 *--------------------------------------------------------------*/

void ss_m::set_sli_enabled(bool /* enable */) 
{
    fprintf(stdout, "SLI not supported\n");
    //lm->set_sli_enabled(enable);
}

void ss_m::set_elr_enabled(bool /* enable */) 
{
    fprintf(stdout, "ELR not supported\n");
    //xct_t::set_elr_enabled(enable);
}

rc_t ss_m::set_log_features(char const* /* features */) 
{
    fprintf(stdout, "Aether not integrated\n");
    return (RCOK);
    //return log->set_log_features(features);
}

char const* ss_m::get_log_features() 
{
    fprintf(stdout, "Aether not integrated\n");
    return ("NOT-IMPL");
    //return log->get_log_features();
}
#endif

lil_global_table* ss_m::get_lil_global_table() {
    if (lm) {
        return lm->get_lil_global_table();
    } else {
        return NULL;
    }
}

/*--------------------------------------------------------------*
 *  ss_m::lock()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::lock(const lockid_t& n, lock_mode_t m,
           bool check_only, timeout_in_ms timeout)
{
    SM_PROLOGUE_RC(ss_m::lock, in_xct, read_only, 0);
    W_DO( lm->lock(n, m, check_only, timeout) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::unlock()                                *
 *--------------------------------------------------------------*/
/*rc_t
ss_m::unlock(const lockid_t& n)
{
    SM_PROLOGUE_RC(ss_m::unlock, in_xct, read_only, 0);
    W_DO( lm->unlock(n) );
    return RCOK;
}
*/

/*
rc_t
ss_m::query_lock(const lockid_t& n, lock_mode_t& m)
{
    SM_PROLOGUE_RC(ss_m::query_lock, in_xct, read_only, 0);
    W_DO( lm->query(n, m, xct()->tid()) );

    return RCOK;
}
*/

/*****************************************************************
 * Internal/physical-ID version of all the storage operations
 *****************************************************************/

/*--------------------------------------------------------------*
 *  ss_m::_begin_xct(sm_stats_info_t *_stats, timeout_in_ms timeout) *
 *
 * @param[in] _stats  If called by begin_xct without a _stats, then _stats is NULL here.
 *                    If not null, the transaction is instrumented.
 *                    The stats structure may be returned to the 
 *                    client through the appropriate version of 
 *                    commit_xct, abort_xct, prepare_xct, or chain_xct.
 *--------------------------------------------------------------*/
rc_t
ss_m::_begin_xct(sm_stats_info_t *_stats, tid_t& tid, timeout_in_ms timeout, bool sys_xct,
    bool single_log_sys_xct, bool deferred_ssx)
{
    w_assert1(!single_log_sys_xct || sys_xct); // SSX is always system-transaction
    w_assert1(!deferred_ssx || single_log_sys_xct); // deferred SSX is always SSX

    // system transaction can be a nested transaction, so
    // xct() could be non-NULL
    if (!sys_xct && xct() != NULL) {
        return RC (eINTRANS);
    }

    xct_t* x;
    if (sys_xct) {
        x = xct();
        if (single_log_sys_xct && x) {
            // in this case, we don't need an independent transaction object.
            // we just piggy back on the outer transaction
            w_assert0(x->is_piggy_backed_single_log_sys_xct() == false); // ssx can't be nested by ssx
            x->set_piggy_backed_single_log_sys_xct(true);
            tid = x->tid();
            return RCOK;
        }
        // system transaction doesn't need synchronization with create_vol etc
        // TODO might need to reconsider. but really needs this change now
        x = xct_t::new_xct(_stats, timeout, sys_xct, single_log_sys_xct, deferred_ssx);
    } else {
        spinlock_read_critical_section cs(&_begin_xct_mutex);
        x = xct_t::new_xct(_stats, timeout, sys_xct);
    }

    if (!x) 
        return RC(eOUTOFMEMORY);

    w_assert3(xct() == x);
    w_assert3(x->state() == xct_t::xct_active);
    tid = x->tid();

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_prepare_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_prepare_xct(sm_stats_info_t*& _stats, vote_t &v)
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();

    DBG(<<"prepare " );

    if( !x.is_extern2pc() ) {
        return rc_t(__FILE__, __LINE__, smlevel_0::eNOTEXTERN2PC);
    }

    W_DO( x.prepare() );
    if(x.is_instrumented()) { 
        _stats = x.steal_stats();
        _stats->compute();
    }

    v = (vote_t)x.vote();
    if(v == vote_readonly) {
        SSMTEST("prepare.readonly.1");
        W_DO( x.commit() );
        SSMTEST("prepare.readonly.2");
        xct_t::destroy_xct(&x);
        w_assert3(xct() == 0);
    } else if(v == vote_abort) {
        SSMTEST("prepare.abort.1");
        W_DO( x.abort() );
        SSMTEST("prepare.abort.2");
        xct_t::destroy_xct(&x);
        w_assert3(xct() == 0);
    } else if(v == vote_bad) {
        W_DO( x.abort() );
        xct_t::destroy_xct(&x);
        w_assert3(xct() == 0);
    }
    return RCOK;
}
/*--------------------------------------------------------------*
 *  ss_m::_commit_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_commit_xct(sm_stats_info_t*& _stats, bool lazy,
                  lsn_t* plastlsn)
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();
    DBG(<<"commit " << ((char *)lazy?" LAZY":"") << x );
    
    if (x.is_piggy_backed_single_log_sys_xct()) {
        // then, commit() does nothing
        x.set_piggy_backed_single_log_sys_xct(false); // but resets the flag
        return RCOK;
    }


    if(x.is_extern2pc()) {
        w_assert3(x.state()==xct_prepared);
        SSMTEST("extern2pc.commit.1");
    } else {
        w_assert3(x.state()==xct_active);
    }

    W_DO( x.commit(lazy,plastlsn) );

    if(x.is_instrumented()) {
        _stats = x.steal_stats();
        _stats->compute();
    }
#if W_DEBUG_LEVEL >= 3
    bool was_sys_xct = x.is_sys_xct();
#endif //W_DEBUG_LEVEL
    xct_t::destroy_xct(&x);
    w_assert3(was_sys_xct || xct() == 0);

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_commit_xct_group( xct_t *list[], len)                *
 *--------------------------------------------------------------*/

rc_t
ss_m::_commit_xct_group(xct_t *list[], int listlen)
{
    // We don't care what, if any, xct is attached
    xct_t* x = xct();
    if(x) me()->detach_xct(x);

    DBG(<<"commit group " );

    // 1) verify either all are participating in 2pc
    // in same way (not, prepared, not prepared)
    // Some may be read-only
    // 2) do the first part of the commit for each one.
    // 3) write the group-commit log record.
    // (TODO: we should remove the read-only xcts from this list)
    //
    int participating=0;
    for(int i=0; i < listlen; i++) {
        // verify list
        x = list[i];
        if(x->is_extern2pc()) {
            participating++;
            w_assert3(x->state()==xct_prepared);
        } else {
            w_assert3(x->state()==xct_active);
        }
    }
    if(participating > 0 && participating < listlen) {
        // some transaction is not participating in external 2-phase commit 
        // but others are. Don't delete any xcts.
        // Leave it up to the server to decide how to deal with this; it's
        // a server error.
        return RC(eNOTEXTERN2PC);
    }

    for(int i=0; i < listlen; i++) {
        x = list[i];
        /*
         * Do a partial commit -- all but logging the
         * commit and freeing the locks.
         */
        me()->attach_xct(x);
        {
        SM_PROLOGUE_RC(ss_m::mount_dev, commitable_xct, read_write, 0);
        W_DO( x->commit_as_group_member() );
        }
        w_assert1(me()->xct() == NULL);

        if(x->is_instrumented()) {
            // remove the stats, delete them
            sm_stats_info_t* _stats = x->steal_stats();
            delete _stats;
        }
    }

    // Write group commit record
    // Failure here requires that the server abort them individually.
    // I don't know why the compiler won't convert from a
    // non-const to a const xct_t * list.
    W_DO(xct_t::group_commit((const xct_t **)list, listlen));

    // Destroy the xcts
    for(int i=0; i < listlen; i++) {
        /*
         *  Free all locks for each transaction
         */
        x = list[i];
        w_assert1(me()->xct() == NULL);
        me()->attach_xct(x);
        W_DO(x->commit_free_locks());
        me()->detach_xct(x);
        xct_t::destroy_xct(x);
    }
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_set_coordinator(...)                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_set_coordinator(const server_handle_t &h)
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();

    DBG(<<"set_coordinator " );
    x.set_coordinator(h); 
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_force_vote_readonly()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_force_vote_readonly()
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();

    DBG(<<"force readonly " );
    x.force_readonly();
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_enter_2pc()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_enter_2pc(const gtid_t &gtid)
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();

    DBG(<<"enter 2pc " );
    W_DO( x.enter2pc(gtid) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_recover_2pc()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_recover_2pc(const gtid_t &gtid,        
        bool                mayblock,
        tid_t                &t
)
{
    // Caller checked this:
    // SM_PROLOGUE_RC(ss_m::recover_2pc, not_in_xct, 0);
    //
    w_assert3(xct() == 0);
    DBG(<<"recover 2pc " );

    xct_t        *x;
    W_DO(xct_t::recover2pc(gtid,mayblock,x));
    if(x) {
        t = x->tid();
        me()->attach_xct(x);
    }
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::query_prepared_xct(int& numtids);
 *  ss_m::query_prepared_xct(int numtids, gtid_t l[]);
 *--------------------------------------------------------------*/
rc_t
ss_m::query_prepared_xct(int &numtids)
{
    SM_PROLOGUE_RC(ss_m::query_prepared_xct, not_in_xct, read_only, 0);
    return xct_t::query_prepared(numtids);
}

rc_t
ss_m::query_prepared_xct(int numtids, gtid_t list[])
{
    SM_PROLOGUE_RC(ss_m::query_prepared_xct, not_in_xct, read_only, 0);
    return xct_t::query_prepared(numtids, list);
}



/*--------------------------------------------------------------*
 *  ss_m::_chain_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_chain_xct(
        sm_stats_info_t*&  _stats, /* pass in a new one, get back the old */
        bool lazy)
{
    sm_stats_info_t*  new_stats = _stats;
    w_assert3(xct() != 0);
    xct_t* x = xct();

    W_DO( x->chain(lazy) );
    w_assert3(xct() == x);
    if(x->is_instrumented()) {
        _stats = x->steal_stats();
        _stats->compute();
    }
    x->give_stats(new_stats);

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_abort_xct()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_abort_xct(sm_stats_info_t*&             _stats)
{
    w_assert3(xct() != 0);
    xct_t& x = *xct();
    
    // if this is "piggy-backed" ssx, just end the status
    if (x.is_piggy_backed_single_log_sys_xct()) {
        x.set_piggy_backed_single_log_sys_xct(false);
        return RCOK;
    }
    
#if W_DEBUG_LEVEL>=3
    bool was_sys_xct = x.is_sys_xct();
#endif // W_DEBUG_LEVEL>=3

    W_DO( x.abort(true /* save _stats structure */) );
    if(x.is_instrumented()) {
        _stats = x.steal_stats();
        _stats->compute();
    }

    xct_t::destroy_xct(&x);
    w_assert3(was_sys_xct || xct() == 0);

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::save_work()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_save_work(sm_save_point_t& sp)
{
    w_assert3(xct() != 0);
    xct_t* x = xct();

    W_DO(x->save_point(sp));
    sp._tid = x->tid();
#if W_DEBUG_LEVEL > 4
    {
        w_ostrstream s;
        s << "save_point @ " << (void *)(&sp)
            << " " << sp
            << " created for tid " << x->tid();
        fprintf(stderr,  "%s\n", s.c_str());
    }
#endif
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::rollback_work()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::_rollback_work(const sm_save_point_t& sp)
{
    w_assert3(xct() != 0);
    xct_t* x = xct();
#if W_DEBUG_LEVEL > 4
    {
        w_ostrstream s;
        s << "rollback_work for " << (void *)(&sp)
            << " " << sp
            << " in tid " << x->tid();
        fprintf(stderr,  "%s\n", s.c_str());
    }
#endif
    if (sp._tid != x->tid())  {
        return RC(eBADSAVEPOINT);
    }
    W_DO( x->rollback(sp) );
    return RCOK;
}

rc_t
ss_m::_mount_dev(const char* device, u_int& vol_cnt, vid_t local_vid)
{
    vid_t vid;
    DBG(<<"_mount_dev " << device);

    // inform device_m about the device
    W_DO(io->mount_dev(device, vol_cnt));
    if (vol_cnt == 0) return RCOK;

    DBG(<<"_mount_dev vol count " << vol_cnt );
    // make sure volumes on the dev are not already mounted
    lvid_t lvid;
    W_DO(io->get_lvid(device, lvid));
    vid = io->get_vid(lvid);
    if (vid != vid_t::null) {
                // already mounted
                return RCOK;
    }

    if (local_vid == vid_t::null) {
        W_DO(io->get_new_vid(vid));
    } else {
        if (io->is_mounted(local_vid)) {
            // vid already in use
            return RC(eBADVOL);
        }
        vid = local_vid;
    }

    W_DO(io->mount(device, vid));
    // take a checkpoint to record the mount
    chkpt->take();

    return RCOK;
}

rc_t
ss_m::_dismount_dev(const char* device)
{
    vid_t        vid;
    lvid_t       lvid;
    rc_t         rc;

    DBG(<<"dismount_dev");
    W_DO(io->get_lvid(device, lvid));
    DBG(<<"dismount_dev" << lvid);
    if (lvid != lvid_t::null) {
        vid = io->get_vid(lvid);
        DBG(<<"dismount_dev" << vid);
        if (vid == vid_t::null) return RC(eDEVNOTMOUNTED);
    }

    W_DO( io->dismount_dev(device) );

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_create_vol()                                *
 *--------------------------------------------------------------*/
rc_t
ss_m::_create_vol(const char* dev_name, const lvid_t& lvid, 
                  smksize_t quota_KB, bool skip_raw_init,
                  const bool apply_fake_io_latency, 
                  const int fake_disk_latency)
{
    vid_t tmp_vid;
    W_DO(io->get_new_vid(tmp_vid));
    DBG(<<"got new vid " << tmp_vid 
        << " formatting " << dev_name);

    W_DO(vol_t::format_vol(dev_name, lvid, tmp_vid,
        /* XXX possible loss of bits */
       shpid_t(quota_KB/(page_sz/1024)), skip_raw_init));

    DBG(<<"vid " << tmp_vid  << " mounting " << dev_name);
    W_DO(io->mount(dev_name, tmp_vid, apply_fake_io_latency, fake_disk_latency));
    DBG(<<" mount done " << dev_name << " tmp_vid " << tmp_vid);
    DBG(<<" dismounting volume");
    W_DO(io->dismount(tmp_vid));
    
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::get_du_statistics()        DU DF
 *--------------------------------------------------------------*/
rc_t
ss_m::get_du_statistics(vid_t vid, sm_du_stats_t& du, bool audit)
{
    SM_PROLOGUE_RC(ss_m::get_du_statistics, in_xct, read_only, 0);
    W_DO(_get_du_statistics(vid, du, audit));
    return RCOK;
}


/*--------------------------------------------------------------*
 *  ss_m::get_du_statistics()        DU DF                    *    
 *--------------------------------------------------------------*/
rc_t
ss_m::get_du_statistics(const stid_t& stid, sm_du_stats_t& du, bool audit)
{
    SM_PROLOGUE_RC(ss_m::get_du_statistics, in_xct, read_only, 0);
    W_DO(_get_du_statistics(stid, du, audit));
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_get_du_statistics()        DU DF                    *    
 *--------------------------------------------------------------*/
rc_t
ss_m::_get_du_statistics( const stid_t& stpgid, sm_du_stats_t& du, bool audit)
{
    // TODO this should take S lock, not IS
    lpid_t root_pid;
    W_DO(open_store(stpgid, root_pid));

    btree_stats_t btree_stats;
    W_DO( bt->get_du_statistics(root_pid, btree_stats, audit));
    if (audit) {
        W_DO(btree_stats.audit());
    }
    du.btree.add(btree_stats);
    du.btree_cnt++;
    return RCOK;
}

/*
 * check_volume_page_types
 * For each store in the volume, check that each allocated page in the
 * store has a reasonable page tag.
 * Slow, involves linear search of each store.
 */
rc_t
ss_m::check_volume_page_types(vid_t vid)
{
    SM_PROLOGUE_RC(ss_m::check_volume_page_types, in_xct, read_only, 0);
    /*
     * Cannot call this during recovery, even for 
     * debugging purposes
     */
    if(smlevel_0::in_recovery()) {
        return RCOK;
    }

    if(!io) {
        W_FATAL_MSG(eINTERNAL, << "io manager not insantiated");
    }

    W_DO(lm->intent_vol_lock(vid, SH));

    // every other store
    lpid_t root_pid;
    for (stid_t stid(vid, 0); stid.store < stnode_p::max; stid.store++) {
        W_DO(open_store_nolock(stid, root_pid));
        if (root_pid.page) {
            W_DO( io->check_store_pages(stid,  page_p::t_btree_p));
        }
    }
    return RCOK;
}


/*--------------------------------------------------------------*
 *  ss_m::_get_du_statistics()  DU DF                           *
 *--------------------------------------------------------------*/
rc_t
ss_m::_get_du_statistics(vid_t vid, sm_du_stats_t& du, bool audit)
{
    /*
     * Cannot call this during recovery, even for 
     * debugging purposes
     */
    if(smlevel_0::in_recovery()) {
        return RCOK;
    }
    W_DO(lm->intent_vol_lock(vid, audit ? SH : IS));
    sm_du_stats_t new_stats;

    /*********************************************************
     * First get stats on all the special stores in the volume.
     *********************************************************/

    stid_t stid;

    /**************************************************
     * Now get stats on every other store on the volume
     **************************************************/

    rc_t rc;
    // get du stats on every store
    for (stid_t s(vid, 0); s.store < stnode_p::max; s.store++) {
        DBG(<<"look at store " << s);
        
        store_flag_t flags;
        rc = io->get_store_flags(s, flags);
        if (rc.is_error()) {
            if (rc.err_num() == eBADSTID) {
                DBG(<<"skipping bad STID " << s );
                continue;  // skip any stores that don't exist
            } else {
                return rc;
            }
        }
        DBG(<<" getting stats for store " << s << " flags=" << flags);
        rc = _get_du_statistics(s, new_stats, audit);
        if (rc.is_error()) {
            if (rc.err_num() == eBADSTID) {
                DBG(<<"skipping large object or missing store " << s );
                continue;  // skip any stores that don't show
                           // up in the directory index
                           // in this case it this means stores for
                           // large object pages
            } else {
                return rc;
            }
        }
        DBG(<<"end for loop with s=" << s );
    }

    W_DO( io->get_du_statistics(vid, new_stats.volume_hdr, audit));

    if (audit) {
        W_DO(new_stats.audit());
    }
    du.add(new_stats);

    return RCOK;
}



/*--------------------------------------------------------------*
 *  ss_m::{enable,disable,set}_fake_disk_latency()              *        
 *--------------------------------------------------------------*/
rc_t 
ss_m::enable_fake_disk_latency(vid_t vid)
{
  SM_PROLOGUE_RC(ss_m::enable_fake_disk_latency, not_in_xct, read_only, 0);
  W_DO( io->enable_fake_disk_latency(vid) );
  return RCOK;
}

rc_t 
ss_m::disable_fake_disk_latency(vid_t vid)
{
  SM_PROLOGUE_RC(ss_m::disable_fake_disk_latency, not_in_xct, read_only, 0);
  W_DO( io->disable_fake_disk_latency(vid) );
  return RCOK;
}

rc_t 
ss_m::set_fake_disk_latency(vid_t vid, const int adelay)
{
  SM_PROLOGUE_RC(ss_m::set_fake_disk_latency, not_in_xct, read_only, 0);
  W_DO( io->set_fake_disk_latency(vid,adelay) );
  return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::get_volume_meta_stats()                                *        
 *--------------------------------------------------------------*/
rc_t
ss_m::get_volume_meta_stats(vid_t vid, SmVolumeMetaStats& volume_stats, concurrency_t cc)
{
    SM_PROLOGUE_RC(ss_m::get_volume_meta_stats, in_xct, read_only, 0);
    W_DO( _get_volume_meta_stats(vid, volume_stats, cc) );
    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::_get_volume_meta_stats()                                *        
 *--------------------------------------------------------------*/
rc_t
ss_m::_get_volume_meta_stats(vid_t vid, SmVolumeMetaStats& volume_stats, concurrency_t cc)
{
    if (cc == t_cc_vol)  {
        W_DO(lm->intent_vol_lock(vid, SH));
    }  else if (cc != t_cc_none)  {
        return RC(eBADCCLEVEL);
    }

    W_DO( io->get_volume_meta_stats(vid, volume_stats) );

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::gather_xct_stats()                            *
 *  Add the stats from this thread into the per-xct stats structure
 *  and return a copy in the given struct _stats.
 *  If reset==true,  clear the per-xct copy.
 *  Doing this has the side-effect of clearing the per-thread copy.
 *--------------------------------------------------------------*/
rc_t
ss_m::gather_xct_stats(sm_stats_info_t& _stats, bool reset)
{
    // Use commitable_xct to ensure exactly 1 thread attached for
    // clean collection of all stats,
    // even those that read-only threads would increment.
    //
    SM_PROLOGUE_RC(ss_m::gather_xct_stats, commitable_xct, read_only, 0);

    w_assert3(xct() != 0);
    xct_t& x = *xct();

    if(x.is_instrumented()) {
        DBGTHRD(<<"instrumented, reset= " << reset );
        // detach_xct adds the per-thread stats to the xct's stats,
        // then clears the per-thread stats so that
        // the next time some stats from this thread are gathered like this
        // into an xct, they aren't duplicated.
        // They are added to the global_stats before they are cleared, so 
        // they don't get lost entirely.
        me()->detach_xct(&x); 
        me()->attach_xct(&x);

        // Copy out the stats structure stored for this xct.
        _stats = x.const_stats_ref(); 

        if(reset) {
            DBGTHRD(<<"clearing stats " );
            // clear
            // NOTE!!!!!!!!!!!!!!!!!  NOT THREAD-SAFE:
            x.clear_stats();
        }
#ifdef COMMENT
        /* help debugging sort stuff -see also code in bf.cpp  */
        {
            // print -grot
            extern int bffix_SH[];
            extern int bffix_EX[];
            static const char *names[] = {
                "t_bad_p",
                "t_alloc_p",
                "t_stnode_p",
                "t_btree_p",
                "t_any_p",
                "none"
                };

            cout << "PAGE FIXES " <<endl;
            for (int i=0; i<=14; i++) {
                    cout  << names[i] << "="  
                        << '\t' << bffix_SH[i] << "+" 
                    << '\t' << bffix_EX[i] << "=" 
                    << '\t' << bffix_EX[i] + bffix_SH[i]
                     << endl;

            }
            int sumSH=0, sumEX=0;
            for (int i=0; i<=14; i++) {
                    sumSH += bffix_SH[i];
                    sumEX += bffix_EX[i];
            }
            cout  << "TOTALS" << "="  
                        << '\t' << sumSH<< "+" 
                    << '\t' << sumEX << "=" 
                    << '\t' << sumSH+sumEX
                     << endl;
        }
        if(reset) {
            extern int bffix_SH[];
            extern int bffix_EX[];
            for (int i=0; i<=14; i++) {
                bffix_SH[i] = 0;
                bffix_EX[i] = 0;
            }
        }
#endif /* COMMENT */
    } else {
        DBGTHRD(<<"xct not instrumented");
    }

    return RCOK;
}

/*--------------------------------------------------------------*
 *  ss_m::gather_stats()                            *
 *  NOTE: the client is assumed to pass in a copy that's not
 *  referenced by any other threads right now.
 *  Resetting is not an option. Clients have to gather twice, then
 *  subtract.
 *  NOTE: you do not have to be in a transaction to call this.
 *--------------------------------------------------------------*/
rc_t
ss_m::gather_stats(sm_stats_info_t& _stats)
{
    class GatherSmthreadStats : public SmthreadFunc
    {
    public:
        GatherSmthreadStats(sm_stats_info_t &s) : _stats(s)
        {
            new (&_stats) sm_stats_info_t; // clear the stats
            // by invoking the constructor.
        };
        void operator()(const smthread_t& t)
        {
            t.add_from_TL_stats(_stats);
        }
        void compute() { _stats.compute(); }
    private:
        sm_stats_info_t &_stats;
    } F(_stats);

    //Gather all the threads' statistics into the copy given by
    //the client.
    smthread_t::for_each_smthread(F);
    // F.compute();

    // Now add in the global stats.
    // Global stats contain all the per-thread stats that were collected
    // before a per-thread stats structure was cleared. 
    // (This happens when per-xct stats get gathered for instrumented xcts.)
    add_from_global_stats(_stats); // from finished threads and cleared stats
	_stats.compute();
    return RCOK;
}

#if W_DEBUG_LEVEL > 0
extern void dump_all_sm_stats();
void dump_all_sm_stats()
{
    static sm_stats_info_t s;
    W_COERCE(ss_m::gather_stats(s));
    w_ostrstream o;
    o << s << endl; 
    fprintf(stderr, "%s\n", o.c_str());
}
#endif

ostream &
operator<<(ostream &o, const sm_stats_info_t &s) 
{
    o << s.bfht;
    o << s.sm;
    return o;
}


/*--------------------------------------------------------------*
 *  ss_m::get_store_info()                            *
 *--------------------------------------------------------------*/
rc_t
ss_m::get_store_info(
    const stid_t&           stpgid, 
    sm_store_info_t&        info)
{
    SM_PROLOGUE_RC(ss_m::get_store_info, in_xct, read_only, 0);
    W_DO(_get_store_info(stpgid, info));
    return RCOK;
}


ostream&
operator<<(ostream& o, smlevel_3::sm_store_property_t p)
{
    if (p == smlevel_3::t_regular)                o << "regular";
    if (p == smlevel_3::t_temporary)                o << "temporary";
    if (p == smlevel_3::t_load_file)                o << "load_file";
    if (p == smlevel_3::t_insert_file)                o << "insert_file";
    if (p == smlevel_3::t_bad_storeproperty)        o << "bad_storeproperty";
    if (p & !(smlevel_3::t_regular
                | smlevel_3::t_temporary
                | smlevel_3::t_load_file
                | smlevel_3::t_insert_file
                | smlevel_3::t_bad_storeproperty))  {
        o << "unknown_property";
        w_assert3(1);
    }
    return o;
}

ostream&
operator<<(ostream& o, smlevel_0::store_flag_t flag)
{
    if (flag == smlevel_0::st_bad)            o << "|bad";
    if (flag & smlevel_0::st_regular)            o << "|regular";
    if (flag & smlevel_0::st_tmp)            o << "|tmp";
    if (flag & smlevel_0::st_load_file)            o << "|load_file";
    if (flag & smlevel_0::st_insert_file)   o << "|insert_file";
    if (flag & smlevel_0::st_empty)            o << "|empty";
    if (flag & !(smlevel_0::st_bad
                | smlevel_0::st_regular
                | smlevel_0::st_tmp
                | smlevel_0::st_load_file 
                | smlevel_0::st_insert_file 
                | smlevel_0::st_empty))  {
        o << "|unknown";
        w_assert3(1);
    }

    return o << "|";
}

ostream& 
operator<<(ostream& o, const smlevel_0::store_operation_t op)
{
    const char *names[] = {"delete_store", 
                        "create_store", 
                        "set_deleting", 
                        "set_store_flags", 
                        "set_root"};

    if (op <= smlevel_0::t_set_root)  {
        return o << names[op];
    }  
    // else:
    w_assert3(1);
    return o << "unknown";
}

ostream& 
operator<<(ostream& o, const smlevel_0::store_deleting_t value)
{
    const char *names[] = { "not_deleting_store", 
                        "deleting_store", 
                        "store_freeing_exts", 
                        "unknown_deleting"};
    
    if (value <= smlevel_0::t_unknown_deleting)  {
        return o << names[value];
    }  
    // else:
    w_assert3(1);
    return o << "unknown_deleting_store_value";
}

rc_t         
ss_m::log_file_was_archived(const char * logfile)
{
    if(log) return log->file_was_archived(logfile);
    // should be a programming error to get here!
    return RCOK;
}


extern "C" {
/* Debugger-callable functions to dump various SM tables. */

    void        sm_dumplocks()
    {
        if (smlevel_0::lm) {
                W_IGNORE(ss_m::dump_locks(cout));
        }
        else
                cout << "no smlevel_0::lm" << endl;
        cout << flush;
    }

    void   sm_dumpxcts()
    {
        W_IGNORE(ss_m::dump_xcts(cout));
        cout << flush;
    }

    void        sm_dumpbuffers()
    {
        W_IGNORE(ss_m::dump_buffers(cout));
        cout << flush;
    }
}

/*
 * descend to io_m to check the disk containing the given volume
 */
w_rc_t ss_m::dump_vol_store_info(const vid_t &vid)
{
    SM_PROLOGUE_RC(ss_m::dump_vol_store_info, in_xct, read_only,  0);
    return io_m::check_disk(vid); 
}


w_rc_t 
ss_m::log_message(const char * const msg)
{
    SM_PROLOGUE_RC(ss_m::log_message, in_xct, read_write,  0);
    w_ostrstream out;
    out <<  msg << ends;
    return log_comment(out.c_str());
}
