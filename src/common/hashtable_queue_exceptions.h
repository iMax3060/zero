#ifndef ZERO_HASHTABLE_QUEUE_EXCEPTIONS_H
#define ZERO_HASHTABLE_QUEUE_EXCEPTIONS_H

#include <exception>
#include <sstream>
#include <string>

template<class key> class hashtable_queue;

template<class key>
class hashtable_queue_exception : std::exception {
protected:
    hashtable_queue<key>*   _pointer;
    uint64_t                _size;
    key                     _back;
    key                     _front;

    
public:
    hashtable_queue_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front)
            : _pointer(pointer), _size(size), _back(back), _front(front) {};
    
    virtual ~hashtable_queue_exception() {};
    
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "An unknown exception happened in the hashtable_queue instance " << _pointer << ".";
        return oss.str().c_str();
    };
    
    virtual const char* details() const {
        std::ostringstream oss;
        oss << "&hashtable_queue = " << _pointer << ", hashtable_queue.size() = " << _size
            << "hashtable_queue._back = " << _back << "hashtable_queue._front = " << _front;
        return oss.str().c_str();
    };
    
    key getBack() {
        return _back;
    };
    
    key getFront() {
        return _front;
    };
    
};

template<class key>
class hashtable_queue_already_contains_exception : public hashtable_queue_exception<key> {
private:
    key _duplicate;

    
public:
    hashtable_queue_already_contains_exception(hashtable_queue<key>* pointer, uint64_t size, key back,
                                               key front, key duplicate)
            : hashtable_queue_exception<key>(pointer, size, back, front), _duplicate(duplicate) {};
    
    virtual ~hashtable_queue_already_contains_exception() {};
    
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << _duplicate << " was tried to be inserted into the hashtable_queue instance "
                         << this->_pointer << ", but it was already contained in there.";
        return oss.str().c_str();
    };
    
    virtual const char* details() const {
        std::ostringstream oss;
        oss << hashtable_queue_exception<key>::details() << "dupicate = " << _duplicate;
        return oss.str().c_str();
    };
    
    key getDuplicate() {
        return _duplicate;
    };
    
};

template<class key>
class hashtable_queue_empty_exception : public hashtable_queue_exception<key> {
public:
    using hashtable_queue_exception<key>::hashtable_queue_exception;
    
    virtual ~hashtable_queue_empty_exception() {};
    
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The hashtable_queue instance " << this->_pointer << " was already empty.";
        return oss.str().c_str();
    };
    
};

template<class key>
class hashtable_queue_not_contained_exception : public hashtable_queue_exception<key> {
private:
    key _requested;
    
    
public:
    hashtable_queue_not_contained_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front, key requested)
            : hashtable_queue_exception<key>(pointer, size, back, front), _requested(requested) {};
    
    virtual ~hashtable_queue_not_contained_exception() {};
    
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The hashtable_queue instance " << this->_pointer << " doesn't contain key " << _requested << ".";
        return oss.str().c_str();
    };
    
    virtual const char* details() const {
        std::ostringstream oss;
        oss << hashtable_queue_exception<key>::details() << "not contained = " << _requested;
        return oss.str().c_str();
    };
    
};

#endif //ZERO_HASHTABLE_QUEUE_EXCEPTIONS_H
