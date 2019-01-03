#include "page_evictioner_filter.hpp"

#include "buffer_pool.hpp"

using namespace zero::buffer_pool;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////// PageEvictionerFilter ////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerFilter::PageEvictionerFilter() {}

PageEvictionerFilter::~PageEvictionerFilter() {}

//////////////////////////////////////////////////////////////////////////////
////////////////////////// PageEvictionerFilterNone //////////////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerFilterNone::PageEvictionerFilterNone() :
        PageEvictionerFilter() {}

bool PageEvictionerFilterNone::preFilter(bf_idx idx) const noexcept {
    return true;
}

bool PageEvictionerFilterNone::filter(bf_idx idx) noexcept {
    return true;
}

void PageEvictionerFilterNone::updateOnPageHit(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageUnfix(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageMiss(bf_idx idx, PageID pid) noexcept {}

void PageEvictionerFilterNone::updateOnPageFixed(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageDirty(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageBlocked(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageSwizzled(bf_idx idx) noexcept {}

void PageEvictionerFilterNone::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {}

//////////////////////////////////////////////////////////////////////////////
////////////////////////// PageEvictionerFilterCLOCK /////////////////////////
//////////////////////////////////////////////////////////////////////////////

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::PageEvictionerFilterCLOCK() :
        PageEvictionerFilter(),
        _refBits(smlevel_0::bf->getBlockCount()) {}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
bool PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::preFilter(bf_idx idx) const noexcept {
    if (_refBits[idx]) {
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
bool PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::filter(bf_idx idx) noexcept {
    if (_refBits[idx]) {
        _refBits[idx] = false;
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) noexcept {
    if constexpr (on_hit) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) noexcept {
    if constexpr (on_unfix) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept {
    if constexpr (on_miss) {
        _refBits[b_idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) noexcept {
    if constexpr (on_used) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) noexcept {
    if constexpr (on_dirty) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageBlocked(bf_idx idx) noexcept {
    if constexpr (on_block) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageSwizzled(bf_idx idx) noexcept {
    if constexpr (on_swizzle) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_used, on_dirty, on_block, on_swizzle>::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {
    _refBits[idx] = true;
}
