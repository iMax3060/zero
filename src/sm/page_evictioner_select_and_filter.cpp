#include "page_evictioner_select_and_filter.hpp"

#include "buffer_pool.hpp"

using namespace zero::buffer_pool;

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::PageEvictionerSelectAndFilter(const BufferPool* bufferPool) :
        PageEvictioner(bufferPool),
        _selector(bufferPool),
        _filter(bufferPool) {
    static_assert(std::is_base_of<PageEvictionerSelector, selector_class>::value,
                  "'selector_class' is not of type 'PageEvictionerSelector'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value,
                  "'filter_class' is not of type 'PageEvictionerFilter'!");
};

template <class selector_class, class filter_class, bool filter_early>
PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::~PageEvictionerSelectAndFilter() {};

template <class selector_class, class filter_class, bool filter_early>
bf_idx PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::pickVictim() noexcept {
    uint32_t attempts = 0;

    while (true) {

        if (should_exit()) return 0; // the buffer index 0 has the semantics of null

        bf_idx idx = _selector.select();

        if constexpr (filter_early) {
            if (!_filter.preFilter(idx)) {
                continue;
            }
        }

        attempts++;
        if (attempts >= _maxAttempts) {
            W_FATAL_MSG(fcINTERNAL, << "Eviction got stuck!");
        } else if (!(_flushDirty && smlevel_0::bf->isNoDBMode() && smlevel_0::bf->usesWriteElision())
                && attempts % _wakeupCleanerAttempts == 0) {
            smlevel_0::bf->wakeupPageCleaner();
        }

        bf_tree_cb_t &cb = smlevel_0::bf->getControlBlock(idx);

        if (!cb._used) {
            continue;
        }

        // If I already hold the latch on this page (e.g., with latch coupling), then the latch acquisition below will
        // succeed, but the page is obviously not available for eviction. This would not happen if every fix would also
        // pin the page, which I didn't want to do because it seems like a waste. Note that this is only a problem with
        // threads performing their own eviction (i.e., with the option _asyncEviction set to false in BufferPool),
        // because otherwise the evictioner thread never holds any latches other than when trying to evict a page.
        // This bug cost me 2 days of work. Anyway, it should work with the check below for now.
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
        if (!_filter.filter(idx)) {
            cb.latch().latch_release();
            continue; 
        }

        // Only evict actually evictable pages (not required to stay in the buffer pool)
        if (!smlevel_0::bf->isEvictable(idx, _flushDirty)) {
            cb.latch().latch_release();
            continue;
        }

        // If we got here, we passed all tests and have a victim!
        w_assert1(smlevel_0::bf->isActiveIndex(idx));
        w_assert0(idx != 0);
        ADD_TSTAT(bf_eviction_attempts, attempts);
        return idx;
    }
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageHit(bf_idx idx) noexcept {
    _selector.updateOnPageHit(idx);
    _filter.updateOnPageHit(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageUnfix(bf_idx idx) noexcept {
    _selector.updateOnPageUnfix(idx);
    _filter.updateOnPageUnfix(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageMiss(bf_idx idx, PageID pid) noexcept {
    _selector.updateOnPageMiss(idx, pid);
    _filter.updateOnPageMiss(idx, pid);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageFixed(bf_idx idx) noexcept {
    _selector.updateOnPageFixed(idx);
    _filter.updateOnPageFixed(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageDirty(bf_idx idx) noexcept {
    _selector.updateOnPageDirty(idx);
    _filter.updateOnPageDirty(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageBlocked(bf_idx idx) noexcept {
    _selector.updateOnPageBlocked(idx);
    _filter.updateOnPageBlocked(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageSwizzled(bf_idx idx) noexcept {
    _selector.updateOnPageSwizzled(idx);
    _filter.updateOnPageSwizzled(idx);
};

template <class selector_class, class filter_class, bool filter_early>
void PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {
    _selector.updateOnPageExplicitlyUnbuffered(idx);
    _filter.updateOnPageExplicitlyUnbuffered(idx);
};

template PageEvictionerLOOPAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerLOOPAbsolutelyAccurate::pickVictim();
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerLOOPAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKFixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKFixAbsolutelyAccurate::pickVictim();
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKFixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKUnfixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKUnfixAbsolutelyAccurate::pickVictim();
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKUnfixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::pickVictim();
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerGCLOCKV1FixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerGCLOCKV1FixAbsolutelyAccurate::pickVictim();
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerGCLOCKV1FixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerGCLOCKV2FixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerGCLOCKV2FixAbsolutelyAccurate::pickVictim();
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerGCLOCKV2FixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::pickVictim();
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::pickVictim();
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerLOOPPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerLOOPPracticallyAccurate::pickVictim();
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerLOOPPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKFixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKFixPracticallyAccurate::pickVictim();
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKFixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKUnfixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKUnfixPracticallyAccurate::pickVictim();
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKUnfixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerCLOCKFixUnfixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerCLOCKFixUnfixPracticallyAccurate::pickVictim();
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerCLOCKFixUnfixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerGCLOCKV1FixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerGCLOCKV1FixPracticallyAccurate::pickVictim();
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerGCLOCKV1FixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerGCLOCKV2FixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerGCLOCKV2FixPracticallyAccurate::pickVictim();
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerGCLOCKV2FixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerDGCLOCKV1FixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerDGCLOCKV1FixPracticallyAccurate::pickVictim();
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerDGCLOCKV1FixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);

template PageEvictionerDGCLOCKV2FixPracticallyAccurate::PageEvictionerSelectAndFilter(const BufferPool* bufferPool);
template bf_idx PageEvictionerDGCLOCKV2FixPracticallyAccurate::pickVictim();
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageHit(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageUnfix(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageMiss(bf_idx idx, PageID pid);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageFixed(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageDirty(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageBlocked(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageSwizzled(bf_idx idx);
template void PageEvictionerDGCLOCKV2FixPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx);
// to be continued ...
