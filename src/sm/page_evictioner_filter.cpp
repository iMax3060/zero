#include "page_evictioner_filter.hpp"

using namespace zero::buffer_pool;

PageEvictionerFilter::PageEvictionerFilter(const BufferPool* bufferPool) {}

PageEvictionerFilterNone::PageEvictionerFilterNone(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool) {}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::PageEvictionerFilterCLOCK(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool),
        _refBits(bufferPool->getBlockCount()) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
PageEvictionerFilterGCLOCK<decrement, discriminate_pages,
        on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit,
        on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix,
        on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss,
        on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed,
        on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty,
        on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked,
        on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::PageEvictionerFilterGCLOCK(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool),
        _refInts(bufferPool->getBlockCount()) {}