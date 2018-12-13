#ifndef __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
#define __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP

#include "page_evictioner.hpp"

#include <random>

class bf_tree_m;
class generic_page;
struct bf_tree_cb_t;

namespace zero::buffer_pool {

    template<class selector_class, class filter_class, bool filter_early = false>
    class PageEvictionerSFSkeleton : public PageEvictioner {
    public:
        PageEvictionerSFSkeleton(BufferPool* bufferPool, const sm_options& options);

        ~PageEvictionerSFSkeleton() final;

        bf_idx pickVictim() final;

        void updateOnPageHit(bf_idx idx) final;

        void updateOnPageUnfix(bf_idx idx) final;

        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        void updateOnPageFixed(bf_idx idx) final;

        void updateOnPageDirty(bf_idx idx) final;

        void updateOnPageBlocked(bf_idx idx) final;

        void updateOnPageSwizzled(bf_idx idx) final;

        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        std::unique_ptr<selector_class> _selector;

        std::unique_ptr<filter_class>   _filter;

    };

} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
