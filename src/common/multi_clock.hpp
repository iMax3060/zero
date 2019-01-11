#ifndef __MULTI_CLOCK_HPP
#define __MULTI_CLOCK_HPP

#include "multi_clock_exceptions.hpp"

#include <cstdint>
#include <cstring>
#include <vector>
#include <array>

namespace zero::multi_clock {

    /*!\typedef ClockIndex
     * \brief   Data type of clock indexes
     * \details The datatype used to index the specific clocks.
     */
    typedef uint32_t ClockIndex;

    /*!\class   MultiHandedClock
     * \brief   Multiple Clocks with a Common Set of Entries
     * \details Represents multiple clocks of key-value pairs using one common set of entries. The total size of the
     *          clocks (number of key-value pairs) is fixed but the sizes of the different clocks are variable and
     *          limiting those is not supported. The keys are stored implicitly as array indices for the values and
     *          therefore it works best when the domain of the keys is very limited. Each clock has an index (starting
     *          from 0) which is required when working with it. It is possible to add an entry at the tail of a clock
     *          and to remove one from its head. It is also possible to get the key or get/set the value of the entry
     *          where the clock hand of a clock points to. In addition to the typical interface of a single clock, it is
     *          possible to swap one entry from one clock's head to another clock's tail.
     *
     *          The computational complexity of the methods of this class is in \f$\mathcal{O}\left(1\right)\f$ and the
     *          space complexity of this class is in \f$\mathcal{O}\left(n\right)\f$ regarding the key range.
     *
     * @tparam key_type            The data type of the key of the key-value pairs where each key is unique within one
     *                             instance of this data structure.
     * @tparam value_type          The data type of the value of the key-value pairs where each value instance
     *                             corresponds to a key.
     * @tparam clock_count         Contains the total number of clocks contained in this \link MultiHandedClock \endlink
     *                             and therefore it specifies the highest valid \link ClockIndex \endlink, the number of
     *                             clock \link _hands \endlink etc. The actual number of clocks might be smaller as some
     *                             clocks can be empty.
     * @tparam invalid_index       This specifies an invalid key which can be used to mark that a clock is empty and
     *                             therefore the clock hand points to this value. This should have the semantics of
     *                             \c null for the specified key template parameter therefore a natural choice of
     *                             this for the case that key is a pointer would be \c nullptr.
     * @tparam invalid_clock_index This specifies an invalid clock index which can be used to mark inside
     *                             \link _clockMembership \endlink that an index does not belong to any clock. This
     *                             should have the semantics of \c null for \link ClockIndex \endlink and is equal to
     *                             \c clock_count (greatest clock index plus 1).
     *
     * \author Max Gilbert
     */
    template<class key_type, class value_type, uint32_t clock_count, key_type invalid_index, key_type invalid_clock_index = clock_count>
    class MultiHandedClock {
    public:
        /*!\fn      MultiHandedClock(key_type entryCount)
         * \brief   Constructor of Multiple Clocks with a Common Set of Entries
         * \details Constructs a new \link MultiHandedClock \endlink with a specified combined capacity of the clocks
         *          and a number of (initially empty) clocks specified in the template parameter
         *          \c clock_count . This constructor allocates the memory to store
         *          \link _entryCount \endlink entries.
         *
         * @param entryCount The range of the clock indexes and the combined size of the clocks.
         */
        MultiHandedClock(key_type entryCount) :
                _entryCount(entryCount),
                _values(_entryCount),
                _clocks(_entryCount),
                _clockMembership(_entryCount, invalid_clock_index) {
            _hands.fill(invalid_index);
            _sizes.fill(0);
        };

        /*!\fn      ~MultiHandedClock()
         * \brief   Destructor of Multiple Clocks with a Common Set of Entries
         * \details Destructs this instance of \link MultiHandedClock \endlink and deallocates the memory used to store
         *          the clocks.
         */
        ~MultiHandedClock() {};

        /*!\fn      getHead(value_type& head) const
         * \brief   Get the value of the entry where the clock hand of the specified clock points to
         * \details Returns the value of the head of the specified clock.
         *
         * @tparam clock    The clock whose head's value should be returned.
         * @param[out] head The value of the head of the specified clock.
         *
         * @throws MultiHandedClockEmptyException If the specified \c clock is empty.
         */
        template<ClockIndex clock>
        void getHead(value_type& head) const {
            static_assert(clock < clock_count);
            if (!isEmpty<clock>()) {
                w_assert1(_clockMembership[_hands[clock]] == clock);
                head = _values[_hands[clock]];
            } else {
                w_assert1(_hands[clock] == invalid_index);
                throw MultiHandedClockEmptyException<key_type, value_type, clock_count, invalid_index,
                                                     invalid_clock_index>(this, clock);
            }
        };

