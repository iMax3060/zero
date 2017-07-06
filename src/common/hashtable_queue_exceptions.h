#ifndef ZERO_HASHTABLE_QUEUE_EXCEPTIONS_H
#define ZERO_HASHTABLE_QUEUE_EXCEPTIONS_H

#include <exception>
#include <sstream>
#include <string>

template<class key> class hashtable_queue;

/*!\class   hashtable_queue_exception
 * \brief   Exception Thrown in \link hashtable_queue \endlink
 * \details Represents an arbitrary exception that can be thrown in instances of
 *          \link hashtable_queue \endlink . Every exception that is specific for the
 *          \link hashtable_queue \endlink have to inherit from this class.
 *
 * @tparam key The data type of the entries stored in the \link hashtable_queue \endlink
 *             where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key>
class hashtable_queue_exception : public std::exception {
protected:
    /*!\var     _pointer
     * \brief   The \link hashtable_queue \endlink
     * \details A pointer to the \link hashtable_queue \endlink where this exception was
     *          thrown.
     */
    const hashtable_queue<key>*   _pointer;
    
    /*!\var     _size
     * \brief   The Size of the \link hashtable_queue \endlink
     * \details The number of entries of the \link hashtable_queue \endlink where this
     *          exception was thrown at the time when this exception happened.
     */
    const uint64_t                _size;
    
    /*!\var     _back
     * \brief   The Back of the \link hashtable_queue \endlink
     * \details The value of the back of the \link hashtable_queue \endlink where this
     *          exception was thrown at the time when this exception happened.
     */
    const key                     _back;
    
    /*!\var     _front
     * \brief   The Front of the \link hashtable_queue \endlink
     * \details The value of the front of the \link hashtable_queue \endlink where this
     *          exception was thrown at the time when this exception happened.
     */
    const key                     _front;

    
public:
    /*!\fn      hashtable_queue_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front)
     * \brief   Constructor of an Exception Thrown in \link hashtable_queue \endlink
     * \details This constructor instantiates an \c hashtable_queue_exception with all
     *          its variables.
     *
     * @param pointer See \link _pointer \endlink .
     * @param size    See \link _size \endlink .
     * @param back    See \link _back \endlink .
     * @param front   See \link _front \endlink .
     */
    hashtable_queue_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front)
            : _pointer(pointer), _size(size), _back(back), _front(front) {};
    
    /*!\fn      ~hashtable_queue_exception()
     * \brief   Destructor of an Exception Thrown in \link hashtable_queue \endlink
     * \details As this class doesn't allocate memory dynamically, this destructor
     *          doesn't do anything.
     */
    virtual ~hashtable_queue_exception() {};
    
    /*!\fn      what()
     * \brief   What Caused the Exception Happened?
     * \details Returns description about what caused this exception to be thrown.
     *
     * @return A description about what caused this exception to be thrown.
     */
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "An unknown exception happened in the hashtable_queue instance " << _pointer << ".";
        return oss.str().c_str();
    };
    
    /*!\fn      details()
     * \brief   Details About the Exceptional State
     * \details Returns all known details of the state that caused this exception to be thrown.
     *
     * @return A description about the state that caused this exception to be thrown.
     */
    virtual const char* details() const {
        std::ostringstream oss;
        oss << "&hashtable_queue = " << _pointer << ", hashtable_queue.size() = " << _size
            << "hashtable_queue._back = " << _back << "hashtable_queue._front = " << _front;
        return oss.str().c_str();
    };
    
};

/*!\class   hashtable_queue_already_contains_exception
 * \brief   Exception Thrown When an Entry was Already Contained in an \link hashtable_queue \endlink
 * \details Represents an exception that can be thrown in instances of \link hashtable_queue \endlink
 *          when an entry was unexpectedly already contained in the queue.
 *
 * @tparam key The data type of the entries stored in the \link hashtable_queue \endlink where this
 *             exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key>
class hashtable_queue_already_contains_exception : public hashtable_queue_exception<key> {
private:
    /*!\var     _duplicate
     * \brief   The Unexpected Entry
     * \brief   The value of the entry that was unexpectedly already contained.
     */
    const key _duplicate;

    
