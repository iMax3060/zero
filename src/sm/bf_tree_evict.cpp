#include "bf_tree_cb.h"
#include "bf_tree.h"
#include "btree_page_h.h"
#include "log_core.h" // debug: for printing log tail

#include "bf_hashtable.cpp"

w_rc_t bf_tree_m::_grab_free_block(bf_idx& ret, bool evict)
{
    ret = 0;
    while (true) {
        // once the bufferpool becomes full, getting _freelist_lock everytime will be
        // too costly. so, we check _freelist_len without lock first.
        //   false positive : fine. we do real check with locks in it
        //   false negative : fine. we will eventually get some free block anyways.
        if (_freelist_len > 0) {
            CRITICAL_SECTION(cs, &_freelist_lock);
            if (_freelist_len > 0) { // here, we do the real check
                bf_idx idx = _freelist_tail;
                w_assert1(_freelist[idx].second == 0);
                _freelist_tail = _freelist[idx].first;
                _freelist[_freelist[idx].first].second = 0;
                _freelist[idx].first = 0;
                DBG5(<< "Grabbing idx " << idx);
                w_assert1(_is_valid_idx(idx));
                w_assert1 (!get_cb(idx)._used);
                ret = idx;

                --_freelist_len;
                if (_freelist_len == 0) {
                    _freelist_head = 0;
                    w_assert1(_freelist_head == _freelist_tail);
                } else if (_freelist_len == 1) {
                    w_assert1(_freelist_head == _freelist_tail);
                } else {
                    w_assert1(_freelist_head != _freelist_tail);
                    w_assert1(_freelist_head != 0);
                    w_assert1(_freelist_tail != 0);
                }
                DBG5(<< "New tail " << _freelist_tail);
                w_assert1(ret != _freelist_tail);
                return RCOK;
            }
        } // exit the scope to do the following out of the critical section

        // if the freelist was empty, let's evict some page.
        if (evict) {
            W_DO (_get_replacement_block());
        }
        else {
            return RC(eBFFULL);
        }
    }
    return RCOK;
}

w_rc_t bf_tree_m::_get_replacement_block()
{
    // Evictioner should be responsible for waking up cleaner.
    pthread_mutex_lock(&_eviction_lock);
    while(_freelist_len == 0)
    {
        get_evictioner()->evict();
    }
    pthread_mutex_unlock(&_eviction_lock);
    return RCOK;
}


void bf_tree_m::_add_free_block(bf_idx idx)
{
    CRITICAL_SECTION(cs, &_freelist_lock);
    // CS TODO: Eviction is apparently broken, since I'm seeing the same
    // frame being freed twice by two different threads.
    w_assert1(idx != _freelist_head && idx != _freelist_tail);
    w_assert1(_freelist[idx].first == 0 && _freelist[idx].second == 0);
    w_assert1(!get_cb(idx)._used);
    ++_freelist_len;
    _freelist[idx].second = _freelist_head;
    _freelist[_freelist_head].first = idx;
    _freelist_head = idx;
}