        /*!\fn      setHead(const value_type newValue)
         * \brief   Set the value of the entry where the clock hand of the specified clock points to
         * \details Sets the value of the head of the specified clock to the specified value.
         *
         * @tparam clock   The clock whose head's value should be set.
         * @param newValue The new value of the head of the specified clock.
         *
         * @throws MultiHandedClockEmptyException If the specified \c clock is empty.
         */
        template<ClockIndex clock>
        void setHead(const value_type newValue) {
            static_assert(clock < clock_count);
            if (!isEmpty<clock>()) {
                _values[_hands[clock]] = newValue;
            } else {
                throw MultiHandedClockEmptyException<key_type, value_type, clock_count, invalid_index,
                                                     invalid_clock_index>(this, clock);
            }
        };

        /*!\fn      getHeadIndex()getHeadIndex(key_type& headIndex) const
         * \brief   Get the index of the entry where the clock hand of the specified clock points to
         * \details Returns the index of the head of the specified clock.
         *
         * @tparam clock         The clock whose head should be returned.
         * @param[out] headIndex The index of the head of the specified clock.
         *
         * @throws MultiHandedClockEmptyException If the specified \c clock is empty.
         */
        template<ClockIndex clock>
        void getHeadIndex(key_type& headIndex) const {
            static_assert(clock < clock_count);
            if (!isEmpty<clock>()) {
                w_assert1(_clockMembership[_hands[clock]] == clock);
                headIndex = _hands[clock];
            } else {
                throw MultiHandedClockEmptyException<key_type, value_type, clock_count, invalid_index,
                                                     invalid_clock_index>(this, clock);
            }
        };

        /*!\fn      moveHead()
         * \brief   Move the clock hand forward
         * \details Moves the tail entry of the specified clock before the head of the same clock. Therefore
         *          the previous tail entry becomes the new head entry. The previous head will become the element
         *          \link IndexPair._before \endlink the new head and the new tail will be the element
         *          \link IndexPair._after \endlink the new head.
         *
         * @tparam clock The clock whose clock hand should be moved.
         *
         * @throws MultiHandedClockEmptyException If the specified \c clock is empty.
         */
        template<ClockIndex clock>
        void moveHead() {
            static_assert(clock < clock_count);
            if (!isEmpty<clock>()) {
                _hands[clock] = _clocks[_hands[clock]]._after;
                w_assert1(_clockMembership[_hands[clock]] == clock);
            } else {
                throw MultiHandedClockEmptyException<key_type, value_type, clock_count, invalid_index,
                                                     invalid_clock_index>(this, clock);
            }
        };

