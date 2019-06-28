#include "page_evictioner_selector.hpp"

using namespace zero::buffer_pool;

PageEvictionerSelector::PageEvictionerSelector(const BufferPool* bufferPool) :
        _maxBufferpoolIndex(bufferPool->getBlockCount() - 1) {}

PageEvictionerSelectorLOOPAbsolutelyAccurate::PageEvictionerSelectorLOOPAbsolutelyAccurate(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _currentFrame(_maxBufferpoolIndex) {}

PageEvictionerSelectorLOOPPracticallyAccurate::PageEvictionerSelectorLOOPPracticallyAccurate(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _currentFrame(1) {}

PageEvictionerSelectorLOOPThreadLocallyAccurate::PageEvictionerSelectorLOOPThreadLocallyAccurate(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool) {}

template <uint32_t retry_list_check_ppm, uint32_t initial_list_check_ppm>
PageEvictionerSelectorQuasiFIFOLowContention<retry_list_check_ppm, initial_list_check_ppm>::PageEvictionerSelectorQuasiFIFOLowContention(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _notExplicitlyEvictedList(bufferPool->getBlockCount()) {}

template <uint32_t retry_list_check_ppm, uint32_t initial_list_check_ppm>
PageEvictionerSelectorQuasiFIFOHighContention<retry_list_check_ppm, initial_list_check_ppm>::PageEvictionerSelectorQuasiFIFOHighContention(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _approximateInitialListLength(0),
        _approximateRetryListLength(0),
        _initialList(bufferPool->getBlockCount()),
        _retryList(bufferPool->getBlockCount()),
        _notExplicitlyEvictedList(bufferPool->getBlockCount()) {}

template <uint32_t retry_list_check_ppm, uint32_t initial_list_check_ppm>
PageEvictionerSelectorQuasiFILOLowContention<retry_list_check_ppm, initial_list_check_ppm>::PageEvictionerSelectorQuasiFILOLowContention(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _notExplicitlyEvictedList(bufferPool->getBlockCount()) {}

PageEvictionerSelectorLRU::PageEvictionerSelectorLRU(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _lruList(bufferPool->getBlockCount()) {}

template <bf_idx protected_block_ppm>
PageEvictionerSelectorSLRU<protected_block_ppm>::PageEvictionerSelectorSLRU(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _protectedBlockCount(static_cast<bf_idx>(protected_block_ppm * 0.000001 * bufferPool->getBlockCount())),
        _protectedLRUList(_protectedBlockCount),
        _probationaryLRUList(bufferPool->getBlockCount()) {}

template <size_t k, bool on_page_unfix>
PageEvictionerSelectorLRUK<k, on_page_unfix>::PageEvictionerSelectorLRUK(const zero::buffer_pool::BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _lruList(k * bufferPool->getBlockCount()),
        _frameReferences(bufferPool->getBlockCount(), 0) {}

template <uint32_t retry_list_check_ppm/* = 1000000*/, uint32_t mru_list_check_ppm/* = 10000*/>
PageEvictionerSelectorQuasiMRU<retry_list_check_ppm, mru_list_check_ppm>::PageEvictionerSelectorQuasiMRU(const zero::buffer_pool::BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _mruList(bufferPool->getBlockCount()) {}

template <bf_idx resort_threshold_ppm>
PageEvictionerSelectorTimestampLRU<resort_threshold_ppm>::PageEvictionerSelectorTimestampLRU(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _timestampsLive(bufferPool->getBlockCount()),
        _lruList0(bufferPool->getBlockCount()),
        _lruList1(bufferPool->getBlockCount()),
        _lastChecked(0),
        _useLRUList0(false),
        _useLRUList1(false) {
    _sortingInProgress.clear(); // _sortingInProgress should be initialized false but sometimes it is true!?
}

template <size_t k, bf_idx resort_threshold_ppm, bool on_page_unfix>
PageEvictionerSelectorTimestampLRUK<k, resort_threshold_ppm, on_page_unfix>::PageEvictionerSelectorTimestampLRUK(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _timestampsLiveOldestTimestamp(bufferPool->getBlockCount()),
        _timestampsLive(bufferPool->getBlockCount()),
        _lruList0(bufferPool->getBlockCount()),
        _lruList1(bufferPool->getBlockCount()),
        _lastChecked(0),
        _useLRUList0(false),
        _useLRUList1(false) {
    _sortingInProgress.clear(); // _sortingInProgress should be initialized false but sometimes it is true!?
}

template <bf_idx resort_threshold_ppm>
PageEvictionerSelectorLFU<resort_threshold_ppm>::PageEvictionerSelectorLFU(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _frequenciesLive(bufferPool->getBlockCount()),
        _lfuList0(bufferPool->getBlockCount()),
        _lfuList1(bufferPool->getBlockCount()),
        _lastChecked(0),
        _useLFUList0(false),
        _useLFUList1(false) {
    _sortingInProgress.clear(); // _sortingInProgress should be initialized false but sometimes it is true!?
}

template <bf_idx resort_threshold_ppm>
PageEvictionerSelectorLFUDA<resort_threshold_ppm>::PageEvictionerSelectorLFUDA(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _frequenciesLive(bufferPool->getBlockCount()),
        _lfuList0(bufferPool->getBlockCount()),
        _lfuList1(bufferPool->getBlockCount()),
        _lastChecked(0),
        _ageFactor(1),
        _useLFUList0(false),
        _useLFUList1(false) {
    _sortingInProgress.clear(); // _sortingInProgress should be initialized false but sometimes it is true!?
}

PageEvictionerSelectorLRDV1::PageEvictionerSelectorLRDV1(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _frameReferences(bufferPool->getBlockCount()),
        _frameFirstReferenced(bufferPool->getBlockCount()),
        _frameAlreadySelected(bufferPool->getBlockCount()),
        _globalReferences(0) {}

template <uint64_t aging_frequency, class aging_function>
PageEvictionerSelectorLRDV2<aging_frequency, aging_function>::PageEvictionerSelectorLRDV2(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _frameReferences(bufferPool->getBlockCount()),
        _frameFirstReferenced(bufferPool->getBlockCount()),
        _frameAlreadySelected(bufferPool->getBlockCount()),
        _globalReferences(0),
        _agingFrequency(aging_frequency * bufferPool->getBlockCount()) {
    static_assert(std::is_base_of<AgingFunction, aging_function>::value,
                  "'selector_class' is not of type 'AgingFunction'!");
}