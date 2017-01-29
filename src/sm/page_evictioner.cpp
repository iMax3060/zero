#include <w_findprime.h>
#include "page_evictioner.h"

#include "bf_tree.h"
#include "bf_hashtable.cpp"
#include "generic_page.h"

page_evictioner_base::page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options)
    :
    _bufferpool(bufferpool)
{
    _swizziling_enabled = options.get_bool_option("sm_bufferpool_swizzle", false);
    _current_frame = 0;
}

page_evictioner_base::~page_evictioner_base() {}

void page_evictioner_base::evict()
{
    uint32_t preferred_count = EVICT_BATCH_RATIO * _bufferpool->_block_cnt + 1;

    while(_bufferpool->_approx_freelist_length < preferred_count) // TODO: increment _freelist_len atomically
    {
        bf_idx victim = pick_victim();
    
        if(victim == 0) {
            /* idx 0 is never used, so this means pick_victim() exited without
             * finding a victim. This might happen when the page_evictioner is
             * shutting down, for example. */
            return;
        }

        w_assert1(victim != 0);

        bf_tree_cb_t& cb = _bufferpool->get_cb(victim);
        w_assert1(cb.latch().is_mine());

        if(unswizzle_and_update_emlsn(victim) == false) {
            /* We were not able to unswizzle/update parent, therefore we cannot
             * proceed with this victim. We just jump to the next iteration and
             * hope for better luck next time. */
            cb.latch().latch_release();
            continue;
        }

        // remove it from hashtable.
        PageID pid = _bufferpool->_buffer[victim].pid;
        w_assert1(cb._pin_cnt < 0 || pid == cb._pid);

        bool removed = _bufferpool->_hashtable->remove(pid);
        w_assert1(removed);

        DBG2(<< "EVICTED " << victim << " pid " << pid
                                 << " log-tail " << smlevel_0::log->curr_lsn());
        cb.clear_except_latch();
        //-1 indicates page was evicted(i.e., it's invalid and can be read into)
        cb._pin_cnt = -1;

        _bufferpool->_add_free_block(victim);

        cb.latch().latch_release();

        INC_TSTAT(bf_evict);
    }
}

void page_evictioner_base::ref(bf_idx idx) {}

void page_evictioner_base::miss_ref(bf_idx b_idx, PageID pid) {}

bf_idx page_evictioner_base::pick_victim()
{
    /*
     * CS: strategy is to try acquiring an EX latch imediately. If it works,
     * page is not that busy, so we can evict it. But only evict leaf pages.
     * This is like a random policy that only evicts uncontented pages. It is
     * not as effective as LRU or CLOCK, but it is better than RANDOM, simple
     * to implement and, most importantly, does not have concurrency bugs!
     */

     bf_idx idx = _current_frame;
     while(true) {
         
        if(idx == _bufferpool->_block_cnt) {
            idx = 1;
        }

        if (idx == _current_frame - 1) {
            // We iterate over all pages and no victim was found.
            // Wake up cleaner and wait here.
            _bufferpool->get_cleaner()->wakeup(true);
        }

        // CS TODO -- why do we latch CB manually instead of simply fixing
        // the page??

        bf_tree_cb_t& cb = _bufferpool->get_cb(idx);

        // Step 1: latch page in EX mode and check if eligible for eviction
        rc_t latch_rc;
        latch_rc = cb.latch().latch_acquire(LATCH_EX, sthread_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error()) {
            idx++;
            DBG3(<< "Eviction failed on latch for " << idx);
            continue;
        }
        w_assert1(cb.latch().is_mine());

        // now we hold an EX latch -- check if leaf and not dirty
        btree_page_h p;
        p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
        if (p.tag() != t_btree_p || !p.is_leaf() || cb.is_dirty()
                || !cb._used || p.pid() == p.root() || p.get_foster() != 0)
        {
            cb.latch().latch_release();
            DBG5(<< "Eviction failed on flags for " << idx);
            idx++;
            continue;
        }

        // page is a B-tree leaf -- check if pin count is zero
        if (cb._pin_cnt != 0)
        {
            // pin count -1 means page was already evicted
            cb.latch().latch_release();
            DBG3(<< "Eviction failed on for " << idx
                    << " pin count is " << cb._pin_cnt);
            idx++;
            continue;
        }
        w_assert1(_bufferpool->_is_active_idx(idx));

        // If we got here, we passed all tests and have a victim!
        _current_frame = idx +1;
        return idx;
    }
}

