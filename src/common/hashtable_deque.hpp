#ifndef __HASHTABLE_DEQUE_HPP
#define __HASHTABLE_DEQUE_HPP

#include <cstdint>
#include <unordered_map>
#include <memory>

#include "hashtable_deque_exceptions.hpp"

namespace zero::hashtable_dequeu {

/*!\class   HashtableDeque
 * \brief   Deque with Direct Access
 * \details Represents a deque of keys with direct access using the keys. It offers the usual deque semantics where
 *          entries are inserted either at the back or the front of the deque and where entries are removed either from
 *          the front or from the back of it. But it also offers the possibility to remove a specified element from
 *          somewhere within the deque. The data type of the entries is specified using a template parameter. Each value
 *          contained in the deque needs to be unique and inserts of duplicate keys are prevented.
 *          The computational complexity of the direct access as well as removal and insertion with deque semantics
 *          depends on the implementation of \c std::unordered_map, as this class is used for that. The space complexity
 *          also depends on the implementation of \c std::unordered_map where \c key has a size of the \c key template
 *          parameter and where \c T has double the size of the \c key template parameter.
 *
 * \note    Could also be implemented using \c Boost.MultiIndex .
 *
 * @tparam key_type    The data type of the entries stored in this data structure.
 * @tparam invalid_key This specifies an invalid \c key which can be used to mark that an element in the deque does not
 *                     have a previous or next element. It can also be used to mark that there is no back or front of
 *                     the deque when there is no deque. This should have the semantics of \c null for the specified
 *                     \c key template parameter therefore a natural choice for the case that \c key is a pointer would
 *                     be \c nullptr .
 *
 * \author Max Gilbert
 */
    template<class key_type, key_type invalid_key>
    class HashtableDeque {
    public:
        /*!\fn      HashtableDeque(key_type initialSize)
         * \brief   Constructor of a Deque with Direct Access
         * \details Creates a new Deque with Direct Access with an optional maximum size. If the \c initialSize is
         *          greater 0, the memory required for the specified number of keys is allocated during the creation to
         *          reduce the overhead due to allocation of memory.
         *
         * @param initialSize The maximum number of keys that can be managed by this Deque with Direct Access, or \c 0
         *                    (default) for a Deque with Direct Access with an unlimited capacity (depends on the number
         *                    of unique \c key values).
         */
        HashtableDeque(key_type initialSize = 0) :
                _directAccessDeque(initialSize ? std::make_unique<std::unordered_map<key_type, KeyPair>>(initialSize)
                                               : std::make_unique<std::unordered_map<key_type, KeyPair>>()),
                _back(invalid_key),
                _front(invalid_key) {};

        /*!\fn      ~HashtableDeque()
         * \brief   Destructor of a Deque with Direct Access
         * \details Destructs this Deque with Direct Access including the dynamically allocated memory used for the
         *          data.
         */
        ~HashtableDeque() {};

        /*!\fn      contains(const key_type& k) const noexcept
         * \brief   Entry with given key contained
         * \details Searches this deque for the given key and the return value gives information about if the key could
         *          be found.
         *
         * @param k The key that should be searched for in this deque.
         * @return  \c true if this deque contains an entry with \c k as key, \c false else.
         */
        bool contains(const key_type &k) const noexcept {
            return static_cast<bool>(_directAccessDeque->count(k));
        }

        /*!\fn      pushToBack(const key_type& k)
         * \brief   Add the key to the back of this deque
         * \details Adds an entry to the back of this deque.
         *
         * @param k                                       The key that is added to this deque.
         * @throws HashtableDequeAlreadyContainsException Thrown if the key was already contained in this deque.
         */
        void pushToBack(const key_type &k) {
            if (!_directAccessDeque->empty()) {
                auto oldSize = _directAccessDeque->size();
                w_assert1(_back != invalid_key);
                w_assert1((*_directAccessDeque)[_back]._next == invalid_key);

                if (_directAccessDeque->count(k)) {
                    throw HashtableDequeAlreadyContainsException<key_type, invalid_key>(_directAccessDeque->size(),
                                                                                        _back, _front, k);
                }
                (*_directAccessDeque)[k] = KeyPair(_back, invalid_key);
                (*_directAccessDeque)[_back]._next = k;
                _back = k;
                w_assert1(_directAccessDeque->size() == oldSize + 1);
            } else {
                w_assert1(_back == invalid_key);
                w_assert1(_front == invalid_key);

                (*_directAccessDeque)[k] = KeyPair(invalid_key, invalid_key);
                _back = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == 1);
            }
        }

