#ifndef ZERO_MULTI_CLOCK_H
#define ZERO_MULTI_CLOCK_H

#include <cstdint>
#include "multi_clock_exceptions.h"
#include <vector>
#include <array>

/*!\class   multi_clock
 * \brief   Multiple Clocks with a Common Set of Entries
 * \details Represents multiple clocks of key-value pairs using one common set of entries.
 *          The total size of the clocks (number of key-value pairs) is fixed but the sizes
 *          of the different clocks are variable and limiting those is not supported. The
 *          keys are stored implicitly as array indices for the values and therefore it
 *          works best when the domain of the keys is very limited.
 *          Each clock has an index (starting from 0) which is required when working with
 *          it. It is possible to add an entry at the tail of a clock and to remove one from
 *          its head. It is also possible to get the key or get/set the value of the entry
 *          where the clock hand of a clock points to. In addition to the typical interface
 *          of a single clock, it is possible to swap one entry from one clock's head to
 *          another clock's tail.
 *          The computational complexity of the methods of this class is in
 *          \f$\mathcal{O}\left(1\right)\f$ and the space complexity of this class is in
 *          \f$\mathcal{O}\left(n\right)\f$ regarding the key range.
 *
 * @tparam key                  The data type of the key of the key-value pairs where each
 *                              key is unique within one instance of this data structure.
 * @tparam value                The data type of the value of the key-value pairs where
 *                              each value instance corresponds to a key.
 * @tparam _clocknumber         Contains the total number of clocks contained in this
 *                              \link multi_clock \endlink an therefore it specifies the
 *                              highest valid \link clk_idx \endlink, the number of clock
 *                              \link _hands \endlink etc. The actual number of clocks might
 *                              be smaller as some clocks can be empty.
 * @tparam _invalid_index       This specifies an invalid \c key which can be used to mark that
 *                              a clock is empty and therefore the clock hand points to this value.
 *                              This should have the semantics of \c null for the specified \c key
 *                              template parameter therefore a natural choice of this for the case
 *                              that \c key is a pointer would be \c nullptr.
 * @tparam _invalid_clock_index This specifies an invalid \c clock index which can be used to mark
 *                              inside \link _clock_membership \endlink that an index does not
 *                              belong to any clock. This should have the semantics of \c null for
 *                              \link clk_idx \endlink and is equal to \link _clocknumber \endlink
 *                              (greatest clock index plus 1).
 */
template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index = _clocknumber>
class multi_clock {
public:
    /*!\typedef clk_idx
     * \brief   Data type of clock indexes
     * \details The datatype used to index the specific clocks.
     */
    typedef uint32_t clk_idx;


private:
    /*!\class   index_pair
     * \brief   Pair of keys
     * \details Pairs of keys used to create a linked list of those.
     */
    class index_pair {
    public:
        /*!\fn      index_pair()
         * \brief   Constructor for an empty pair of keys
         * \details This constructor instantiates an \link index_pair \endlink without
         *          setting the members \link _before \endlink and \link _after \endlink.
         */
        index_pair() {};

        /*!\fn      index_pair(key before, key after)
         * \brief   Constructor of a pair of keys with initial values
         * \details This constructor instantiates an \link index_pair \endlink and
         *          initializes the members \link _before \endlink and
         *          \link _after \endlink as specified.
         *
         * @param before The initial value of \link _before \endlink.
         * @param after  The initial value of \link _after \endlink.
         */
        index_pair(key before, key after) {
            this->_before = before;
            this->_after = after;
        };

        /*!\var     _before
         * \brief   Key before this key
         * \details The key that is closer to the tail of the clock. It was visited by
         *          the clock hand before this value (specified by the index of this
         *          elements within the array it is stored in).
         */
        key     _before;

        /*!\var     _after
         * \brief   Key after this key
         * \details The key that is closer to the head of the clock. It gets visited by
         *          the clock hand after this value (specified by the index of this
         *          elements within the array it is stored in).
         */
        key     _after;
    };

    /*!\var     _clocksize
     * \brief   Number of entries the clocks can hold
     * \details Contains the number of key-value pairs that can be stored in the clocks
     *          combined. When this \link multi_clock \endlink is initialized, it allocates
     *          memory to hold this many entries. This also specifies the highest key
     *          that is allowed in the clocks (\c _clocksize \c - \c 1 ).
     */
    key                             _clocksize;