bool page_evictioner_base::unswizzle_and_update_emlsn(bf_idx idx)
{
    bf_tree_cb_t& cb = _bufferpool->get_cb(idx);
    w_assert1(cb.latch().is_mine());

    //==========================================================================
    // STEP 1: Look for parent.
    //==========================================================================
    PageID pid = _bufferpool->_buffer[idx].pid;
    bf_idx_pair idx_pair;
    bool found = _bufferpool->_hashtable->lookup(pid, idx_pair);

    bf_idx parent_idx = idx_pair.second;
    w_assert1(!found || idx == idx_pair.first);

    // Index zero is never used, so it means invalid pointer
    if (!found || parent_idx == 0) {
        return false;
    }

    bf_tree_cb_t& parent_cb = _bufferpool->get_cb(parent_idx);
    rc_t r = parent_cb.latch().latch_acquire(LATCH_EX, sthread_t::WAIT_IMMEDIATE);
    if (r.is_error()) {
        /* Just give up. If we try to latch it unconditionally, we may deadlock,
         * because other threads are also waiting on the eviction mutex. */
        return false;
    }
    w_assert1(parent_cb.latch().is_mine());

    /* Look for emlsn slot on parent (must be found because parent pointer is
     * kept consistent at all times). */
    w_assert1(_bufferpool->_is_active_idx(parent_idx));
    generic_page *parent = &_bufferpool->_buffer[parent_idx];
    btree_page_h parent_h;
    parent_h.fix_nonbufferpool_page(parent);

    general_recordid_t child_slotid;
    if (_swizziling_enabled && cb._swizzled) {
        // Search for swizzled address
        PageID swizzled_pid = idx | SWIZZLED_PID_BIT;
        child_slotid = _bufferpool->find_page_id_slot(parent, swizzled_pid);
    }
    else {
        child_slotid = _bufferpool->find_page_id_slot(parent, pid);
    }
    w_assert1 (child_slotid != GeneralRecordIds::INVALID);

    //==========================================================================
    // STEP 2: Unswizzle pointer on parent before evicting.
    //==========================================================================
    if (_swizziling_enabled && cb._swizzled) {
        bool ret = _bufferpool->unswizzle(parent, child_slotid);
        w_assert0(ret);
        w_assert1(!cb._swizzled);
    }

    //==========================================================================
    // STEP 3: Page will be evicted -- update EMLSN on parent.
    //==========================================================================
    lsn_t old = parent_h.get_emlsn_general(child_slotid);
    _bufferpool->_buffer[idx].lsn = cb.get_page_lsn();
    if (old < _bufferpool->_buffer[idx].lsn) {
        DBG3(<< "Updated EMLSN on page " << parent_h.pid()
                << " slot=" << child_slotid
                << " (child pid=" << pid << ")"
                << ", OldEMLSN=" << old
                << " NewEMLSN=" << _bufferpool->_buffer[idx].lsn);
    
        w_assert1(parent_cb.latch().is_mine());
        w_assert1(parent_cb.latch().mode() == LATCH_EX);
    
        W_COERCE(_bufferpool->_sx_update_child_emlsn(parent_h, child_slotid,
                                                     _bufferpool->_buffer[idx].lsn));
    
        w_assert1(parent_h.get_emlsn_general(child_slotid)
                  == _bufferpool->_buffer[idx].lsn);
    }

    parent_cb.latch().latch_release();
    return true;
}

