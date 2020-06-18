#ifndef __HASHTABLE_DEQUE_EXCEPTIONS_HPP
#define __HASHTABLE_DEQUE_EXCEPTIONS_HPP

namespace zero::hashtable_deque {

    /*!\class   HashtableDequeException
     * \brief   Exception thrown in \link HashtableDeque \endlink
     * \details Represents an arbitrary exception that can be thrown in instances of \link HashtableDeque \endlink .
     *          Every exception that is specific for the \link HashtableDeque \endlink have to inherit from this class.
     *
     * @tparam key_type    The key type of the corresponding \link HashtableDeque \endlink .
     * @tparam invalid_key The invalid key used in the corresponding \link HashtableDeque \endlink  .
     *
     * \author Max Gilbert
     */
    template<class key_type, key_type invalid_key>
    class HashtableDequeException : public std::exception {
    public:
        /*!\fn      HashtableDequeException(const uint64_t& size, const key_type& back, const key_type& front)
         * \brief   Constructor of an exception thrown in \link HashtableDeque \endlink
         * \details This constructor instantiates an \c HashtableDequeException with all its variables.
         *
         * @param size    See \link _size \endlink .
         * @param back    See \link _back \endlink .
         * @param front   See \link _front \endlink .
         */
        HashtableDequeException(const uint64_t& size, const key_type& back, const key_type& front) :
                _size(size),
                _back(back),
                _front(front) {};

        /*!\fn      what() const noexcept
         * \brief   What caused the exception happened?
         * \details Returns description about what caused this exception to be thrown.
         *
         * @return A description about what caused this exception to be thrown.
         */
        const char* what() const noexcept override {
            return "An unknown exception happened in an instance of HashtableDeque.";
        };

        /*!\fn      details() const noexcept
         * \brief   Details about the exceptional state
         * \details Returns all known details of the state that caused this exception to be thrown.
         *
         * @return A description about the state that caused this exception to be thrown.
         */
        virtual const char* details() const noexcept {
            int whatStringLength = snprintf(nullptr, 0,
                                            "HashtableDeque.size() = %zd, HashtableDeque._back = %zd, HashtableDeque._front = %zd",
                                            _size, _back, _front);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "HashtableDeque.size() = %zd, HashtableDeque._back = %zd, HashtableDeque._front = %zd",
                     _size, _back, _front);
            return whatSentence;
        };

    protected:
        /*!\var     _size
         * \brief   The size of the \link HashtableDeque \endlink
         * \details The number of entries of the \link HashtableDeque \endlink where this exception was thrown at the
         *          time when this exception happened.
         */
        const uint64_t _size;

        /*!\var     _back
         * \brief   The back of the \link HashtableDeque \endlink
         * \details The value of the back of the \link HashtableDeque \endlink where this exception was thrown at the
         *          time when this exception happened.
         */
        const key_type _back;

