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

/** @file:   shore_tpce_schema.h
 *
 *  @brief:  Declaration of the TPC-E tables
 *
 *  @author: Ippokratis Pandis, January 2008
 *
 */

#ifndef __SHORE_TPCE_SCHEMA_H
#define __SHORE_TPCE_SCHEMA_H

#include <math.h>

#include "sm_vas.h"

#include "table_man.h"
#include "tpce_const.h"

namespace tpce {

/*********************************************************************
 *
 * TPC-E SCHEMA
 *
 *********************************************************************/

/* -------------------------------------------------- */
/* --- All the tables used in the TPC-E benchmark --- */
/* ---                                            --- */
/* --- Schema details at:                         --- */
/* --- src/workload/tpce/shore_tpce_schema.cpp    --- */
/* -------------------------------------------------- */

// Fixed tables
    DECLARE_TABLE_SCHEMA(sector_t);

    DECLARE_TABLE_SCHEMA(charge_t);

    DECLARE_TABLE_SCHEMA(commission_rate_t);

    DECLARE_TABLE_SCHEMA(exchange_t);

    DECLARE_TABLE_SCHEMA(industry_t);

    DECLARE_TABLE_SCHEMA(status_type_t);

    DECLARE_TABLE_SCHEMA(taxrate_t);

    DECLARE_TABLE_SCHEMA(trade_type_t);

    DECLARE_TABLE_SCHEMA(zip_code_t);

// Growing tables
    DECLARE_TABLE_SCHEMA(cash_transaction_t);

    DECLARE_TABLE_SCHEMA(settlement_t);

    DECLARE_TABLE_SCHEMA(trade_t);

    DECLARE_TABLE_SCHEMA(trade_history_t);

    DECLARE_TABLE_SCHEMA(trade_request_t);

// Scaling tables
    DECLARE_TABLE_SCHEMA(account_permission_t);

    DECLARE_TABLE_SCHEMA(broker_t);

    DECLARE_TABLE_SCHEMA(company_t);

    DECLARE_TABLE_SCHEMA(customer_t);

    DECLARE_TABLE_SCHEMA(company_competitor_t);

    DECLARE_TABLE_SCHEMA(security_t);

    DECLARE_TABLE_SCHEMA(customer_account_t);

    DECLARE_TABLE_SCHEMA(daily_market_t);

    DECLARE_TABLE_SCHEMA(customer_taxrate_t);

    DECLARE_TABLE_SCHEMA(holding_t);

    DECLARE_TABLE_SCHEMA(financial_t);

    DECLARE_TABLE_SCHEMA(holding_history_t);

    DECLARE_TABLE_SCHEMA(address_t);

    DECLARE_TABLE_SCHEMA(holding_summary_t);

    DECLARE_TABLE_SCHEMA(last_trade_t);

    DECLARE_TABLE_SCHEMA(watch_item_t);

    DECLARE_TABLE_SCHEMA(news_item_t);

    DECLARE_TABLE_SCHEMA(watch_list_t);

    DECLARE_TABLE_SCHEMA(news_xref_t);

// Unknown
    DECLARE_TABLE_SCHEMA(dimension_t);

}

#endif /* __SHORE_TPCE_SCHEMA_H */