        /*!\fn      pushToBack(key_type&& k)
         * \brief   Add the key to the back of this deque (with move semantics)
         * \details Adds an entry to the back of this deque using move semantics.
         *
         * @param k                                       The key that is added to this deque.
         * @throws HashtableDequeAlreadyContainsException Thrown if the key was already contained in this deque.
         */
        void pushToBack(key_type &&k) {
            if (!_directAccessDeque->empty()) {
                auto oldSize = _directAccessDeque->size();
                w_assert1(_back != invalid_key);
                w_assert1((*_directAccessDeque)[_back]._next == invalid_key);

                if (_directAccessDeque->count(k)) {
                    throw HashtableDequeAlreadyContainsException<key_type, invalid_key>(_directAccessDeque->size(),
                                                                                        _back, _front, k);
                }
                (*_directAccessDeque)[k] = KeyPair(_back, invalid_key);
                (*_directAccessDeque)[_back]._next = k;
                _back = k;
                w_assert1(_directAccessDeque->size() == oldSize + 1);
            } else {
                w_assert1(_back == invalid_key);
                w_assert1(_front == invalid_key);

                (*_directAccessDeque)[k] = KeyPair(invalid_key, invalid_key);
                _back = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == 1);
            }
        }

        /*!\fn      popFromFront(key_type& k)
         * \brief   Removes the next key from this deque
         * \details Removes an entry from the front of this deque.
         *
         * @param[out] k The key that was removed from the front of this queue.
         * @throws       HashtableDequeEmptyException Thrown if this deque was already empty.
         */
        void popFromFront(key_type &k) {
            if (_directAccessDeque->empty()) {
                throw HashtableDequeEmptyException<key_type, invalid_key>(_directAccessDeque->size(), _back, _front);
            } else if (_directAccessDeque->size() == 1) {
                w_assert1(_back == _front);
                w_assert1((*_directAccessDeque)[_front]._next == invalid_key);
                w_assert1((*_directAccessDeque)[_front]._previous == invalid_key);

                _directAccessDeque->erase(_front);
                k = _front;
                _front = invalid_key;
                _back = invalid_key;
                w_assert1(_directAccessDeque->size() == 0);
            } else {
                auto oldSize = _directAccessDeque->size();
                key_type oldFront = _front;
                KeyPair oldFrontEntry = (*_directAccessDeque)[_front];
                w_assert1(_back != _front);
                w_assert1(_back != invalid_key);

                _front = oldFrontEntry._next;
                (*_directAccessDeque)[oldFrontEntry._next]._previous = invalid_key;
                _directAccessDeque->erase(oldFront);
                w_assert1(_directAccessDeque->size() == oldSize - 1);
                k = oldFront;
            }
        }

        /*!\fn      pushToFront(const key_type& k)
         * \brief   Add the key to the front of this deque
         * \details Adds an entry to the front of this deque.
         *
         * @param k                                       The key that is added to this deque.
         * @throws HashtableDequeAlreadyContainsException Thrown if the key was already contained in this deque.
         */
        void pushToFront(const key_type &k) {
            if (!_directAccessDeque->empty()) {
                auto oldSize = _directAccessDeque->size();
                w_assert1(_back != invalid_key);
                w_assert1((*_directAccessDeque)[_back]._next == invalid_key);

                if (_directAccessDeque->count(k)) {
                    throw HashtableDequeAlreadyContainsException<key_type, invalid_key>(_directAccessDeque->size(),
                                                                                        _back, _front, k);
                }
                (*_directAccessDeque)[k] = KeyPair(invalid_key, _front);
                (*_directAccessDeque)[_front]._previous = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == oldSize + 1);
            } else {
                w_assert1(_back == invalid_key);
                w_assert1(_front == invalid_key);

                (*_directAccessDeque)[k] = KeyPair(invalid_key, invalid_key);
                _back = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == 1);
            }
        }

        /*!\fn      pushToFront(key_type&& k)
         * \brief   Add the key to the front of this deque (with move semantics)
         * \details Adds an entry to the front of this deque using move semantics.
         *
         * @param k                                       The key that is added to this deque.
         * @throws HashtableDequeAlreadyContainsException Thrown if the key was already contained in this deque.
         */
        void pushToFront(key_type &&k) {
            if (!_directAccessDeque->empty()) {
                auto oldSize = _directAccessDeque->size();
                w_assert1(_back != invalid_key);
                w_assert1((*_directAccessDeque)[_back]._next == invalid_key);

                if (_directAccessDeque->count(k)) {
                    throw HashtableDequeAlreadyContainsException<key_type, invalid_key>(_directAccessDeque->size(),
                                                                                        _back, _front, k);
                }
                (*_directAccessDeque)[k] = KeyPair(invalid_key, _front);
                (*_directAccessDeque)[_front]._previous = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == oldSize + 1);
            } else {
                w_assert1(_back == invalid_key);
                w_assert1(_front == invalid_key);

                (*_directAccessDeque)[k] = KeyPair(invalid_key, invalid_key);
                _back = k;
                _front = k;
                w_assert1(_directAccessDeque->size() == 1);
            }
        }

        /*!\fn      popFromBack(key_type& k)
         * \brief   Removes the next key from this deque
         * \details Removes an entry from the back of this deque.
         *
         * @param[out] k The key that was removed from the front of this queue.
         * @throws       HashtableDequeEmptyException Thrown if this deque was already empty.
         */
        void popFromBack(key_type &k) {
            if (_directAccessDeque->empty()) {
                throw HashtableDequeEmptyException<key_type, invalid_key>(_directAccessDeque->size(), _back, _front);
            } else if (_directAccessDeque->size() == 1) {
                w_assert1(_back == _front);
                w_assert1((*_directAccessDeque)[_front]._next == invalid_key);
                w_assert1((*_directAccessDeque)[_front]._previous == invalid_key);

                _directAccessDeque->erase(_front);
                k = _front;
                _front = invalid_key;
                _back = invalid_key;
                w_assert1(_directAccessDeque->size() == 0);
            } else {
                auto oldSize = _directAccessDeque->size();
                key_type oldBack = _back;
                KeyPair oldBackEntry = (*_directAccessDeque)[_back];
                w_assert1(_back != _front);
                w_assert1(_back != invalid_key);

                _back = oldBackEntry._previous;
                (*_directAccessDeque)[oldBackEntry._previous]._next = invalid_key;
                _directAccessDeque->erase(oldBack);
                w_assert1(_directAccessDeque->size() == oldSize - 1);
                k = oldBack;
            }
        }

        /*!\fn      remove(const key_type& k)
         * \brief   Removes a specific key from this deque
         * \details Removes the specified key \c k from this deque using the hash table over the deque entries, if the
         *          key \c k was contained in this deque.
         *
         * @param k                                    The key to remove from this deque.
         * @throws HashtableDequeNotContainedException Thrown if the key was not contained in this deque.
         */
        void remove(const key_type &k) {
            if (!_directAccessDeque->count(k)) {
                throw HashtableDequeNotContainedException<key_type, invalid_key>(_directAccessDeque->size(), _back,
                                                                                 _front, k);
            } else {
                auto old_size = _directAccessDeque->size();
                KeyPair old_key = (*_directAccessDeque)[k];
                if (old_key._next != invalid_key) {
                    (*_directAccessDeque)[old_key._next]._previous = old_key._previous;
                } else {
                    _back = old_key._previous;
                }
                if (old_key._previous != invalid_key) {
                    (*_directAccessDeque)[old_key._previous]._next = old_key._next;
                } else {
                    _front = old_key._next;
                }
                _directAccessDeque->erase(k);
                w_assert1(_directAccessDeque->size() == old_size - 1);
            }
        }

        /*!\fn      remove(key_type&& k)
         * \brief   Removes a specific key from this deque (with move semantic)
         * \details Removes the specified key \c k from this deque using the hash table over the deque entries and move
         *          semantics, if the key \c k was contained in this deque.
         *
         * @param k                                    The key to remove from this deque.
         * @throws HashtableDequeNotContainedException Thrown if the key was not contained in this deque.
         */
        void remove(key_type &&k) {
            if (!_directAccessDeque->count(k)) {
                throw HashtableDequeNotContainedException<key_type, invalid_key>(_directAccessDeque->size(), _back,
                                                                                 _front, k);
            } else {
                auto old_size = _directAccessDeque->size();
                KeyPair old_key = (*_directAccessDeque)[k];
                if (old_key._next != invalid_key) {
                    (*_directAccessDeque)[old_key._next]._previous = old_key._previous;
                } else {
                    _back = old_key._previous;
                }
                if (old_key._previous != invalid_key) {
                    (*_directAccessDeque)[old_key._previous]._next = old_key._next;
                } else {
                    _front = old_key._next;
                }
                _directAccessDeque->erase(k);
                w_assert1(_directAccessDeque->size() == old_size - 1);
            }
        }

        /*!\fn      length() const noexcept
         * \brief   Number of entries in this deque
         * \details Returns the number of entries (keys) that are contained in this deque.
         *
         * @return The number of entries contained in this deque.
         */
        inline uint64_t length() const noexcept {
            return _directAccessDeque->size();
        }

    private:
        /*!\class   KeyPair
         * \brief   A pair of keys for the implementation of a deque as a doubly-linked list
         * \details Instances of this class can be used to represent entries of a doubly-linked list which only stores
         *          the pointer without any other value.
         *
         * \author  Max Gilbert
         */
        class KeyPair {
        public:
            /*!\fn      KeyPair()
             * \brief   Constructor for an empty pair of keys
             * \details This constructor instantiates a key pair without setting the members \link _previous \endlink
             *          and \link _next \endlink .
             */
            KeyPair() {}

            /*!\fn      KeyPair(const key& previous, const key& next)
             * \brief   Constructor for a pair of keys with initial values
             * \details This constructor instantiates a key pair and initializes the members \link _previous \endlink
             *          and \link _next \endlink as specified.
             *
             * @param previous The initial value of \link _previous \endlink.
             * @param next     The initial value of \link _next \endlink.
             */
            KeyPair(const key_type &previous, const key_type &next) {
                this->_previous = previous;
                this->_next = next;
            }

            /*!\fn      ~KeyPair()
             * \brief   Destructor for a pair of keys
             * \details As this class does not allocate memory dynamically, this destructor does not do anything.
             */
            virtual ~KeyPair() {}

            /*!\var     _previous
             * \brief   The previous element of this element
             * \details The key of the previous element with regard to the deque order. The previous element is closer
             *          to the front of the deque. If this element represents the front of the deque, this member
             *          variable will contain an invalid key.
             */
            key_type _previous;

            /*!\var     _next
             * \brief   The next element of this element
             * \details The key of the next element with regard to the deque order. The next element is closer to the
             *          back of the deque. If this element represents the back of the deque, this member variable will
             *          contain an invalid key.
             */
            key_type _next;
        };

        /*!\var     _directAccessDeque
         * \brief   Maps from keys to their deque entry
         * \details Allows direct access to specific elements of the deque and stores the inner deque elements. Every
         *          access on deque elements happens through the interface of this data structure but this does not
         *          directly support the access with deque semantics. The \c key represents an deque entry and the
         *          \c keyPair which is mapped to that \c key stores the information about previous and next \c key in
         *          the deque.
         *
         * \see keyPair
         */
        std::unique_ptr<std::unordered_map<key_type, KeyPair>> _directAccessDeque;

        /*!\var     _back
         * \brief   Element at the back
         * \details Stores the \c key of the element at the back of the deque. This element does not have a next element
         *          but the previous element can be accesses using \link _directAccessDeque \endlink .
         */
        key_type _back;

        /*!\var     _front
         * \brief   Element at the front
         * \details Stores the \c key of the element at the front of the deque. This element does not have a previous
         *          element but the next element can be accesses using \link _directAccessDeque \endlink .
         */
        key_type _front;

    };

}

#endif // __HASHTABLE_DEQUE_HPP