bool page_evictioner_base::evict_page(bf_idx idx, PageID &evicted_page) {
    evicted_page = 0;
    bf_tree_cb_t& cb = _bufferpool->get_cb(idx);
    
    rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, sthread_t::WAIT_IMMEDIATE);
    if (latch_rc.is_error()) {
        return false;
    }
    
    w_assert1(cb.latch().held_by_me());
    
    /* There are some pages we want to ignore in our policies:
     * 1) Non B+Tree pages
     * 2) Dirty pages (the cleaner should have cleaned it already)
     * 3) Pages being used by someon else
     * 4) The root
     */
    btree_page_h p;
    p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
    if (p.tag() != t_btree_p || cb.is_dirty() ||
        !cb._used || p.pid() == p.root())
    {
        // LL: Should we also decrement the clock count in this case?
        cb.latch().latch_release();
        return false;
    }
    
    // Ignore pages that still have swizzled children
    if(_swizziling_enabled && _bufferpool->has_swizzled_child(idx)) {
        // LL: Should we also decrement the clock count in this case?
        cb.latch().latch_release();
        return false;
    }
    
    evicted_page = cb._pid;
    
    cb.latch().latch_release();
    return true;
}

page_evictioner_gclock::page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options)
    : page_evictioner_base(bufferpool, options)
{
    _k = options.get_int_option("sm_bufferpool_gclock_k", 10);
    _counts = new uint16_t [_bufferpool->_block_cnt];
    _current_frame = 0;

}

page_evictioner_gclock::~page_evictioner_gclock()
{
    delete [] _counts;
}

void page_evictioner_gclock::ref(bf_idx idx)
{
    _counts[idx] = _k;
}

void page_evictioner_gclock::miss_ref(bf_idx b_idx, PageID pid) {}

bf_idx page_evictioner_gclock::pick_victim()
{
    // Check if we still need to evict
    bf_idx idx = _current_frame;
    while(true)
    {
        // Circular iteration, jump idx 0
        idx = (idx % (_bufferpool->_block_cnt-1)) + 1;
        w_assert1(idx != 0);

        // Before starting, let's fire some prefetching for the next step.
        bf_idx next_idx = ((idx+1) % (_bufferpool->_block_cnt-1)) + 1;
        __builtin_prefetch(&_bufferpool->_buffer[next_idx]);
        __builtin_prefetch(_bufferpool->get_cbp(next_idx));

        // Now we do the real work.
        bf_tree_cb_t& cb = _bufferpool->get_cb(idx);

        rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, sthread_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error())
        {
            idx++;
            continue;
        }

        w_assert1(cb.latch().held_by_me());

        /* There are some pages we want to ignore in our policy:
         * 1) Non B+Tree pages
         * 2) Dirty pages (the cleaner should have cleaned it already)
         * 3) Pages being used by someon else
         * 4) The root
         */
        btree_page_h p;
        p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
        if (p.tag() != t_btree_p || cb.is_dirty() ||
            !cb._used || p.pid() == p.root())
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            idx++;
            continue;
        }

        // Ignore pages that still have swizzled children
        if(_swizziling_enabled && _bufferpool->has_swizzled_child(idx))
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            idx++;
            continue;
        }

        if(_counts[idx] <= 0)
        {
            // We have found our victim!
            bool would_block;
            cb.latch().upgrade_if_not_block(would_block); //Try to upgrade latch
            if(!would_block) {
                w_assert1(cb.latch().is_mine());

                /* No need to re-check the values above, because the cb was
                 * already latched in SH mode, so they cannot change. */

                if (cb._pin_cnt != 0) {
                    cb.latch().latch_release(); // pin count -1 means page was already evicted
                    idx++;
                    continue;
                }

                _current_frame = idx + 1;
                return idx;
            }
        }
        cb.latch().latch_release();
        --_counts[idx]; //TODO: MAKE ATOMIC
        idx++;
    }
}

