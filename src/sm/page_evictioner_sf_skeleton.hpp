#ifndef __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
#define __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP

#include "smthread.h"
#include "sm_options.h"
#include "lsn.h"
#include "bf_hashtable.h"
#include "allocator.h"
#include "generic_page.h"
#include "bf_tree_cb.h"
#include "worker_thread.h"

#include "page_evictioner_selector.hpp"
#include "page_evictioner_filter.hpp"

#include <random>

class bf_tree_m;
class generic_page;
struct bf_tree_cb_t;

namespace zero::buffer_pool {

    template<class selector_class, class filter_class, bool filter_early>
    class PageEvictionerSFSkeleton : public PageEvictioner {
    public:
        PageEvictionerSFSkeleton(bf_tree_m *bufferpool, const sm_options &options);

        virtual                               ~PageEvictionerSFSkeleton();

        virtual bf_idx pickVictim() final;

        virtual void updateOnPageHit(bf_idx idx) final;

        virtual void updateOnPageUnfix(bf_idx idx) final;

        virtual void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        virtual void updateOnPageFixed(bf_idx idx) final;

        virtual void updateOnPageDirty(bf_idx idx) final;

        virtual void updateOnPageBlocked(bf_idx idx) final;

        virtual void updateOnPageSwizzled(bf_idx idx) final;

        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        selector_class _selector;

        filter_class _filter;

    };

} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
