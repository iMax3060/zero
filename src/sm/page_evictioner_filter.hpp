#ifndef __PAGE_EVICTIONER_FILTER_HPP
#define __PAGE_EVICTIONER_FILTER_HPP

#include "page_evictioner.hpp"

#include <vector>
#include <atomic>

namespace zero::buffer_pool {

    /*!
     *
     */
    class PageEvictionerFilter {
    public:
        /*!
         *
         * @param bufferPool
         * @param options
         */
        PageEvictionerFilter(BufferPool* bufferPool, const sm_options& options);

        /*!
         *
         */
        virtual                               ~PageEvictionerFilter() = 0;

        /*!
         *
         * @return
         */
        virtual bool preFilter(bf_idx) = 0;

        /*!
         *
         * @param idx
         * @return
         */
        virtual bool filter(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageHit(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageUnfix(bf_idx idx) = 0;

        /*!
         *
         * @param b_idx
         * @param pid
         */
        virtual void updateOnPageMiss(bf_idx b_idx, PageID pid) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageFixed(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageDirty(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageBlocked(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageSwizzled(bf_idx idx) = 0;

        /*!
         *
         * @param idx
         */
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) = 0;

    protected:
        /*!
         *
         */
        const BufferPool*          _bufferpool;

    };

    /*!
     *
     */
    class PageEvictionerFilterNone : public PageEvictionerFilter {
    public:
        /*!
         *
         * @param bufferPool
         * @param options
         */
        PageEvictionerFilterNone(BufferPool* bufferPool, const sm_options& options);

        /*!
         *
         * @param idx
         * @return
         */
        bool preFilter(bf_idx idx) final;

        /*!
         *
         * @param idx
         * @return
         */
        bool filter(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageHit(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageUnfix(bf_idx idx) final;

        /*!
         *
         * @param b_idx
         * @param pid
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageFixed(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageDirty(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageBlocked(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageSwizzled(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    };

    /*!
     *
     * @tparam on_hit
     * @tparam on_miss
     * @tparam on_unfix
     * @tparam on_used
     * @tparam on_dirty
     * @tparam on_block
     * @tparam on_swizzle
     */
    template <bool on_hit = true, bool on_miss = true, bool on_unfix = false, bool on_used = false, bool on_dirty = false, bool on_block = false, bool on_swizzle = false>
    class PageEvictionerFilterCLOCK : public PageEvictionerFilter {
    public:
        /*!
         *
         * @param bufferPool
         * @param options
         */
        PageEvictionerFilterCLOCK(BufferPool* bufferPool, const sm_options& options);

        /*!
         *
         * @param idx
         * @return
         */
        bool preFilter(bf_idx idx) final;

        /*!
         *
         * @param idx
         * @return
         */
        bool filter(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageHit(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageUnfix(bf_idx idx) final;

        /*!
         *
         * @param b_idx
         * @param pid
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageFixed(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageDirty(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageBlocked(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageSwizzled(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        /*!
         *
         */
        std::vector<bool> _refBits;

    };

    // MG TODO: Allow more flexibility by adding "update" functions as template parameters!
    /*!
     *
     * @tparam decrement
     * @tparam discriminate_pages
     * @tparam on_hit
     * @tparam set_on_hit
     * @tparam level0_on_hit
     * @tparam level1_on_hit
     * @tparam level2_on_hit
     * @tparam on_miss
     * @tparam set_on_miss
     * @tparam level0_on_miss
     * @tparam level1_on_miss
     * @tparam level2_on_miss
     * @tparam on_unfix
     * @tparam set_on_unfix
     * @tparam level0_on_unfix
     * @tparam level1_on_unfix
     * @tparam level2_on_unfix
     * @tparam on_used
     * @tparam set_on_used
     * @tparam level0_on_used
     * @tparam level1_on_used
     * @tparam level2_on_used
     * @tparam on_dirty
     * @tparam set_on_dirty
     * @tparam level0_on_dirty
     * @tparam level1_on_dirty
     * @tparam level2_on_dirty
     * @tparam on_block
     * @tparam set_on_block
     * @tparam level0_on_block
     * @tparam level1_on_block
     * @tparam level2_on_block
     * @tparam on_swizzle
     * @tparam set_on_swizzle
     * @tparam level0_on_swizzle
     * @tparam level1_on_swizzle
     * @tparam level2_on_swizzle
     */
    template <uint16_t decrement = 1, bool discriminate_pages = false,
            bool on_hit = true, bool set_on_hit = false, uint16_t level0_on_hit = 5, uint16_t level1_on_hit = 2, uint16_t level2_on_hit = 1,
            bool on_miss = true, bool set_on_miss = true, uint16_t level0_on_miss = 25, uint16_t level1_on_miss = 10, uint16_t level2_on_miss = 5,
            bool on_unfix = false, bool set_on_unfix = false, uint16_t level0_on_unfix = 5, uint16_t level1_on_unfix = 2, uint16_t level2_on_unfix = 1,
            bool on_used = false, bool set_on_used = false, uint16_t level0_on_used = 5, uint16_t level1_on_used = 2, uint16_t level2_on_used = 1,
            bool on_dirty = false, bool set_on_dirty = false, uint16_t level0_on_dirty = 5, uint16_t level1_on_dirty = 2, uint16_t level2_on_dirty = 1,
            bool on_block = false, bool set_on_block = false, uint16_t level0_on_block = 5, uint16_t level1_on_block = 2, uint16_t level2_on_block = 1,
            bool on_swizzle = false, bool set_on_swizzle = false, uint16_t level0_on_swizzle = 5, uint16_t level1_on_swizzle = 2, uint16_t level2_on_swizzle = 1>
    class PageEvictionerFilterGCLOCK : PageEvictionerFilter {
    public:
        /*!
         *
         * @param bufferPool
         * @param options
         */
        PageEvictionerFilterGCLOCK(BufferPool* bufferPool, const sm_options& options);

        // MG TODO: Range Check in DEBUG mode?
        /*!
         *
         * @param idx
         * @return
         */
        bool preFilter(bf_idx idx) final;

        /*!
         *
         * @param idx
         * @return
         */
        bool filter(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageHit(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageUnfix(bf_idx idx) final;

        /*!
         *
         * @param b_idx
         * @param pid
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageFixed(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageDirty(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageBlocked(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageSwizzled(bf_idx idx) final;

        /*!
         *
         * @param idx
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        /*!
         *
         * @param idx
         * @return
         */
        inline uint8_t getLevel(const bf_idx& idx) const;

        /*!
         *
         */
        std::vector<std::atomic<uint16_t>> _refInts;

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_FILTER_HPP
