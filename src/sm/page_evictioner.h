#ifndef PAGE_EVICTIONER_H
#define PAGE_EVICTIONER_H

#include <unordered_map>
#include "smthread.h"
#include "sm_options.h"
#include "lsn.h"
#include "bf_hashtable.h"
#include "allocator.h"
#include "generic_page.h"
#include "bf_tree_cb.h"

#include "worker_thread.h"

typedef uint32_t clk_idx;

class bf_tree_m;
class generic_page;
struct bf_tree_cb_t;

class page_evictioner_base {
public:
	page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options);
    virtual ~page_evictioner_base();

    /**
     * Every time a page is fixed, this method is called. The policy then should
     * do whatever it wants.
     */
    virtual void            hit_ref(bf_idx idx);
    
    /**
     *
     * @param b_idx
     * @param pid
     */
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    
    /**
     *
     * @param idx
     */
    virtual void            used_ref(bf_idx idx);
    
    /**
     *
     * @param idx
     */
    virtual void            dirty_ref(bf_idx idx);
    
    /**
     *
     * @param idx
     */
    virtual void            block_ref(bf_idx idx);
    
    /**
     *
     * @param idx
     */
    virtual void            swizzle_ref(bf_idx idx);
    
    /**
     *
     * @param idx
     */
    virtual void            unbuffered(bf_idx idx);

    /**
     * Evicts pages from the buffer pool until the preferred amount of frames
     * (\link page_evictioner_base::EVICT_BATCH_RATIO) is in use. This method
     * needs to be called to free a frame in the buffer pool.
     */
    virtual void            evict();

protected:
    /** the buffer pool this cleaner deals with. */
    bf_tree_m*                  _bufferpool;
    bool                        _swizziling_enabled;

    /**
     * Pick victim must return the bf_idx. The corresponding CB must be latched
     * in EX mode. If for any reason it must exit without a victim, this method
     * must return bf_idx 0.
     */
    virtual bf_idx          pick_victim();
    
    bool evict_page(bf_idx idx, PageID &evicted_page);
    
    bool                        _logstats_evict;
private:
	/**
	 * When eviction is triggered, _about_ this number of cb will be evicted at
	 * once. If this amount of cb is already free, the eviction does nothing and
	 * goes back to sleep. Given as a ratio of the buffer size (currently 1%).
	 */
	const float EVICT_BATCH_RATIO = 0.01;

	/**
	 * Last control block examined.
	 */
	bf_idx                      _current_frame;

	/**
	 * In case swizziling is enabled, it will unswizzle the parent point.
	 * Additionally, it will update the parent emlsn.
	 * This two operations are kept in a single method because both require
	 * looking up the parent, latching, etc, so we save some work.
	 */
	bool unswizzle_and_update_emlsn(bf_idx idx);
    
};

class page_evictioner_gclock : public page_evictioner_base {
public:
    page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options);
    virtual ~page_evictioner_gclock();

    virtual void            hit_ref(bf_idx idx);
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    virtual void            used_ref(bf_idx idx);
    virtual void            dirty_ref(bf_idx idx);
    virtual void            block_ref(bf_idx idx);
    virtual void            swizzle_ref(bf_idx idx);
    virtual void            unbuffered(bf_idx idx);

protected:
    virtual bf_idx          pick_victim();

private:
    uint16_t            _k;
    uint16_t*           _counts;
    bf_idx              _current_frame;
};

template<class key>
class hashtable_queue;
template<class key, class value>
class multi_clock;

/*!\class   page_evictioner_car
 * \brief   Page Eviction Algorithm CAR
 *
 * \details Page replacement algorithm CAR as presented in
 *          <A HREF="http://www-cs.stanford.edu/~sbansal/pubs/fast04.pdf">
 *          "CAR: Clock with Adaptive Replacement"</A> by Sorav Bansal and
 *          Dharmendra S. Modha.
 *          To use this page eviction algorithm, the only thing to do is
 *          to set the parameter \c sm_evict_policy \c to \c car \c when
 *          starting the \c zapps \c. Other parameters aren't needed as this
 *          page replacement algorithm is self-tuning.
 *          On construction, this page evictioner needs to be connected to
 *          a bufferpool bf_tree_m for which this will serve. The bufferpool
 *          needs to call ref(bf_idx idx) on every page hit,
 *          miss_ref(bf_idx b_idx, PageID pid) on every page miss and
 *          pick_victim() needs to be called to get a page to evict from the
 *          bufferpool.
 *
 * \author  Max Gilbert
 */
