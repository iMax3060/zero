#ifndef ZERO_HASHTABLE_QUEUE_H
#define ZERO_HASHTABLE_QUEUE_H

#include <cstdint>
#include <unordered_map>
#include "hashtable_queue_exceptions.h"

/*!\class   hashtable_queue
 * \brief   Queue with Direct Access
 * \details Represents a queue of keys with direct access using the keys. It
 *          offers the usual queue semantics where entries are inserted at
 *          the back of the queue and where entries are removed from the
 *          front of it. But it also offers the possibility to remove a
 *          specified element from somewhere within the queue. The data type
 *          of the entries is specified using the template parameter. Each
 *          value contained in the queue needs to be unique and inserts of
 *          duplicate keys are prevented.
 *          The computational complexity of the direct access as well as
 *          removal and insertion with queue semantics depends on the
 *          implementation of std::unordered_map, as this class is used for
 *          that. The space complexity also depends on the implementation of
 *          std::unordered_map where \c Key has a size of the \c key
 *          template parameter and where \c T has double the size of the
 *          \c key template parameter.
 *
 * \note    Could also be implemented using \c Boost.MultiIndex .
 *
 * @tparam key          The data type of the entries stored in this data structure.
 * @tparam _invalid_key This specifies an invalid \c key which can be used to mark
 *                      that an element in the queue doesn't have a previous or
 *                      next element. It can also be used to mark that there is no
 *                      back or front of the queue when there is no queue. This
 *                      should have the semantics of \c null for the specified
 *                      \c key template parameter therefore a natural choice of a
 *                      this for the case that \c key is a pointer would be \c null.
 *
 * \author Max Gilbert
 */
template<class key, key _invalid_key>
class hashtable_queue {
private:
    /*!\class   key_pair
     * \brief   A pair of keys for the implementation of a queue as a
     *          doubly-linked list.
     * \details Instances of this class can be used to represent entries of
     *          a doubly-linked list which only stores the pointer without
     *          any other value.
     *
     * \author  Max Gilbert
     */
    class key_pair {
    public:
        /*!\fn      key_pair()
         * \brief   Constructor for an empty pair of keys
         * \details This constructor instantiates a \link key_pair \endlink without setting
         *          the members \link _previous \endlink and \link _next \endlink.
         */
        key_pair() {}
        
        /*!\fn      key_pair(key previous, key next)
         * \brief   Constructor a pair of keys with initial values
         * \details This constructor instantiates a \link key_pair \endlink and
         *          initializes the members \link _previous \endlink and
         *          \link _next \endlink as specified.
         *
         * @param previous The initial value of \link _previous \endlink.
         * @param next     The initial value of \link _next \endlink.
         */
        key_pair(key previous, key next) {
            this->_previous = previous;
            this->_next = next;
        }
        
        /*!\fn      ~key_pair()
         * \brief   Destructor of a pair of keys
         * \details As this class doesn't allocate memory dynamically, this destructor
         *          doesn't do anything.
         */
        virtual ~key_pair() {}
        
        /*!\var     _previous
         * \brief   The previous element of this element
         * \details The key of the previous element with regard to the queue order.
         *          The previous element is closer to the front of the queue and
         *          was therefore inserted earlier and will get removed later. If
         *          this element represents the front of the queue, this member
         *          variable will contain an invalid key.
         */
        key     _previous;
        
        /*!\var     _next
         * \brief   The next element of this element
         * \details The key of the next element with regard to the queue order.
         *          The next element is closer to the back of the queue and
         *          was therefore inserted later and will get removed earlier. If
         *          this element represents the back of the queue, this member
         *          variable will contain an invalid key.
         */
        key     _next;
    };
    
    /*!\var     _direct_access_queue
     * \brief   Maps from keys to their queue entry
     * \details Allows direct access to specific elements of the queue and stores
     *          the inner queue elements. Every access on queue elements happens
     *          through the interface of this data structure but this doesn't
     *          directly support the access with queue semantics.
     *          The \c key represents an queue entry and the \c key_pair
     *          which is mapped to that \c key stores the information about
     *          previous and next \c key in the queue.
     *
     * \see key_pair
     */
    std::unordered_map<key, key_pair>*      _direct_access_queue;
    
