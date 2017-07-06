#ifndef ZERO_MULTI_CLOCK_EXCEPTIONS_H
#define ZERO_MULTI_CLOCK_EXCEPTIONS_H

#include <exception>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <array>

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index> class multi_clock;

/*!\class   multi_clock_exception
 * \brief   Exception Thrown in a \link multi_clock \endlink
 * \details Represents an arbitrary exception that can be thrown in instances of
 *          \link multi_clock \endlink . Every exception that is specific for the
 *          \link multi_clock \endlink have to inherit from this class.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_exception : public std::exception {
protected:
    /*!\var     _pointer
     * \brief   The \link multi_clock \endlink
     * \details A pointer to the \link multi_clock \endlink where this exception was
     *          thrown.
     */
    const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>*                              _pointer;

    /*!\var     _hands
     * \brief   The Clock Hands of the \link multi_clock \endlink
     * \details The \link key \endlink (s) where the clock hands of the \link multi_clock \endlink
     *          where this exception was thrown, points to.
     */
    std::array<key, _clocknumber>                                                                                   _hands;

    /*!\var     _sizes
     * \brief   The Sizes of the Clocks of the \link multi_clock \endlink
     * \details The sizes of the clocks of the \link multi_clock \endlink
     *          where this exception was thrown.
     */
    std::array<key, _clocknumber>                                                                                   _sizes;


public:
    /*!\fn      multi_clock_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>* pointer, const key* hands, const key* sizes)
     * \brief   Constructor of an Exception Thrown in \link multi_clock \endlink
     * \details This constructor instantiates an \link multi_clock_exception \endlink with all
     *          its variables.
     *
     * @param pointer See \link _pointer \endlink .
     * @param hands   See \link _hands \endlink .
     * @param sizes   See \link _sizes \endlink .
     */
    multi_clock_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>* pointer, const key* hands, const key* sizes)
            : _pointer(pointer) {
        for (uint32_t i = 0; i <= _clocknumber - 1; i++) {
            _hands[i] = hands[i];
            _sizes[i] = sizes[i];
        }
    }

    /*!\fn      what()
     * \brief   What Caused the Exception Happened?
     * \details Returns description about what caused this exception to be thrown.
     *
     * @return A description about what caused this exception to be thrown.
     */
    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "An unknown exception happened in the multi_clock instance " << _pointer << ".";
        return oss.str().c_str();
    }

    /*!\fn      details()
     * \brief   Details About the Exceptional State
     * \details Returns all known details of the state that caused this exception to be thrown.
     *
     * @return A description about the state that caused this exception to be thrown.
     */
    virtual const char* details() const {
        std::ostringstream oss;
        oss << "&multi_clock = " << _pointer;
        for (uint32_t i = 0; i <= _clocknumber - 1; i++) {
            oss << ", multi_clock.size_of(" << i << ") = " << _sizes[i];
        }
        for (uint32_t i = 0; i <= _clocknumber - 1; i++) {
            oss << ", multi_clock.get_head_index(" << i << ") = " << _hands[i];
        }
        return oss.str().c_str();
    };

};

/*!\class   multi_clock_multi_exception
 * \brief   Exception Thrown When Multiple Exceptions Happen at the Same Time in a
 *          \link multi_clock \endlink
 * \details Represents multiple exceptions that can be thrown in instances of
 *          \link multi_clock \endlink . The exceptions of type
 *          \link multi_clock_exception \endlink that would be thrown are stored
 *          in this exception.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_multi_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _exceptions
     * \brief   The Exceptions that Would have been Thrown
     * \details A list of exceptions that would have been thrown at the same time in one
     *          instance of \link multi_clock \endlink .
     */
    std::vector<multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>>              _exceptions;


public:
    using multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::multi_clock_exception;

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        for (multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> exception : _exceptions) {
            oss << exception.what() << std::endl;
        }
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        for (multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> exception : _exceptions) {
            oss << typeid(exception).name() << ":" << std::endl;
            oss << exception.details() << std::endl;
        }
        return oss.str().c_str();
    }

    /*!\fn      getExceptions()
     * \brief   Get the Exceptions that Would have been Thrown
     * \details Returns an iterator over the \link _exceptions \endlink .
     *
     * @return  An iterator over the \link _exceptions \endlink .
     */
    typename std::vector<multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>>::iterator getExceptions() {
        return _exceptions.begin();
    };

    /*!\fn      addException(multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> exception)
     * \brief   Adds the Specified Exception to This \link multi_clock_multi_exception \endlink
     * \details The specified \c exception gets appended to \link _exceptions \endlink
     *          or if \c exception is of type \link multi_clock_multi_exception \endlink,
     *          the contained exceptions get appended to \link _exceptions \endlink .
     *
     * @param exception The exception(s) to add to this \link multi_clock_multi_exception \endlink .
     */
    void addException(multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> exception) {
        if (typeid(exception) == typeid(multi_clock_multi_exception)) {
            _exceptions.insert(_exceptions.end(), dynamic_cast<multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>&>(exception)._exceptions.begin(), dynamic_cast<multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>&>(exception)._exceptions.end());
        } else {
            _exceptions.push_back(exception);
        }
    }

    /*!\fn      size()
     * \brief   Get the Number of Exceptions Contained
     * \details Returns the number of exceptions that would have been thrown (contained in
     *          \link _exceptions \endlink )
     *
     * @return The number of exceptions that would have been thrown.
     */
    uint32_t size() {
        return _exceptions.size();
    }

};

