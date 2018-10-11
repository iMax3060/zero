#ifndef __PAGE_EVICTIONER_FILTER_HPP
#define __PAGE_EVICTIONER_FILTER_HPP

#include <vector>
#include <atomic>

class PageEvictionerFilter {
public:
    PageEvictionerFilter(bf_tree_m& bufferpool, const sm_options& options);

    virtual                               ~PageEvictionerFilter() = 0;

    template <bool first_call>
    virtual bool                          filter<first_call>(bf_idx idx) = 0;

    virtual void                          updateOnPageHit(bf_idx idx) = 0;

    virtual void                          updateOnPageUnfix(bf_idx idx) = 0;

    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) = 0;

    virtual void                          updateOnPageFixed(bf_idx idx) = 0;

    virtual void                          updateOnPageDirty(bf_idx idx) = 0;

    virtual void                          updateOnPageBlocked(bf_idx idx) = 0;

    virtual void                          updateOnPageSwizzled(bf_idx idx) = 0;

    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) = 0;

};

class PageEvictionerFilterNone : public PageEvictionerFilter {
public:
    PageEvictionerFilterNone(bf_tree_m& bufferpool, const sm_options& options);

    template <bool first_call>
    virtual bool                          filter<first_call>(bf_idx idx) override final;

    virtual void                          updateOnPageHit(bf_idx idx) override final;

    virtual void                          updateOnPageUnfix(bf_idx idx) override final;

    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) override final;

    virtual void                          updateOnPageFixed(bf_idx idx) override final;

    virtual void                          updateOnPageDirty(bf_idx idx) override final;

    virtual void                          updateOnPageBlocked(bf_idx idx) override final;

    virtual void                          updateOnPageSwizzled(bf_idx idx) override final;

    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) override final;

};

template <bool on_hit = true, bool on_miss = true, bool on_unfix = false, bool on_used = false, bool on_dirty = false, bool on_block = false, bool on_swizzle = false>
class PageEvictionerFilterCLOCK : public PageEvictionerFilter {
public:
    PageEvictionerFilterCLOCK(bf_tree_m& bufferpool, const sm_options& options);

    template <bool first_call>
    virtual bool                          filter<first_call>(bf_idx idx) override final;

    virtual void                          updateOnPageHit(bf_idx idx) final;

    virtual void                          updateOnPageUnfix(bf_idx idx) final;

    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) final;

    virtual void                          updateOnPageFixed(bf_idx idx) final;

    virtual void                          updateOnPageDirty(bf_idx idx) final;

    virtual void                          updateOnPageBlocked(bf_idx idx) final;

    virtual void                          updateOnPageSwizzled(bf_idx idx) final;

    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

private:
    std::vector<bool>                     _clock_ref_bits;

};

template <uint16_t decrement = 1, bool discriminate_pages = false,
          bool on_hit = true, bool set_on_hit = false, uint16_t level0_on_hit = 5, uint16_t level1_on_hit = 2, uint16_t level2_on_hit = 1,
          bool on_miss = true, bool set_on_miss = true, uint16_t level0_on_miss = 25, uint16_t level1_on_miss = 10, uint16_t level2_on_miss = 5,
          bool on_unfix = false, bool set_on_unfix = false, uint16_t level0_on_unfix = 5, uint16_t level1_on_unfix = 2, uint16_t level2_on_unfix = 1,
          bool on_used = false, bool set_on_used = false, uint16_t level0_on_used = 5, uint16_t level1_on_used = 2, uint16_t level2_on_used = 1,
          bool on_dirty = false, bool set_on_dirty = false, uint16_t level0_on_dirty = 5, uint16_t level1_on_dirty = 2, uint16_t level2_on_dirty = 1,
          bool on_block = false, bool set_on_block = false, uint16_t level0_on_block = 5, uint16_t level1_on_block = 2, uint16_t level2_on_block = 1,
          bool on_swizzle = false, bool set_on_swizzle = false, uint16_t level0_on_swizzle = 5, uint16_t level1_on_swizzle = 2, uint16_t level2_on_swizzle = 1,>
class PageEvictionerFilterGCLOCK : PageEvictionerFilter {
public:
    PageEvictionerFilterGCLOCK(bf_tree_m& bufferpool, const sm_options& options);

    // MG TODO: Range Check in DEBUG mode?
    template <bool first_call>
    virtual bool                          filter<first_call>(bf_idx idx) override final;

    virtual void                          updateOnPageHit(bf_idx idx) override final;

    virtual void                          updateOnPageUnfix(bf_idx idx) override final;

    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) override final;

    virtual void                          updateOnPageFixed(bf_idx idx) override final;

    virtual void                          updateOnPageDirty(bf_idx idx) override final;

    virtual void                          updateOnPageBlocked(bf_idx idx) override final;

    virtual void                          updateOnPageSwizzled(bf_idx idx) override final;

    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) override final;

private:
    inline uint8_t                        get_level(bf_idx idx) final;

    std::vector<std::atomic<uint16_t>>    _gclock_ref_ints;
    
};

#endif // __PAGE_EVICTIONER_FILTER_HPP
