#ifndef __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
#define __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP

#include <cstdint>

namespace zero::buffer_pool {
    /* BEGIN --- Forward Declarations ---BEGIN */
    template <class selector_class, class filter_class, bool filter_early> class PageEvictionerSelectAndFilter;

    class PageEvictionerSelectorLOOPAbsolutelyAccurate;
    class PageEvictionerSelectorLOOPPracticallyAccurate;
    class PageEvictionerSelectorRANDOMDefault;
    class PageEvictionerSelectorRANDOMFastRand;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOLowContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOHighContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFILOLowContention;
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
    /* END --- Forward Declarations --- END */

    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPPracticallyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMDefault, PageEvictionerFilterNone, false> PageEvictionerDafaultRandom;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterNone, false> PageEvictionerFastRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<>, true> PageEvictionerCLOCKFix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<false, true>, true> PageEvictionerCLOCKUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<true, true>, true> PageEvictionerCLOCKFixUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<>, true> PageEvictionerGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true>, true> PageEvictionerDGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerDGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterCLOCK<>, true> PageEvictioner0CLOCKFix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterCLOCK<false, true>, true> PageEvictioner0CLOCKUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterCLOCK<true, true>, true> PageEvictioner0CLOCKFixUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterGCLOCK<>, true> PageEvictioner0GCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictioner0GCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterGCLOCK<1, true>, true> PageEvictioner0DGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictioner0DGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOLowContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOHighContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOHighContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFILOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFILOLowContention;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