/*!\class   multi_clock_empty_exception
 * \brief   Exception Thrown When a \link multi_clock \endlink is Empty
 * \details Represents an exception that can be thrown in instances of \link multi_clock \endlink
 *          when it is unexpectedly empty.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_empty_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _empty_clock
     * \brief   Index of the Empty Clock
     * \details The index of the clock that was unexpectedly empty (corresponding to \link _pointer \endlink ).
     */
    const uint32_t                                                                                                  _empty_clock;


public:
    /*!\fn      multi_clock_empty_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                            const key* hands, const key* sizes, const uint32_t empty_clock)
     * \brief   Constructor of an Exception Thrown when an \link multi_clock \endlink is Empty
     * \details This constructor instantiates an \link multi_clock_empty_exception \endlink with all
     *          its variables.
     *
     * @param pointer     See \link _pointer \endlink .
     * @param hands       See \link _hands \endlink .
     * @param sizes       See \link _sizes \endlink .
     * @param empty_clock See \link _empty_clock \endlink .
     */
    multi_clock_empty_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                const key* hands, const key* sizes, const uint32_t empty_clock)
            : multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(pointer, hands, sizes), _empty_clock(empty_clock) {};

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The clock " << _empty_clock << " of multi_clock instance " << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::_pointer << " was unexpectedly empty.";
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        oss << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::details() << ", empty clock = " << _empty_clock;
        return oss.str().c_str();
    }

};

/*!\class   multi_clock_invalid_clock_index_exception
 * \brief   Exception Thrown When an Access to a \link multi_clock \endlink with an Invalid
 *          Clock Index Happens
 * \details Represents an exception that can be thrown in instances of \link multi_clock \endlink
 *          when it gets accessed using an invalid clock index.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_invalid_clock_index_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _clock_index
     * \brief   Invalid Clock Index That was Used
     * \details The index of the clock that was unexpectedly not existing in the \link multi_clock \endlink .
     */
    const uint32_t                                                                                                  _clock_index;


public:
    /*!\fn      multi_clock_invalid_clock_index_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                                          const key* hands, const key* sizes, const uint32_t clock_index)
     * \brief   Constructor of an Exception Thrown When an Access to a \link multi_clock \endlink
     *          with an Invalid Clock Index Happens
     * \details This constructor instantiates an \link multi_clock_invalid_clock_index_exception \endlink with all
     *          its variables.
     *
     * @param pointer     See \link _pointer \endlink .
     * @param hands       See \link _hands \endlink .
     * @param sizes       See \link _sizes \endlink .
     * @param clock_index See \link _clock_index \endlink .
     */
    multi_clock_invalid_clock_index_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                              const key* hands, const key* sizes, const uint32_t clock_index)
            : multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(pointer, hands, sizes), _clock_index(clock_index) {};

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The clock index " << _clock_index << " is invalid for the multi_clock instance " << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::_pointer << ".";
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        oss << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::details() << ", invalid clock index = " << _clock_index;
        return oss.str().c_str();
    }

};

/*!\class   multi_clock_invalid_index_exception
 * \brief   Exception Thrown When an Access to a \link multi_clock \endlink with an Invalid
 *          Index Happens
 * \details Represents an exception that can be thrown in instances of \link multi_clock \endlink
 *          when it gets accessed using an invalid index.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_invalid_index_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _index
     * \brief   Invalid Index That was Used
     * \details The index that was unexpectedly not existing in the \link multi_clock \endlink .
     */
    const uint32_t                                                                                                  _index;


public:
    /*!\fn      multi_clock_invalid_index_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                                    const key* hands, const key* sizes, const uint32_t index)
     * \brief   Constructor of an Exception Thrown When an Access to a \link multi_clock \endlink
     *          with an Invalid Index Happens
     * \details This constructor instantiates an \link multi_clock_invalid_index_exception \endlink with all
     *          its variables.
     *
     * @param pointer     See \link _pointer \endlink .
     * @param hands       See \link _hands \endlink .
     * @param sizes       See \link _sizes \endlink .
     * @param clock_index See \link _clock_index \endlink .
     */
    multi_clock_invalid_index_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                        const key* hands, const key* sizes, const uint32_t index)
            : multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(pointer, hands, sizes), _index(index) {};

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The index " << _index << " is invalid for the multi_clock instance " << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::_pointer << ".";
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        oss << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::details() << ", invalid index = " << _index;
        return oss.str().c_str();
    }
};

