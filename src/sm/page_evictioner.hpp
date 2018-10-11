#ifndef __ZERO_SM_PAGE_EVICTIONER_HPP
#define __ZERO_SM_PAGE_EVICTIONER_HPP

#include "bf_hashtable.h"
#include "sm_options.h"
#include "worker_thread.h"

class bf_tree_m;
struct bf_tree_cb_t;

namespace zero::buffer_pool {

    class PageEvictioner : public worker_thread_t {
    public:
        PageEvictioner(bf_tree_m* bufferpool, const sm_options& options);

        virtual             ~PageEvictioner();

        virtual bf_idx      pickVictim() = 0;

        virtual void        updateOnPageHit(bf_idx idx) = 0;

        virtual void        updateOnPageUnfix(bf_idx idx) = 0;

        virtual void        updateOnPageMiss(bf_idx idx, PageID pid) = 0;

        virtual void        updateOnPageFixed(bf_idx idx) = 0;

        virtual void        updateOnPageDirty(bf_idx idx) = 0;

        virtual void        updateOnPageBlocked(bf_idx idx) = 0;

        virtual void        updateOnPageSwizzled(bf_idx idx) = 0;

        virtual void        updateOnPageExplicitlyUnbuffered(bf_idx idx) = 0;

        /*!
         *
         * \pre The buffer frame at index \c victim needs to be latched in EX mode.
         *
         * @param victim
         * @return
         */
        bool                evictOne(bf_idx victim);

    protected:
        bf_tree_m*          _bufferpool;

        bool                _enabledSwizzling;

        bool                _maintainEMLSN;

        bool                _flushDirty;

        bool                _logEvictions;

        uint_fast32_t       _evictionBatchSize;

        uint_fast32_t       _maxAttempts;

        uint_fast32_t       _wakeupCleanerAttempts;

    private:
        bool                unswizzleAndUpdateEMLSN(bf_idx victim);

        void                flushDirtyPage(const bf_tree_cb_t& victimControlBlock);

        virtual void        do_work() = 0;
    };

} // zero::buffer_pool

#endif // __ZERO_SM_PAGE_EVICTIONER_HPP
