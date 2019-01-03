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

    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<>, false> PageEvictionerCLOCKFixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<false, true>, false> PageEvictionerCLOCKUnfixAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterCLOCK<true, true>, false> PageEvictionerCLOCKFixUnfixAbsolutelyAccurate;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
