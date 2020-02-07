#ifndef __ZERO_SM_PAGE_EVICTIONER_HPP
#define __ZERO_SM_PAGE_EVICTIONER_HPP

#include "sm_options.h"
#include "worker_thread.h"
#include "buffer_pool_pointer_swizzling.hpp"

struct bf_tree_cb_t;

namespace zero::buffer_pool {
    using namespace zero::buffer_pool;

    /*!\class   PageEvictioner
     * \brief   Page evictioner for the buffer pool
     * \details This is the abstract class for page eviction which implements common functionality required for all page
     *          evictioners independent of the used algorithm. All those page evictioners should inherit from this
     *          class.
     *
     * \note    This class is strongly coupled to the buffer pool and some other components of Zero. The inheriting page
     *          evictioners implementing specific page eviction algorithms do not need to be coupled to all those
     *          components but should just rely on the implementations of \link evictOne() \endlink,
     *          \link _unswizzleAndUpdateEMLSN() \endlink, \link _flushDirtyPage() \endlink and \link do_work() \endlink.
     */
    class PageEvictioner : public worker_thread_t {
    public:
        /*!\fn      PageEvictioner(const BufferPool* bufferPool)
         * \brief   Constructs an abstract page evictioner
         * \details This constructor initializes the member variables according to specifications of the
         *          \link BufferPool \endlink and settings from the \link sm_options \endlink.
         *
         * \note    Due to the fact that this is a constructor of an abstract class, this can only be called from within
         *          the member initializer list of a constructor of an inheriting class.
         *
         * @param bufferPool The buffer pool this page evictioner is responsible for.
         */
        explicit PageEvictioner(const BufferPool* bufferPool);

        /*!\fn      PageEvictioner(const PageEvictioner&)
         * \brief   Explicitly deleted copy constructor of an abstract page evictioner
         * \details The copy constructor of an abstract page evictioner is explicitly removed because a page evictioner
         *          depends on its corresponding buffer pool and there can only be one buffer pool per Zero instance.
         */
        PageEvictioner(const PageEvictioner&) = delete;
        /*!\fn      operator=(const PageEvictioner&)
         * \brief   Explicitly deleted assignment operator of an abstract page evictioner
         * \details The assignment operator of an abstract page evictioner is explicitly removed because a page
         *          evictioner depends on its corresponding buffer pool and there can only be one buffer pool per Zero
         *          instance.
         */
        PageEvictioner& operator=(const PageEvictioner&) = delete;

        /*!\fn      ~PageEvictioner()
         * \brief   Destructs an abstract page evictioner
         */
        virtual ~PageEvictioner();

