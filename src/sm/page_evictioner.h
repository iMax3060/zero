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
    virtual void            ref(bf_idx idx);
    
    /**
     *
     * @param b_idx
     * @param pid
     */
    virtual void            miss_ref(bf_idx b_idx, PageID pid);

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

    virtual void            ref(bf_idx idx);
    virtual void            miss_ref(bf_idx b_idx, PageID pid);

protected:
    virtual bf_idx          pick_victim();

private:
    uint16_t            _k;
    uint16_t*           _counts;
    bf_idx              _current_frame;
};

class page_evictioner_car : public page_evictioner_base {
public:
                        page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options);
    virtual             ~page_evictioner_car();
    
    virtual void        ref(bf_idx idx);
    virtual void        miss_ref(bf_idx b_idx, PageID pid);

protected:
    bf_idx              pick_victim();

private:
    // Alternative: Boost.MultiIndex
    template<class key>
    class hashtable_queue {
    private:
        class key_pair {
        public:
            key_pair() {}
            key_pair(key previous, key next) {
                this->_previous = previous;
                this->_next = next;
            }
            virtual ~key_pair() {}
    
            key     _previous;          // closer to the front of the queue
            key     _next;              // closer to the back of the queue
        };
        
        std::unordered_map<key, key_pair>*      _direct_access_queue;
        key                                     _back;          // the back of the list (MRU-element); insert here
        key                                     _front;         // the front of the list (LRU-element); remove here
	    
	    key                                     _invalid_key;

    public:
                        hashtable_queue(key invalid_key);
        virtual         ~hashtable_queue();

        bool            contains(key k);
        bool            insert_back(key k);
        bool            remove_front();
        bool            remove(key k);
        u_int32_t       length();
    };
    
    template<class value>
    class multi_clock {
    private:
        class index_pair {
        public:
            index_pair() {};
            index_pair(u_int32_t before, u_int32_t after) {
                this->_before = before;
                this->_after = after;
            };
            virtual ~index_pair() {};
    
            u_int32_t     _before;              // visited before
            u_int32_t     _after;               // visited after
        };
    
        u_int32_t                       _clocksize;             // number of elements in the multi clock
        value*                          _values;                // stored values of the elements
        index_pair*                     _clocks;                // .first == before, .second == after
        u_int32_t                       _invalid_index;         // index value with NULL semantics
        u_int32_t*                      _clock_membership;      // to which clock does an element belong?
        
        u_int32_t                       _clocknumber;           // number of clocks in the multi clock
        u_int32_t*                      _hands;                 // always points to the clocks head
        u_int32_t*                      _sizes;                 // number of elements within a clock
        u_int32_t                       _invalid_clock_index;   // index of a clock value with NULL semantics
    public:
                        multi_clock(u_int32_t clocksize, u_int32_t clocknumber, u_int32_t invalid_index);
        virtual         ~multi_clock();
        
        bool            get_head(u_int32_t clock, value &head_value);
        bool            set_head(u_int32_t clock, value new_value);
        bool            get_head_index(u_int32_t clock, u_int32_t &head_index);
        bool            move_head(u_int32_t clock);
        bool            add_tail(u_int32_t clock, u_int32_t index);
        bool            remove_head(u_int32_t clock, u_int32_t &removed_index);
        bool            switch_head_to_tail(u_int32_t source, u_int32_t destination, u_int32_t &moved_index);
        u_int32_t       size_of(u_int32_t clock);
        
        inline value&   get(u_int32_t index) {
            return _values[index];
        }
        inline void     set(u_int32_t index, value new_value) {
            _values[index] = new_value;
        }
    };

private:
	multi_clock<bool>*              _clocks;            // T_1 and T_2

	hashtable_queue<PageID>*        _b1;                // B_1
	hashtable_queue<PageID>*        _b2;                // B_2

	u_int32_t                       _p;                 // parameter p
	
	pthread_mutex_t                 _lock;

	enum clock_index {
		T_1 = 0,
		T_2 = 1
	};
};

/*
class page_evictioner_clockpro : public page_evictioner_base {
public:
                        page_evictioner_clockpro(bf_tree_m* bufferpool, const sm_options& options);
    virtual             ~page_evictioner_clockpro();

    virtual void        ref(bf_idx idx);
    virtual void        miss_ref(bf_idx b_idx, PageID pid);

protected:
    virtual bf_idx      pick_victim();

private:
    virtual void            run_hand_hot();
    virtual bf_idx          run_hand_cold();
    virtual void            run_hand_test();
    
    bool*                   _hot;           // 1: page is hot; 0: page is cold
    bool*                   _test;          // 1: cold page in test period; 0: hot page or cold page not in test period
    bool*                   _referenced;    // reference bit like in CLOCK
    
    clk_idx*                _bf_idx_to_clk_idx;
    bf_idx*                 _clk_idx_to_bf_idx;
    bf_hashtable<clk_idx>*  _page_id_to_clk_idx;
    
    clk_idx                 _hand_hot;
    clk_idx                 _hand_cold;
    clk_idx                 _hand_test;
};
*/

#endif