#include "page_evictioner_sf_skeleton.hpp"

#include "page_evictioner_selector.hpp"
#include "page_evictioner_filter.hpp"

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSFSkeleton::PageEvictionerSFSkeleton(bf_tree_m *bufferpool, const sm_options &options) :
        page_evictioner_base(bufferpool, options),
        _selector(bufferpool, options),
        _filter(bufferpool, options) {
    static_assert(std::is_base_of<page_evictioner_selector, selector_class>::value, "'selector_class' is not of type 'page_evictioner_selector'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value, "'filter_class' is not of type 'PageEvictionerFilter'!");
};

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSFSkeleton::~PageEvictionerSFSkeleton() {};

template <class selector_class, class filter_class, bool filter_early>
bf_idx PageEvictionerSFSkeleton::pickVictim() final {
    uint32_t attempts = 0;

    while (true) {

        if (should_exit()) return 0; // in bf_tree.h, 0 is never used, means null

        bf_idx idx = _selector.select();

        if (filter_early && !_filter.filter<true>(idx)) {
            continue;
        }
        
        attempts++;
        if (attempts >= _max_attempts) {
            W_FATAL_MSG(fcINTERNAL, << "Eviction got stuck!");
        } else if (!(_flush_dirty && _bufferpool->_no_db_mode && _bufferpool->_write_elision)
                && attempts % _wakeup_cleaner_attempts == 0) {
            _bufferpool->wakeup_cleaner();
        }

        bf_tree_cb_t& cb = _bufferpool->get_cb(idx);

        if (!cb._used) {
            continue;
        }

        // If I already hold the latch on this page (e.g., with latch
        // coupling), then the latch acquisition below will succeed, but the
        // page is obviously not available for eviction. This would not happen
        // if every fix would also pin the page, which I didn't wan't to do
        // because it seems like a waste.  Note that this is only a problem
        // with threads perform their own eviction (i.e., with the option
        // _async_eviction set to false in bf_tree_m), because otherwise the
        // evictioner thread never holds any latches other than when trying to
        // evict a page.  This bug cost me 2 days of work. Anyway, it should
        // work with the check below for now.
        if (cb.latch().held_by_me()) {
            // I (this thread) currently have the latch on this frame, so
            // obviously I should not evict it
            updateOnPageFixed(idx);
            continue;
        }

        // Step 1: latch page in EX mode and check if eligible for eviction
        rc_t latch_rc;
        latch_rc = cb.latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error()) {
            updateOnPageFixed(idx);
            DBG3(<< "Eviction failed on latch for " << idx);
            continue;
        }
        w_assert1(cb.latch().is_mine());

        // Only evict if clock referenced bit is not set
        if (!_filter.filter<!filter_early>(idx)) {
            cb.latch().latch_release();
            continue;
        }

        // Only evict actually evictable pages (not required to stay in the buffer pool)
        if (!_bufferpool->is_evictable(idx, _flush_dirty)) {
            cb.latch().latch_release();
            continue;
        }

        // If we got here, we passed all tests and have a victim!
        w_assert1(_bufferpool->_is_active_idx(idx));
        w_assert0(idx != 0);
        ADD_TSTAT(bf_eviction_attempts, attempts);
        return idx;
    }
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx) final {
    _selector.updateOnPageHit(idx);
    _filter.updateOnPageHit(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx) final {
    _selector.updateOnPageUnfix(idx);
    _filter.updateOnPageUnfix(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid) final {
    _selector.updateOnPageMiss(b_idx, pid);
    _filter.updateOnPageMiss(b_idx, pid);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx) final {
    _selector.updateOnPageFixed(idx);
    _filter.updateOnPageFixed(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx) final {
    _selector.updateOnPageDirty(idx);
    _filter.updateOnPageDirty(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx) final {
    _selector.updateOnPageBlocked(idx);
    _filter.updateOnPageBlocked(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx) final {
    _selector.updateOnPageSwizzled(idx);
    _filter.updateOnPageSwizzled(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx) final {
    _selector.updateOnPageExplicitlyUnbuffered(idx);
    _filter.updateOnPageExplicitlyUnbuffered(idx);
};

template class PageEvictionerSFSkeleton<page_evictioner_selector_loop, PageEvictionerFilterNone, false>;
// to be continued ...