/*
page_evictioner_clockpro::page_evictioner_clockpro(bf_tree_m* bufferpool, const sm_options& options)
        : page_evictioner_base(bufferpool, options) {
    _referenced = new bool[2 * (_bufferpool->_block_cnt - 1)];
    _hot = new bool[2 * (_bufferpool->_block_cnt - 1)];
    _test = new bool[2 * (_bufferpool->_block_cnt - 1)];
    _bf_idx_to_clk_idx = new clk_idx[_bufferpool->_block_cnt];
    _clk_idx_to_bf_idx = new bf_idx[2 * (_bufferpool->_block_cnt - 1)];
    _page_id_to_clk_idx = new bf_hashtable<clk_idx >(w_findprime(2048 + (_bufferpool->_block_cnt / 2)));
}

page_evictioner_clockpro::~page_evictioner_clockpro() {
    delete[](_referenced);
    delete[](_hot);
    delete[](_test);
    delete[](_bf_idx_to_clk_idx);
    delete[](_clk_idx_to_bf_idx);
    delete (_page_id_to_clk_idx);
}

void page_evictioner_clockpro::ref(bf_idx idx) {
    _referenced[_bf_idx_to_clk_idx[idx]] = true;
}

void page_evictioner_clockpro::miss_ref(bf_idx b_idx, PageID pid) {
    clk_idx c_idx;
    if (_page_id_to_clk_idx->lookup(pid, c_idx)) {
        _bf_idx_to_clk_idx[b_idx] = c_idx;
        _clk_idx_to_bf_idx[c_idx] = b_idx;
        _referenced[c_idx] = true;
    } else {
        c_idx =
        _page_id_to_clk_idx->insert_if_not_exists(pid, c_idx);
        _referenced[c_idx] = true;
        _test[c_idx] = true;
    }
}

bf_idx page_evictioner_clockpro::pick_victim() {
    
    return run_hand_cold();
}

void page_evictioner_clockpro::run_hand_hot() {
    
}

bf_idx page_evictioner_clockpro::run_hand_cold() {
    
    // Check if we still need to evict
    clk_idx c_idx = _hand_cold;
    while(true)
    {
        c_idx = (c_idx == 2 * (_bufferpool->_block_cnt - 1)) ? 0 : c_idx;
        
        w_assert1(0 <= c_idx && c_idx < 2 * (_bufferpool->_block_cnt - 1));

//        // Circular iteration, jump idx 0
//        idx = (idx % (_bufferpool->_block_cnt-1)) + 1;
//        w_assert1(idx != 0);

//        // Before starting, let's fire some prefetching for the next step.
//        bf_idx next_idx = ((c_idx+1) % (_bufferpool->_block_cnt-1)) + 1;
//        __builtin_prefetch(&_bufferpool->_buffer[next_idx]);
//        __builtin_prefetch(_bufferpool->get_cbp(next_idx));
        
        bf_idx b_idx = _clk_idx_to_bf_idx[c_idx];       // latch the clock-element ?
        
        if (b_idx == 0) {
            c_idx++;
            continue;
        }
        
        // Now we do the real work.
        bf_tree_cb_t& cb = _bufferpool->get_cb(b_idx);
        
        rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, sthread_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error()) {
            c_idx++;
            continue;
        }
        
        w_assert1(cb.latch().held_by_me());
        
        */
/* There are some pages we want to ignore in our policy:
         * 1) Non B+Tree pages
         * 2) Dirty pages (the cleaner should have cleaned it already)
         * 3) Pages being used by someon else
         * 4) The root
         *//*

        btree_page_h p;
        p.fix_nonbufferpool_page(_bufferpool->_buffer + c_idx);
        if (p.tag() != t_btree_p || cb.is_dirty() ||
            !cb._used || p.pid() == p.root())
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            c_idx++;
            continue;
        }
        
        // Ignore pages that still have swizzled children
        if(_swizziling_enabled && _bufferpool->has_swizzled_child(c_idx))
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            c_idx++;
            continue;
        }
        
        if (!_hot[c_idx]) {
            if (!_referenced[c_idx]) {
                // We have found our victim!
                bool would_block;
                cb.latch().upgrade_if_not_block(would_block); //Try to upgrade latch
                if (!would_block) {
                    w_assert1(cb.latch().is_mine());
                    
                    */
/* No need to re-check the values above, because the cb was
                     * already latched in SH mode, so they cannot change. *//*

                    
                    if (cb._pin_cnt != 0) {
                        cb.latch().latch_release(); // pin count -1 means page was already evicted
                        c_idx++;
                        continue;
                    }
                    
                    _bf_idx_to_clk_idx[b_idx] = 2 * (_bufferpool->_block_cnt - 1);
                    _clk_idx_to_bf_idx[c_idx] = 0;
                    _hand_cold = c_idx++;
                    return c_idx;
                }
            } else if (_test[c_idx]) {
                _hot[c_idx] = true;
            }
        }
        cb.latch().latch_release();
        --_counts[c_idx]; //TODO: MAKE ATOMIC
        c_idx++;
    }
}

void page_evictioner_clockpro::run_hand_test() {
    
}
*/