public:
    /*!\fn      hashtable_queue_already_contains_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front, key duplicate)
     * \brief   Constructor of an Exception Thrown When an Entry was Already Contained in an
     *          \link hashtable_queue \endlink
     * \details This constructor instantiates an \c hashtable_queue_already_contains_exception with all
     *          its variables.
     *
     * @param pointer   See \link _pointer \endlink .
     * @param size      See \link _size \endlink .
     * @param back      See \link _back \endlink .
     * @param front     See \link _front \endlink .
     * @param duplicate See \link _duplicate \endLink .
     */
    hashtable_queue_already_contains_exception(hashtable_queue<key>* pointer, uint64_t size, key back,
                                               key front, key duplicate)
            : hashtable_queue_exception<key>(pointer, size, back, front), _duplicate(duplicate) {};
    
    /*!\fn      ~hashtable_queue_already_contains_exception()
     * \brief   Destructor of an Exception Thrown When an Entry was Already Contained in an
     *          \link hashtable_queue \endlink
     * \details As this class doesn't allocate memory dynamically, this destructor
     *          doesn't do anything.
     */
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
    
};

/*!\class   hashtable_queue_empty_exception
 * \brief   Exception Thrown When an \link hashtable_queue \endlink is Empty
 * \details Represents an exception that can be thrown in instances of \link hashtable_queue \endlink
 *          when it is unexpectedly empty.
 *
 * @tparam key The data type of the entries stored in the \link hashtable_queue \endlink where this
 *             exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key>
class hashtable_queue_empty_exception : public hashtable_queue_exception<key> {
public:
    using hashtable_queue_exception<key>::hashtable_queue_exception;
    
    /*!\fn      ~hashtable_queue_empty_exception()
     * \brief   Destructor of an Exception Thrown When an \link hashtable_queue \endlink is Empty
     * \details As this class doesn't allocate memory dynamically, this destructor
     *          doesn't do anything.
     */
    virtual ~hashtable_queue_empty_exception() {};
    
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The hashtable_queue instance " << this->_pointer << " was already empty.";
        return oss.str().c_str();
    };
    
};

/*!\class   hashtable_queue_not_contained_exception
 * \brief   Exception Thrown When an Entry was not Already Contained in an \link hashtable_queue \endlink
 * \details Represents an exception that can be thrown in instances of \link hashtable_queue \endlink
 *          when an entry was unexpectedly not already contained in the queue.
 *
 * @tparam key The data type of the entries stored in the \link hashtable_queue \endlink where this
 *             exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key>
class hashtable_queue_not_contained_exception : public hashtable_queue_exception<key> {
private:
    /*!\var     _requested
     * \brief   The Expected Entry
     * \brief   The value of the entry that was unexpectedly not already contained.
     */
    const key _requested;
    
    
public:
    /*!\fn      hashtable_queue_not_contained_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front, key requested)
     * \brief   Constructor of an Exception Thrown When an Entry was not Already Contained in an
     *          \link hashtable_queue \endlink
     * \details This constructor instantiates an \c hashtable_queue_not_contained_exception with all
     *          its variables.
     *
     * @param pointer   See \link _pointer \endlink .
     * @param size      See \link _size \endlink .
     * @param back      See \link _back \endlink .
     * @param front     See \link _front \endlink .
     * @param requested See \link _requested \endLink .
     */
    hashtable_queue_not_contained_exception(hashtable_queue<key>* pointer, uint64_t size, key back, key front, key requested)
            : hashtable_queue_exception<key>(pointer, size, back, front), _requested(requested) {};
    
    /*!\fn      ~hashtable_queue_not_contained_exception()
     * \brief   Destructor of an Exception Thrown When an Entry was not Already Contained in an
     *          \link hashtable_queue \endlink
     * \details As this class doesn't allocate memory dynamically, this destructor
     *          doesn't do anything.
     */
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
