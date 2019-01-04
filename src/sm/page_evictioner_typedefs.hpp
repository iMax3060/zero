#ifndef __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
#define __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP

#include "page_evictioner_select_and_filter.hpp"
#include "page_evictioner_selector.hpp"
#include "page_evictioner_filter.hpp"

namespace zero::buffer_pool {
    template <class selector_class, class filter_class, bool filter_early> class PageEvictionerSelectAndFilter;

    class PageEvictionerSelectorLOOPAbsolutelyAccurate;
    class PageEvictionerFilterNone;
    template <bool on_hit = true, bool on_unfix = false, bool on_miss = true, bool on_fixed = false, bool on_dirty = false, bool on_blocked = false, bool on_swizzled = false> class PageEvictionerFilterCLOCK;
    template <uint16_t decrement = 1, bool discriminate_pages = false,
            bool on_hit = true, bool set_on_hit = false, uint16_t level0_on_hit = 5, uint16_t level1_on_hit = 2, uint16_t level2_on_hit = 1,
            bool on_unfix = false, bool set_on_unfix = false, uint16_t level0_on_unfix = 5, uint16_t level1_on_unfix = 2, uint16_t level2_on_unfix = 1,
            bool on_miss = true, bool set_on_miss = true, uint16_t level0_on_miss = 25, uint16_t level1_on_miss = 10, uint16_t level2_on_miss = 5,
            bool on_fixed = false, bool set_on_fixed = false, uint16_t level0_on_fixed = 5, uint16_t level1_on_fixed = 2, uint16_t level2_on_fixed = 1,
            bool on_dirty = false, bool set_on_dirty = false, uint16_t level0_on_dirty = 5, uint16_t level1_on_dirty = 2, uint16_t level2_on_dirty = 1,
            bool on_blocked = false, bool set_on_blocked = false, uint16_t level0_on_blocked = 5, uint16_t level1_on_blocked = 2, uint16_t level2_on_blocked = 1,
            bool on_swizzled = false, bool set_on_swizzled = false, uint16_t level0_on_swizzled = 5, uint16_t level1_on_swizzled = 2, uint16_t level2_on_swizzled = 1> class PageEvictionerFilterGCLOCK;

    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<>, true> PageEvictionerCLOCKFixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<false, true>, true> PageEvictionerCLOCKUnfixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<true, true>, true> PageEvictionerCLOCKFixUnfixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterGCLOCK<>, true> PageEvictionerGCLOCKV1FixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerGCLOCKV2FixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterGCLOCK<1, true>, true> PageEvictionerDGCLOCKV1FixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerDGCLOCKV2FixAbsolutelyAccurate;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