page_evictioner_cart::page_evictioner_cart(bf_tree_m *bufferpool, const sm_options &options)
        : page_evictioner_base(bufferpool, options)
{
    _clocks = new multi_clock<bool>(_bufferpool->_block_cnt, 2, 0);
    
    for (int i = 1; i <= _bufferpool->_block_cnt - 1; i++) {
        _clocks->add_tail(T_1, i);
    }
    
    _b1 = new hashtable_queue<PageID>(1 | SWIZZLED_PID_BIT);
    _b2 = new hashtable_queue<PageID>(1 | SWIZZLED_PID_BIT);
    
    _p = 0;
    
    DO_PTHREAD(pthread_mutex_init(&_lock, nullptr));
}

page_evictioner_cart::~page_evictioner_cart() {
    delete(_clocks);
    
    delete(_b1);
    delete(_b2);
    
    DO_PTHREAD(pthread_mutex_destroy(&_lock));
}

void page_evictioner_cart::ref(bf_idx idx) {
    _clocks->get(idx) = true;
}

void page_evictioner_cart::miss_ref(bf_idx b_idx, PageID pid) {
    DO_PTHREAD(pthread_mutex_lock(&_lock));
    if (!_b1->contains(pid) && !_b2->contains(pid)) {
        if (_clocks->size_of(T_1) + _b1->length() == _bufferpool->_block_cnt - 1) {
            _b1->remove_tail();
        } else if (_clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() == 2 * (_bufferpool->_block_cnt - 1)) {
            _b2->remove_tail();
        }
        _clocks->add_tail(T_1, b_idx);
        _clocks->get(b_idx) = 0;
    } else if (_b1->contains(pid)) {
        _p = std::min(_p + std::max(u_int32_t(1), _b2->length() / _b1->length()), _bufferpool->_block_cnt - 1);
        _b1->remove(pid);
        _clocks->add_tail(T_2, b_idx);
        _clocks->get(b_idx) = 0;
    } else {
        _p = std::max(_p + std::max(u_int32_t(1), _b1->length() / _b2->length()), u_int32_t(0));
        _b2->remove(pid);
        _clocks->add_tail(T_2, b_idx);
        _clocks->get(b_idx) = 0;
    }
    w_assert1(_clocks->size_of(T_1) + _clocks->size_of(T_2) <= _bufferpool->_block_cnt - 1);
    w_assert1(_clocks->size_of(T_1) + _b1->length() <= _bufferpool->_block_cnt - 1);
    w_assert1(_clocks->size_of(T_2) + _b2->length() <= 2 * (_bufferpool->_block_cnt - 1));
    w_assert1(_clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() <= 2 * (_bufferpool->_block_cnt - 1));
    DO_PTHREAD(pthread_mutex_unlock(&_lock));
}

bf_idx page_evictioner_cart::pick_victim() {
    bool evicted_page = false;
    while (!evicted_page) {
        DO_PTHREAD(pthread_mutex_lock(&_lock));
        if (_clocks->size_of(T_1) >= std::max(u_int32_t(1), _p)) {
            bool t_1_head;
            bf_idx t_1_head_index = 0;
            _clocks->get_head(T_1, t_1_head);
            _clocks->get_head_index(T_1, t_1_head_index);
            
            if (!t_1_head) {
                PageID evicted_pid;
                evicted_page = evict_page(t_1_head_index, evicted_pid);
                
                if (evicted_page) {
                    bool removed = _clocks->remove_head(T_1, t_1_head_index);
                    bool inserted = _b1->insert_head(evicted_pid);
                    w_assert1(removed && inserted);
                    return t_1_head_index;
                } else {
                    _clocks->move_head(T_1);
                }
            } else {
                t_1_head = 0;
                _clocks->switch_head_to_tail(T_1, T_2, t_1_head_index);
            }
        } else {
            bool t_2_head;
            bf_idx t_2_head_index = 0;
            _clocks->get_head(T_2, t_2_head);
            _clocks->get_head_index(T_2, t_2_head_index);
    
            if (!t_2_head) {
                PageID evicted_pid;
                evicted_page = evict_page(t_2_head_index, evicted_pid);
        
                if (evicted_page) {
                    bool removed = _clocks->remove_head(T_2, t_2_head_index);
                    bool inserted = _b2->insert_head(evicted_pid);
                    w_assert1(removed && inserted);
                    return t_2_head_index;
                } else {
                    _clocks->move_head(T_1);
                }
            } else {
                t_2_head = 0;
                _clocks->move_head(T_2);
            }
        }
        DO_PTHREAD(pthread_mutex_unlock(&_lock));
    }
    return 0;
}