    /*!\var     _values
     * \brief   Values
     * \details Holds the values corresponding the keys. The corresponding key is the index
     *          of this array.
     */
    value*                          _values;

    /*!\var     _clocks
     * \brief   Clocks
     * \details Contains the doubly linked, circular lists representing the clocks (every
     *          not empty clock is contained in here). The \link index_pair \endlink
     *          stored at index \c i contains the indexes within the same clock after
     *          \c i and before \c i .
     */
    index_pair*                     _clocks;

    /*!\var     _clock_membership
     * \brief   Membership of indexes to clocks
     * \details This array specifies for each index in the domain to which clock it
     *          belongs. If an index is not part of a clock, the
     *          \link _invalid_clock_index \endlink is used.
     */
    clk_idx*                        _clock_membership;

    /*!\var     _hands
     * \brief   Clock hands
     * \details Contains the clock hands of the clocks. Therefore it contains the index
     *          of each clock's head. If a clock is empty, this contains the
     *          \link _invalid_index \endlink.
     */
    key*                            _hands;

    /*!\var     _sizes
     * \brief   Number of elements in the clocks
     * \details Contains for each clock the number of elements this clock currently has.
     */
    key*                            _sizes;


public:
    /*!\fn      multi_clock()
     * \brief   Constructor of Multiple Clocks with a Common Set of Entries
     * \details Constructs a new \link multi_clock \endlink with a specified combined
     *          capacity of the clocks and a number of (initially empty) clocks specified
     *          in the template parameter \link _clocknumber \endlink . This constructor
     *          allocates the memory to store \link _clocksize \endlink entries.
     *
     * @param clocksize The range of the clock indexes and the combined size of the clocks.
     */
    multi_clock(key clocksize);


    /*!\fn      ~multi_clock()
     * \brief   Destructor of Multiple Clocks with a Common Set of Entries
     * \details Destructs this instance of \link multi_clock \endlink and deallocates the
     *          memory used to store the clocks.
     */
    virtual ~multi_clock();

