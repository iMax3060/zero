#include "page_evictioner_filter.hpp"

#include "buffer_pool.hpp"

using namespace zero::buffer_pool;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////// PageEvictionerFilter ////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerFilter::PageEvictionerFilter(const BufferPool* bufferPool) {}

PageEvictionerFilter::~PageEvictionerFilter() {}

//////////////////////////////////////////////////////////////////////////////
////////////////////////// PageEvictionerFilterNone //////////////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerFilterNone::PageEvictionerFilterNone(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool) {}

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

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::PageEvictionerFilterCLOCK(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool),
        _refBits(bufferPool->getBlockCount()) {}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
bool PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::preFilter(bf_idx idx) const noexcept {
    if (_refBits[idx]) {
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
bool PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::filter(bf_idx idx) noexcept {
    if (_refBits[idx]) {
        _refBits[idx] = false;
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageHit(bf_idx idx) noexcept {
    if constexpr (on_hit) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageUnfix(bf_idx idx) noexcept {
    if constexpr (on_unfix) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept {
    if constexpr (on_miss) {
        _refBits[b_idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageFixed(bf_idx idx) noexcept {
    if constexpr (on_fixed) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageDirty(bf_idx idx) noexcept {
    if constexpr (on_dirty) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageBlocked(bf_idx idx) noexcept {
    if constexpr (on_blocked) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageSwizzled(bf_idx idx) noexcept {
    if constexpr (on_swizzled) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_unfix, bool on_miss, bool on_fixed, bool on_dirty, bool on_blocked, bool on_swizzled>
void PageEvictionerFilterCLOCK<on_hit, on_unfix, on_miss, on_fixed, on_dirty, on_blocked, on_swizzled>::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {
    _refBits[idx] = true;
}