        /*!\fn      pickVictim() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a page to be evicted according to the selected page eviction strategy.
         *
         * \post    The picked victim is latched in \link latch_mode_t::LATCH_EX \endlink mode as the buffer pool frame
         *          will be changed during eviction (page will be removed).
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @return The buffer frame that can be freed or \c 0 if no eviction victim could be found.
         */
        virtual bf_idx pickVictim() noexcept = 0;

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics according to the selected page eviction strategy when a page hit occurred.
         *
         * \warning If a page eviction strategy is sensitive to recognizing the same page reference multiple times,
         *          implement this different from \link updateOnPageUnfix() \endlink.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        virtual void updateOnPageHit(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics according to the selected page eviction strategy when a page unfix occurred.
         *
         * \warning If a page eviction strategy is sensitive to recognizing the same page reference multiple times,
         *          implement this different from \link updateOnPageHit() \endlink.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        virtual void updateOnPageUnfix(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics according to the selected page eviction strategy when a page miss occurred.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        virtual void updateOnPageMiss(bf_idx idx, PageID pid) noexcept = 0;

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics according to the selected page eviction strategy when a page was discovered
         *          fixed during eviction.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        virtual void updateOnPageFixed(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics according to the selected page eviction strategy when a page was discovered
         *          dirty during eviction.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        virtual void updateOnPageDirty(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics according to the selected page eviction strategy when a page cannot be
         *          evicted at all.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        virtual void updateOnPageBlocked(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics according to the selected page eviction strategy when a page was discovered
         *          containing swizzled pointers during eviction.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        virtual void updateOnPageSwizzled(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics according to the selected page eviction strategy when a page got explicitly
         *          unbuffered.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details Updates the statistics according to the selected page eviction strategy when the pointer of a page
         *          got swizzled in its parent page.
         *
         * \note    This member function must be implemented by every specific page evictioner which inherits from this
         *          abstract page evictioner.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        virtual void updateOnPointerSwizzling(bf_idx idx) noexcept = 0;

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches
         * \details Some methods of page evictioners hold internal latches beyond the invocation of one method but
         *          expect another method to be called later to release those internal latches. This should be used to
         *          explicitly release those latches.
         */
        virtual void releaseInternalLatches() noexcept = 0;

        /*!\fn      evictOne(bf_idx& victim)
         * \brief   Evicts a page from the buffer pool
         * \details Selects a page for eviction and executes the eviction of it.
         *
         * @param[out] victim The index of the buffer frame from which the page was evicted.
         * @return            Returns \c true if the \c victim could successfully be evicted, otherwise \c false .
         */
        bool evictOne(bf_idx& victim);

    protected:
        /*!\var     _enabledSwizzling
         * \brief   Pointer swizzling used in the buffer pool
         * \details Set if the \link BufferPool \endlink uses pointer swizzling for page references.
         */
        static constexpr bool     _enabledSwizzling = POINTER_SWIZZLER::usesPointerSwizzling;

        /*!\var     _maintainEMLSN
         * \brief   Maintain the page's EMLSNs on eviction
         * \details Set if the EMLSNs of the pages should be maintained.
         */
        const bool                _maintainEMLSN;

        /*!\var     _flushDirty
         * \brief   Flush dirty pages
         * \details Set if dirty pages should be flushed when they get evicted from the buffer pool.
         */
        const bool                _flushDirty;

        /*!\var     _logEvictions
         * \brief   Log page evictions
         * \details Set if a log record should be created for each eviction of a page.
         */
        const bool                _logEvictions;

        /*!\var     _evictionBatchSize
         * \brief   Target value of free buffer frames
         * \details Once page evictioning is started, it runs till there are this many free buffer frames in the
         *          \link BufferPool \endlink.
         */
        const uint_fast32_t       _evictionBatchSize;

        /*!\var     _maxAttempts
         * \brief   Maximum number of unsuccessful picks of eviction victims
         * \details Maximum number of picks of pages in a row that cannot be evicted before an error
         *          (\"Eviction got stuck!\") is thrown.
         */
        const uint_fast32_t       _maxAttempts;

        /*!\var     _wakeupCleanerAttempts
         * \brief   Number of unsuccessful picks of eviction victims before waking up the page cleaner
         * \details Number of picks of pages in a row that cannot be evicted before the page cleaner of the
         *          \link BufferPool \endlink gets woken up (\link BufferPool::wakeupPageCleaner() \endlink).
         *
         * \remark  Dirty pages that need to get cleaned are a typical reason for stuck page evictions.
         */
        const uint_fast32_t       _wakeupCleanerAttempts = 42;

    protected:
        /*!\fn      _doEviction(bf_idx victim) noexcept
         * \brief   Evicts a page from the buffer pool
         * \details Evicts the page from the specified buffer pool frame by:
         *            - Unswizzling its pointer in its parent (if needed)
         *            - Updating its EMLSN (if needed)
         *            - Preparing the corresponding \link bf_tree_cb_t \endlink for the eviction
         *            - Flushing the page to the database (if needed and allowed)
         *            - Logging the eviction (if supposed to)
         *            - Adding the page for recovery (if NoDB is used)
         *            - Removing the page from the \link Hashtable \endlink
         *            - Releasing the latch of the buffer frame
         *
         * \post    The buffer frame at index \c victim is not used (if this is successful) and it is not latched. If
         *          \link _flushDirty \endlink is set and if the page was dirty, the page is flushed to the database and
         *          if \link _logEvictions \endlink is set, a log record of type \link evict_page_log \endlink is in the
         *          log.
         *
         * @param victim The index of the buffer frame from which the page should be evicted.
         * @return       Returns \c true if the page could successfully be evicted, otherwise \c false .
         */
        bool                _doEviction(bf_idx victim) noexcept;

    private:
        /*!\fn      _unswizzleAndUpdateEMLSN(bf_idx victim) noexcept
         * \brief   Unswizzles the pointer in the parent page and updates the EMLSN of that page
         * \details In case swizzling is enabled, this unswizzles the pointer in the parent page. Additionally, this
         *          updates the parent EMLSN. This two operations are kept in a single method because both require
         *          looking up the parent, latching, etc., so we save some work.
         *
         * @param victim The buffer frame index where the page that gets evicted can be found.
         * @return       \c true if the updates to the parent page were successfully completed and not required,
         *               \c false otherwise.
         */
        bool                _unswizzleAndUpdateEMLSN(bf_idx victim) noexcept;

        /*!\fn      _flushDirtyPage(const bf_tree_cb_t& victimControlBlock) noexcept
         * \brief   Flush the specified page
         * \details Writes the specified page to the database file.
         *
         * \pre     Specified \link bf_tree_cb_t \endlink is latched in exclusive mode and corresponds to a used buffer
         *          frame.
         *
         * @param victimControlBlock The control block that corresponds to the buffer frame whose contained page gets
         *                           flushed.
         */
        void                _flushDirtyPage(const bf_tree_cb_t &victimControlBlock) noexcept;

        /*!\fn      do_work() override
         * \brief   Function evicting pages in the eviction thread
         * \details Runs in the eviction thread (executed when the eviction thread gets woken up and when terminated it
         *          terminates the eviction thread) and evicts pages as long as there are not
         *          \link _evictionBatchSize \endlink free buffer frames in the \link BufferPool \endlink.
         */
        void                do_work() override;
    };

} // zero::buffer_pool

#endif // __ZERO_SM_PAGE_EVICTIONER_HPP
