#include "page_evictioner_car.h"

#include "multi_clock_impl.h"
#include "hashtable_queue_impl.h"
#include <cmath>

page_evictioner_car::page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options)
        : page_evictioner_base(bufferpool, options)
{
    _clocks = new multi_clock<bf_idx, bool>(_bufferpool->_block_cnt, 2, 0);
    
    _b1 = new hashtable_queue<PageID>(1 | SWIZZLED_PID_BIT, _bufferpool->_block_cnt - 1);
    _b2 = new hashtable_queue<PageID>(1 | SWIZZLED_PID_BIT, PageID(std::ceil((_bufferpool->_block_cnt - 1) * (1.0 + EVICT_BATCH_RATIO))));
    
    _p = 0;
    _c = _bufferpool->_block_cnt - 1;
    
    _hand_movement = 0;
    
    DO_PTHREAD(pthread_mutex_init(&_lock, nullptr));
}

page_evictioner_car::~page_evictioner_car() {
    DO_PTHREAD(pthread_mutex_destroy(&_lock));
    
    delete(_clocks);
    
    delete(_b1);
    delete(_b2);
}

void page_evictioner_car::hit_ref(bf_idx /*idx*/) {}

void page_evictioner_car::unfix_ref(bf_idx idx) {
    _clocks->set(idx, true);
}

void page_evictioner_car::miss_ref(bf_idx b_idx, PageID pid) {
    DO_PTHREAD(pthread_mutex_lock(&_lock));
    if (!_b1->contains(pid) && !_b2->contains(pid)) {
        if (_clocks->size_of(T_1) + _b1->length() >= _c) {
            _b1->pop();
        } else if (_clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() >= 2 * (_c)) {
            _b2->pop();
        }
        w_assert0(_clocks->add_tail(T_1, b_idx));
        DBG5(<< "Added to T_1: " << b_idx << "; New size: " << _clocks->size_of(T_1) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        _clocks->set(b_idx, false);
    } else if (_b1->contains(pid)) {
        _p = std::min(_p + std::max<uint32_t>(uint32_t(1), (_b2->length() / _b1->length())), _c);
        _b1->remove(pid);
        w_assert0(_clocks->add_tail(T_2, b_idx));
        DBG5(<< "Added to T_2: " << b_idx << "; New size: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        _clocks->set(b_idx, false);
    } else {
        _p = std::max<int32_t>(int32_t(_p) - std::max<int32_t>(1, (_b1->length() / _b2->length())), 0);
        _b2->remove(pid);
        w_assert0(_clocks->add_tail(T_2, b_idx));
        DBG5(<< "Added to T_2: " << b_idx << "; New size: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
        _clocks->set(b_idx, false);
    }
    w_assert1(/*0 <= _clocks->size_of(T_1) + _clocks->size_of(T_2) &&*/ _clocks->size_of(T_1) + _clocks->size_of(T_2) <= _c);
    w_assert1(/*0 <= _clocks->size_of(T_1) + _b1->length() &&*/ _clocks->size_of(T_1) + _b1->length() <= _c);
    w_assert1(/*0 <= _clocks->size_of(T_2) + _b2->length() &&*/ _clocks->size_of(T_2) + _b2->length() <= 2 * (_c));
    w_assert1(/*0 <= _clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() &&*/ _clocks->size_of(T_1) + _clocks->size_of(T_2) + _b1->length() + _b2->length() <= 2 * (_c));
    DO_PTHREAD(pthread_mutex_unlock(&_lock));
}

void page_evictioner_car::used_ref(bf_idx idx) {
    hit_ref(idx);
}

void page_evictioner_car::dirty_ref(bf_idx /*idx*/) {}

void page_evictioner_car::block_ref(bf_idx /*idx*/) {}

void page_evictioner_car::swizzle_ref(bf_idx /*idx*/) {}

void page_evictioner_car::unbuffered(bf_idx idx) {
    _clocks->remove(idx);
}

bf_idx page_evictioner_car::pick_victim() {
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
        DBG3(<< "p = " << _p);
        DO_PTHREAD(pthread_mutex_lock(&_lock));
        if ((_clocks->size_of(T_1) >= std::max<uint32_t>(uint32_t(1), _p) || blocked_t_2 >= _clocks->size_of(T_2)) && blocked_t_1 < _clocks->size_of(T_1)) {
            bool t_1_head = false;
            bf_idx t_1_head_index = 0;
            _clocks->get_head(T_1, t_1_head);
            _clocks->get_head_index(T_1, t_1_head_index);
            w_assert1(t_1_head_index != 0);
            
            if (!t_1_head) {
                PageID evicted_pid;
                evicted_page = evict_page(t_1_head_index, evicted_pid);
                
                if (evicted_page) {
                    w_assert0(_clocks->remove_head(T_1, t_1_head_index));
                    _b1->push(evicted_pid);
                    DBG5(<< "Removed from T_1: " << t_1_head_index << "; New size: " << _clocks->size_of(T_1) << "; Free frames: " << _bufferpool->_approx_freelist_length);
    
                    DO_PTHREAD(pthread_mutex_unlock(&_lock));
                    return t_1_head_index;
                } else {
                    _clocks->move_head(T_1);
                    blocked_t_1++;
                    _hand_movement++;
                    DO_PTHREAD(pthread_mutex_unlock(&_lock));
                    continue;
                }
            } else {
                w_assert0(_clocks->set_head(T_1, false));
                
                _clocks->switch_head_to_tail(T_1, T_2, t_1_head_index);
                DBG5(<< "Removed from T_1: " << t_1_head_index << "; New size: " << _clocks->size_of(T_1) << "; Free frames: " << _bufferpool->_approx_freelist_length);
                DBG5(<< "Added to T_2: " << t_1_head_index << "; New size: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
                DO_PTHREAD(pthread_mutex_unlock(&_lock));
                continue;
            }
        } else if (blocked_t_2 < _clocks->size_of(T_2)) {
            bool t_2_head = false;
            bf_idx t_2_head_index = 0;
            _clocks->get_head(T_2, t_2_head);
            _clocks->get_head_index(T_2, t_2_head_index);
            w_assert1(t_2_head_index != 0);
            
            if (!t_2_head) {
                PageID evicted_pid;
                evicted_page = evict_page(t_2_head_index, evicted_pid);
                
                if (evicted_page) {
                    w_assert0(_clocks->remove_head(T_2, t_2_head_index));
                    _b2->push(evicted_pid);
                    DBG5(<< "Removed from T_2: " << t_2_head_index << "; New size: " << _clocks->size_of(T_2) << "; Free frames: " << _bufferpool->_approx_freelist_length);
    
                    DO_PTHREAD(pthread_mutex_unlock(&_lock));
                    return t_2_head_index;
                } else {
                    _clocks->move_head(T_2);
                    blocked_t_2++;
                    _hand_movement++;
                    DO_PTHREAD(pthread_mutex_unlock(&_lock));
                    continue;
                }
            } else {
                w_assert0(_clocks->set_head(T_2, false));
                
                _clocks->move_head(T_2);
                _hand_movement++;
                DO_PTHREAD(pthread_mutex_unlock(&_lock));
                continue;
            }
        } else {
            DO_PTHREAD(pthread_mutex_unlock(&_lock));
            return 0;
        }
    
        DO_PTHREAD(pthread_mutex_unlock(&_lock));
    }
    
    return 0;
}
