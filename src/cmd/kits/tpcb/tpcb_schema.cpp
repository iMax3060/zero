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

/** @file:   tpcb_schema.h
 *
 *  @brief:  Declaration of the TPC-B tables
 *
 *  @author: Ippokratis Pandis, January 2008
 *
 */

#include "tpcb_schema.h"

namespace tpcb {

/*********************************************************************
 *
 * TPC-B SCHEMA
 *
 * This file contains the classes for tables in tpcb benchmark.
 * A class derived from tpcb_table_t (which inherits from table_desc_t)
 * is created for each table in the databases.
 *
 *********************************************************************/


/*
 * A primary index is created on each table except HISTORY
 *
 * 1. BRANCH
 * a. primary (unique) index on branch(b_id)
 *
 * 2. TELLER
 * a. primary (unique) index on teller(t_id)
 *
 * 3. ACCOUNT
 * a. primary (unique) index on account(a_id)
 *
 */


branch_t::branch_t()
      : table_desc_t("BRANCH", 2)
{
    // Schema
    _desc[0].setup(SQL_INT,   "B_ID");
    _desc[1].setup(SQL_FLOAT, "B_BALANCE");

    // create unique index b_idx on (b_id)
    uint  keys1[1] = { 0 }; // IDX { B_ID }
    create_primary_idx_desc(keys1, 1);
}



teller_t::teller_t()
      : table_desc_t("TELLER", 3)
{
    // Schema
    _desc[0].setup(SQL_INT,   "T_ID");
    _desc[1].setup(SQL_INT,   "T_B_ID");
    _desc[2].setup(SQL_FLOAT, "T_BALANCE");

    // create unique index t_idx on (t_id)
    uint keys1[1] = { 0 }; // IDX { T_ID }
    create_primary_idx_desc(keys1, 1);
}



account_t::account_t()
      : table_desc_t("ACCOUNT", 3)
{
    // Schema
    _desc[0].setup(SQL_INT,    "A_ID");
    _desc[1].setup(SQL_INT,    "A_B_ID");
    _desc[2].setup(SQL_FLOAT,  "A_BALANCE");

#ifdef PLP_MBENCH
#warning PLP MBench !!!!
    uint keys1[3] = { 0, 1, 2};
    uint nkeys = 3;
#else
    uint keys1[1] = {0 }; // IDX { A_ID }
    uint nkeys = 1;
#endif

    // create unique index a_idx on (a_id)
    create_primary_idx_desc(keys1, nkeys);
}


history_t::history_t()
      : table_desc_t("HISTORY", 5)
#
{
    // Schema
    _desc[0].setup(SQL_INT,   "H_B_ID");
    _desc[1].setup(SQL_INT,   "H_T_ID");
    _desc[2].setup(SQL_INT,   "H_A_ID");
    _desc[3].setup(SQL_FLOAT, "H_DELTA");   /* old: INT */
    _desc[4].setup(SQL_FLOAT, "H_TIME");    /* old: TIME */

    // index is required in Zero -- use all fields
    unsigned keys[5] = { 0, 1, 2, 3, 4 };
    create_primary_idx_desc(keys, 5);
}

}; // namespace