template<class key>
bool page_evictioner_cart::hashtable_queue<key>::contains(key k) {
    return _list->count(k);
}

template<class key>
page_evictioner_cart::hashtable_queue<key>::hashtable_queue(key invalid_value) {
    _list = new std::unordered_map<key, key_pair>();
	_invalid_value = invalid_value;
    _head = _invalid_value;
    _tail = _invalid_value;
}

template<class key>
page_evictioner_cart::hashtable_queue<key>::~hashtable_queue() {
    delete(_list);
    _list = nullptr;
}

template<class key>
bool page_evictioner_cart::hashtable_queue<key>::insert_head(key k) {
    if (!_list->empty()) {
        auto previous_size = _list->size();
        key previous_head = _head;
        key_pair previous_head_entry = (*_list)[previous_head];
        w_assert1(previous_head != _invalid_value);
        w_assert1(previous_head_entry.first == _invalid_value);
        
        if (_list->count(k) > 0) {
            return false;
        }
        (*_list)[k] = key_pair(_invalid_value, previous_head);
        (*_list)[previous_head].first = k;
        _head = k;
        w_assert1(_list->size() == previous_size + 1);
    } else {
        w_assert1(_head == _invalid_value);
        w_assert1(_tail == _invalid_value);
    
        (*_list)[k] = key_pair(_invalid_value, _invalid_value);
        _head = k;
        _tail = k;
        w_assert1(_list->size() == 1);
    }
    return true;
}

template<class key>
bool page_evictioner_cart::hashtable_queue<key>::remove_tail() {
    if (_list->empty()) {
        return false;
    } else if (_list->size() == 1) {
        w_assert1(_head == _tail);
        w_assert1((*_list)[_head].first == _invalid_value);
        w_assert1((*_list)[_head].second == _invalid_value);
        
        _list->erase(_head);
        _head = _invalid_value;
        _tail = _invalid_value;
        w_assert1(_list->size() == 0);
    } else {
        auto previous_size = _list->size();
        key previous_tail = _tail;
        key_pair previous_tail_entry = (*_list)[_tail];
        w_assert1(_head != _tail);
        w_assert1(_head != _invalid_value);
        
        _tail = previous_tail_entry.first;
        (*_list)[previous_tail_entry.first].second = _invalid_value;
        _list->erase(previous_tail);
        w_assert1(_list->size() == previous_size - 1);
    }
    return false;
}

template<class key>
bool page_evictioner_cart::hashtable_queue<key>::remove(key k) {
    if (_list->count(k) == 0) {
        return false;
    } else {
        auto previous_size = _list->size();
        key_pair previous_value = (*_list)[k];
        if (previous_value.first != _invalid_value) {
            (*_list)[previous_value.first].second = previous_value.second;
        } else {
            _head = previous_value.second;
        }
        if (previous_value.second != _invalid_value) {
            (*_list)[previous_value.second].first = previous_value.first;
        } else {
            _tail = previous_value.first;
        }
        _list->erase(k);
        w_assert1(_list->size() == previous_size - 1);
    }
    return true;
}

template<class key>
u_int32_t page_evictioner_cart::hashtable_queue<key>::length() {
    return _list->size();
}

template<class value>
page_evictioner_cart::multi_clock<value>::multi_clock(u_int32_t clocksize, u_int32_t clocknumber, u_int32_t invalid_index) {
    _clocksize = clocksize;
    _values = new value[_clocksize]();
    _clocks = new pair<u_int32_t, u_int32_t>[_clocksize]();
    _invalid_index = invalid_index;
    
    _clocknumber = clocknumber;
    _hands = new u_int32_t[_clocknumber]();
    _sizes = new u_int32_t[_clocknumber]();
    for (int i = 0; i <= _clocknumber - 1; i++) {
        _hands[i] = _invalid_index;
    }
    _invalid_clock_index = _clocknumber;
    _clock_membership = new u_int32_t[_clocksize]();
    for (int i = 0; i <= _clocksize - 1; i++) {
        _clock_membership[i] = _invalid_clock_index;
    }
}

