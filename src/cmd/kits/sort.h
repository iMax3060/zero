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

/** @file:   sort.h
 *
 *  @brief:  In-memory ascending sort buffer structure
 *
 *  @author: Ippokratis Pandis, January 2008
 *  @author: Ilknur Cansu Kaynak, April 2010
 *
 */

#ifndef __SORT_H
#define __SORT_H

#include "table_man.h"

/**********************************************************************
 *
 * This file contains the in-memory sort buffer structure definition.
 * The sort buffer is defined as a subclass of shore_table (defined in
 * shore_table.h) to take advantage of the schema and tuple value
 * operations. The data waiting to be sorted is stored in a memory
 * buffer (sort_asc_buffer_t::_sort_asc_buf).
 *
 * @note: To simplify the memory management, the sort buffer only works
 *        on fixed length fields.
 *        Supported sql_types_t: SQL_INT, SQL_SMALLINT.
 *        Under test: SQL_BIT
 *
 **********************************************************************/

class asc_sort_iter_impl;

/**********************************************************************
 *
 * @class:   sort_asc_buffer_t
 *
 * @brief:   Description of a sort buffer
 *
 **********************************************************************/

class asc_sort_buffer_t : public table_desc_t {
public:

    asc_sort_buffer_t(const size_t field_count)
            : table_desc_t("ASC_SORT_BUF", field_count) {}

    ~asc_sort_buffer_t() {}

    /* set the schema - accepts only fixed length */
    void setup(const size_t index, sqltype_t type, const int len = 0) {
        assert(index < _field_count);
        _desc[index].setup(type, "", len);
        assert(!_desc[index].is_variable_length());
        assert(!_desc[index].allow_null());
    }
}; // EOF: sort_asc_t



/**********************************************************************
 *
 * @class:   asc_sort_man_impl
 *
 * @warning: NO THREAD-SAFE the caller should make sure that only one
 *           thread is accessing objects of this class
 *
 **********************************************************************/

class asc_sort_man_impl : public table_man_t<asc_sort_buffer_t> {
    friend class asc_sort_iter_impl;

protected:

    char* _sort_buf;     /* memory buffer */
    int _tuple_size;   /* tuple size */
    int _tuple_count;  /* # of tuples in buffer */
    int _buf_size;     /* size of the buffer (in # of tuples) */
    bool _is_sorted;    /* shows if sorted */
    tatas_lock _sorted_lock;

    rep_row_t* _preprow;      /* used for the tuple->format() */

    /* count _tuple_size and allocate buffer */
    void init();

    /* retrieve a tuple */
    bool get_sorted(const int index, table_row_t* ptuple);

public:

    asc_sort_man_impl(asc_sort_buffer_t* aSortBufferAsc, rep_row_t* aprow)
            : table_man_t<asc_sort_buffer_t>(aSortBufferAsc, false),
              _sort_buf(nullptr),
              _tuple_size(0),
              _tuple_count(0),
              _buf_size(0),
              _is_sorted(false),
              _preprow(aprow) {}

    ~asc_sort_man_impl() {
        if (_sort_buf) {
            delete[] _sort_buf;
        }
    }

    /* add current tuple to the sort buffer */
    void add_tuple(table_row_t& atuple);

    /* return a sort iterator */
    w_rc_t get_asc_sort_iter(asc_sort_iter_impl*& asc_sort_iter);

    w_rc_t get_sort_iter(asc_sort_iter_impl*& sort_iter);

    /* sort tuples on the first field value */
    void sort();

    inline int count() {
        return (_tuple_count);
    }

    void reset();
}; // EOF: asc_sort_man_impl


/**********************************************************************
 *
 * @class: asc_sort_iter_impl
 *
 * @brief: Iterator over a sorted buffer
 *
 * @note:  Iterator that does not need a db handle, since the sorting
 *         takes place only in memory
 *
 **********************************************************************/

class asc_sort_iter_impl {
private:
    asc_sort_buffer_t* psortbuf;

    asc_sort_man_impl* _manager;

    int _index;

public:

    asc_sort_iter_impl(asc_sort_buffer_t* psortbuf, asc_sort_man_impl* psortman)
            : psortbuf(psortbuf),
              _manager(psortman),
              _index(0) {
        assert (_manager);
        assert (psortbuf);
        W_COERCE(open_scan());
    }


    /* ------------------------------ */
    /* --- sorted iter operations --- */
    /* ------------------------------ */

    w_rc_t open_scan();

    w_rc_t close_scan() {
        return (RCOK);
    };

    w_rc_t next(bool& eof, table_row_t& tuple);

    void reset();
}; // EOF: asc_sort_iter_impl



/**********************************************************************
 *
 * asc_sort_iter_impl methods
 *
 **********************************************************************/


/*********************************************************************
 *
 *  @fn:    open_scan
 *
 *  @brief: Opens a scan operator
 *
 *  @note:  If the sorted buffer is not sorted, it sorts it
 *
 *********************************************************************/

inline w_rc_t asc_sort_iter_impl::open_scan() {
    _manager->sort();

    _index = 0;
    return (RCOK);
}

/*********************************************************************
 *
 *  @fn:    next
 *
 *  @brief: Gets the next tuple pointed by the index
 *
 *********************************************************************/

inline w_rc_t asc_sort_iter_impl::next(bool& eof, table_row_t& tuple) {
    _manager->get_sorted(_index, &tuple);
    eof = (++_index > _manager->_tuple_count);
    return (RCOK);
}

/*********************************************************************
 *
 *  @fn:    reset
 *
 *  @brief: Clear the fields and prepares for re-use
 *
 *********************************************************************/

inline void asc_sort_iter_impl::reset() {
    // the sorter_manager should already be set
    assert (_manager);
    _index = 0;
}

#endif // __SORT_H