    /*!\var     _back
     * \brief   Element at the back
     * \details Stores the \c key of the element at the back of the queue. This
     *          element was inserted most recently and it will be removed the furthest
     *          in the future (regarding queue semantics). This element doesn't have
     *          a next element but the previous element can be accesses using
     *          \link _direct_access_queue \endlink.
     */
    key                                     _back;
    
    /*!\var     _front
     * \brief   Element at the front
     * \details Stores the \c key of the element at the front of the queue. This
     *          element was inserted least recently and it will be removed next
     *          (regarding queue semantics). This element doesn't have a previous
     *          element but the next element can be accesses using
     *          \link _direct_access_queue \endlink.
     */
    key                                     _front;


public:
    /*!\fn      hashtable_queue(key invalid_key)
     * \brief   Constructor of a Queue with Direct Access
     * \details Creates a new instance of \link hashtable_queue \endlink with an optional
     *          maximum size. If the \c max_size is greater 0, the memory required for the
     *          specified number of keys is allocated during the creation to reduce the
     *          overhead due to allocation of memory.
     *
     * @param init_size   The maximum number of keys that can be managed by this
     *                    \link hashtable_queue \endlink, or \c 0 (default) for a
     *                    \link hashtable_queue \endlink with an unlimited capacity (depends
     *                    on the number of unique \c key values).
     */
    hashtable_queue(key init_size = 0);
    
    /*!\fn      ~hashtable_queue()
     * \brief   Destructor of a Queue with Direct Access
     * \details Destructs this instance of \link hashtable_queue \endlink including the
     *          dynamically allocated memory used for the data.
     */
    virtual          ~hashtable_queue();
    
    /*!\fn      contains(key k)
     * \brief   Entry with given key contained
     * \details Searches the Queue with Direct Access for the given key and the return value
     *          gives information about if the key could be found.
     *
     * @param k The key that should be searched for in the \link hashtable_queue \endlink.
     * @return  \c true if this \link hashtable_queue \endlink contains an entry with
     *          \c k as key, \c false else.
     */
    bool             contains(key k);
    
    /*!\fn      push(key k)
     * \brief   Add the key to the queue
     * \details Adds an entry to the back of the queue. Every entry that was added to the
     *          queue before will be removed from the queue before \c k.
     *
     * @param k The key that is added to the queue.
     * @throws hashtable_queue_already_contains_exception Thrown if the key was already
     *                                                    contained in the queue.
     */
    void push(key k) throw (hashtable_queue_already_contains_exception<key, _invalid_key>);
    
    /*!\fn      pop()
     * \brief   Removes the next key from the queue
     * \details Removes an entry from the front of the queue. The removed entry was the
     *          entry that was added the furthest in the past.
     *
     * @throws hashtable_queue_empty_exception Thrown if the queue was already empty.
     */
    void pop() throw (hashtable_queue_empty_exception<key, _invalid_key>);
    
    /*!\fn      remove(key k)
     * \brief   Removes a specific key from the queue
     * \details Removes the specified key \c k from the queue using the hash table over
     *          the queue entries if the key \c k was contained in the queue. The entry
     *          behind the key \c k (inserted immediately after it) will now be removed
     *          after the entry that was in front of \c k is removed.
     *
     * @param k The key to remove from the queue.
     * @throws hashtable_queue_not_contained_exception Thrown if the key was not contained
     *                                                 in the queue.
     */
    bool             remove(key k) throw (hashtable_queue_not_contained_exception<key, _invalid_key>);
    
    /*!\fn      length()
     * \brief   Number of entries in the queue
     * \details Returns the number of entries (keys) that are contained in the queue.
     *
     * @return The number of entries contained in the queue.
     */
    inline uint64_t length();
};

#endif //ZERO_HASHTABLE_QUEUE_H
