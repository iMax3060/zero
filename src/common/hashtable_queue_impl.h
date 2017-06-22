#ifndef ZERO_HASHTABLE_QUEUE_IMPL_H
#define ZERO_HASHTABLE_QUEUE_IMPL_H

#include "hashtable_queue.h"

#include "w_base.h"

template<class key>
bool hashtable_queue<key>::contains(key k) {
    return _direct_access_queue->count(k);
}

template<class key>
hashtable_queue<key>::hashtable_queue(key invalid_key) {
    _direct_access_queue = new std::unordered_map<key, key_pair>();
    _invalid_key = invalid_key;
    _back = _invalid_key;
    _front = _invalid_key;
}

template<class key>
hashtable_queue<key>::~hashtable_queue() {
    delete(_direct_access_queue);
    _direct_access_queue = nullptr;
}

template<class key>
bool hashtable_queue<key>::push(key k) {
    if (!_direct_access_queue->empty()) {
        auto old_size = _direct_access_queue->size();
        key old_back = _back;
        key_pair old_back_entry = (*_direct_access_queue)[old_back];
        w_assert1(old_back != _invalid_key);
        w_assert1(old_back_entry._next == _invalid_key);
        
        if (this->contains(k)) {
            return false;
        }
        (*_direct_access_queue)[k] = key_pair(old_back, _invalid_key);
        (*_direct_access_queue)[old_back]._next = k;
        _back = k;
        w_assert1(_direct_access_queue->size() == old_size + 1);
    } else {
        w_assert1(_back == _invalid_key);
        w_assert1(_front == _invalid_key);
        
        (*_direct_access_queue)[k] = key_pair(_invalid_key, _invalid_key);
        _back = k;
        _front = k;
        w_assert1(_direct_access_queue->size() == 1);
    }
    return true;
}

template<class key>
bool hashtable_queue<key>::pop() {
    if (_direct_access_queue->empty()) {
        return false;
    } else if (_direct_access_queue->size() == 1) {
        w_assert1(_back == _front);
        w_assert1((*_direct_access_queue)[_front]._next == _invalid_key);
        w_assert1((*_direct_access_queue)[_front]._previous == _invalid_key);
        
        _direct_access_queue->erase(_front);
        _front = _invalid_key;
        _back = _invalid_key;
        w_assert1(_direct_access_queue->size() == 0);
    } else {
        auto old_size = _direct_access_queue->size();
        key old_front = _front;
        key_pair old_front_entry = (*_direct_access_queue)[_front];
        w_assert1(_back != _front);
        w_assert1(_back != _invalid_key);
        
        _front = old_front_entry._next;
        (*_direct_access_queue)[old_front_entry._next]._previous = _invalid_key;
        _direct_access_queue->erase(old_front);
        w_assert1(_direct_access_queue->size() == old_size - 1);
    }
    return true;
}

template<class key>
bool hashtable_queue<key>::remove(key k) {
    if (!this->contains(k)) {
        return false;
    } else {
        auto old_size = _direct_access_queue->size();
        key_pair old_key = (*_direct_access_queue)[k];
        if (old_key._next != _invalid_key) {
            (*_direct_access_queue)[old_key._next]._previous = old_key._previous;
        } else {
            _back = old_key._previous;
        }
        if (old_key._previous != _invalid_key) {
            (*_direct_access_queue)[old_key._previous]._next = old_key._next;
        } else {
            _front = old_key._next;
        }
        _direct_access_queue->erase(k);
        w_assert1(_direct_access_queue->size() == old_size - 1);
    }
    return true;
}

template<class key>
uint64_t hashtable_queue<key>::length() {
    return _direct_access_queue->size();
}

#endif //ZERO_HASHTABLE_QUEUE_IMPL_H
