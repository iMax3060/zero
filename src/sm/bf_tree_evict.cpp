#include "bf_tree_cb.h"
#include "bf_tree.h"
#include "btree_page_h.h"
#include "log_core.h" // debug: for printing log tail

w_rc_t bf_tree_m::_grab_free_block(bf_idx &ret, bool &evicted, bool evict)
{
    evicted = false;
    ret = 0;
    while (true) {
        bool got_frame = _freelist->pop(ret);
        if (got_frame) {
            w_assert1(_is_valid_idx(ret));
            w_assert1(!get_cb(ret)._used);
            _approx_freelist_length--;
            DBG5(<< "Grabbing idx " << ret);
            return RCOK;
        } else {
            if (evict) {
                W_DO(_get_replacement_block());
                evicted = true;
            } else {
                return RC(eBFFULL);
            }
        }
    }
    return RCOK;
}

w_rc_t bf_tree_m::_get_replacement_block()
{
    // Evictioner should be responsible for waking up cleaner.
    DO_PTHREAD(pthread_mutex_lock(&_eviction_lock));
    while(_freelist->empty())
    {
        get_evictioner()->evict();
    }
    DO_PTHREAD(pthread_mutex_unlock(&_eviction_lock));
    return RCOK;
}


void bf_tree_m::_add_free_block(bf_idx idx)
{
    w_assert0(_freelist->push(idx));
    if(_evictioner) _evictioner->unbuffered(idx);
    _approx_freelist_length++;
}