    /*!\fn      get_head(clk_idx clock, value &head_value)
     * \brief   Get the value of the entry where the clock hand of the specified
     *          clock points to
     * \details Returns the value of the head of the specified clock.
     *
     * @param clock The clock whose head's value should be returned.
     * @return      The value of the head of the specified clock.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_empty_exception               If the specified \c clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    value get_head(const clk_idx clock) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                               multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                               multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      set_head(clk_idx clock, value new_value)
     * \brief   Set the value of the entry where the clock hand of the specified
     *          clock points to
     * \details Sets the value of the head of the specified clock to the specified value.
     *
     * @param clock     The clock whose head's value should be set.
     * @param new_value The new value of the head of the specified clock.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_empty_exception               If the specified \c clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    void set_head(const clk_idx clock, const value new_value) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                     multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                     multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      get_head_index(clk_idx clock, key &head_index)
     * \brief   Get the index of the entry where the clock hand of the specified
     *          clock points to
     * \details Returns the index of the head of the specified clock.
     *
     * @param clock The clock whose head should be returned.
     * @return      The index of the head of the specified clock.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_empty_exception               If the specified \c clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    key get_head_index(const clk_idx clock) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                   multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                   multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      move_head(clk_idx clock)
     * \brief   Move the clock hand forward
     * \details Moves the tail entry of the specified clock before the head of the same
     *          clock. Therefore the previous tail entry becomes the new head entry. The
     *          previous head will become the element \link index_pair._before \endlink
     *          the new head and the new tail will be the element
     *          \link index_pair._after \endlink the new head.
     *
     * @param clock The clock whose clock hand should be moved.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_empty_exception               If the specified \c clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    void move_head(const clk_idx clock) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                               multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                               multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      add_tail(clk_idx clock, key index)
     * \brief   Make the specified index the tail of the specified clock
     * \details Adds a new entry with the specified index to the tail of the specified
     *          clock. The new entry will be the tail of the clock and the previous tail
     *          entry will be \link index_pair._before \endlink the new entry. Adding a
     *          new entry is only possible if the index is not already contained inside
     *          any clock of the same \link multi_clock \endlink.
     *
     * @param clock The clock where the new entry should be added to at the tail.
     * @param index The index of the new entry.
     *
     * @throws multi_clock_invalid_index_exception       If the specified \c index is invalid.
     * @throws multi_clock_already_contained_exception   If the specified \c index could not be
     *                                                   added because it is already contained
     *                                                   in some clock.
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    void add_tail(const clk_idx clock, const key index) throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                               multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                               multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                               multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      add_before(const key inside, const key new_entry)
     * \brief   Add the specified index before another index in an arbitrary clock
     * \details Adds a new entry with the specified index \c new_entry in the clock
     *          before the entry \c inside. The entry that was before \c inside
     *          before will be \link index_pair._before \endlink \c new_entry. Adding a
     *          new entry is only possible if the index is not already contained inside
     *          any clock of the same \link multi_clock \endlink.
     *
     * @param inside    This index will be the entry after the new entry.
     * @param new_entry This is the index of the new entry.
     *
     * @throws multi_clock_invalid_index_exception     If the specified \c inside or
     *                                                 \c new_entry is invalid.
     * @throws multi_clock_already_contained_exception If the specified \c new_entry could not be
     *                                                 added because it is already contained in
     *                                                 some clock.
     * @throws multi_clock_not_contained_exception     If the position for the insertion is not
     *                                                 valid because the specified \c inside is
     *                                                 not contained in any clock.
     * @throws multi_clock_multi_exception             If multiple of those exceptions would be
     *                                                 thrown.
     */
    void add_before(const key inside, const key new_entry) throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                  multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                  multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                  multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      add_after(const key inside, const key new_entry)
     * \brief   Add the specified index after another index in an arbitrary clock
     * \details Adds a new entry with the specified index \c new_entry in the clock
     *          after the entry \c inside. The entry that was after \c inside
     *          before will be \link index_pair._after \endlink \c new_entry. Adding a
     *          new entry is only possible if the index is not already contained inside
     *          any clock of the same \link multi_clock \endlink.
     *
     * @param inside    This index will be the entry before the new entry.
     * @param new_entry This is the index of the new entry.
     *
     * @throws multi_clock_invalid_index_exception     If the specified \c inside or
     *                                                 \c new_entry is invalid.
     * @throws multi_clock_already_contained_exception If the specified \c new_entry could not be
     *                                                 added because it is already contained in
     *                                                 some clock.
     * @throws multi_clock_not_contained_exception     If the position for the insertion is not
     *                                                 valid because the specified \c inside is
     *                                                 not contained in any clock.
     * @throws multi_clock_multi_exception             If multiple of those exceptions would be
     *                                                 thrown.
     */
    void add_after(const key inside, const key new_entry) throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                 multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                 multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                 multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);


    /*!\fn      remove_head(clk_idx clock, key &removed_index)
     * \brief   Remove the head entry from the specified clock
     * \details Removes the entry at the head of the specified clock from that clock.
     *          The new head of the clock will be the entry after the removed entry
     *          and therefore the clock hand will point to that index.
     *
     * @param clock The index of the clock whose head entry will be removed.
     * @return      The index of the entry that was removed.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c clock does not
     *                                                   exist.
     * @throws multi_clock_empty_exception               If the specified \c clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions would
     *                                                   be thrown.
     */
    key remove_head(const clk_idx clock) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      remove(key &index)
     * \brief   Remove the specified entry from any clock
     * \details Removed the specified entry from any clock. The entry before this entry
     *          will be before the entry after the specfied entry and the entry after
     *          this entry will be after the entry before the specfied entry.
     *
     * @param index The index of the entry that gets removed.
     *
     * @throws multi_clock_invalid_index_exception If the specified \c index is invalid.
     * @throws multi_clock_not_contained_exception If the specified \c index could not be
     *                                             removed from some clock because it is
     *                                             not contained in one.
     * @throws multi_clock_multi_exception         If multiple of those exceptions would be
     *                                             thrown.
     */
    void remove(key index) throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                  multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                  multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      switch_head_to_tail(clk_idx source, clk_idx destination, key &moved_index)
     * \brief   Moves an entry from the head of one clock to the tail of another one
     * \details Removes the index at the head of the \c source clock and adds it as tail
     *          of the \c destination clock.
     *
     * @param source      The index of the clock whose head gets moved. The head will be
     *                    removed from this clock.
     * @param destination The index of the clock where the moved entry gets added to the tail.
     * @return            The index of the entry that was moved from one clock to another.
     *
     * @throws multi_clock_invalid_clock_index_exception If the specified \c source or
     *                                                   \c destination does not exist.
     * @throws multi_clock_empty_exception               If the head of the specified
     *                                                   \c source could not be taken
     *                                                   because the clock is empty.
     * @throws multi_clock_multi_exception               If multiple of those exceptions
     *                                                   would be thrown.
     */
    key switch_head_to_tail(const clk_idx source, const clk_idx destination) throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                                                                    multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>);

    /*!\fn      size_of(clk_idx clock)
     * \brief   Returns the number of entries in the specified clock
     * \details Returns the number of entries that is currently contained in the specified
     *          clock, if it exists.
     *
     * @param clock The clock whose current size gets returned.
     * @return      Number of entries in the specified \c clock or \0 if the clock does not exist.
     */
    inline key      size_of(const clk_idx clock) {
        return valid_clock_index(clock) * _sizes[clock];
    }

    /*!\fn      empty(const clk_idx clock)
     * \brief   Returns \c true if the specified clock is empty
     * \details Returns \c true if the specified clock currently contains entries, if it exists.
     *
     * @param clock The clock whose emptiness gets returned.
     * @return      \c true if the specified clock exists and contains entries, \c false else.
     */
    inline bool     empty(const clk_idx clock) {
        return size_of(clock) == 0;
    }

    /*!\fn      valid_index(const key index)
     * \brief   Returns \c true if the specified index is valid
     * \details Returns \c true if the specified index is valid in this \link multi_clock \endlink.
     *
     * @param index The index whose validity is checked.
     * @return      \c true if the specified index is valid.
     */
    inline bool     valid_index(const key index) {
        return index != _invalid_index && index >= 0 && index <= _clocksize - 1;
    }

    /*!\fn      contained_index(const key index)
     * \brief   Returns \c true if the specified index is contained in any clock
     * \details Returns \c true if the specified index is valid in this \link multi_clock \endlink
     *          and if it is contained in any clock within this \link multi_clock \endlink.
     *
     * @param index The index whose clock membership is checked.
     * @return      \c true if the specified index is contained in any clock.
     */
    inline bool     contained_index(const key index) {
        return valid_index(index) && valid_clock_index(_clock_membership[index]);
    }

    /*!\fn      valid_clock_index(const clk_idx clock_index)
     * \brief   Returns \c true if the specified clock exists
     * \details Returns \c true if the specified clock exists in this \link multi_clock \endlink.
     *
     * @param clock_index The index of the clock whose existence is checked.
     * @return            \c true if the specified clock exists.
     */
    inline bool     valid_clock_index(const clk_idx clock_index) {
        return clock_index >= 0 && clock_index <= _clocknumber - 1;
    }

    /*!\fn      get(key index)
     * \brief   Returns a reference to the value that corresponds to the specified index
     * \details Returns a reference to the value that corresponds to the specified index,
     *          independent of the membership of that index to any clock.
     *
     * @param index The index whose value gets returned.
     * @return      A reference to the value corresponding the specified index if this index
     *              is valid inside this \link multi_clock \endlink or a reference to the value
     *              of the \link _invalid_index \endlink, else.
     */
    inline value&   get(const key index) {
        if (valid_index(index)) {
            return _values[index];
        } else {
            return _values[_invalid_index];
        }
    }

    /*!\fn      set(key index, value new_value)
     * \brief   Sets the value that corresponds to the specified index
     * \details Sets the value that corresponds to the specified index, independent of the
     *          membership of that index to any clock.
     *
     * @param index     The index whose value gets set.
     * @param new_value The new value for the specified index if this index is valid inside this
     *                  \link multi_clock \endlink or for the \link _invalid_index \endlink, else.
     */
    inline void     set(const key index, value const new_value) {
        if (valid_index(index)) {
            _values[index] = new_value;
        }
    }

    /*!\fn      operator[](key index)
     * \brief   Returns a reference to the value that corresponds to the specified index
     * \details Returns a reference to the value that corresponds to the specified index,
     *          independent of the membership of that index to any clock.
     *
     * @param index The index whose value gets returned.
     * @return      A reference to the value corresponding the specified index if this index
     *              is valid inside this \link multi_clock \endlink or a reference to the value
     *              of the \link _invalid_index \endlink, else.
     */
    inline value&   operator[](const key index) {
        if (valid_index(index)) {
            return _values[index];
        } else {
            return _values[_invalid_index];
        }
    }
};

#endif //ZERO_MULTI_CLOCK_H
