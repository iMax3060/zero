#include "page_evictioner_filter.hpp"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PageEvictionerFilter ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerFilter::PageEvictionerFilter(bf_tree_m& bufferpool, const sm_options& options) {}

PageEvictionerFilter::~PageEvictionerFilter() {}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterNone //////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerFilterNone::PageEvictionerFilterNone(bf_tree_m& bufferpool, const sm_options& options) :
        PageEvictionerFilter(bufferpool, options) {}

template <bool first_call>
bool PageEvictionerFilterNone::filter<first_call>(bf_idx idx) {
    return true;
}

void PageEvictionerFilterNone::updateOnPageHit(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageUnfix(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

void PageEvictionerFilterNone::updateOnPageFixed(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageDirty(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageBlocked(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageSwizzled(bf_idx idx) {}

void PageEvictionerFilterNone::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterCLOCK /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::PageEvictionerFilterCLOCK(bf_tree_m &bufferpool, const sm_options &options) :
        PageEvictionerFilter(bufferpool, options),
        _clock_ref_bits(bufferpool.get_block_cnt()) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
template <>
bool PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::filter<true>(bf_idx idx) {
    if (_clock_ref_bits[idx]) {
        _clock_ref_bits[idx] = false;
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
template <>
bool PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::filter<false>(bf_idx idx) {
    if (_clock_ref_bits[idx]) {
        return false;
    } else {
        return true;
    }
}

template <bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<true, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<false, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) {}

template <bool on_hit, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, true, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, false, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) {}

template <bool on_hit, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, true, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, false, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, true, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, false, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, true, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, false, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, true, on_swizzle>::updateOnPageBlocked(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, false, on_swizzle>::updateOnPageBlocked(bf_idx idx) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, true>::updateOnPageSwizzled(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, false>::updateOnPageSwizzled(bf_idx idx) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template class page_evictioner_filter_clock<true, true, false, false, false, false, false>;

////////////////////////////////////////////////////////////////////////////////
//////////////////////// page_evictioner_filter_gclock /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::page_evictioner_filter_clock(bf_tree_m &bufferpool, const sm_options &options) :
        page_evictioner_filter(bufferpool, options),
        _clock_ref_bits(bufferpool.get_block_cnt()) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
template <>
bool page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::filter<true>(bf_idx idx) {
    if (_clock_ref_bits[idx]) {
        _clock_ref_bits[idx] = false;
        return false;
    } else {
        return true;
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
template <>
bool page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::filter<false>(bf_idx idx) {
    if (_clock_ref_bits[idx]) {
        return false;
    } else {
        return true;
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<true, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<false, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, true, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, false, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, true, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, false, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, true, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, false, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, true, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, false, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, true, on_swizzle>::updateOnPageBlocked(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, false, on_swizzle>::updateOnPageBlocked(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, true>::updateOnPageSwizzled(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, false>::updateOnPageSwizzled(bf_idx idx) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle,>
void page_evictioner_filter_clock<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _clock_ref_bits[idx] = true;
}