        /*!\fn      addTail(const key_type& index)
         * \brief   Make the specified index the__ZERO_MULTICLOCK_HPP tail of the specified clock
         * \details Adds a new entry with the specified index to the tail of the specified clock. The new entry will be
         *          the tail of the clock and the previous tail entry will be \link IndexPair._before \endlink the new
         *          entry. Adding a new entry is only possible if the index is not already contained inside any clock of
         *          the same \link MultiHandedClock \endlink.
         *
         * @tparam clock The clock where the new entry should be added to at the tail.
         * @param index  The index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c index is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c index could not be added because it is
         *                                                   already contained in some clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        template<ClockIndex clock>
        void addTail(const key_type& index) {
            static_assert(clock < clock_count);
            if (isValidIndex(index) && !isContainedIndex(index)) {
                if (isEmpty<clock>()) {
                    _hands[clock] = index;
                    _clocks[index]._before = index;
                    _clocks[index]._after = index;
                } else {
                    _clocks[index]._before = _clocks[_hands[clock]]._before;
                    _clocks[index]._after = _hands[clock];
                    _clocks[_clocks[_hands[clock]]._before]._after = index;
                    _clocks[_hands[clock]]._before = index;
                }
                _sizes[clock]++;
                _clockMembership[index] = clock;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(index))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));
                if (isContainedIndex(index))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));

                throwMultiple(multiException);
            }
        };

        /*!\fn      addTail(key_type&& index)
         * \brief   Make the specified index the tail of the specified clock (with move semantics)
         * \details Adds a new entry with the specified index to the tail of the specified clock using move semantics.
         *          The new entry will be the tail of the clock and the previous tail entry will be
         *          \link IndexPair._before \endlink the new entry. Adding a new entry is only possible if the index is
         *          not already contained inside any clock of the same \link MultiHandedClock \endlink.
         *
         * @tparam clock The clock where the new entry should be added to at the tail.
         * @param index  The index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c index is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c index could not be added because it is
         *                                                   already contained in some clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        template<ClockIndex clock>
        void addTail(key_type&& index) {
            static_assert(clock < clock_count);
            if (isValidIndex(index) && !isContainedIndex(index)) {
                if (isEmpty<clock>()) {
                    _hands[clock] = index;
                    _clocks[index]._before = index;
                    _clocks[index]._after = index;
                } else {
                    _clocks[index]._before = _clocks[_hands[clock]]._before;
                    _clocks[index]._after = _hands[clock];
                    _clocks[_clocks[_hands[clock]]._before]._after = index;
                    _clocks[_hands[clock]]._before = index;
                }
                _sizes[clock]++;
                _clockMembership[index] = clock;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(index))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));
                if (isContainedIndex(index))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));

                throwMultiple(multiException);
            }
        };

        /*!\fn      addBefore(const key_type& inside, const key_type& newEntry)
         * \brief   Add the specified index before another index in an arbitrary clock
         * \details Adds a new entry with the specified index \c newEntry in the clock before the entry \c inside. The
         *          entry that was before \c inside before will be \link IndexPair._before \endlink \c newEntry. Adding
         *          a new entry is only possible if the index is not already contained inside any clock of the same
         *          \link MultiHandedClock \endlink.
         *
         * @param inside    This index will be the entry after the new entry.
         * @param newEntry This is the index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c inside or \c new_entry is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c new_entry could not be added because it
         *                                                   is already contained in some clock.
         * @throws MultiHandedClockNotContainedException     If the position for the insertion is not valid because the
         *                                                   specified \c inside is not contained in any clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        void addBefore(const key_type& inside, const key_type& newEntry) {
            if (isValidIndex(newEntry) && !isContainedIndex(newEntry) && isContainedIndex(inside)) {
                w_assert1(_sizes[_clockMembership[inside]] >= 1);
                _clocks[newEntry]._before = _clocks[inside]._before;
                _clocks[newEntry]._after = inside;
                _clocks[inside]._before = newEntry;
                _clockMembership[newEntry] = _clockMembership[inside];
                _sizes[_clockMembership[inside]]++;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (isContainedIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (!isValidIndex(inside))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));
                else if (!isContainedIndex(inside))
                    multiException.addException(MultiHandedClockNotContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));

                throwMultiple(multiException);
            }
        };

        /*!\fn      addBefore(const key_type& inside, key_type&& newEntry)
         * \brief   Add the specified index before another index in an arbitrary clock (with move semantics)
         * \details Adds a new entry with the specified index \c newEntry in the clock before the entry \c inside using
         *          move semantics. The entry that was before \c inside before will be \link IndexPair._before \endlink
         *          \c newEntry. Adding a new entry is only possible if the index is not already contained inside any
         *          clock of the same \link MultiHandedClock \endlink.
         *
         * @param inside   This index will be the entry after the new entry.
         * @param newEntry This is the index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c inside or \c new_entry is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c new_entry could not be added because it
         *                                                   is already contained in some clock.
         * @throws MultiHandedClockNotContainedException     If the position for the insertion is not valid because the
         *                                                   specified \c inside is not contained in any clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        void addBefore(const key_type& inside, key_type&& newEntry) {
            if (isValidIndex(newEntry) && !isContainedIndex(newEntry) && isContainedIndex(inside)) {
                w_assert1(_sizes[_clockMembership[inside]] >= 1);
                _clocks[newEntry]._before = _clocks[inside]._before;
                _clocks[newEntry]._after = inside;
                _clocks[inside]._before = newEntry;
                _clockMembership[newEntry] = _clockMembership[inside];
                _sizes[_clockMembership[inside]]++;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (isContainedIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (!isValidIndex(inside))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));
                else if (!isContainedIndex(inside))
                    multiException.addException(MultiHandedClockNotContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));

                throwMultiple(multiException);
            }
        };

        /*!\fn      addAfter(const key_type& inside, const key_type& newEntry)
         * \brief   Add the specified index after another index in an arbitrary clock
         * \details Adds a new entry with the specified index \c newEntry in the clock after the entry \c inside. The
         *          entry that was after \c inside before will be \link IndexPair._after \endlink \c newEntry. Adding a
         *          new entry is only possible if the index is not already contained inside any clock of the same
         *          \link MultiHandedClock \endlink.
         *
         * @param inside    This index will be the entry before the new entry.
         * @param newEntry This is the index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c inside or \c new_entry is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c new_entry could not be added because it
         *                                                   is already contained in some clock.
         * @throws MultiHandedClockNotContainedException     If the position for the insertion is not valid because the
         *                                                   specified \c inside is not contained in any clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        void addAfter(const key_type& inside, const key_type& newEntry) {
            if (isValidIndex(newEntry) && !isContainedIndex(newEntry) && isContainedIndex(inside)) {
                w_assert1(_sizes[_clockMembership[inside]] >= 1);
                _clocks[newEntry]._after = _clocks[inside]._after;
                _clocks[newEntry]._before = inside;
                _clocks[inside]._after = newEntry;
                _clockMembership[newEntry] = _clockMembership[inside];
                _sizes[_clockMembership[inside]]++;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (isContainedIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (!isValidIndex(inside))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));
                else if (!isContainedIndex(inside))
                    multiException.addException(MultiHandedClockNotContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));

                throwMultiple(multiException);
            }
        };

        /*!\fn      addAfter(const key_type& inside, const key_type&& newEntry)
         * \brief   Add the specified index after another index in an arbitrary clock (with move semantics)
         * \details Adds a new entry with the specified index \c newEntry in the clock after the entry \c inside using
         *          move semantics. The entry that was after \c inside before will be \link IndexPair._after \endlink
         *          \c newEntry. Adding a new entry is only possible if the index is not already contained inside any
         *          clock of the same \link MultiHandedClock \endlink.
         *
         * @param inside    This index will be the entry before the new entry.
         * @param newEntry This is the index of the new entry.
         *
         * @throws MultiHandedClockInvalidIndexException     If the specified \c inside or \c new_entry is invalid.
         * @throws MultiHandedClockAlreadyContainedException If the specified \c new_entry could not be added because it
         *                                                   is already contained in some clock.
         * @throws MultiHandedClockNotContainedException     If the position for the insertion is not valid because the
         *                                                   specified \c inside is not contained in any clock.
         * @throws MultiHandedClockMultiException            If multiple of those exceptions would be thrown.
         */
        void addAfter(const key_type& inside, const key_type&& newEntry) {
            if (isValidIndex(newEntry) && !isContainedIndex(newEntry) && isContainedIndex(inside)) {
                w_assert1(_sizes[_clockMembership[inside]] >= 1);
                _clocks[newEntry]._after = _clocks[inside]._after;
                _clocks[newEntry]._before = inside;
                _clocks[inside]._after = newEntry;
                _clockMembership[newEntry] = _clockMembership[inside];
                _sizes[_clockMembership[inside]]++;
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (isContainedIndex(newEntry))
                    multiException.addException(
                            MultiHandedClockAlreadyContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, newEntry));
                if (!isValidIndex(inside))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));
                else if (!isContainedIndex(inside))
                    multiException.addException(MultiHandedClockNotContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, inside));

                throwMultiple(multiException);
            }
        };

        /*!\fn      removeHead(key_type& removedKey)
         * \brief   Remove the head entry from the specified clock
         * \details Removes the entry at the head of the specified clock from that clock. The new head of the clock will
         *          be the entry after the removed entry and therefore the clock hand will point to that index.
         *
         * @tparam clock          The index of the clock whose head entry will be removed.
         * @param[out] removedKey The index of the entry that was removed.
         *
         * @throws MultiHandedClockEmptyException If the specified \c clock is empty.
         */
        template<ClockIndex clock>
        void removeHead(key_type& removedIndex) {
            static_assert(clock < clock_count);
            if (!isEmpty<clock>()) {
                removedIndex = _hands[clock];
                remove(removedIndex);
            } else {
                w_assert1(_hands[clock] == invalid_index);
                throw MultiHandedClockEmptyException<key_type, value_type, clock_count, invalid_index,
                                                     invalid_clock_index>(this, clock);
            }
        };

        /*!\fn      remove(const key_type& index)
         * \brief   Remove the specified entry from any clock
         * \details Removed the specified entry from any clock. The entry before this entry will be before the entry
         *          after the specified entry and the entry after this entry will be after the entry before the
         *          specified entry.
         *
         * @param index The index of the entry that gets removed.
         *
         * @throws MultiHandedClockInvalidIndexException If the specified \c index is invalid.
         * @throws MultiHandedClockNotContainedException If the specified \c index could not be removed from some clock
         *                                               because it is not contained in one.
         * @throws MultiHandedClockMultiException        If multiple of those exceptions would be thrown.
         */
        void remove(const key_type& index) {
            if (isContainedIndex(index)) {
                ClockIndex clock = _clockMembership[index];
                if (_sizes[clock] == 1) {
                    w_assert1(_hands[clock] >= 0 && _hands[clock] <= _entryCount - 1 && _hands[clock] != invalid_index);
                    w_assert1(_clocks[_hands[clock]]._before == _hands[clock]);
                    w_assert1(_clocks[_hands[clock]]._after == _hands[clock]);

                    _clocks[index]._before = invalid_index;
                    _clocks[index]._after = invalid_index;
                    _hands[clock] = invalid_index;
                    _clockMembership[index] = invalid_clock_index;
                    _sizes[clock]--;
                } else {
                    _clocks[_clocks[index]._before]._after = _clocks[index]._after;
                    _clocks[_clocks[index]._after]._before = _clocks[index]._before;
                    _hands[clock] = _clocks[index]._after;
                    _clocks[index]._before = invalid_index;
                    _clocks[index]._after = invalid_index;
                    _clockMembership[index] = invalid_clock_index;
                    _sizes[clock]--;

                    w_assert1(_hands[clock] != invalid_index);
                }
            } else {
                MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index> multiException =
                        MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this);

                if (!isValidIndex(index))
                    multiException.addException(MultiHandedClockInvalidIndexException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));
                else if (!isContainedIndex(index))
                    multiException.addException(MultiHandedClockNotContainedException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>(this, index));

                throwMultiple(multiException);
            }
        };

        /*!\fn      switchHeadToTail(key_type& movedIndex)
         * \brief   Moves an entry from the head of one clock to the tail of another one
         * \details Removes the index at the head of the \c source clock and adds it as tail of the \c destination
         *          clock.
         *
         * @tparam source         The index of the clock whose head gets moved. The head will be removed from this
         *                        clock.
         * @tparam destination    The index of the clock where the moved entry gets added to the tail.
         * @param[out] movedIndex The index of the entry that was moved from one clock to another.
         *
         * @throws MultiHandedClockEmptyException If the head of the specified \c source could not be taken because the
         *                                        clock is empty.
         */
        template<ClockIndex source, ClockIndex destination>
        void switchHeadToTail(key_type& movedIndex) {
            static_assert(source < clock_count);
            static_assert(destination < clock_count);
            movedIndex = invalid_index;
            removeHead<source>(movedIndex);

            w_assert1(movedIndex != invalid_index);

            addTail<destination>(movedIndex);
        };

        /*!\fn      sizeOf() const noexcept
         * \brief   Returns the number of entries in the specified clock
         * \details Returns the number of entries that is currently contained in the specified clock.
         *
         * @tparam clock The clock whose current size gets returned.
         * @return       Number of entries in the specified \c clock .
         */
        template<ClockIndex clock>
        inline key_type sizeOf() const noexcept {
            static_assert(clock < clock_count);
            return _sizes[clock];
        }

        /*!\fn      isEmpty() const noexcept
         * \brief   Returns \c true if the specified clock is empty
         * \details Returns \c true if the specified clock currently contains entries.
         *
         * @tparam clock The clock whose emptiness gets returned.
         * @return       \c true if the specified clock contains entries, \c false else.
         */
        template<ClockIndex clock>
        inline bool isEmpty() const noexcept {
            static_assert(clock < clock_count);
            return sizeOf<clock>() == 0;
        }

        /*!\fn      isValidIndex(const key_type& index) const noexcept
         * \brief   Returns \c true if the specified index is valid
         * \details Returns \c true if the specified index is valid in this \link MultiHandedClock \endlink.
         *
         * @param index The index whose validity is checked.
         * @return      \c true if the specified index is valid.
         */
        inline bool isValidIndex(const key_type& index) const noexcept {
            return index != invalid_index && index >= 0 && index <= _entryCount - 1;
        }

        /*!\fn      isContainedIndex(const key_type& index) const noexcept
         * \brief   Returns \c true if the specified index is contained in any clock
         * \details Returns \c true if the specified index is valid in this \link MultiHandedClock \endlink and if it is
         *          contained in any clock within this \link MultiHandedClock \endlink.
         *
         * @param index The index whose clock membership is checked.
         * @return      \c true if the specified index is contained in any clock.
         */
        inline bool isContainedIndex(const key_type& index) const noexcept {
            return isValidIndex(index) && _clockMembership[index] != invalid_clock_index;
        }

        /*!\fn      get(const key_type& index) noexcept
         * \brief   Returns a reference to the value that corresponds to the specified index
         * \details Returns a reference to the value that corresponds to the specified index, independent of the
         *          membership of that index to any clock.
         *
         * @param index The index whose value gets returned.
         * @return      A reference to the value corresponding the specified index if this index is valid inside this
         *              \link MultiHandedClock \endlink or a reference to the value of the template parameter
         *              \c invalid_index else.
         */
        inline value_type& get(const key_type& index) noexcept {
            if (isValidIndex(index)) {
                return _values[index];
            } else {
                return _values[invalid_index];
            }
        }

        //!@copydoc get(const key_type& index) noexcept
        inline const value_type& get(const key_type& index) const noexcept {
            if (isValidIndex(index)) {
                return _values[index];
            } else {
                return _values[invalid_index];
            }
        }

        /*!\fn      set(const key_type& index, const value_type& newValue)
         * \brief   Sets the value that corresponds to the specified index
         * \details Sets the value that corresponds to the specified index, independent of the membership of that index
         *          to any clock.
         *
         * @param index    The index whose value gets set.
         * @param newValue The new value for the specified index if this index is valid inside this
         *                 \link MultiHandedClock \endlink or for the template parameter \c invalid_index , else.
         */
        inline void set(const key_type& index, const value_type& newValue) noexcept {
            if (isValidIndex(index)) {
                _values[index] = newValue;
            }
        }

        /*!\fn      set(const key_type& index, value_type&& newValue)
         * \brief   Sets the value that corresponds to the specified index (with move semantics)
         * \details Sets the value that corresponds to the specified index using move semantics, independent of the
         *          membership of that index to any clock.
         *
         * @param index    The index whose value gets set.
         * @param newValue The new value for the specified index if this index is valid inside this
         *                 \link MultiHandedClock \endlink or for the template parameter \c invalid_index , else.
         */
        inline void set(const key_type& index, value_type&& newValue) noexcept {
            if (isValidIndex(index)) {
                _values[index] = newValue;
            }
        }

        /*!\fn      operator[](const key_type& index) noexcept
         * \brief   Returns a reference to the value that corresponds to the specified index
         * \details Returns a reference to the value that corresponds to the specified index, independent of the
         *          membership of that index to any clock.
         *
         * @param index The index whose value gets returned.
         * @return      A reference to the value corresponding the specified index if this index is valid inside this
         *              \link MultiHandedClock \endlink or a reference to the value of the template parameter
         *              \c invalid_index , else.
         */
        inline value_type& operator[](const key_type& index) noexcept {
            if (isValidIndex(index)) {
                return _values[index];
            } else {
                return _values[invalid_index];
            }
        }

        //!@copydoc operator[](const key_type& index) noexcept
        inline const value_type& operator[](const key_type& index) const noexcept {
            if (isValidIndex(index)) {
                return _values[index];
            } else {
                return _values[invalid_index];
            }
        }

        /*!\fn      getClockIndex(const key_type& index) noexcept
         * \brief   Returns a reference to the index of the clock where the specified index is contained in
         * \details Returns a reference to the index of the clock where the specified index is contained in or
         *          template parameter \c invalid_clock_index of the specified index is invalid or not contained in any
         *          clock.
         *
         * @param index The index whose clock index gets returned.
         * @return      A reference to the index of the clock where the specified index is contained in or a reference
         *              to the value of the template parameter \c invalid_clock_index of the \c index is invalid in this
         *              \link MultiHandedClock \endlink or if it is not contained in any clock.
         */
        inline ClockIndex& getClockIndex(const key_type& index) noexcept {
            if (isValidIndex(index)) {
                return _clockMembership[index];
            } else {
                return invalid_clock_index;
            }
        }

        //!@copydoc getClockIndex(const key_type& index) noexcept
        inline const ClockIndex& getClockIndex(const key_type& index) const noexcept {
            if (isValidIndex(index)) {
                return _clockMembership[index];
            } else {
                return invalid_clock_index;
            }
        }

    private:
        /*!\class   IndexPair
         * \brief   Pair of keys
         * \details Pairs of keys used to create a linked list of those.
         */
        class IndexPair {
        public:
            /*!\fn      IndexPair()
             * \brief   Constructor for an empty pair of keys
             * \details This constructor instantiates an \link IndexPair \endlink without setting the members
             *          \link _before \endlink and \link _after \endlink.
             */
            IndexPair() {};

            /*!\fn      IndexPair(const key_type& before, const key_type& after)
             * \brief   Constructor of a pair of keys with initial values
             * \details This constructor instantiates an \link IndexPair \endlink and initializes the members
             *          \link _before \endlink and \link _after \endlink is specified.
             *
             * @param before The initial value of \link _before \endlink.
             * @param after  The initial value of \link _after \endlink.
             */
            IndexPair(const key_type& before, const key_type& after) :
                    _before(before),
                    _after(after) {};

            /*!\var     _before
             * \brief   Key before this key
             * \details The key that is closer to the tail of the clock. It was visited by the clock hand before this
             *          value (specified by the index of this elements within the array it is stored in).
             */
            key_type _before;

            /*!\var     _after
             * \brief   Key after this key
             * \details The key that is closer to the head of the clock. It gets visited by the clock hand after this
             *          value (specified by the index of this elements within the array it is stored in).
             */
            key_type _after;
        };

        /*!\var     _entryCount
         * \brief   Number of entries the clocks can hold
         * \details Contains the number of key-value pairs that can be stored in the clocks combined. When this
         *          \link MultiHandedClock \endlink is initialized, it allocates memory to hold this many entries. This
         *          also specifies the highest key that is allowed in the clocks (\c _entryCount \c - \c 1 ).
         */
        key_type                            _entryCount;

        /*!\var     _values
         * \brief   Values
         * \details Holds the values corresponding the keys. The corresponding key is the index of this array.
         */
        std::vector<value_type>             _values;

        /*!\var     _clocks
         * \brief   Clocks
         * \details Contains the doubly linked, circular lists representing the clocks (every not empty clock is
         *          contained in here). The \link IndexPair \endlink stored at index \c i contains the indexes within
         *          the same clock after \c i and before \c i .
         */
        std::vector<IndexPair>              _clocks;

        /*!\var     _clockMembership
         * \brief   Membership of indexes to clocks
         * \details This array specifies for each index in the domain to which clock it belongs. If an index is not part
         *          of a clock, the template parameter \c invalid_clock_index is used.
         */
        std::vector<ClockIndex>             _clockMembership;

        /*!\var     _hands
         * \brief   Clock hands
         * \details Contains the clock hands of the clocks. Therefore it contains the index of each clock's head. If a
         *          clock is empty, this contains the template parameter \c invalid_index .
         */
        std::array<key_type, clock_count>   _hands;

        /*!\var     _sizes
         * \brief   Number of elements in the clocks
         * \details Contains for each clock the number of elements this clock currently has.
         */
        std::array<key_type, clock_count>   _sizes;

    public:
        /*!\fn      throwMultiple(MultiHandedClockMultiException& multiException)
         * \brief   Throws the specified \link MultiHandedClockMultiException \endlink
         * \details Replaces the keyword \c throw for exceptions of type \link MultiHandedClockMultiException \endlink.
         *          If the specified \link MultiHandedClockMultiException \endlink only contains one
         *          \link MultiHandedClockException \endlink, the contained exception will be thrown without the
         *          \link MultiHandedClockMultiException \endlink is container. If the specified
         *          \link MultiHandedClockMultiException \endlink contains multiple exceptions of type
         *          \link MultiHandedClockException \endlink, the specified
         *          \link MultiHandedClockMultiException \endlink will be thrown.
         *
         * @param multiException The exception that should be thrown.
         */
        static void throwMultiple(MultiHandedClockMultiException<key_type, value_type, clock_count, invalid_index, invalid_clock_index>& multiException) {
            if (multiException.size() == 1) {
                throw multiException.getExceptions()[0];
            } else {
                throw multiException;
            }
        };

    };
    
} // zero::multi_clock

#endif // __MULTI_CLOCK_HPP
