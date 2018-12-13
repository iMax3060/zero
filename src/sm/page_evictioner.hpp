#ifndef __ZERO_SM_PAGE_EVICTIONER_HPP
#define __ZERO_SM_PAGE_EVICTIONER_HPP

#include "sm_options.h"
#include "worker_thread.h"

class bf_tree_m;
struct bf_tree_cb_t;

namespace zero::buffer_pool {
    using namespace zero::buffer_pool;

    class PageEvictioner : public worker_thread_t {
    public:
        PageEvictioner(BufferPool* bufferPool, const sm_options& options);

        PageEvictioner(const PageEvictioner&) = delete;
        PageEvictioner& operator=(const PageEvictioner&) = delete;

        virtual             ~PageEvictioner();

        /*!\fn      pickVictim()
         * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
         * \details Selects a page to be evicted according to the selected page eviction
         *          strategy.
         *
         * \post    The picked victim is latched in \link latch_mode_t::LATCH_EX \endlink
         *          mode as the buffer pool frame will be changed during eviction (page
         *          will be removed).
         *
         * @return The buffer frame that can be freed or \c 0 if no eviction victim could
         *         be found.
         */
        virtual bf_idx      pickVictim() = 0;

        // MG TODO: hit_ref, unfix_ref or both?
        /*!\fn      updateOnPageHit(bf_idx idx)
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * \warning If a page eviction strategy is sensitive to recognizing the same page
         *          reference multiple times, implement this different from
         *          \link unfix_ref(bf_idx) \endlink.
         *
         * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
         *            page hit.
         */
        virtual void        updateOnPageHit(bf_idx idx) = 0;

        /*!\fn      updateOnPageUnfix(bf_idx idx)
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * \warning If a page eviction strategy is sensitive to recognizing the same page
         *          reference multiple times, implement this different from
         *          \link unfix_ref(bf_idx) \endlink.
         *
         * @param idx The frame of the \link _bufferpool \endlink that was unfixed.
         */
        virtual void        updateOnPageUnfix(bf_idx idx) = 0;

        /*!\fn      updateOnPageMiss(bf_idx b_idx, PageID pid)
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param b_idx The frame of the \link _bufferpool \endlink that was fixed with a
         *              page miss.
         * @param pid   The \link PageID \endlink of the \link generic_page \endlink that
         *              was loaded into the buffer frame.
         */
        virtual void        updateOnPageMiss(bf_idx idx, PageID pid) = 0;

        /*!\fn      updateOnPageFixed(bf_idx idx)
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param idx The frame of the \link _bufferpool \endlink that was picked for
         *            eviction while it was fixed.
         */
        virtual void        updateOnPageFixed(bf_idx idx) = 0;

        /*!\fn      updateOnPageDirty(bf_idx idx)
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param idx The frame of the \link _bufferpool \endlink that was picked for
         *            eviction while the contained page is dirty.
         */
        virtual void        updateOnPageDirty(bf_idx idx) = 0;

        /*!\fn      updateOnPageBlocked(bf_idx idx)
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param idx The frame of the \link _bufferpool \endlink that contains a page that
         *            cannot be evicted at all.
         */
        virtual void        updateOnPageBlocked(bf_idx idx) = 0;

        /*!\fn      updateOnPageSwizzled(bf_idx idx)
         * \brief   Updates the eviction statistics of pages containing swizzled pointers
         *          during eviction
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param idx The frame of the \link _bufferpool \endlink that was picked for
         *            eviction while containing a page with swizzled pointers.
         */
        virtual void        updateOnPageSwizzled(bf_idx idx) = 0;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx)
         * \brief   Updates the eviction statistics on explicit eviction
         * \details Updates the statistics according to the selected page eviction strategy.
         *
         * @param idx The frame of the \link _bufferpool \endlink that is freed explicitly.
         */
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
        /*!\var     _bufferpool
         * \brief   This evictioner's corresponding buffer pool
         * \details The buffer pool for which this evictioner is responsible to evict pages
         *          from.
         */
        BufferPool*               _bufferPool;

        /*!\var     _enabledSwizzling
         * \brief   Pointer swizzling used in the buffer pool
         * \details Set if the \link _bufferpool \endlink uses pointer swizzling for page
         *          references.
         */
        const bool                _enabledSwizzling;

        /*!\var     _maintainEMLSN
         * \brief   MG TODO
         * \details MG TODO
         */
        const bool                _maintainEMLSN;

        /*!\var     _flushDirty
         * \brief   Flush dirty pages
         * \details Set if dirty pages should be flushed when they get evicted from the
         *          buffer pool.
         */
        const bool                _flushDirty;

        /*!\var     _logEvictions
         * \brief   Log page evictions
         * \details Set if a log record should be created for each eviction of a page.
         */
        const bool                _logEvictions;

        /*!\var     _evictionBatchSize
         * \brief   MG TODO
         * \details MG TODO
         */
        const uint_fast32_t       _evictionBatchSize;

        /*!\var     _maxAttempts
         * \brief   Maximum number of unsuccessful picks of eviction victims
         * \details Maximum number of picks of pages in a row that cannot be evicted before
         *          an error (\"Eviction got stuck!\") is thrown.
         */
        const uint_fast32_t       _maxAttempts;

        /*!\var     _wakeupCleanerAttempts
         * \brief   Number of unsuccessful picks of eviction victims before waking up the
         *          page cleaner
         * \details Number of picks of pages in a row that cannot be evicted before the
         *          page cleaner of the \link _bufferpool \endlink gets woken up
         *          (\link bf_tree_m::wakeup_cleaner() \endlink).
         *
         * \remark  Dirty pages that need to get cleaned are a typical reason for stuck
         *          page evictions.
         */
        const uint_fast32_t       _wakeupCleanerAttempts = 42;

    private:
        /*!\fn      unswizzleAndUpdateEMLSN(bf_idx victim)
         * \brief   Unswizzles the pointer in the parent page and updates the EMLSN of that
         *          page
         * \details In case swizzling is enabled, it will unswizzle the parent point.
         *          Additionally, it will update the parent EMLSN.
         *          This two operations are kept in a single method because both require
         *          looking up the parent, latching, etc., so we save some work.
         *
         * @param victim The buffer frame index where the page that gets evicted can be found.
         * @return       \c true if the .
         */
        bool                unswizzleAndUpdateEMLSN(bf_idx victim);

        /*!\fn      flushDirtyPage(const bf_tree_cb_t& victimControlBlock)
         * \brief   Flush specified page
         * \details Writes the specified page to the database file.
         *
         * \pre     Specified \link bf_tree_cb_t \endlink is latched in exclusive mode and
         *          corresponds to a used buffer frame.
         *
         * @param victimControlBlock The control block that corresponds to the buffer frame
         *                           whose contained page gets flushed.
         */
        void                flushDirtyPage(const bf_tree_cb_t& victimControlBlock);

        /*!\fn      do_work()
         * \brief   Function evicting pages in the eviction thread
         * \details Runs in the eviction thread (executed when the eviction thread gets
         *          woken up and when terminated it terminates the eviction thread) and
         *          evicts pages as long as there are not \link _evictionBatchSize \endlink
         *          buffer frames free as defined in .
         */
        virtual void        do_work();
    };

} // zero::buffer_pool

#endif // __ZERO_SM_PAGE_EVICTIONER_HPP
