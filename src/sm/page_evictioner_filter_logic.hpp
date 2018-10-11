#ifndef __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
#define __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP

#include "page_evictioner_filter.hpp"

namespace zero::buffer_pool {

    template <class filter_class>
    class PageEvictionerFilterLogicNOT : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicNOT(bf_tree_m& bufferpool, const sm_options& options);

        template <bool first_call>
        virtual bool                          filter<first_call>(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        virtual void                          updateOnPageHit(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        virtual void                          updateOnPageUnfix(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        virtual void                          updateOnPageFixed(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        virtual void                          updateOnPageDirty(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        virtual void                          updateOnPageBlocked(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        virtual void                          updateOnPageSwizzled(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) override final;

    private:
        filter_class                          _filter;

    };

    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicAND : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicAND(bf_tree_m& bufferpool, const sm_options& options);

        template <bool first_call>
        virtual bool                          filter<first_call>(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        virtual void                          updateOnPageHit(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        virtual void                          updateOnPageUnfix(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        virtual void                          updateOnPageFixed(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        virtual void                          updateOnPageDirty(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        virtual void                          updateOnPageBlocked(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        virtual void                          updateOnPageSwizzled(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) override final;

    private:
        filter_class0                         _filter0;

        filter_class1                         _filter1;

    };

    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicOR : public PageEvictionerFilter {
    public:
        PageEvictionerFilterLogicOR(bf_tree_m& bufferpool, const sm_options& options);

        template <bool first_call>
        virtual bool                          filter<first_call>(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        virtual void                          updateOnPageHit(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        virtual void                          updateOnPageUnfix(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        virtual void                          updateOnPageFixed(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        virtual void                          updateOnPageDirty(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        virtual void                          updateOnPageBlocked(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        virtual void                          updateOnPageSwizzled(bf_idx idx) override final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) override final;

    private:
        filter_class0                         _filter0;

        filter_class1                         _filter1;

    };

} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
