/* -*- mode:C++; c-basic-offset:4 -*-
     Shore-kits -- Benchmark implementations for Shore-MT

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

/** @file:   tpcb_env.cpp
 *
 *  @brief:  Declaration of the Shore TPC-C environment (database)
 *
 *  @author: Ryan Johnson      (ryanjohn)
 *  @author: Ippokratis Pandis (ipandis)
 *  @date:   Feb 2009
 */

#include "tpcb_env.h"

#include "lock.h"

DEFINE_ROW_CACHE_TLS(tpcb, branch);
DEFINE_ROW_CACHE_TLS(tpcb, teller);
DEFINE_ROW_CACHE_TLS(tpcb, account);
DEFINE_ROW_CACHE_TLS(tpcb, history);

namespace tpcb {

/********************************************************************
 *
 * ShoreTPCBEnv functions
 *
 ********************************************************************/

ShoreTPCBEnv::ShoreTPCBEnv(boost::program_options::variables_map vm)
    : ShoreEnv(vm)
{
}

ShoreTPCBEnv::~ShoreTPCBEnv()
{
}



/********************************************************************
 *
 *  @fn:    load_schema()
 *
 *  @brief: Creates the table_desc_t and table_man_impl objects for
 *          each TPC-B table
 *
 ********************************************************************/

w_rc_t ShoreTPCBEnv::load_schema()
{
    // initiate the table managers
    branch_man   = new branch_man_impl(new branch_t());
    teller_man   = new teller_man_impl(new teller_t());
    account_man  = new account_man_impl(new account_t());
    history_man  = new history_man_impl(new history_t());

    return (RCOK);
}



/********************************************************************
 *
 *  @fn:    load_and_register_fids()
 *
 *  @brief: loads the store ids for each table and index at kits side
 *          as well as registering the tables
 *
 ********************************************************************/

w_rc_t ShoreTPCBEnv::load_and_register_fids()
{
    W_DO(branch_man->load_and_register_fid(db()));
    W_DO(teller_man->load_and_register_fid(db()));
    W_DO(account_man->load_and_register_fid(db()));
    W_DO(history_man->load_and_register_fid(db()));
    return (RCOK);
}


/********************************************************************
 *
 *  @fn:    set_skew()
 *
 *  @brief: sets load imbalance for TPC-B
 *
 ********************************************************************/
void ShoreTPCBEnv::set_skew(int area, int load, int start_imbalance, int skew_type, bool shifting)
{
    ShoreEnv::set_skew(area, load, start_imbalance, skew_type);
    // for branches
    b_skewer.set(area, 0, _scaling_factor-1, load, shifting);
    // for tellers
    t_skewer.set(area, 0, TPCB_TELLERS_PER_BRANCH-1, load, shifting);
    // for accounts
    a_skewer.set(area, 0, TPCB_ACCOUNTS_PER_BRANCH-1, load, shifting);
}


/********************************************************************
 *
 *  @fn:    start_load_imbalance()
 *
 *  @brief: sets the flag that triggers load imbalance for TPC-B
 *          resets the intervals if necessary (depending on the skew type)
 *
 ********************************************************************/
void ShoreTPCBEnv::start_load_imbalance()
{
    if(b_skewer.is_used()) {
	_change_load = false;
	// for branches
	b_skewer.reset(_skew_type);
	// for tellers
	t_skewer.reset(_skew_type);
	// for accounts
	a_skewer.reset(_skew_type);
    }
    if(_skew_type != SKEW_CHAOTIC || URand(1,100) > 30) {
	_change_load = true;
    }
    ShoreEnv::start_load_imbalance();
}


/********************************************************************
 *
 *  @fn:    reset_skew()
 *
 *  @brief: sets the flag that stops the load imbalance for TPC-B
 *          and cleans the intervals
 *
 ********************************************************************/
void ShoreTPCBEnv::reset_skew()
{
    ShoreEnv::reset_skew();
    _change_load = false;
    b_skewer.clear();
    t_skewer.clear();
    a_skewer.clear();
}


/********************************************************************
 *
 *  @fn:    info()
 *
 *  @brief: Prints information about the current db instance status
 *
 ********************************************************************/

int ShoreTPCBEnv::info() const
{
    TRACE( TRACE_ALWAYS, "SF      = (%.1f)\n", _scaling_factor);
    TRACE( TRACE_ALWAYS, "Workers = (%d)\n", _worker_cnt);
    return (0);
}



/********************************************************************
 *
 *  @fn:    statistics
 *
 *  @brief: Prints statistics
 *
 ********************************************************************/

int ShoreTPCBEnv::statistics()
{
    // read the current trx statistics
    CRITICAL_SECTION(cs, _statmap_mutex);
    ShoreTPCBTrxStats rval;
    rval -= rval; // dirty hack to set all zeros
    for (statmap_t::iterator it=_statmap.begin(); it != _statmap.end(); ++it)
	rval += *it->second;

    TRACE( TRACE_STATISTICS, "AcctUpd. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.acct_update,
           rval.failed.acct_update,
           rval.deadlocked.acct_update);

    TRACE( TRACE_STATISTICS, "MbenchInsertOnly. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_insert_only,
           rval.failed.mbench_insert_only,
           rval.deadlocked.mbench_insert_only);

    TRACE( TRACE_STATISTICS, "MbenchDeleteOnly. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_delete_only,
           rval.failed.mbench_delete_only,
           rval.deadlocked.mbench_delete_only);

    TRACE( TRACE_STATISTICS, "MbenchProbeOnly. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_probe_only,
           rval.failed.mbench_probe_only,
           rval.deadlocked.mbench_probe_only);

    TRACE( TRACE_STATISTICS, "MbenchInsertDelte. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_insert_delete,
           rval.failed.mbench_insert_delete,
           rval.deadlocked.mbench_insert_delete);

    TRACE( TRACE_STATISTICS, "MbenchInsertProbe. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_insert_probe,
           rval.failed.mbench_insert_probe,
           rval.deadlocked.mbench_insert_probe);

    TRACE( TRACE_STATISTICS, "MbenchDeleteProbe. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_delete_probe,
           rval.failed.mbench_delete_probe,
           rval.deadlocked.mbench_delete_probe);

    TRACE( TRACE_STATISTICS, "MbenchMix. Att (%d). Abt (%d). Dld (%d)\n",
           rval.attempted.mbench_mix,
           rval.failed.mbench_mix,
           rval.deadlocked.mbench_mix);

    ShoreEnv::statistics();

    return (0);
}


/********************************************************************
 *
 *  @fn:    start/stop
 *
 *  @brief: Simply call the corresponding functions of shore_env
 *
 ********************************************************************/

int ShoreTPCBEnv::start()
{
    return (ShoreEnv::start());
}

int ShoreTPCBEnv::stop()
{
    return (ShoreEnv::stop());
}

/******************************************************************
 *
 * @class: table_builder_t
 *
 * @brief: Parallel workers for loading the TPC-B tables
 *
 ******************************************************************/

class ShoreTPCBEnv::table_builder_t : public thread_t
{
    ShoreTPCBEnv* _env;
    int _sf;
    long _start;
    long _count;

public:
    table_builder_t(ShoreTPCBEnv* env, int id, int sf, long start, long count)
	: thread_t(std::string("LD-%d",id)),
          _env(env), _sf(sf), _start(start), _count(count)
    { }

