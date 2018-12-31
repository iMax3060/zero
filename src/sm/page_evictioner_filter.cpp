#include "page_evictioner_filter.hpp"

using namespace zero::buffer_pool;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PageEvictionerFilter ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerFilter::PageEvictionerFilter() {}

PageEvictionerFilter::~PageEvictionerFilter() {}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterNone //////////////////////////
////////////////////////////////////////////////////////////////////////////////

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
