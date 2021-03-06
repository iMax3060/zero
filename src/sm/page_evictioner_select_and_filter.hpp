#ifndef __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
#define __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP

#include "page_evictioner.hpp"
#include "page_evictioner_selector.hpp"
#include "page_evictioner_selector_random.hpp"
#include "page_evictioner_selector_loop.hpp"
#include "page_evictioner_filter.hpp"
#include "buffer_pool.hpp"

#include <random>

class generic_page;
struct bf_tree_cb_t;

namespace zero::buffer_pool {

    /*!\class   PageEvictionerSelectAndFilter
     * \brief   _Select-and-Filter_ page evictioner for the buffer pool
     * \details This is the _Select-and-Filter page_ evictioner for the \link BufferPool \endlink. During the eviction,
     *          it uses first the selector to select a buffer frame to evict a page from and then, it uses the filter
     *          to decide if the selected page should really be freed.<br>
     *          E.g. the _CLOCK_ page eviction algorithm can be implemented using this _Select-and-Filter_ page
     *          evictioner using:
     *            - Selector: _LOOP_ (just selecting buffer frames by looping over the buffer frame IDs)
     *            - Filter: _CLOCK_-filter using a referenced bit according to the _CLOCK_ page eviction algorithm
     *
     *          E.g. the _0CLOCK_ (_Zero-Handed-CLOCK_) page eviction algorithm can be implemented using:
     *            - Selector: _RANDOM_ (just selecting random buffer frames)
     *            - Filter: _CLOCK_-filter using a referenced bit according to the _CLOCK_ page eviction algorithm
     *
     *          Therefore this allows reusing the _CLOCK_-filter for multiple page replacement strategies.<br>
     *          The functions \link pickVictim() \endlink, \link updateOnPageHit() \endlink,
     *          \link updateOnPageUnfix() \endlink, \link updateOnPageMiss() \endlink,
     *          \link updateOnPageFixed() \endlink, \link updateOnPageDirty() \endlink,
     *          \link updateOnPageBlocked() \endlink, \link updateOnPageSwizzled() \endlink and
     *          \link updateOnPageExplicitlyUnbuffered() \endlink call the appropriate functions of both, the selector
     *          and the filter.
     *
     * @tparam selector_class The buffer frame selector used during page eviction which has to be of type
     *                        \link PageEvictionerSelector \endlink.
     * @tparam filter_class   The buffer frame filter used during page eviction which has to be of type
     *                        \link PageEvictionerFilter \endlink.
     * @tparam filter_early   If set, the buffer frame filter is additionally used before the buffer pool is used to
     *                        check, if the contained page can actually be evicted from the selected buffer frame. This
     *                        is an optimization because the check using the buffer pool requires the potentially
     *                        expensive latching of the selected buffer frame. But some buffer frame filters might not
     *                        properly support this early filtering because they change their buffer frame statistics
     *                        when filtered (e.g. the _CLOCK_-filter resets the referenced bit of the selected buffer
     *                        frame).
     */
    template<class selector_class, class filter_class, bool filter_early = false>
    class PageEvictionerSelectAndFilter : public PageEvictioner {
        static_assert(std::is_base_of<PageEvictionerSelector, selector_class>::value,
                      "'selector_class' is not of type 'PageEvictionerSelector'!");
        static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value,
                      "'filter_class' is not of type 'PageEvictionerFilter'!");

    public:
        /*!\fn      PageEvictionerSelectAndFilter(const BufferPool* bufferPool)
         * \brief   Constructs a _Select-and-Filter_ page evictioner
         * \details This constructor also constructs the buffer frame selector and filter.
         *
         * @param bufferPool The buffer pool this _Select-and-Filter_ page evictioner is responsible for.
         */
        explicit PageEvictionerSelectAndFilter(const BufferPool* bufferPool) :
                PageEvictioner(bufferPool),
                _selector(bufferPool),
                _filter(bufferPool) {};

        /*!\fn      ~PageEvictionerSelectAndFilter()
         * \brief   Destructs a _Select-and-Filter_ page evictioner
         */
        ~PageEvictionerSelectAndFilter() final {};

