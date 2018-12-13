#include "page_evictioner_sf_skeleton.hpp"

#include "buffer_pool.hpp"
#include "page_evictioner_selector.hpp"
#include "page_evictioner_filter.hpp"

using namespace zero::buffer_pool;

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::PageEvictionerSFSkeleton(BufferPool* bufferPool, const sm_options& options) :
        _selector(std::make_unique<selector_class>(bufferPool, options)),
        _filter(std::make_unique<filter_class>(bufferPool, options)),
        PageEvictioner(bufferPool, options) {
    static_assert(std::is_base_of<PageEvictionerSelector, selector_class>::value, "'selector_class' is not of type 'PageEvictionerSelector'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value, "'filter_class' is not of type 'PageEvictionerFilter'!");
};

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::~PageEvictionerSFSkeleton() {};

template <class selector_class, class filter_class, bool filter_early>
bf_idx PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::pickVictim() {
    uint32_t attempts = 0;

    while (true) {

        if (should_exit()) return 0; // in bf_tree.h, 0 is never used, means null

        bf_idx idx = _selector->select();

        if constexpr (filter_early) {
            if (!_filter.preFilter(idx)) {
                continue;
            }
        }

        attempts++;
        if (attempts >= _maxAttempts) {
            W_FATAL_MSG(fcINTERNAL, << "Eviction got stuck!");
        } else if (!(_flushDirty &&_bufferPool->isNoDBMode() &&_bufferPool->usesWriteElision()) && attempts % _wakeupCleanerAttempts == 0) {
            _bufferPool->wakeupPageCleaner();
        }

        bf_tree_cb_t &cb = _bufferPool->getControlBlock(idx);

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
        if (!_filter->filter(idx)) {
            cb.latch().latch_release();
            continue; 
        }

        // Only evict actually evictable pages (not required to stay in the buffer pool)
        if (!_bufferPool->isEvictable(idx, _flushDirty)) {
            cb.latch().latch_release();
            continue;
        }

        // If we got here, we passed all tests and have a victim!
        w_assert1(_bufferPool->isActiveIndex(idx));
        w_assert0(idx != 0);
        ADD_TSTAT(bf_eviction_attempts, attempts);
        return idx;
    }
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageHit(bf_idx idx) {
    _selector->updateOnPageHit(idx);
    _filter->updateOnPageHit(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageUnfix(bf_idx idx) {
    _selector->updateOnPageUnfix(idx);
    _filter->updateOnPageUnfix(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _selector->updateOnPageMiss(b_idx, pid);
    _filter->updateOnPageMiss(b_idx, pid);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageFixed(bf_idx idx) {
    _selector->updateOnPageFixed(idx);
    _filter->updateOnPageFixed(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageDirty(bf_idx idx) {
    _selector->updateOnPageDirty(idx);
    _filter->updateOnPageDirty(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageBlocked(bf_idx idx) {
    _selector->updateOnPageBlocked(idx);
    _filter->updateOnPageBlocked(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageSwizzled(bf_idx idx) {
    _selector->updateOnPageSwizzled(idx);
    _filter->updateOnPageSwizzled(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSFSkeleton<selector_class, filter_class, filter_early>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _selector->updateOnPageExplicitlyUnbuffered(idx);
    _filter->updateOnPageExplicitlyUnbuffered(idx);
};

template PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::PageEvictionerSFSkeleton(BufferPool* bufferPool, const sm_options& options);
template bf_idx PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::pickVictim();
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageHit(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageMiss(bf_idx b_idx, PageID pid);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageFixed(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageDirty(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerSFSkeleton<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>::updateOnPageExplicitlyUnbuffered(bf_idx idx);
// to be continued ...
