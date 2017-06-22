#ifndef ZERO_MULTI_CLOCK_H
#define ZERO_MULTI_CLOCK_H

#include "stdint.h"

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
 * @tparam key   The data type of the key of the key-value pairs where each key is unique
 *               within one instance of this data structure.
 * @tparam value The data type of the value of the key-value pairs where each value
 *               instance corresponds to a key.
 */
template<class key, class value>
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
    
    /*!\var     _invalid_index
     * \brief   Invalid (Unused) \c index
     * \details This specifies an invalid \c key which can be used to mark that
     *          a clock is empty and therefore the clock hand points to this value.
     *          This should have the semantics of \c null for the specified \c key
     *          template parameter therefore a natural choice of a this for the case
     *          that \c key is a pointer would be \c nullptr.
     */
    key                             _invalid_index;
    
    /*!\var     _clock_membership
     * \brief   Membership of indexes to clocks
     * \details This array specifies for each index in the domain to which clock it
     *          belongs. If an index is not part of a clock, the
     *          \link _invalid_clock_index \endlink is used.
     */
    clk_idx*                        _clock_membership;
    
    /*!\var     _clocknumber
     * \brief   Number of clocks
     * \details Contains the total number of clocks contained in this
     *          \link multi_clock \endlink an therefore it specifies the highest valid
     *          \link clk_idx \endlink, the number of clock \link _hands \endlink etc.
     *          The actual number of clocks might be smaller as some clocks can be
     *          empty.
     */
    clk_idx                         _clocknumber;
    
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
    
    /*!\var     _invalid_clock_index
     * \brief   Invalid (Unused) clock index
     * \details This specifies an invalid \c clock index which can be used to mark inside
     *          \link _clock_membership \endlink that an index does not belong to any
     *          clock. This should have the semantics of \c null for \link clk_idx \endlink
     *          and is equal to \link _clocknumber \endlink (greatest clock index plus 1).
     */
    clk_idx                         _invalid_clock_index;


