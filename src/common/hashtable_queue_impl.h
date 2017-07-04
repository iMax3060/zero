#ifndef ZERO_HASHTABLE_QUEUE_IMPL_H
#define ZERO_HASHTABLE_QUEUE_IMPL_H

#include "hashtable_queue.h"

#include "w_base.h"

template<class key>
hashtable_queue<key>::hashtable_queue(key invalid_key, key init_size) {
    if (init_size) {
        _direct_access_queue = new std::unordered_map<key, key_pair>(init_size);
    } else {
        _direct_access_queue = new std::unordered_map<key, key_pair>();
    }
    _invalid_key = invalid_key;
    _back = _invalid_key;
    _front = _invalid_key;
}

template<class key>
hashtable_queue<key>::~hashtable_queue() {
    delete(_direct_access_queue);
}

template<class key>
bool hashtable_queue<key>::contains(key k) {
    return _direct_access_queue->count(k);
}

template<class key>
void hashtable_queue<key>::push(key k) throw (hashtable_queue_already_contains_exception<key>) {
    if (!_direct_access_queue->empty()) {
        auto old_size = _direct_access_queue->size();
        w_assert1(_back != _invalid_key);
        w_assert1((*_direct_access_queue)[_back]._next == _invalid_key);
        
        if (this->contains(k)) {
            throw hashtable_queue_already_contains_exception<key>(this, _direct_access_queue->size(), _back, _front, k);
        }
        (*_direct_access_queue)[k] = key_pair(_back, _invalid_key);
        (*_direct_access_queue)[_back]._next = k;
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
}

template<class key>
void hashtable_queue<key>::pop() throw (hashtable_queue_empty_exception<key>) {
    if (_direct_access_queue->empty()) {
        throw hashtable_queue_empty_exception<key>(this, _direct_access_queue->size(), _back, _front);
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
}

template<class key>
bool hashtable_queue<key>::remove(key k) throw (hashtable_queue_not_contained_exception<key>) {
    if (!this->contains(k)) {
        throw hashtable_queue_not_contained_exception<key>(this, _direct_access_queue->size(), _back, _front, k);
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
}

template<class key>
uint64_t hashtable_queue<key>::length() {
    return _direct_access_queue->size();
}

#endif //ZERO_HASHTABLE_QUEUE_IMPL_H
