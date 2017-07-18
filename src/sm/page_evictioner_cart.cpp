#include "page_evictioner_cart.h"

#include "multi_clock.hpp"
#include "hashtable_queue.hpp"

page_evictioner_cart::page_evictioner_cart(bf_tree_m *bufferpool, const sm_options &options)
        : page_evictioner_base(bufferpool, options)
{
    _clocks = new multi_clock<bf_idx, referenced_filter, 2, 0>(_bufferpool->_block_cnt);
    
    _b1 = new hashtable_queue<PageID, 1 | SWIZZLED_PID_BIT>();
    _b2 = new hashtable_queue<PageID, 1 | SWIZZLED_PID_BIT>();
    
    _p = _q = _n_s = _n_l = 0;
    _c = _bufferpool->_block_cnt - 1;
    
    _hand_movement = 0;
}

page_evictioner_cart::~page_evictioner_cart() {
    delete(_clocks);
    
    delete(_b1);
    delete(_b2);
}

void page_evictioner_cart::hit_ref(bf_idx /*idx*/) {}

void page_evictioner_cart::unfix_ref(bf_idx idx) {
    _lock.acquire_read();
    (*_clocks)[idx]._referenced = true;
    _lock.release_read();
}

void page_evictioner_cart::miss_ref(bf_idx b_idx, PageID pid) {
    _lock.acquire_write();
    
    if (!_b1->contains(pid) && !_b2->contains(pid)) {
        if (_b1->length() + _b2->length() >= _c + 1
            && (_b1->length() > std::max<uint32_t>(uint32_t(0), _q) || _b2->length() == 0)) {
            _b1->pop();
            DBG5(<< "Removed from B_1: " << pid << "; |B_1|: " << _b1->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
        } else if (_b1->length() + _b2->length() >= _c + 1) {
            _b2->pop();
            DBG5(<< "Removed from B_2: " << pid << "; |B_2|: " << _b2->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
        }
        _clocks->add_tail(T_1, b_idx);
        DBG5(<< "Added to T_1: " << b_idx << "; |T_1|: " << _clocks->size_of(T_1) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        (*_clocks)[b_idx]._referenced = false;
        (*_clocks)[b_idx]._filter = S;
        _n_s = _n_s + 1;
    } else if (_b1->contains(pid)) {
        _p = std::min<uint32_t>(_p + std::max<uint32_t>(uint32_t(1), uint32_t(_n_s / _b1->length())), _c);
        _b1->remove(pid);
        DBG5(<< "Removed from B_1: " << pid << "; |B_1|: " << _b1->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
        _clocks->add_tail(T_1, b_idx);
        DBG5(<< "Added to T_2: " << b_idx << "; |T_2|: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        (*_clocks)[b_idx]._referenced = false;
        (*_clocks)[b_idx]._filter = L;
        _n_l = _n_l + 1;
    } else {
        _p = std::max<int32_t>(int32_t(_p) - std::max<int32_t>(uint32_t(1), uint32_t(_n_l / _b2->length())), uint32_t(0));
        _b2->remove(pid);
        DBG5(<< "Removed from B_2: " << pid << "; |B_2|: " << _b2->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
        _clocks->add_tail(T_1, b_idx);
        DBG5(<< "Added to T_2: " << b_idx << "; |T_2|: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        (*_clocks)[b_idx]._referenced = false;
        (*_clocks)[b_idx]._filter = L;
        _n_l = _n_l + 1;
        
        if (_clocks->size_of(T_1) + _clocks->size_of(T_2) + _b2->length() - _n_s >= _c) {
            _q = std::min<int32_t>(_q + 1, int32_t(2 * _c) - int32_t(_clocks->size_of(T_1)));
        }
    }
    
    w_assert1(/*0 <= _clocks->size_of(T_1) + _clocks->size_of(T_2) &&*/ _clocks->size_of(T_1) + _clocks->size_of(T_2) <= _c);
    w_assert1(/*0 <= _clocks->size_of(T_2) + _b2->length() &&*/ _clocks->size_of(T_2) + _b2->length() <= _c);
    w_assert1(/*0 <= _clocks->size_of(T_1) + _b1->length() &&*/ _clocks->size_of(T_1) + _b1->length() <= 2 * (_c));
    w_assert1(/*0 <= _clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() &&*/ _clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() <= 2 * (_c));
    
    _lock.release_write();
}

void page_evictioner_cart::used_ref(bf_idx /*idx*/) {}

void page_evictioner_cart::dirty_ref(bf_idx /*idx*/) {}

void page_evictioner_cart::block_ref(bf_idx /*idx*/) {}

void page_evictioner_cart::swizzle_ref(bf_idx /*idx*/) {}

void page_evictioner_cart::unbuffered(bf_idx idx) {
    _lock.acquire_write();
    _clocks->remove(idx);
    _lock.release_write();
}

bf_idx page_evictioner_cart::pick_victim() {
    
    bool evicted_page = false;
    bf_idx blocked_t_1 = 0;
    bf_idx blocked_t_2 = 0;
    
    while (!evicted_page) {
        if (_hand_movement >= _c) {
            _bufferpool->get_cleaner()->wakeup(false);
            DBG3(<< "Run Page_Cleaner ...");
            _hand_movement = 0;
        }
        uint32_t iterations = (blocked_t_1 + blocked_t_2) / _c;
        if ((blocked_t_1 + blocked_t_2) % _c == 0 && (blocked_t_1 + blocked_t_2) > 0) {
            DBG1(<< "Iterated " << iterations << "-times in CAR's pick_victim().");
        }
        w_assert1(iterations < 3);
        _lock.acquire_write();

        bf_idx size_t_1_before = _clocks->size_of(T_1);
        bf_idx size_t_2_before = _clocks->size_of(T_2);

        referenced_filter t_2_head = referenced_filter(false, S);
        bf_idx t_2_head_index = 0;
        try {
            t_2_head = _clocks->get_head(T_2);
        } catch (multi_clock_empty_exception<bf_idx, referenced_filter, 2, 0> e) {}

        while (t_2_head._referenced && size_t_2_before) {
            (*_clocks)[t_2_head_index]._referenced = false;
            t_2_head_index = _clocks->switch_head_to_tail(T_2, T_1);
            DBG5(<< "Moved from T_2 to T_1: " << t_2_head_index << "; |T_1|: " << _clocks->size_of(T_1) << "; |T_2|: " << _clocks->size_of(T_2));

            if (_clocks->size_of(T_1) + _clocks->size_of(T_2) + _b2->length() - _n_s >= _c) {
                _q = std::min<int32_t>(_q + 1, int32_t(2 * _c) - int32_t(_clocks->size_of(T_1)));
            }

            try {
                t_2_head = _clocks->get_head(T_2);
            } catch (multi_clock_empty_exception<bf_idx, referenced_filter, 2, 0> e) {
                break;
            }
        }
        w_assert1(size_t_1_before + size_t_2_before == _clocks->size_of(T_1) + _clocks->size_of(T_2));

        size_t_1_before = _clocks->size_of(T_1);
        size_t_2_before = _clocks->size_of(T_2);

        referenced_filter t_1_head = referenced_filter(false, S);
        bf_idx t_1_head_index = 0;
        try {
            t_1_head = _clocks->get_head(T_1);
            t_1_head_index = _clocks->get_head_index(T_1);
        } catch (multi_clock_empty_exception<bf_idx, referenced_filter, 2, 0> e) {}

        while ((t_1_head._filter == L || t_1_head._referenced) && size_t_1_before) {
            if (t_1_head._referenced) {
                (*_clocks)[t_1_head_index]._referenced = false;
                _clocks->move_head(T_1);
                _hand_movement++;
                DBG5(<< "Moved hand of T_1; |T_1|: " << _clocks->size_of(T_1) << "; |T_2|: " << _clocks->size_of(T_2));

                if (_clocks->size_of(T_1) >= std::min<uint32_t>(_p + 1, _b1->length()) && t_1_head._filter == S) {
                    (*_clocks)[t_1_head_index]._filter = L;
                    _n_s = _n_s - 1;
                    _n_l = _n_l + 1;
                }
            } else {
                (*_clocks)[t_1_head_index]._referenced = false;
                t_1_head_index = _clocks->switch_head_to_tail(T_1, T_2);
                DBG5(<< "Moved from T_1 to T_2: " << t_1_head_index << "; |T_1|: " << _clocks->size_of(T_1) << "; |T_2|: " << _clocks->size_of(T_2));
                _q = std::max<int32_t>(int32_t(_q) - int32_t(1), int32_t(_c) - int32_t(_clocks->size_of(T_1)));
            }

            try {
                t_1_head = _clocks->get_head(T_1);
                t_1_head_index = _clocks->get_head_index(T_1);
            } catch (multi_clock_empty_exception<bf_idx, referenced_filter, 2, 0> e) {
                break;
            }
        }
        w_assert1(size_t_1_before == _clocks->size_of(T_1));
        w_assert1(size_t_2_before == _clocks->size_of(T_2));
        
        if ((_clocks->size_of(T_1) >= std::max<uint32_t>(uint32_t(1), _p) || blocked_t_2 > _clocks->size_of(T_2))
            && blocked_t_1 <= _clocks->size_of(T_1)) {
            t_1_head_index = _clocks->get_head_index(T_1);
            
            PageID evicted_pid;
            evicted_page = evict_page(t_1_head_index, evicted_pid);
            
            if (evicted_page) {
                t_1_head_index = _clocks->remove_head(T_1);
                _b1->push(evicted_pid);
                DBG5(<< "Added to B_1: " << evicted_pid << "; |B_1|: " << _b1->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
                
                _n_s = _n_s - 1;
                _lock.release_write();
                return t_1_head_index;
            } else {
                blocked_t_1 = blocked_t_1 + 1;
                _clocks->move_head(T_1);
                _hand_movement++;
                DBG5(<< "Moved hand of T_1; |T_1|: " << _clocks->size_of(T_1) << "; |T_2|: " << _clocks->size_of(T_2));
                
                _lock.release_write();
                continue;
            }
        } else if (blocked_t_2 <= _clocks->size_of(T_2)) {
            t_2_head_index = _clocks->get_head_index(T_1);
            
            PageID evicted_pid;
            evicted_page = evict_page(t_2_head_index, evicted_pid);
            
            if (evicted_page) {
                t_2_head_index = _clocks->remove_head(T_1);
                DBG5(<< "Removed from T_2: " << t_1_head_index << "; |T_2|: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
                _b2->push(evicted_pid);
                DBG5(<< "Added to B_2: " << evicted_pid << "; |B_2|: " << _b2->length() << "; Free frames: " << _bufferpool->_approx_freelist_length);
                
                _n_l = _n_l - 1;
                _lock.release_write();
                return t_2_head_index;
            } else {
                blocked_t_2 = blocked_t_2 + 1;
                _clocks->move_head(T_2);
                _hand_movement++;
                DBG5(<< "Moved hand of T_2; |T_1|: " << _clocks->size_of(T_1) << "; |T_2|: " << _clocks->size_of(T_2));
                
                _lock.release_write();
                continue;
            }
        } else {
            _lock.release_write();
            return 0;
        }
    }
    return 0;           // Suppress compiler warning about missing return statement!
}