public:
    /*!\fn      multi_clock(key clocksize, uint32_t clocknumber, key invalid_index)
     * \brief   Constructor of Multiple Clocks with a Common Set of Entries
     * \details Constructs a new \link multi_clock \endlink with a specified combined capacity
     *          of the clocks, a specified number of (initially empty) clocks and with an
     *          \link _invalid_index \endlink corresponding to the \link key \endlink data
     *          type. The \c clocksize also specifies the range of the indexes. This constructor
     *          allocates the memory to store \c clocksize entries.
     *
     * @param clocksize     The range of the clock indexes and the combined size of the clocks.
     * @param clocknumber   The number of clocks maintained by this \link multi_clock \endlink
     * @param invalid_index The \link key \endlink value with the semantics of \c null .
     */
    multi_clock(key clocksize, uint32_t clocknumber, key invalid_index);
    
    /*!\fn      ~multi_clock()
     * \brief   Destructor of Multiple Clocks with a Common Set of Entries
     * \details Destructs this instance of \link multi_clock \endlink and deallocates the
     *          memory used to store the clocks.
     */
    virtual         ~multi_clock();
    
    /*!\fn      get_head(clk_idx clock, value &head_value)
     * \brief   Get the value of the entry where the clock hand of the specified
     *          clock points to
     * \details Returns the value of the head of the specified clock.
     *
     * @param clock      The clock whose head's value should be returned.
     * @param head_value The value of the head of the specified clock (return parameter).
     * @return           \c false if the specified clock does not exist or if it is empty,
     *                   \c true else.
     */
    bool            get_head(const clk_idx clock, value &head_value);
    
    /*!\fn      set_head(clk_idx clock, value new_value)
     * \brief   Set the value of the entry where the clock hand of the specified
     *          clock points to
     * \details Sets the value of the head of the specified clock to the specified value.
     *
     * @param clock     The clock whose head's value should be set.
     * @param new_value The new value of the head of the specified clock.
     * @return          \c false if the specified clock does not exist or if it is empty,
     *                  \c true else.
     */
    bool            set_head(const clk_idx clock, const value new_value);
    
    /*!\fn      get_head_index(clk_idx clock, key &head_index)
     * \brief   Get the index of the entry where the clock hand of the specified
     *          clock points to
     * \details Returns the index of the head of the specified clock.
     *
     * @param clock      The clock whose head should be returned.
     * @param head_index The index of the head of the specified clock (return parameter).
     * @return           \c false if the specified clock does not exist or if it is empty,
     *                   \c true else.
     */
    bool            get_head_index(const clk_idx clock, key &head_index);
    
    /*!\fn      move_head(clk_idx clock)
     * \brief   Move the clock hand forward
     * \details Moves the tail entry of the specified clock before the head of the same
     *          clock. Therefore the previous tail entry becomes the new head entry. The
     *          previous head will become the element \link index_pair._before \endlink
     *          the new head and the new tail will be the element
     *          \link index_pair._after \endlink the new head.
     *
     * @param clock The clock whose clock hand should be moved.
     * @return      \c true if the specified clock index is valid and if the clock is not
     *              empty, \c false else.
     */
    bool            move_head(const clk_idx clock);
    
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
     * @return      \c true if the \c clock index is valid and if the new entry's
     *              \c index is valid and \c false else.
     */
    bool            add_tail(const clk_idx clock, const key index);
    
    /*!\fn      add_before(const key inside, const key new_entry)
     * \brief   Add the specified index before another index in an arbitrary clock
     * \details Adds a new entry with the specified index \c new_entry in the clock
     *          before the entry \c inside. The entry the was before \c inside
     *          before will be \link index_pair._before \endlink \c new_entry. Adding a
     *          new entry is only possible if the index is not already contained inside
     *          any clock of the same \link multi_clock \endlink.
     *
     * @param inside    This index will be the entry after the new entry.
     * @param new_entry This is the index of the new entry.
     * @return          \c true if \c inside was contained in any clock and if
     *                  \c new_entry was not, \c false else
     */
    bool            add_before(const key inside, const key new_entry);
    
    /*!\fn      add_after(const key inside, const key new_entry)
     * \brief   Add the specified index after another index in an arbitrary clock
     * \details Adds a new entry with the specified index \c new_entry in the clock
     *          after the entry \c inside. The entry the was after \c inside
     *          before will be \link index_pair._after \endlink \c new_entry. Adding a
     *          new entry is only possible if the index is not already contained inside
     *          any clock of the same \link multi_clock \endlink.
     *
     * @param inside    This index will be the entry before the new entry.
     * @param new_entry This is the index of the new entry.
     * @return          \c true if \c inside was contained in any clock and if
     *                  \c new_entry was not, \c false else
     */
    bool            add_after(const key inside, const key new_entry);
    
    /*!\fn      remove_head(clk_idx clock, key &removed_index)
     * \brief   Remove the head entry from the specified clock
     * \details Removes the entry at the head of the specified clock from that clock.
     *          The new head of the clock will be the entry after the removed entry
     *          and therefore the clock hand will point to that index.
     *
     * @param clock         The index of the clock whose head entry will be removed.
     * @param removed_index The index of the entry that was removed (return parameter).
     * @return              \c true if the specified clock exists and it is not empty
     *                      , \c false else.
     */
    bool            remove_head(const clk_idx clock, key &removed_index);
    
    /*!\fn      remove(key &index)
     * \brief   Remove the specified entry from any clock
     * \details Removed the specified entry from any clock. The entry before this entry
     *          will be before the entry after the specfied entry and the entry after
     *          this entry will be after the entry before the specfied entry.
     *
     * @param index The index of the entry that gets removed.
     * @return      \c true if the specified index is valid and contained in any clock,
     *              \c false else.
     */
    bool            remove(key &index);
    
    /*!\fn      switch_head_to_tail(clk_idx source, clk_idx destination, key &moved_index)
     * \brief   Moves an entry from the head of one clock to the tail of another one
     * \details Removes the index at the head of the \c source clock and adds it as tail
     *          of the \c destination clock.
     *
     * @param source      The index of the clock whose head gets moved. The head will be
     *                    removed from this clock.
     * @param destination The index of the clock where the moved entry gets added to the tail.
     * @param moved_index The index of the entry that was moved from one clock to another
     *                    (return parameter).
     * @return            \c true if the \c source clock exists, if it is not empty and if
     *                    the \c destination clock exists, \c false else.
     */
    bool            switch_head_to_tail(const clk_idx source, const clk_idx destination, key &moved_index);
    
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
    
    /*!\fn      get(int64_t index)
     * \brief   Returns a reference to the value that corresponds to the specified index
     * \details Returns a reference to the value that corresponds to the specified index,
     *          independent of the membership of that index to any clock.
     *
     * @param index The index whose value gets returned.
     * @return      A reference to the value corresponding the specified index if this index
     *              is valid inside this \link multi_clock \endlink or a reference to the value
     *              of the \link _invalid_index \endlink, else.
     */
    inline int64_t&   get(const int64_t index) {
        return valid_index(index) * _values[index]
               + !valid_index(index) * _values[_invalid_index];
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
    
    /*!\fn      set(int64_t index, int64_t new_value)
     * \brief   Sets the value that corresponds to the specified index
     * \details Sets the value that corresponds to the specified index, independent of the
     *          membership of that index to any clock.
     *
     * @param index     The index whose value gets set.
     * @param new_value The new value for the specified index if this index is valid inside this
     *                  \link multi_clock \endlink or for the \link _invalid_index \endlink, else.
     */
    inline void     set(const int64_t index, int64_t const new_value) {
        _values[valid_index(index) * index
                + !valid_index(index) * _invalid_index] = new_value;
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
    
    /*!\fn      operator[](int64_t index)
     * \brief   Returns a reference to the value that corresponds to the specified index
     * \details Returns a reference to the value that corresponds to the specified index,
     *          independent of the membership of that index to any clock.
     *
     * @param index The index whose value gets returned.
     * @return      A reference to the value corresponding the specified index if this index
     *              is valid inside this \link multi_clock \endlink or a reference to the value
     *              of the \link _invalid_index \endlink, else.
     */
    inline int64_t&   operator[](const int64_t index) {
        return valid_index(index) * _values[index]
               + !valid_index(index) * _values[_invalid_index];
    }
};

#endif //ZERO_MULTI_CLOCK_H
