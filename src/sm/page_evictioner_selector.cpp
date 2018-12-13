#include "page_evictioner_selector.hpp"

#include "buffer_pool.hpp"

using namespace zero::buffer_pool;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// PageEvictionerSelector ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelector::PageEvictionerSelector(BufferPool* bufferPool, const sm_options& options) :
        _max_bufferpool_index(bufferPool->getBlockCount() - 1) {}

PageEvictionerSelector::~PageEvictionerSelector() {}

////////////////////////////////////////////////////////////////////////////////
///////////////// PageEvictionerSelectorLOOPAbsolutelyAccurate /////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorLOOPAbsolutelyAccurate::PageEvictionerSelectorLOOPAbsolutelyAccurate(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerSelector(bufferPool, options),
        _current_frame(1) {}

bf_idx PageEvictionerSelectorLOOPAbsolutelyAccurate::select() {
    w_assert1(_current_frame > 0 && _current_frame <= _max_bufferpool_index);

    std::lock_guard<std::mutex> guard(_current_frame_lock);
    if (_current_frame >= _max_bufferpool_index) {
        _current_frame = 1;
    } else {
        _current_frame++;
    }
    return _current_frame;
}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageHit(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageUnfix(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageFixed(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageDirty(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageBlocked(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageSwizzled(bf_idx idx) {}

void PageEvictionerSelectorLOOPAbsolutelyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}

////////////////////////////////////////////////////////////////////////////////
///////////////// PageEvictionerSelectorLOOPPracticallyAccurate ////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorLOOPPracticallyAccurate::PageEvictionerSelectorLOOPPracticallyAccurate(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerSelector(bufferPool, options),
        _current_frame(1) {}

bf_idx PageEvictionerSelectorLOOPPracticallyAccurate::select() { // Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
    w_assert1(_current_frame > 0 && _current_frame <= _max_bufferpool_index);

    while (true) {
        uint_fast32_t this_frame = _current_frame++ % (_max_bufferpool_index + 1);
        if (this_frame == 0) {
            continue;
        } else {
            return this_frame;
        }
    }
}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageHit(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageUnfix(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageFixed(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageDirty(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageBlocked(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageSwizzled(bf_idx idx) {}

void PageEvictionerSelectorLOOPPracticallyAccurate::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}

////////////////////////////////////////////////////////////////////////////////
////////////////////// PageEvictionerSelectorRANDOMDefault /////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorRANDOMDefault::PageEvictionerSelectorRANDOMDefault(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerSelector(bufferPool, options),
        _random_distribution(1, _max_bufferpool_index) {}

bf_idx PageEvictionerSelectorRANDOMDefault::select() {
    return _random_distribution(_standard_random_engine);
}

void PageEvictionerSelectorRANDOMDefault::updateOnPageHit(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageUnfix(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageFixed(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageDirty(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageBlocked(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageSwizzled(bf_idx idx) {}

void PageEvictionerSelectorRANDOMDefault::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}

////////////////////////////////////////////////////////////////////////////////
///////////////////// PageEvictionerSelectorRANDOMFastRand /////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerSelectorRANDOMFastRand::PageEvictionerSelectorRANDOMFastRand(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerSelector(bufferPool, options) {}

bf_idx PageEvictionerSelectorRANDOMFastRand::select() {
    if (!_random_state_initialized) {
        _random_state = std::random_device{}();
        _random_state_initialized = true;
    }
    _random_state = 214013 * _random_state + 2531011;
    return ((_random_state >> 16 & 0x7FFF) % _max_bufferpool_index) + 1;
}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageHit(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageUnfix(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageFixed(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageDirty(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageBlocked(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageSwizzled(bf_idx idx) {}

void PageEvictionerSelectorRANDOMFastRand::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}
