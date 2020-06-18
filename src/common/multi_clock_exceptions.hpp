#ifndef __MULTI_CLOCK_EXCEPTIONS_HPP
#define __MULTI_CLOCK_EXCEPTIONS_HPP

#include <exception>

namespace zero::multi_clock {

    typedef uint32_t ClockIndex;

    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index>
    class MultiHandedClock;

    /*!\class   MultiHandedClockException
     * \brief   Exception thrown in a \link MultiHandedClock \endlink
     * \details Represents an arbitrary exception that can be thrown in instances of \link MultiHandedClock \endlink.
     *          Every exception that is specific for the \link MultiHandedClock \endlink have to inherit from this
     *          class.
     *
     * @tparam key_type            The key type of the corresponding \link MultiHandedClock \endlink.
     * @tparam value_type          The value type of the corresponding \link MultiHandedClock \endlink.
     * @tparam clock_count         The number of clocks contained in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_index       The invalid key used in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_clock_index The invalid clock index used in the corresponding \link MultiHandedClock \endlink.
     *
     * \author Max Gilbert
     */
    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClockException : public std::exception {
    public:
        /*!\fn      MultiHandedClockException(const MultiHandedClock* multiHandedClock)
         * \brief   Constructor of an exception thrown in \link MultiHandedClock \endlink
         * \details This constructor instantiates an \link MultiHandedClockException \endlink with all
         *          its variables.
         *
         * @param multiHandedClock See \link _multiHandedClock \endlink.
         */
        MultiHandedClockException(const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                                                         invalid_clock_index>* multiHandedClock) :
                _multiHandedClock(multiHandedClock) {};

        /*!\fn      what()
         * \brief   What caused the exception happened?
         * \details Returns description about what caused this exception to be thrown.
         *
         * @return A description about what caused this exception to be thrown.
         */
        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "An unknown exception happened in the MultiHandedClock instance %p",
                                            _multiHandedClock);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "An unknown exception happened in the MultiHandedClock instance %p",
                     _multiHandedClock);
            return whatSentence;
        };

    protected:
        /*!\var     _multiHandedClock
         * \brief   The \link MultiHandedClock \endlink
         * \details A pointer to the \link MultiHandedClock \endlink where this exception was thrown.
         */
        const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                               invalid_clock_index>* _multiHandedClock;
    };

    /*!\class   MultiHandedClockEmptyException
     * \brief   Exception thrown when a \link MultiHandedClock \endlink is empty
     * \details Represents an exception that can be thrown in instances of \link MultiHandedClock \endlink when it is
     *          unexpectedly empty.
     *
     * @tparam key_type            The key type of the corresponding \link MultiHandedClock \endlink.
     * @tparam value_type          The value type of the corresponding \link MultiHandedClock \endlink.
     * @tparam clock_count         The number of clocks contained in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_index       The invalid key used in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_clock_index The invalid clock index used in the corresponding \link MultiHandedClock \endlink.
     *
     * \author Max Gilbert
     */
    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClockEmptyException : public MultiHandedClockException<key_type, value_type, clock_count,
                                                                            invalid_index, invalid_clock_index> {
    public:
        /*!\fn      MultiHandedClockEmptyException(const MultiHandedClock* pointer, const ClockIndex& emptyClock)
         * \brief   Constructor of an exception thrown when an \link MultiHandedClock \endlink is empty
         * \details This constructor instantiates an \link MultiHandedClockEmptyException \endlink with all its
         *          variables.
         *
         * @param multiHandedClock See \link _multiHandedClock \endlink.
         * @param emptyClock       See \link _emptyClock \endlink.
         */
        MultiHandedClockEmptyException(const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                                                              invalid_clock_index>* multiHandedClock,
                                       const ClockIndex& emptyClock) :
                MultiHandedClockException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(
                        multiHandedClock),
                _emptyClock(emptyClock) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The clock %u of MultiHandedClock instance %p was unexpectedly empty.",
                                            _emptyClock, this->_multiHandedClock);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The clock %u of MultiHandedClock instance %p was unexpectedly empty.",
                     _emptyClock, this->_multiHandedClock);
            return whatSentence;
        };

    protected:
        /*!\var     _emptyClock
         * \brief   Index of the Empty Clock
         * \details The index of the clock that was unexpectedly empty (corresponding to
         *          \link _multiHandedClock \endlink ).
         */
        const ClockIndex _emptyClock;
    };

    /*!\class   MultiHandedClockInvalidIndexException
     * \brief   Exception Thrown When an Access to a \link MultiHandedClock \endlink with an Invalid
     *          Index Happens
     * \details Represents an exception that can be thrown in instances of \link MultiHandedClock \endlink
     *          when it gets accessed using an invalid index.
     *
     * @tparam key_type            The key type of the corresponding \link MultiHandedClock \endlink.
     * @tparam value_type          The value type of the corresponding \link MultiHandedClock \endlink.
     * @tparam clock_count         The number of clocks contained in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_index       The invalid key used in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_clock_index The invalid clock index used in the corresponding \link MultiHandedClock \endlink.
     *
     * \author Max Gilbert
     */
    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClockInvalidIndexException : public MultiHandedClockException<key_type, value_type, clock_count,
                                                                                   invalid_index, invalid_clock_index> {
    public:
        /*!\fn      MultiHandedClockInvalidIndexException(const MultiHandedClock* multiHandedClock, const key_type& invalidIndex)
         * \brief   Constructor of an exception thrown when an access to a \link MultiHandedClock \endlink with an
         *          invalid index happens
         * \details This constructor instantiates an \link MultiHandedClockInvalidIndexException \endlink with all
         *          its variables.
         *
         * @param multiHandedClock See \link _multiHandedClock \endlink.
         * @param invalidIndex     See \link _invalidIndex \endlink.
         */
        MultiHandedClockInvalidIndexException(const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                                                                     invalid_clock_index>* multiHandedClock,
                                              const key_type& invalidIndex) :
                MultiHandedClockException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(
                        multiHandedClock),
                _invalidIndex(invalidIndex) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The index %u is invalid for the MultiHandedClock instance %p.",
                                            _invalidIndex, this->_multiHandedClock);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The index %u is invalid for the MultiHandedClock instance %p.",
                     _invalidIndex, this->_multiHandedClock);
            return whatSentence;
        };

    protected:
        /*!\var     _invalidIndex
         * \brief   Invalid Index That was Used
         * \details The index that was unexpectedly not existing in the \link MultiHandedClock \endlink.
         */
        const key_type _invalidIndex;
    };

    /*!\class   MultiHandedClockAlreadyContainedException
     * \brief   Exception thrown when a key is already contained in a \link MultiHandedClock \endlink
     * \details Represents an exception that can be thrown in instances of \link MultiHandedClock \endlink when a key
     *          was unexpectedly already contained in any clock of the \link MultiHandedClock \endlink.
     *
     * @tparam key_type            The key type of the corresponding \link MultiHandedClock \endlink.
     * @tparam value_type          The value type of the corresponding \link MultiHandedClock \endlink.
     * @tparam clock_count         The number of clocks contained in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_index       The invalid key used in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_clock_index The invalid clock index used in the corresponding \link MultiHandedClock \endlink.
     *
     * \author Max Gilbert
     */
    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClockAlreadyContainedException : public MultiHandedClockException<key_type, value_type,
                                                                                       clock_count, invalid_index,
                                                                                       invalid_clock_index> {
    public:
        /*!\fn      MultiHandedClockAlreadyContainedException(const MultiHandedClock* multiHandedClock, const key_type& alreadyContainedIndex)
         * \brief   Constructor of an exception thrown when a key was unexpectedly already contained in any clock of the
         *          \link MultiHandedClock \endlink.
         * \details This constructor instantiates an \link MultiHandedClockAlreadyContainedException \endlink with all
         *          its variables.
         *
         * @param multiHandedClock      See \link _multiHandedClock \endlink.
         * @param alreadyContainedIndex See \link _alreadyContainedIndex \endlink.
         */
        MultiHandedClockAlreadyContainedException(
                const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                                       invalid_clock_index>* multiHandedClock,
                const key_type& alreadyContainedIndex) :
                MultiHandedClockException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(
                        multiHandedClock),
                _alreadyContainedIndex(alreadyContainedIndex) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The index %u is unexpectedly already contained in the MultiHandedClock instance %p.",
                                            _alreadyContainedIndex, this->_multiHandedClock);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The index %u is unexpectedly already contained in the MultiHandedClock instance %p.",
                     _alreadyContainedIndex, this->_multiHandedClock);
            return whatSentence;
        };

    protected:
        /*!\var     _alreadyContainedIndex
         * \brief   The unexpected key
         * \details The key that was unexpectedly already contained.
         */
        const key_type _alreadyContainedIndex;
    };

    /*!\class   MultiHandedClockNotContainedException
     * \brief   Exception thrown when a key is not already contained in a \link MultiHandedClock \endlink
     * \details Represents an exception that can be thrown in instances of \link MultiHandedClock \endlink when a key
     *          was unexpectedly not already contained in any clock of the \link MultiHandedClock \endlink.
     *
     * @tparam key_type            The key type of the corresponding \link MultiHandedClock \endlink.
     * @tparam value_type          The value type of the corresponding \link MultiHandedClock \endlink.
     * @tparam clock_count         The number of clocks contained in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_index       The invalid key used in the corresponding \link MultiHandedClock \endlink.
     * @tparam invalid_clock_index The invalid clock index used in the corresponding \link MultiHandedClock \endlink.
     *
     * \author Max Gilbert
     */
    template<class key_type,
             class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClockNotContainedException : public MultiHandedClockException<key_type, value_type, clock_count,
                                                                                   invalid_index, invalid_clock_index> {
    public:
        /*!\fn      MultiHandedClockNotContainedException(const MultiHandedClock* multiHandedClock, const key_type& notContainedIndex)
         * \brief   Constructor of an exception thrown when a key was unexpectedly not already contained in any clock of
         *          the \link MultiHandedClock \endlink.
         * \details This constructor instantiates an \link MultiHandedClockNotContainedException \endlink with all its
         *          variables.
         *
         * @param multiHandedClock  See \link _multiHandedClock \endlink.
         * @param notContainedIndex See \link _notContainedIndex \endlink.
         */
        MultiHandedClockNotContainedException(const MultiHandedClock<key_type, value_type, clock_count, invalid_index,
                                                                     invalid_clock_index>* multiHandedClock,
                                              const key_type& notContainedIndex) :
                MultiHandedClockException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(
                        multiHandedClock),
                _notContainedIndex(notContainedIndex) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The index %u is unexpectedly not contained in the MultiHandedClock instance %p.",
                                            _notContainedIndex, this->_multiHandedClock);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The index %u is unexpectedly not contained in the MultiHandedClock instance %p.",
                     _notContainedIndex, this->_multiHandedClock);
            return whatSentence;
        };

    protected:
        /*!\var     _notContainedIndex
         * \brief   The Expected Key
         * \details The key that was unexpectedly not already contained.
         */
        const key_type _notContainedIndex;
    };
}

#endif // __MULTI_CLOCK_EXCEPTIONS_HPP
