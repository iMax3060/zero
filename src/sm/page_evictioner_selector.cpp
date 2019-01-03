#include "page_evictioner_selector.hpp"

#include "buffer_pool.hpp"

using namespace zero::buffer_pool;

//////////////////////////////////////////////////////////////////////////////
/////////////////////////// PageEvictionerSelector ///////////////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerSelector::PageEvictionerSelector(const BufferPool* bufferPool) :
        _maxBufferpoolIndex(bufferPool->getBlockCount() - 1) {}

PageEvictionerSelector::~PageEvictionerSelector() {}

//////////////////////////////////////////////////////////////////////////////
//////////////// PageEvictionerSelectorLOOPAbsolutelyAccurate ////////////////
//////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorLOOPAbsolutelyAccurate::PageEvictionerSelectorLOOPAbsolutelyAccurate(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _currentFrame(_maxBufferpoolIndex) {}

bf_idx PageEvictionerSelectorLOOPAbsolutelyAccurate::select() noexcept {
    w_assert1(_currentFrame > 0 && _currentFrame <= _maxBufferpoolIndex);

    std::lock_guard<std::mutex> guard(_currentFrameLock);
    if (_currentFrame >= _maxBufferpoolIndex) {
        _currentFrame = 1;
    } else {
        _currentFrame++;
    }
    return _currentFrame;
}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageHit(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageMiss(bf_idx idx, PageID pid) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageFixed(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageDirty(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {}