class page_evictioner_car : public page_evictioner_base {
public:
    /*!\fn      page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options)
     * \brief   Constructor for page_evictioner_car
     * \details This instantiates a page evictioner that uses the CAR algorithm
     *          to select victims for replacement. It will serve the specified
     *          \c bufferpool \c but won't use the specified \c options \c as this
     *          page replacement strategy doesn't need any "magic" parameters.
     *
     * @param bufferpool The bf_tree_m the constructed page evictioner is used to
     *                   select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
                        page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options);
    /*!\fn      ~page_evictioner_car()
     * \brief   Destructor for page_evictioner_car
     * \details Destroys this instance including the \link _lock \endlink.
     */
    virtual             ~page_evictioner_car();
    
    /*!\fn      ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details Sets the referenced bit of the specified buffer frame. This prevents
     *          the evictioner to evict this page during the next circulation of the
     *          corresponding clock.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void        hit_ref(bf_idx idx);
    /*!\fn      miss_ref(bf_idx b_idx, PageID pid)
     * \brief   Updates the eviction statistics on page miss
     * \details Classifies the specified buffer frame to be in clock \f$T_1\f$ or
     *          \f$T_2\f$ based on the membership of the referenced page in either
     *          \f$B_1\f$, \f$B_2\f$ of none of the LRU-lists. It also removes entries
     *          from the LRU-lists \f$B_1\f$ or \f$B_2\f$ if needed. The referenced
     *          bit of the specified buffer frame will be unset.
     *
     * @param b_idx The frame of the \link _bufferpool \endlink where the fixed page
     *              is cached in.
     * @param pid   The \c PageID \c of the fixed page.
     */
    virtual void        miss_ref(bf_idx b_idx, PageID pid);
    virtual void        used_ref(bf_idx idx);
    virtual void        dirty_ref(bf_idx idx);
    virtual void        block_ref(bf_idx idx);
    virtual void        swizzle_ref(bf_idx idx);
    virtual void        unbuffered(bf_idx idx);

protected:
    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details This method uses the CAR algorithm to select one buffer frame which
     *          is expected to be used the furthest in the future (with the currently
     *          cached page). It acquires a LATCH_EX to prohibit the usage of the
     *          frame as the content of the buffer frame will definitely change.
     *
     * @return The buffer frame that can be freed.
     */
    bf_idx              pick_victim();

protected:
    /**
     * Represents the clocks T_1 and T_2 which contain eviction-specific metadata
     * of the pages that are inside the bufferpool. Therefore there needs to be
     * two clocks in the multi_clock and the size of the clock equals the size
     * of the bufferpool. As the CAR algorithm only stores a referenced bit, the
     * value stored for each index is of Boolean type. And as the internal operation
     * of multi_clock needs an invalid index (as well as a range of indexes starting
     * from 0), the used invalid index is 0 which isn't used in the bufferpool as
     * well.
     */
    multi_clock<bf_idx, bool>*      _clocks;
    /**
     * Represents the LRU-list B_1 which contains the PageIDs of the pages evicted
     * from T_1.
     */
    hashtable_queue<PageID>*        _b1;
    /**
     * Represents the LRU-list B_2 which contains the PageIDs of the pages evicted
     * from T_2.
     */
    hashtable_queue<PageID>*        _b2;                // B_2
    
    /**
     * Represents the parameter p which acts as a target size of T_1.
     */
    u_int32_t                       _p;                 // parameter p
    
    u_int32_t                       _c;
    
    bf_idx                          _hand_movement;

    /**
     * As the data structures \link _clocks, \link _b1 and \link _b2 aren't thread-safe
     * and as the \link pick_victim() and the \link miss_ref(bf_idx, PageID) methods
     * might change those data structures concurrently, this lock needs to be acquired
     * by those methods. The \link ref() method is only called with the corresponding
     * buffer frame latched and the access is also only atomic and therefore this method
     * doesn't need to acquire this lock for its changes.
     */
    pthread_mutex_t                 _lock;
    
    /**
     * Contains constants that map the names of the clocks used by the CAR algorithm to
     * the indexes used by the \link _clocks data structure.
     */
    enum clock_index {
		T_1 = 0,
		T_2 = 1
	};
};

