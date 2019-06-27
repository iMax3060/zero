#include "page_evictioner_select_and_filter.hpp"

using namespace zero::buffer_pool;

template<class selector_class, class filter_class, bool filter_early>
PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::PageEvictionerSelectAndFilter(const zero::buffer_pool::BufferPool* bufferPool) :
        PageEvictioner(bufferPool),
        _selector(bufferPool),
        _filter(bufferPool) {
    static_assert(std::is_base_of<PageEvictionerSelector, selector_class>::value,
                  "'selector_class' is not of type 'PageEvictionerSelector'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value,
                  "'filter_class' is not of type 'PageEvictionerFilter'!");
}

template<class selector_class, class filter_class, bool filter_early>
bf_idx PageEvictionerSelectAndFilter<selector_class, filter_class, filter_early>::pickVictim() noexcept {
    uint32_t attempts = 0;

    while (true) {
        if (should_exit()) return 0; // the buffer index 0 has the semantics of null

        bf_idx selected_index = _selector.select();

        if (!_filter.filterAndUpdate(selected_index)) {
            continue;
        }

        attempts++;
        if (attempts >= _maxAttempts) {
            W_FATAL_MSG(fcINTERNAL, << "Eviction got stuck!");
        } else if (!(_flushDirty
                  && smlevel_0::bf->isNoDBMode()
                  && smlevel_0::bf->usesWriteElision())
                && attempts % _wakeupCleanerAttempts == 0) {
            smlevel_0::bf->wakeupPageCleaner();
        }

        w_assert0(selected_index != 0);

        if (!evictOne(selected_index)) {
            continue;
        }

        ADD_TSTAT(bf_eviction_attempts, attempts);
        return selected_index;
    }
}