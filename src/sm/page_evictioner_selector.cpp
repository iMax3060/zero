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

////////////////////////////////////////////////////////////////////////////////
///////////////// PageEvictionerSelectorLOOPPracticallyAccurate ////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorLOOPPracticallyAccurate::PageEvictionerSelectorLOOPPracticallyAccurate(const BufferPool* bufferPool) :
        PageEvictionerSelector(bufferPool),
        _current_frame(1) {}

bf_idx PageEvictionerSelectorLOOPPracticallyAccurate::select() noexcept { // Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
    w_assert1(_current_frame > 0 && _current_frame <= _maxBufferpoolIndex);

    while (true) {
        uint_fast32_t this_frame = _current_frame++ % (_maxBufferpoolIndex + 1);
        if (this_frame == 0) {
            continue;
        } else {
            return this_frame;
        }
    }
}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageHit(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageUnfix(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageFixed(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageDirty(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageBlocked(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageSwizzled(bf_idx idx) noexcept {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {}