template<class value>
page_evictioner_cart::multi_clock<value>::~multi_clock() {
    _clocksize = 0;
    delete[](_values);
    delete[](_clocks);
    _invalid_index = 0;
    
    _clocknumber = 0;
    delete[](_hands);
    delete[](_sizes);
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::get_head(u_int32_t clock, value &head_value) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        head_value = _values[_hands[clock]];
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        return false;
    }
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::get_head_index(u_int32_t clock, u_int32_t &head_index) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        head_index = _hands[clock];
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        return false;
    }
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::move_head(u_int32_t clock) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        _hands[clock] = _clocks[_hands[clock]].second;
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        return false;
    }
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::add_tail(u_int32_t clock, u_int32_t index) {
    if (index >= 0 && index <= _clocksize - 1 && index != _invalid_index
                   && clock >= 0 && clock <= _clocknumber - 1
                   && _clock_membership[index] == _invalid_clock_index) {
        if (_sizes[clock] == 0) {
            _hands[clock] = index;
            _clocks[index].first = index;
            _clocks[index].second = index;
        } else {
            _clocks[index].first = _clocks[_hands[clock]].first;
            _clocks[index].second = _hands[clock];
            _clocks[_clocks[_hands[clock]].first].second = index;
            _clocks[_hands[clock]].first = index;
        }
        _sizes[clock]++;
        _clock_membership[index] = clock;
//        std::cout << "Added to " << clock << ": " << index << "; New size: " << _sizes[clock] << std::endl;
        return true;
    } else {
        return false;
    }
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::remove_head(u_int32_t clock, u_int32_t &removed_index) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        u_int32_t _old_hand = _invalid_index;
        if (_sizes[clock] == 0) {
            w_assert1(_hands[clock] == _invalid_index);
            return false;
        } else if (_sizes[clock] == 1) {
            w_assert1(_hands[clock] >= 0 && _hands[clock] <= _clocksize - 1 && _hands[clock] != _invalid_index);
            w_assert1(_clocks[_hands[clock]].first == _hands[clock]);
            w_assert1(_clocks[_hands[clock]].second == _hands[clock]);
    
            _old_hand = _hands[clock];
            _clocks[_old_hand].first = _invalid_index;
            _clocks[_old_hand].second = _invalid_index;
            _hands[clock] = _invalid_index;
            _clock_membership[_old_hand] = _invalid_clock_index;
            _sizes[clock]--;
            return true;
//            std::cout << "Removed from " << clock << ": " << removed_index << "; New size: " << _sizes[clock] << std::endl;
        } else {
            _old_hand = _hands[clock];
            _clocks[_clocks[_old_hand].first].second = _clocks[_old_hand].second;
            _clocks[_clocks[_old_hand].second].first = _clocks[_old_hand].first;
            _hands[clock] = _clocks[_old_hand].second;
            _clocks[_old_hand].first = _invalid_index;
            _clocks[_old_hand].second = _invalid_index;
            _clock_membership[_old_hand] = _invalid_clock_index;
            _sizes[clock]--;
//            std::cout << "Removed from " << clock << ": " << removed_index << "; New size: " << _sizes[clock] << std::endl;
            return true;
        }
    } else {
        return false;
    }
}

template<class value>
bool page_evictioner_cart::multi_clock<value>::switch_head_to_tail(u_int32_t source, u_int32_t destination, u_int32_t &moved_index) {
    moved_index = _invalid_index;
    if (_sizes[source] > 0
     && source >= 0 && source <= _clocknumber - 1
     && destination >= 0 && destination <= _clocknumber - 1) {
        bool removed = remove_head(source, moved_index);
        w_assert1(removed);
        
        bool added = add_tail(destination, moved_index);
        w_assert1(added);
        
        return true;
    } else {
        return false;
    }
}

template<class value>
u_int32_t page_evictioner_cart::multi_clock<value>::size_of(u_int32_t clock) {
    return _sizes[clock];
}