/*!\class   multi_clock_already_contained_exception
 * \brief   Exception Thrown When an Key is Already Contained in a \link multi_clock \endlink
 * \details Represents an exception that can be thrown in instances of \link multi_clock \endlink
 *          when a key was unexpectedly already contained in any clock of the
 *          \link multi_clock \endlink .
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_already_contained_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _index
     * \brief   The Unexpected Key
     * \details The key that was unexpectedly already contained.
     */
    const uint32_t                                                                                                  _index;


public:
    /*!\fn      multi_clock_already_contained_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                                        const key* hands, const key* sizes, const uint32_t index)
     * \brief   Constructor of an Exception Thrown When a Key was Unexpectedly Already Contained in Any
     *          Clock of the \link multi_clock \endlink .
     * \details This constructor instantiates an \link multi_clock_already_contained_exception \endlink with all
     *          its variables.
     *
     * @param pointer See \link _pointer \endlink .
     * @param hands   See \link _hands \endlink .
     * @param sizes   See \link _sizes \endlink .
     * @param index   See \link _index \endlink .
     */
    multi_clock_already_contained_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                            const key* hands, const key* sizes, const uint32_t index)
            : multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(pointer, hands, sizes), _index(index) {};

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The index " << _index << " is unexpectedly already contained in the multi_clock instance " << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::_pointer << ".";
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        oss << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::details() << ", contained = " << _index;
        return oss.str().c_str();
    }
};

/*!\class   multi_clock_not_contained_exception
 * \brief   Exception Thrown When an Key is Not Already Contained in a \link multi_clock \endlink
 * \details Represents an exception that can be thrown in instances of \link multi_clock \endlink
 *          when a key was unexpectedly not already contained in any clock of the
 *          \link multi_clock \endlink .
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where this exception was
 *                              thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where this exception was thrown.
 *
 * \author Max Gilbert
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock_not_contained_exception : public multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> {
protected:
    /*!\var     _index
     * \brief   The Expected Key
     * \details The key that was unexpectedly not already contained.
     */
    const uint32_t                                                                                                  _index;


public:
    /*!\fn      multi_clock_not_contained_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                                    const key* hands, const key* sizes, const uint32_t index)
     * \brief   Constructor of an Exception Thrown When a Key was Unexpectedly Not Already Contained in Any
     *          Clock of the \link multi_clock \endlink .
     * \details This constructor instantiates an \link multi_clock_not_contained_exception \endlink with all
     *          its variables.
     *
     * @param pointer See \link _pointer \endlink .
     * @param hands   See \link _hands \endlink .
     * @param sizes   See \link _sizes \endlink .
     * @param index   See \link _index \endlink .
     */
    multi_clock_not_contained_exception(const multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index> *pointer,
                                        const key* hands, const key* sizes, const uint32_t index)
            : multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(pointer, hands, sizes), _index(index) {};

    virtual const char* what() const noexcept {
        std::ostringstream oss;
        oss << "The index " << _index << " is unexpectedly not contained in the multi_clock instance " << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::_pointer << ".";
        return oss.str().c_str();
    }

    virtual const char* details() const {
        std::ostringstream oss;
        oss << multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::details() << ", not contained = " << _index;
        return oss.str().c_str();
    }
};

/*!\fn      throw_multiple(multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception)
 * \brief   Throws the Specified \link multi_clock_multi_exception \endlink
 * \details Replaces the keyword \c throw for exceptions of type \link multi_clock_multi_exception \endlink .
 *          If the specified \link multi_clock_multi_exception \endlink only contains one
 *          \link multi_clock_exception \endlink, the contained exception will be thrown without the
 *          \link multi_clock_multi_exception \endlink as container. If the specified
 *          \link multi_clock_multi_exception \endlink containes multiple exceptions of type
 *          \link multi_clock_exception \endlink, the specified \link multi_clock_multi_exception \endlink
 *          will be thrown.
 *
 * @tparam key                  The data type of the key of the key-value pairs stored in
 *                              the \link multi_clock \endlink where the specified exception
 *                              was thrown.
 * @tparam value                The data type of the value of the key-value pairs stored in
 *                              the \link multi_clock \endlink where the specified exception
 *                              was thrown.
 * @tparam _clocknumber         The number of clocks contained in the
 *                              \link multi_clock \endlink where the specified exception was
 *                              thrown.
 * @tparam _invalid_index       The \c k key with the semantics of \c null within the
 *                              \link multi_clock \endlink where the specified exception
 *                              was thrown.
 * @tparam _invalid_clock_index The index of a clock with the semantics of \c null within the
 *                              \link multi_clock \endlink where the specified exception was
 *                              thrown.
 *
 * @param multi_exception The exception that should be thrown.
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
void throw_multiple(multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception)
                                throw (multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (multi_exception.size() == 1) {
        throw multi_exception.getExceptions()[0];
    } else {
        throw multi_exception;
    }
};

#endif //ZERO_MULTI_CLOCK_EXCEPTIONS_H
