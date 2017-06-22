#ifndef ZERO_PAGE_EVICTIONER_CART_H
#define ZERO_PAGE_EVICTIONER_CART_H

#include "page_evictioner.h"
#include "multi_clock.h"
#include "hashtable_queue.h"

class page_evictioner_cart : public page_evictioner_base {
public:
                        page_evictioner_cart(bf_tree_m *bufferpool, const sm_options &options);
    virtual             ~page_evictioner_cart();
    
    virtual void        hit_ref(bf_idx idx);
    virtual void        unfix_ref(bf_idx idx);
    virtual void        miss_ref(bf_idx b_idx, PageID pid);
    virtual void        used_ref(bf_idx idx);
    virtual void        dirty_ref(bf_idx idx);
    virtual void        block_ref(bf_idx idx);
    virtual void        swizzle_ref(bf_idx idx);
    virtual void        unbuffered(bf_idx idx);

protected:
    virtual bf_idx      pick_victim();
    
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
    uint32_t                                    _p;
    
    uint32_t                                    _c;
    
    bf_idx                                      _hand_movement;
    
    /**
     * As the data structures \link _clocks, \link _b1 and \link _b2 aren't thread-safe
     * and as the \link pick_victim() and the \link miss_ref(bf_idx, PageID) methods
     * might change those data structures concurrently, this lock needs to be acquired
     * by those methods. The \link ref() method is only called with the corresponding
     * buffer frame latched and the access is also only atomic and therefore this method
     * doesn't need to acquire this lock for its changes.
     */
    srwlock_t                                   _lock;
    
    uint32_t                                    _q;
    uint32_t                                    _n_s;
    uint32_t                                    _n_l;
};

#endif //ZERO_PAGE_EVICTIONER_CART_H
