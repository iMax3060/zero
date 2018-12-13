#include "page_evictioner_filter.hpp"

#include "btree_page_h.h"

using namespace zero::buffer_pool;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PageEvictionerFilter ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerFilter::PageEvictionerFilter(BufferPool* bufferPool, const sm_options& options) :
        _bufferpool(bufferPool) {}

PageEvictionerFilter::~PageEvictionerFilter() {}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterNone //////////////////////////
////////////////////////////////////////////////////////////////////////////////

PageEvictionerFilterNone::PageEvictionerFilterNone(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerFilter(bufferPool, options) {}

bool PageEvictionerFilterNone::preFilter(bf_idx idx) {
    return true;
}

bool PageEvictionerFilterNone::filter(bf_idx idx) {
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
PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::PageEvictionerFilterCLOCK(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerFilter(bufferPool, options),
        _refBits(bufferPool->getBlockCount()) {}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
bool PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::preFilter(bf_idx idx) {
    if (_refBits[idx]) {
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
bool PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::filter(bf_idx idx) {
    if (_refBits[idx]) {
        _refBits[idx] = false;
        return false;
    } else {
        return true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageHit(bf_idx idx) {
    if constexpr (on_hit) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageUnfix(bf_idx idx) {
    if constexpr (on_unfix) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    if constexpr (on_miss) {
        _refBits[b_idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageFixed(bf_idx idx) {
    if constexpr (on_used) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageDirty(bf_idx idx) {
    if constexpr (on_dirty) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageBlocked(bf_idx idx) {
    if constexpr (on_block) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageSwizzled(bf_idx idx) {
    if constexpr (on_swizzle) {
        _refBits[idx] = true;
    }
}

template <bool on_hit, bool on_miss, bool on_unfix, bool on_used, bool on_dirty, bool on_block, bool on_swizzle>
void PageEvictionerFilterCLOCK<on_hit, on_miss, on_unfix, on_used, on_dirty, on_block, on_swizzle>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _refBits[idx] = true;
}

template class zero::buffer_pool::PageEvictionerFilterCLOCK<true, true, false, false, false, false, false>;

////////////////////////////////////////////////////////////////////////////////
////////////////////////// PageEvictionerFilterGCLOCK //////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::PageEvictionerFilterGCLOCK(BufferPool* bufferPool, const sm_options& options) :
        PageEvictionerFilter(bufferPool, options),
        _refInts(bufferPool->getBlockCount()) {}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
bool PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::preFilter(bf_idx idx) {
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
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
bool PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::filter(bf_idx idx) {
    if (_refInts[idx] > 0) {
        _refInts[idx] -= decrement;
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
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageHit(bf_idx idx) {
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
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageUnfix(bf_idx idx) {
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
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
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
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageFixed(bf_idx idx) {
    if constexpr (on_used) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_used) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_used;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_used;
                } else {
                    _refInts[idx] = level2_on_used;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_used;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_used;
                } else {
                    _refInts[idx] += level2_on_used;
                }
            }
        } else {
            if constexpr (set_on_used) {
                _refInts[idx] = level2_on_used;
            } else {
                _refInts[idx] += level2_on_used;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageDirty(bf_idx idx) {
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
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageBlocked(bf_idx idx) {
    if constexpr (on_block) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_block) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_block;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_block;
                } else {
                    _refInts[idx] = level2_on_block;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_block;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_block;
                } else {
                    _refInts[idx] += level2_on_block;
                }
            }
        } else {
            if constexpr (set_on_block) {
                _refInts[idx] = level2_on_block;
            } else {
                _refInts[idx] += level2_on_block;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageSwizzled(bf_idx idx) {
    if constexpr (on_swizzle) {
        if constexpr (discriminate_pages) {
            uint8_t page_level = getLevel(idx);
            if constexpr (set_on_swizzle) {
                if (page_level == 0) {
                    _refInts[idx] = level0_on_swizzle;
                } else if (page_level == 1) {
                    _refInts[idx] = level1_on_swizzle;
                } else {
                    _refInts[idx] = level2_on_swizzle;
                }
            } else {
                if (page_level == 0) {
                    _refInts[idx] += level0_on_swizzle;
                } else if (page_level == 1) {
                    _refInts[idx] += level1_on_swizzle;
                } else {
                    _refInts[idx] += level2_on_swizzle;
                }
            }
        } else {
            if constexpr (set_on_swizzle) {
                _refInts[idx] = level2_on_swizzle;
            } else {
                _refInts[idx] += level2_on_swizzle;
            }
        }
    }
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
void PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _refInts[idx] = std::numeric_limits<uint16_t>::max();
}

template <uint16_t decrement, bool discriminate_pages,
        bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
        bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
        bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
        bool on_used, bool set_on_used, uint16_t level0_on_used, uint16_t level1_on_used, uint16_t level2_on_used,
        bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
        bool on_block, bool set_on_block, uint16_t level0_on_block, uint16_t level1_on_block, uint16_t level2_on_block,
        bool on_swizzle, bool set_on_swizzle, uint16_t level0_on_swizzle, uint16_t level1_on_swizzle, uint16_t level2_on_swizzle>
uint8_t PageEvictionerFilterGCLOCK<decrement, discriminate_pages, on_hit, set_on_hit, level0_on_hit, level1_on_hit, level2_on_hit, on_miss, set_on_miss, level0_on_miss, level1_on_miss, level2_on_miss, on_unfix, set_on_unfix, level0_on_unfix, level1_on_unfix, level2_on_unfix, on_used, set_on_used, level0_on_used, level1_on_used, level2_on_used, on_dirty, set_on_dirty, level0_on_dirty, level1_on_dirty, level2_on_dirty, on_block, set_on_block, level0_on_block, level1_on_block, level2_on_block, on_swizzle, set_on_swizzle, level0_on_swizzle, level1_on_swizzle, level2_on_swizzle>::getLevel(const bf_idx& idx) const {
    const generic_page* page = _bufferpool->getPage(idx);
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
