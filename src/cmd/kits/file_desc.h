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

/** @file:   file_desc.h
 *
 *  @brief:  Descriptors for Shore files/indexes, and structures that help in
 *           keeping track of the created files/indexes.
 *
 *  @author: Ippokratis Pandis, January 2008
 *
 */

#ifndef __FILE_DESC_H
#define __FILE_DESC_H

#include "sm_vas.h"

#include <list>

using std::list;

/******** Exported constants ********/

const unsigned int MAX_FIELDNAME_LEN = 40;

const unsigned int MAX_KEYDESC_LEN = 40;

#define  DELIM_CHAR            '|'
#define  ROWEND_CHAR            '\r'

const unsigned int COMMIT_ACTION_COUNT = 2000;

#define  MIN_SMALLINT     0
#define  MAX_SMALLINT     1<<15
#define  MIN_INT          0
#define  MAX_INT          1<<31
#define  MIN_FLOAT        0
#define  MAX_FLOAT        1<<10

#endif // __FILE_DESC_H