        /*!\var     _front
         * \brief   The front of the \link HashtableDeque \endlink
         * \details The value of the front of the \link HashtableDeque \endlink where this exception was thrown at the
         *          time when this exception happened.
         */
        const key_type _front;
    };

    /*!\class   HashtableDequeAlreadyContainsException
     * \brief   Exception thrown when an entry was already contained in an \link HashtableDeque \endlink
     * \details Represents an exception that can be thrown in instances of \link HashtableDeque \endlink when an entry
     *          was unexpectedly already contained in the deque.
     *
     * @tparam key_type    The key type of the corresponding \link HashtableDeque \endlink .
     * @tparam invalid_key The invalid key used in the corresponding \link HashtableDeque \endlink  .
     *
     * \author Max Gilbert
     */
    template<class key_type, key_type invalid_key>
    class HashtableDequeAlreadyContainsException : public HashtableDequeException<key_type, invalid_key> {
    public:
        /*!\fn      HashtableDequeAlreadyContainsException(const uint64_t& size, const key_type& back, const key_type& front, const key_type& containedKey)
         * \brief   Constructor of an exception thrown when an entry was already contained in an
         *          \link HashtableDeque \endlink
         * \details This constructor instantiates an \c HashtableDeque_already_contains_exception with all its
         *          variables.
         *
         * @param size         See \link _size \endlink .
         * @param back         See \link _back \endlink .
         * @param front        See \link _front \endlink .
         * @param containedKey See \link _containedKey \endlink .
         */
        HashtableDequeAlreadyContainsException(const uint64_t& size, const key_type& back, const key_type& front,
                                               const key_type& containedKey) :
                HashtableDequeException<key_type, invalid_key>(size, back, front),
                _containedKey(containedKey) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "%zd was tried to be inserted into an HashtableDeque, but it was already contained in there.",
                                            _containedKey);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "%zd was tried to be inserted into an HashtableDeque, but it was already contained in there.",
                     _containedKey);
            return whatSentence;
        };

    protected:
        /*!\var     _containedKey
         * \brief   The Unexpected Entry
         * \brief   The value of the entry that was unexpectedly already contained.
         */
        const key_type _containedKey;
    };

    /*!\class   HashtableDequeEmptyException
     * \brief   Exception thrown when an \link HashtableDeque \endlink is empty
     * \details Represents an exception that can be thrown in instances of \link HashtableDeque \endlink when it is
     *          unexpectedly empty.
     *
     * @tparam key_type    The key type of the corresponding \link HashtableDeque \endlink .
     * @tparam invalid_key The invalid key used in the corresponding \link HashtableDeque \endlink  .
     *
     * \author Max Gilbert
     */
    template<class key_type, key_type invalid_key>
    class HashtableDequeEmptyException : public HashtableDequeException<key_type, invalid_key> {
    public:
        using HashtableDequeException<key_type, invalid_key>::HashtableDequeException;

        const char* what() const noexcept override {
            return "An HashtableDeque was already empty.";
        };
    };

    /*!\class   HashtableDequeNotContainedException
     * \brief   Exception thrown when an entry was not already contained in an \link HashtableDeque \endlink
     * \details Represents an exception that can be thrown in instances of \link HashtableDeque \endlink when an entry
     *          was unexpectedly not already contained in the deque.
     *
     * @tparam key_type    The key type of the corresponding \link HashtableDeque \endlink .
     * @tparam invalid_key The invalid key used in the corresponding \link HashtableDeque \endlink  .
     *
     * \author Max Gilbert
     */
    template<class key_type, key_type invalid_key>
    class HashtableDequeNotContainedException : public HashtableDequeException<key_type, invalid_key> {
    public:
        /*!\fn      HashtableDequeNotContainedException(const uint64_t& size, const key_type& back, const key_type& front, const key_type& requestedKey)
         * \brief   Constructor of an exception thrown when an entry was not already contained in an
         *          \link HashtableDeque \endlink
         * \details This constructor instantiates an \c HashtableDeque_not_contained_exception with all its variables.
         *
         * @param size         See \link _size \endlink.
         * @param back         See \link _back \endlink.
         * @param front        See \link _front \endlink.
         * @param requestedKey See \link _requestedKey \endlink.
         */
        HashtableDequeNotContainedException(const uint64_t& size, const key_type& back, const key_type& front,
                                            const key_type& requestedKey) :
                HashtableDequeException<key_type, invalid_key>(size, back, front),
                _requestedKey(requestedKey) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0, "An HashtableDeque does not contain key %zd", _requestedKey);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength, "An HashtableDeque does not contain key %zd", _requestedKey);
            return whatSentence;
        };

    private:
        /*!\var     _requestedKey
         * \brief   The Expected Entry
         * \details The value of the entry that was unexpectedly not already contained.
         */
        const key_type _requestedKey;
    };
}

#endif //__HASHTABLE_DEQUE_EXCEPTIONS_HPP
