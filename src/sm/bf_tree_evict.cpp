#include "bf_tree_cb.h"

w_rc_t bf_tree_m::_grab_free_block(bf_idx& ret, bool evict)
{
    ret = 0;
    while (true) {
        bool got_frame = _freelist->pop(ret);
        if (got_frame) {
            _approx_freelist_length--;
            DBG5(<< "Grabbing idx " << ret);
            return RCOK;
        } else {
            if (evict) {
                W_DO (_get_replacement_block());
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
    pthread_mutex_lock(&_eviction_lock);
    while(_freelist->empty())
    {
        get_evictioner()->evict();
    }
    pthread_mutex_unlock(&_eviction_lock);
    return RCOK;
}


void bf_tree_m::_add_free_block(bf_idx idx)
{
    bool added_frame = _freelist->push(idx);
    w_assert1(added_frame);
    _approx_freelist_length++;
}