        /*!\fn      pickVictim() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a page to be evicted by:
         *            -# Select a buffer frame using \link PageEvictionerSelector::select() \endlink of the buffer frame
         *               selector specified in the template parameter \c selector_class .
         *            -# If the template parameter \c filter_early is set, filter the selected buffer frame using
         *               \link PageEvictionerFilter::filterAndUpdate() \endlink of the buffer frame filter specified in
         *               the template parameter \c filter_class .
         *            -# Further filter the selected buffer frame using \link BufferPool::isEvictable() \endlink of the
         *               buffer pool (includes the potentially expensive latching of selected buffer frame in mode
         *               \link latch_mode_t::LATCH_EX \endlink without waiting for other threads).
         *            -# Finally, if the template parameter \c filter_early is not set, filter the selected buffer frame
         *               using \link PageEvictionerFilter::filterAndUpdate() \endlink of the buffer frame filter
         *               specified in the template parameter \c filter_class . Otherwise filter the selected buffer
         *               frame using \link PageEvictionerFilter::filter() \endlink of the buffer frame filter specified
         *               in the template parameter \c filter_class .
         *
         * @return The buffer frame that can be freed or \c 0 if no eviction victim could be found.
         */
        bf_idx pickVictim() noexcept final {
            while (true) {
                if (should_exit()) {
                    return 0;
                } // the buffer index 0 has the semantics of null

                bf_idx selectedIndex = _selector.select();

                if (!_filter.filterAndUpdate(selectedIndex)) {
                    continue;
                }

                return selectedIndex;
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics of the selected buffer frame selector and filter when a page hit occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            _selector.updateOnPageHit(idx);
            _filter.updateOnPageHit(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics of the selected buffer frame selector and filter when a page unfix occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            _selector.updateOnPageUnfix(idx);
            _filter.updateOnPageUnfix(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics of the selected buffer frame selector and filter when a page miss occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _selector.updateOnPageMiss(idx, pid);
            _filter.updateOnPageMiss(idx, pid);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics of the selected buffer frame selector and filter when a page was discovered
         *          fixed during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            _selector.updateOnPageFixed(idx);
            _filter.updateOnPageFixed(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics of the selected buffer frame selector and filter when a page was discovered
         *          dirty during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            _selector.updateOnPageDirty(idx);
            _filter.updateOnPageDirty(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics of the selected buffer frame selector and filter when a page cannot be
         *          evicted at all.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            _selector.updateOnPageBlocked(idx);
            _filter.updateOnPageBlocked(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics of the selected buffer frame selector and filter when a page was discovered
         *          containing swizzled pointers during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _selector.updateOnPageSwizzled(idx);
            _filter.updateOnPageSwizzled(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics of the selected buffer frame selector and filter when a page got explicitly
         *          unbuffered.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _selector.updateOnPageExplicitlyUnbuffered(idx);
            _filter.updateOnPageExplicitlyUnbuffered(idx);
        };

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details Updates the statistics of the selected buffer frame selector and filter when the pointer of a page
         *          got swizzled in its parent page.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {
            _selector.updateOnPointerSwizzling(idx);
            _filter.updateOnPointerSwizzling(idx);
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of
         * \details Some methods of page evictioners hold internal latches beyond the invocation of one method but
         *          expect another method to be called later to release those internal latches. This should be used to
         *          explicitly release those latches.
         */
        inline void releaseInternalLatches() noexcept final {
            _selector.releaseInternalLatches();
            _filter.releaseInternalLatches();
        };

    private:
        /*!\var     _selector
         * \brief   The buffer frame selector
         * \details The buffer frame selector used by this _Select-and-Filter_ page evictioner.
         */
        selector_class _selector;

        /*!\var     _filter
         * \brief   The buffer frame filter
         * \details The buffer frame filter used by this _Select-and-Filter_ page evictioner.
         */
        filter_class _filter;
    };
} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_SF_SKELETON_HPP