    virtual void work();

}; // EOF: table_builder_t


const uint branchesPerRound = 5; // Update branch count every 5 rounds
static uint iBranchesLoaded = 0;

void ShoreTPCBEnv::table_builder_t::work()
{
    w_rc_t e;

    for(int i=0; i < _count; i += TPCB_ACCOUNTS_CREATED_PER_POP_XCT) {
	long a_id = _start + i;
	populate_db_input_t in(_sf, a_id);
    retry:
	W_COERCE(_env->db()->begin_xct());
	e = _env->xct_populate_db(a_id, in);
        CHECK_XCT_RETURN(e,retry,_env);

        if ((i % (branchesPerRound*TPCB_ACCOUNTS_PER_BRANCH)) == 0) {
            lintel::unsafe::atomic_fetch_add(&iBranchesLoaded, branchesPerRound);
            TRACE(TRACE_ALWAYS, "%d branches loaded so far...\n",
                  iBranchesLoaded);
        }
    }
    TRACE( TRACE_STATISTICS,
           "Finished loading account groups %ld .. %ld \n",
           _start, _start+_count);
}



/******************************************************************
 *
 * @struct: table_creator_t
 *
 * @brief:  Helper class for creating the TPC-B tables and
 *          loading an initial number of records in a
 *          single-threaded fashion
 *
 ******************************************************************/

struct ShoreTPCBEnv::table_creator_t : public thread_t
{
    ShoreTPCBEnv* _env;
    int _sf;
    long _psize;
    long _pcount;
    table_creator_t(ShoreTPCBEnv* env, int sf, long psize, long pcount)
	: thread_t("CR"), _env(env), _sf(sf), _psize(psize), _pcount(pcount) { }
    virtual void work();

}; // EOF: table_creator_t


void ShoreTPCBEnv::table_creator_t::work()
{
    // Create the tables, if any partitioning is to be applied, that has already
    // been set at update_partitioning()
    W_COERCE(_env->db()->begin_xct());
    W_COERCE(_env->branch_man->table()->create_physical_table(_env->db()));
    W_COERCE(_env->teller_man->table()->create_physical_table(_env->db()));
    W_COERCE(_env->account_man->table()->create_physical_table(_env->db()));
    W_COERCE(_env->history_man->table()->create_physical_table(_env->db()));
    W_COERCE(_env->db()->commit_xct());

    // Create 10k accounts in each partition to buffer
    // workers from each other
    for(long i=-1; i < _pcount; i++) {
	long a_id = i*_psize;
	populate_db_input_t in(_sf, a_id);
	TRACE( TRACE_STATISTICS, "Populating %ld a_ids starting with %ld\n",
               TPCB_ACCOUNTS_CREATED_PER_POP_XCT, a_id);
	W_COERCE(_env->db()->begin_xct());
	W_COERCE(_env->xct_populate_db(a_id, in));
    }

    // Before returning, run the post initialization phase
    W_COERCE(_env->db()->begin_xct());
    W_COERCE(_env->db()->commit_xct());
}


/********
 ******** Caution: The functions below should be invoked inside
 ********          the context of a smthread
 ********/


/******************************************************************
 *
 * @fn:    create_tables()
 *
 * @brief: Creates TPCB tables. May only be invoked from ShoreEnv::load(),
 * which aquires the necessary mutexes!
 *
 ******************************************************************/
w_rc_t ShoreTPCBEnv::create_tables()
{
    /* partly (no) thanks to Shore's next key index locking, and
       partly due to page latch and SMO issues, we have ridiculous
       deadlock rates if we try to throw lots of threads at a small
       btree. To work around this we'll partition the space of
       accounts into LOADERS_TO_USE segments and have a single thread
       load the first 10k accounts from each partition before firing
       up the real workers.
     */
    long total_accounts = _scaling_factor*TPCB_ACCOUNTS_PER_BRANCH;

    // Adjust the number of loaders to use, if the scaling factor is very small
    // and the total_accounts < #loaders* accounts_per_branch
    if (_scaling_factor<_loaders_to_use) {
        _loaders_to_use = _scaling_factor;
    }
    else {
        // number of accounts must be multiple of number of loaders, otherwise
        // load will fail
        while (total_accounts % _loaders_to_use != 0) {
            _loaders_to_use--;
        }
    }

    long accts_per_worker = total_accounts/_loaders_to_use;

    // time_t tstart = time(nullptr);

    // 2. Create and fire up the table creator which will also start the loading
    {
	guard<table_creator_t> tc;
	tc = new table_creator_t(this, _scaling_factor,
                                 accts_per_worker, _loaders_to_use);
	tc->fork();
	tc->join();
    }

    return RCOK;
}

/******************************************************************
 *
 * @fn:    load_data()
 *
 * @brief: Loads the data for all the TPCB tables, given the current
 *         scaling factor value. During the loading the SF cannot be
 *         changed. May only be invoked from ShoreEnv::load(), which
 *         aquires the necessary mutexes!
 *
 ******************************************************************/

w_rc_t ShoreTPCBEnv::load_data()
{
    // 3. Create and file up the loading workers
	long total_accounts = _scaling_factor*TPCB_ACCOUNTS_PER_BRANCH;
	long accts_per_worker = total_accounts/_loaders_to_use;

    /* This number is really flexible. Basically, it just needs to be
       high enough to give good parallelism, while remaining low
       enough not to cause too much contention. I pulled '40' out of
       thin air.
     */
    array_guard_t< guard<table_builder_t> > loaders(new guard<table_builder_t>[_loaders_to_use]);
    for(int i=0; i < _loaders_to_use; i++) {
	// the preloader thread picked up that first set of accounts...
	long start = accts_per_worker*i+TPCB_ACCOUNTS_CREATED_PER_POP_XCT;
	long count = accts_per_worker-TPCB_ACCOUNTS_CREATED_PER_POP_XCT;
	loaders[i] = new table_builder_t(this, i, _scaling_factor, start, count);
	loaders[i]->fork();
    }

    // 4. Join the loading threads
    for(int i=0; i<_loaders_to_use; i++) {
	loaders[i]->join();
    }

    return RCOK;
}



/******************************************************************
 *
 * @fn:    check_consistency()
 *
 * @brief: Iterates over all tables and checks consistency between
 *         the values stored in the base table (file) and the
 *         corresponding indexes.
 *
 ******************************************************************/

w_rc_t ShoreTPCBEnv::check_consistency()
{
    // not loaded from files, so no inconsistency possible
    return RCOK;
}


/******************************************************************
 *
 * @fn:    warmup()
 *
 * @brief: Touches the entire database - For memory-fitting databases
 *         this is enough to bring it to load it to memory
 *
 ******************************************************************/

w_rc_t ShoreTPCBEnv::warmup()
{
    return (check_consistency());
}


/********************************************************************
 *
 *  @fn:    dump
 *
 *  @brief: Print information for all the tables in the environment
 *
 ********************************************************************/

int ShoreTPCBEnv::dump()
{
    assert (0); // IP: not implemented yet

//     table_man_t* ptable_man = nullptr;
//     for(table_man_list_iter table_man_iter = _table_man_list.begin();
//         table_man_iter != _table_man_list.end(); table_man_iter++)
//         {
//             ptable_man = *table_man_iter;
//             ptable_man->print_table(this->_pssm);
//         }

    return (0);
}


int ShoreTPCBEnv::conf()
{
    // reread the params
    ShoreEnv::conf();
    upd_worker_cnt();
    return (0);
}


/********************************************************************
 *
 * Make sure the very contented tables are padded to one record per page
 *
 *********************************************************************/

int ShoreTPCBEnv::post_init()
{
    conf();

    return (0);
}


/*********************************************************************
 *
 *  @fn:   db_print
 *
 *  @brief: Prints the current tpcb tables to files
 *
 *********************************************************************/

w_rc_t ShoreTPCBEnv::db_print(int /*lines*/)
{
    // ensure a valid environment
    assert (_pssm);
    assert (_initialized);
    assert (_loaded);

    // print tables -- CS TODO
    // W_DO(branch_man->print_table(_pssm, lines));
    // W_DO(teller_man->print_table(_pssm, lines));
    // W_DO(account_man->print_table(_pssm, lines));
    // W_DO(history_man->print_table(_pssm, lines));

    return (RCOK);
}


/*********************************************************************
 *
 *  @fn:   db_fetch
 *
 *  @brief: Fetches the current tpcb tables to buffer pool
 *
 *********************************************************************/

w_rc_t ShoreTPCBEnv::db_fetch()
{
    // ensure a valid environment
    assert (_pssm);
    assert (_initialized);
    assert (_loaded);

    // fetch tables -- CS TODO
    // W_DO(_pbranch_man->fetch_table(_pssm));
    // W_DO(_pteller_man->fetch_table(_pssm));
    // W_DO(_paccount_man->fetch_table(_pssm));
    // W_DO(_phistory_man->fetch_table(_pssm));

    return (RCOK);
}

}; // namespace

