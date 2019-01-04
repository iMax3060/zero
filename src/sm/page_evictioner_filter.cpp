#include "page_evictioner_filter.hpp"

#include "buffer_pool.hpp"

#include "btree_page_h.h"

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////// PageEvictionerFilterGCLOCK //////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::PageEvictionerFilterGCLOCK(const BufferPool* bufferPool) :
        PageEvictionerFilter(bufferPool),
        _refInts(bufferPool->getBlockCount()) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
bool PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::preFilter(bf_idx idx) const noexcept {
    if (_refInts[idx] > 0) {
        return false;
    } else {
        return true;
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
bool PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::filter(bf_idx idx) noexcept {
    if (_refInts[idx] > 0) {
        _refInts[idx] = std::max(_refInts[idx] - decrement, 0);
        return false;
    } else {
        return true;
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageHit(bf_idx idx) noexcept {
    if constexpr (on_hit) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_hit) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_hit;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_hit;
                } else {
                    _refInts[idx] = level2_on_hit;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_hit;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_hit;
                } else {
                    _refInts[idx] += level2_on_hit;
                }
            }
        } else {
            if constexpr (set_on_hit) {
                _refInts[idx] = level2_on_hit;
            } else {
                _refInts[idx] += level2_on_hit;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageUnfix(bf_idx idx) noexcept {
    if constexpr (on_unfix) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_unfix) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_unfix;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_unfix;
                } else {
                    _refInts[idx] = level2_on_unfix;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_unfix;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_unfix;
                } else {
                    _refInts[idx] += level2_on_unfix;
                }
            }
        } else {
            if constexpr (set_on_unfix) {
                _refInts[idx] = level2_on_unfix;
            } else {
                _refInts[idx] += level2_on_unfix;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept {
    if constexpr (on_miss) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(b_idx);
            if constexpr (set_on_miss) {
                if (page_level == 0) {
                    _refInts[b_idx] = level0_on_miss;
                } else if (page_level == 1) {
                    _refInts[b_idx] = level1_on_miss;
                } else {
                    _refInts[b_idx] = level2_on_miss;
                }
            } else {
                if (page_level == 0) {
                    _refInts[b_idx] += level0_on_miss;
                } else if (page_level == 1) {
                    _refInts[b_idx] += level1_on_miss;
                } else {
                    _refInts[b_idx] += level2_on_miss;
                }
            }
        } else {
            if constexpr (set_on_miss) {
                _refInts[b_idx] = level2_on_miss;
            } else {
                _refInts[b_idx] += level2_on_miss;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageFixed(bf_idx idx) noexcept {
    if constexpr (on_fixed) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_fixed) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_fixed;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_fixed;
                } else {
                    _refInts[idx] = level2_on_fixed;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_fixed;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_fixed;
                } else {
                    _refInts[idx] += level2_on_fixed;
                }
            }
        } else {
            if constexpr (set_on_fixed) {
                _refInts[idx] = level2_on_fixed;
            } else {
                _refInts[idx] += level2_on_fixed;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageDirty(bf_idx idx) noexcept {
    if constexpr (on_dirty) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_dirty) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_dirty;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_dirty;
                } else {
                    _refInts[idx] = level2_on_dirty;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_dirty;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_dirty;
                } else {
                    _refInts[idx] += level2_on_dirty;
                }
            }
        } else {
            if constexpr (set_on_dirty) {
                _refInts[idx] = level2_on_dirty;
            } else {
                _refInts[idx] += level2_on_dirty;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageBlocked(bf_idx idx) noexcept {
    if constexpr (on_blocked) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_blocked) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_blocked;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_blocked;
                } else {
                    _refInts[idx] = level2_on_blocked;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_blocked;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_blocked;
                } else {
                    _refInts[idx] += level2_on_blocked;
                }
            }
        } else {
            if constexpr (set_on_blocked) {
                _refInts[idx] = level2_on_blocked;
            } else {
                _refInts[idx] += level2_on_blocked;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageSwizzled(bf_idx idx) noexcept {
    if constexpr (on_swizzled) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_swizzled) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_swizzled;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_swizzled;
                } else {
                    _refInts[idx] = level2_on_swizzled;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_swizzled;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_swizzled;
                } else {
                    _refInts[idx] += level2_on_swizzled;
                }
            }
        } else {
            if constexpr (set_on_swizzled) {
                _refInts[idx] = level2_on_swizzled;
            } else {
                _refInts[idx] += level2_on_swizzled;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept {
    _refInts[idx] = std::numeric_limits<uint16_t>::max();
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
        bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
uint8_t PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_fixed, set_on_fixed, level0_on_fixed, level1_on_fixed, level2_on_fixed, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_blocked, set_on_blocked, level0_on_blocked, level1_on_blocked, level2_on_blocked, on_swizzled, set_on_swizzled, level0_on_swizzled, level1_on_swizzled, level2_on_swizzled>::getLevel(const bf_idx& idx) const {
    const generic_page* page = smlevel_0::bf->getPage(idx);
    w_assert1(page != nullptr);
    if (page->tag == t_btree_p) {
        btree_page_h fixedPage;
        fixedPage.fix_nonbufferpool_page(const_cast<generic_page*>(page));
        if (fixedPage.pid() == fixedPage.btree_root() || fixedPage.level() > 2) {
            return 0;
        } else if (fixedPage.level() == 2) {
            return 1;
        } else if (fixedPage.level() == 1) {
            return 2;
        }
    } else {     // Non-B-Tree pages are interpreted as B-Tree root pages!
        return 0;
    }
}
