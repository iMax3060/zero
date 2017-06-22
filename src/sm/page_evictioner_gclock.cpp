#include "page_evictioner_gclock.h"

#include "btree_page_h.h"

page_evictioner_gclock::page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options)
        : page_evictioner_base(bufferpool, options)
{
    _k = options.get_int_option("sm_bufferpool_gclock_k", 10);
    _counts = new uint16_t [_bufferpool->_block_cnt];
    _current_frame = 0;
    
}

page_evictioner_gclock::~page_evictioner_gclock()
{
    delete [] _counts;
}

void page_evictioner_gclock::hit_ref(bf_idx idx) {
    _counts[idx] = _k;
}

void page_evictioner_gclock::unfix_ref(bf_idx idx) {
    _counts[idx] = _k;
}

void page_evictioner_gclock::miss_ref(bf_idx /*b_idx*/, PageID /*pid*/) {}

void page_evictioner_gclock::used_ref(bf_idx idx) {
    _counts[idx] = _k;
}

void page_evictioner_gclock::dirty_ref(bf_idx /*idx*/) {}

void page_evictioner_gclock::block_ref(bf_idx idx) {
    _counts[idx] = std::numeric_limits<uint16_t>::max();
}

void page_evictioner_gclock::swizzle_ref(bf_idx /*idx*/) {}

void page_evictioner_gclock::unbuffered(bf_idx idx) {
    _counts[idx] = 0;
}

bf_idx page_evictioner_gclock::pick_victim()
{
    // Check if we still need to evict
    bf_idx idx = _current_frame;
    while(true)
    {
        if(should_exit()) return 0; // bf_idx 0 is never used, means NULL
        
        // Circular iteration, jump idx 0
        idx = (idx % (_bufferpool->_block_cnt-1)) + 1;
        w_assert1(idx != 0);
        
        // Before starting, let's fire some prefetching for the next step.
        bf_idx next_idx = ((idx+1) % (_bufferpool->_block_cnt-1)) + 1;
        __builtin_prefetch(&_bufferpool->_buffer[next_idx]);
        __builtin_prefetch(_bufferpool->get_cbp(next_idx));
        
        // Now we do the real work.
        bf_tree_cb_t& cb = _bufferpool->get_cb(idx);
        
        rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, timeout_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error())
        {
            idx++;
            continue;
        }
        
        w_assert1(cb.latch().held_by_me());
        
        /* There are some pages we want to ignore in our policy:
         * 1) Non B+Tree pages
         * 2) Dirty pages (the cleaner should have cleaned it already)
         * 3) Pages being used by someon else
         * 4) The root
         */
        btree_page_h p;
        p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
        if (p.tag() != t_btree_p || cb.is_dirty() ||
            !cb._used || p.pid() == p.root())
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            idx++;
            continue;
        }
        
        // Ignore pages that still have swizzled children
        if(_swizzling_enabled && _bufferpool->has_swizzled_child(idx))
        {
            // LL: Should we also decrement the clock count in this case?
            cb.latch().latch_release();
            idx++;
            continue;
        }
        
        if(_counts[idx] <= 0)
        {
            // We have found our victim!
            bool would_block;
            cb.latch().upgrade_if_not_block(would_block); //Try to upgrade latch
            if(!would_block) {
                w_assert1(cb.latch().is_mine());
                
                /* No need to re-check the values above, because the cb was
                 * already latched in SH mode, so they cannot change. */
                
                if (cb._pin_cnt != 0) {
                    cb.latch().latch_release(); // pin count -1 means page was already evicted
                    idx++;
                    continue;
                }
                
                _current_frame = idx + 1;
                return idx;
            }
        }
        cb.latch().latch_release();
        --_counts[idx]; //TODO: MAKE ATOMIC
        idx++;
    }
}