class page_evictioner_cart : public page_evictioner_base {
public:
            page_evictioner_cart(bf_tree_m *bufferpool, const sm_options &options);
    virtual ~page_evictioner_cart();
    
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    virtual void            hit_ref(bf_idx idx);
    virtual void            used_ref(bf_idx idx);
    virtual void            dirty_ref(bf_idx idx);
    virtual void            block_ref(bf_idx idx);
    virtual void            swizzle_ref(bf_idx idx);
    virtual void            unbuffered(bf_idx idx);

protected:
    virtual bf_idx pick_victim();
    
    /**
     * Contains constants that map the names of the clocks used by the CAR algorithm to
     * the indexes used by the \link _clocks data structure.
     */
    enum clock_index {
        T_1 = 0,
        T_2 = 1
    };
    
    enum filter {
        S = false,
        L = true
    };

private:
    class referenced_filter {
    public:
        referenced_filter() {};
        referenced_filter(bool referenced, filter filter) {
            _referenced = referenced;
            _filter = filter;
        };
        referenced_filter& operator=(referenced_filter& source) {
            this->_referenced = source._referenced;
            this->_filter = source._filter;
    
            return *this;
        }
    
        bool        _referenced;
        filter      _filter;
    };

protected:
    /**
     * Represents the clocks T_1 and T_2 which contain eviction-specific metadata
     * of the pages that are inside the bufferpool. Therefore there needs to be
     * two clocks in the multi_clock and the size of the clock equals the size
     * of the bufferpool. As the CAR algorithm only stores a referenced bit, the
     * value stored for each index is of Boolean type. And as the internal operation
     * of multi_clock needs an invalid index (as well as a range of indexes starting
     * from 0), the used invalid index is 0 which isn't used in the bufferpool as
     * well.
     */
    multi_clock<bf_idx, referenced_filter>*     _clocks;
    /**
     * Represents the LRU-list B_1 which contains the PageIDs of the pages evicted
     * from T_1.
     */
    hashtable_queue<PageID>*                    _b1;
    /**
     * Represents the LRU-list B_2 which contains the PageIDs of the pages evicted
     * from T_2.
     */
    hashtable_queue<PageID>*                    _b2;
    
    /**
     * Represents the parameter p which acts as a target size of T_1.
     */
    u_int32_t                                   _p;
    
    u_int32_t                                   _c;
    
    bf_idx                                      _hand_movement;
    
    /**
     * As the data structures \link _clocks, \link _b1 and \link _b2 aren't thread-safe
     * and as the \link pick_victim() and the \link miss_ref(bf_idx, PageID) methods
     * might change those data structures concurrently, this lock needs to be acquired
     * by those methods. The \link ref() method is only called with the corresponding
     * buffer frame latched and the access is also only atomic and therefore this method
     * doesn't need to acquire this lock for its changes.
     */
    pthread_mutex_t                             _lock;
    
    u_int32_t                                   _q;
    u_int32_t                                   _n_s;
    u_int32_t                                   _n_l;
};

/*!\class   hashtable_queue
 * \brief   Queue with Direct Access
 * \details Represents a queue of keys with direct access using the keys. It
 *          offers the usual queue semantics where entries gets inserted at
 *          the back of the queue and where entries gets removed from the
 *          front of it. But it also offers the possibility to remove a
 *          specified element from somewhere within the queue. The data type
 *          of the entries is specified using the template parameter.
 *          The computational complexity of the direct access as well as
 *          removal and insertion with queue semantics depends on the
 *          implementation of std::unordered_map, as this class is used for
 *          that. The space complexity also depends on the implementation of
 *          std::unordered_map where \c Key \c has a size of the \c key \c
 *          template parameter and where \c T \c has double the size of the
 *          \c key \c template parameter.
 *
 * \note    Could also be implemented using \c Boost.MultiIndex \c.
 *
 * @tparam key The data type of the entries stored in this data structure.
 *
 * \author Max Gilbert
 */
