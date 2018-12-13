#ifndef __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
#define __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP

#include "page_evictioner_filter.hpp"

namespace zero::buffer_pool {

    template <class filter_class>
    class PageEvictionerFilterLogicNOT : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicNOT(BufferPool& bufferpool, const sm_options& options);

        bool                          preFilter(bf_idx idx) final;

        bool                          filter(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void                          updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void                          updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void                          updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void                          updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void                          updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void                          updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void                          updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        filter_class                          _filter;

    };

    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicAND : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicAND(BufferPool& bufferpool, const sm_options& options);

        bool                          preFilter(bf_idx idx) final;

        bool                          filter(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void                          updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void                          updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void                          updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void                          updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void                          updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void                          updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void                          updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        filter_class0                         _filter0;

        filter_class1                         _filter1;

    };

    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicOR : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicOR(BufferPool& bufferpool, const sm_options& options);

        bool                          preFilter(bf_idx idx) final;

        bool                          filter(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void                          updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void                          updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void                          updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void                          updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void                          updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void                          updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void                          updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        filter_class0                         _filter0;

        filter_class1                         _filter1;

    };

} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