template<class key>
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
         * \brief   Empty Constructor for key_pair
         * \details This constructor instantiates a key_pair without setting
         *          the members \link _previous \endlink and \link _next \endlink.
         */
        key_pair() {}
        /*!\fn      key_pair(key previous, key next)
         * \brief   Constructor for key_pair
         * \details This constructor instantiates a key_pair and initializes the
         *          members \link _previous \endlink and \link _next \endlink
         *          as specified.
         *
         * @param previous The initial value of \link _previous \endlink.
         * @param next     The initial value of \link _next \endlink.
         */
        key_pair(key previous, key next) {
            this->_previous = previous;
            this->_next = next;
        }
        /*!\fn      ~key_pair()
         * \brief   Destructor for key_pair
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
     *          The \c key \c represents an queue entry and the \c key_pair \c
     *          which is mapped to that \c key \c stores the information about
     *          previous and next \c key \c in the queue.
     *
     * \see key_pair
     */
    std::unordered_map<key, key_pair>*      _direct_access_queue;
    /*!\var     _back
     * \brief   Element at the back
     * \details Stores the \c key \c of the element at the back of the queue. This
     *          element was inserted most recently and it will be removed the furthest
     *          in the future (regarding queue semantics). This element doesn't have
     *          a next element but the previous element can be accesses using
     *          \link _direct_access_queue \endlink.
     */
    key                                     _back;          // the back of the list (MRU-element); insert here
    /*!\var     _front
     * \brief   Element at the front
     * \details Stores the \c key \c of the element at the front of the queue. This
     *          element was inserted least recently and it will be removed next
     *          (regarding queue semantics). This element doesn't have a previous
     *          element but the next element can be accesses using
     *          \link _direct_access_queue \endlink.
     */
    key                                     _front;         // the front of the list (LRU-element); remove here
    
    /*!\var     _invalid_key
     * \brief   Invalid (Unused) \c key \c
     * \details This specifies an invalid \c key \c which can be used to mark that
     *          an element in the queue doesn't have a previous or next element. It
     *          can also be used to mark that there is no back or front of the queue
     *          when there is no queue. This should have the semantics of \c null \c for
     *          the specified \c key \c template parameter therefore a natural choice
     *          of a this for the case that \c key \c is a pointer would be \c null \c.
     *
     */
    key                                     _invalid_key;

public:
    /*!\fn      hashtable_queue(key invalid_key)
     * \brief
     * \details
     *
     * @param invalid_key
     */
    hashtable_queue(key invalid_key);
    /*!\fn      ~hashtable_queue()
     * \brief
     * \details
     *
     */
    virtual          ~hashtable_queue();
    
    /*!\fn      contains(key k)
     * \brief
     * \details
     *
     * @param k
     * @return
     */
    bool             contains(key k);
    /*!\fn      insert_back(key k)
     * \brief
     * \details
     *
     * @param k
     * @return
     */
    bool             insert_back(key k);
    /*!\fn      remove_front()
     * \brief
     * \details
     *
     * @return
     */
    bool             remove_front();
    /*!\fn      remove(key k)
     * \brief
     * \details
     *
     * @param k
     * @return
     */
    bool             remove(key k);
    /*!\fn      length()
     * \brief
     * \details
     *
     * @return
     */
    inline u_int32_t length();
};

template<class key, class value>
class multi_clock {
public:
    typedef clk_idx u_int32_t;

private:
    class index_pair {
    public:
        index_pair() {};
        index_pair(key before, key after) {
            this->_before = before;
            this->_after = after;
        };
        
        key     _before;              // visited before
        key     _after;               // visited after
    };
    
    key                             _clocksize;             // number of elements in the multi clock
    value*                          _values;                // stored values of the elements
    index_pair*                     _clocks;                // .first == before, .second == after
    key                             _invalid_index;         // index value with NULL semantics
    clk_idx*                        _clock_membership;      // to which clock does an element belong?
    
    clk_idx                         _clocknumber;           // number of clocks in the multi clock
    key*                            _hands;                 // always points to the clocks head
    key*                            _sizes;                 // number of elements within a clock
    clk_idx                         _invalid_clock_index;   // index of a clock value with NULL semantics

public:
    multi_clock(key clocksize, u_int32_t clocknumber, key invalid_index);
    virtual         ~multi_clock();
    
    bool            get_head(clk_idx clock, value &head_value);
    bool            set_head(clk_idx clock, value new_value);
    bool            get_head_index(clk_idx clock, key &head_index);
    bool            move_head(clk_idx clock);
    bool            add_tail(clk_idx clock, key index);
    bool            remove_head(clk_idx clock, key &removed_index);
    bool            remove(key &index);
    bool            switch_head_to_tail(clk_idx source, clk_idx destination, key &moved_index);
    inline key      size_of(clk_idx clock);
    
    inline value&   get(key index) {
        return _values[index];
    }
    inline void     set(key index, value new_value) {
        _values[index] = new_value;
    }
    inline value&   operator[](key index) {
        return _values[index];
    }
};

#endif
