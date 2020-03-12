#ifndef __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
#define __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP

#include "page_evictioner_filter.hpp"

namespace zero::buffer_pool {

    /*!\class   PageEvictionerFilterLogicNOT
     * \brief   Invert buffer frame filter
     * \details This is a buffer frame filter for the _Select-and-Filter_ page evictioner that inverts the results of
     *          any buffer frame filter.
     *
     * @tparam filter_class The buffer frame filter to invert.
     */
    template <class filter_class>
    class PageEvictionerFilterLogicNOT : public PageEvictionerFilter {
        static_assert(std::is_base_of_v<PageEvictionerFilter, filter_class>,
                      "'filter_class' is not of type 'PageEvictionerFilter'!");

    public:
        /*!\fn      PageEvictionerFilterLogicNOT(const BufferPool* bufferPool)
         * \brief   Constructs an invert buffer frame filter
         *
         * @param bufferPool The buffer pool this invert buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterLogicNOT(const BufferPool* bufferPool) :
                _filter(bufferPool) {
        };

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would not be filtered out by the underlying buffer
         *          frame filter.
         *
         * \warning This function does not update the reference statistics of the underlying buffer frame filter. For
         *          each buffer frame discovered evictable, \link filterAndUpdate() \endlink needs to be called exactly
         *          once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx .
         */
        inline bool filter(bf_idx idx) noexcept final {
            return !_filter.filter(idx);
        };

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would not be filtered out by the underlying buffer
         *          frame filter and updates the reference statistics of the underlying buffer frame filter.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx before this
         *            function was called.
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            return !_filter.filterAndUpdate(idx);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics of the underlying buffer frame filter when a page hit occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            _filter.updateOnPageHit(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics of the underlying buffer frame filter when a page unfix occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            _filter.updateOnPageUnfix(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics of the underlying buffer frame filter when a page miss occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _filter.updateOnPageMiss(idx, pid);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics of the underlying buffer frame filter when a page was discovered fixed during
         *          eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            _filter.updateOnPageFixed(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics of the underlying buffer frame filter when a page was discovered dirty during
         *          eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            _filter.updateOnPageDirty(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics of the underlying buffer frame filter when a page cannot be evicted at all.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            _filter.updateOnPageBlocked(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics of the underlying buffer frame filter when a page was discovered containing
         *          swizzled pointers during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _filter.updateOnPageSwizzled(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics of the underlying buffer frame filter when a page got explicitly unbuffered.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _filter.updateOnPageExplicitlyUnbuffered(idx);
        };

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details Updates the statistics of the underlying buffer frame filter when the pointer of a page got swizzled
         *          in its parent page.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        inline void updateOnPointerSwizzling(bf_idx idx) noexcept final {
            _filter.updateOnPointerSwizzling(idx);
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details Releases the internal latches of the underlying buffer frame filter.
         */
        inline void releaseInternalLatches() noexcept final {
            _filter.releaseInternalLatches();
        };

    private:
        /*!\var     _filter
         * \brief   The underlying buffer frame filter
         */
        filter_class _filter;

    };

    /*!\class   PageEvictionerFilterLogicAND
     * \brief   Join two buffer frame filters using conjunction
     * \details This is a buffer frame filter for the _Select-and-Filter_ page evictioner that can be used to combine
     *          two buffer frame filters using a conjunction on the results.
     *
     * @tparam filter_class0 The first buffer frame filter of the conjunction.
     * @tparam filter_class1 The second buffer frame filter of the conjunction.
     */
    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicAND : public PageEvictionerFilter {
        static_assert(std::is_base_of_v<PageEvictionerFilter, filter_class0>,
                      "'filter_class0' is not of type 'PageEvictionerFilter'!");
        static_assert(std::is_base_of_v<PageEvictionerFilter, filter_class1>,
                      "'filter_class1' is not of type 'PageEvictionerFilter'!");

    public:
        /*!\fn      PageEvictionerFilterLogicAND(const BufferPool* bufferPool)
         * \brief   Constructs an ANDed buffer frame filter
         *
         * @param bufferPool The buffer pool this ANDed buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterLogicAND(const BufferPool* bufferPool) :
                _filter0(bufferPool),
                _filter1(bufferPool) {};

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would be filtered out by either of the underlying
         *          buffer frame filters.
         *
         * \warning This function does not update the reference statistics of the underlying buffer frame filters. For
         *          each buffer frame discovered evictable, \link filterAndUpdate() \endlink needs to be called exactly
         *          once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx .
         */
        inline bool filter(bf_idx idx) noexcept final {
            return _filter0.filter(idx) && _filter1.filter(idx);
        };

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would be filtered out by either of the underlying
         *          buffer frame filters and updates the reference statistics of the underlying buffer frame filters.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx before this
         *            function was called.
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            bool filter0Result = _filter0.filterAndUpdate(idx);
            bool filter1Result = _filter1.filterAndUpdate(idx)
            return filter0Result && filter1Result;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics of the underlying buffer frame filters when a page hit occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            _filter0.updateOnPageHit(idx);
            _filter1.updateOnPageHit(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics of the underlying buffer frame filters when a page unfix occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            _filter0.updateOnPageUnfix(idx);
            _filter1.updateOnPageUnfix(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics of the underlying buffer frame filters when a page miss occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _filter0.updateOnPageMiss(idx, pid);
            _filter1.updateOnPageMiss(idx, pid);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered fixed
         *          during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            _filter0.updateOnPageFixed(idx);
            _filter1.updateOnPageFixed(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered dirty
         *          during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            _filter0.updateOnPageDirty(idx);
            _filter1.updateOnPageDirty(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics of the underlying buffer frame filters when a page cannot be evicted at all.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            _filter0.updateOnPageBlocked(idx);
            _filter1.updateOnPageBlocked(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered containing
         *          swizzled pointers during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _filter0.updateOnPageSwizzled(idx);
            _filter1.updateOnPageSwizzled(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics of the underlying buffer frame filters when a page got explicitly unbuffered.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _filter0.updateOnPageExplicitlyUnbuffered(idx);
            _filter1.updateOnPageExplicitlyUnbuffered(idx);
        };

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details Updates the statistics of the underlying buffer frame filters when the pointer of a page got
         *          swizzled in its parent page.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        inline void updateOnPointerSwizzling(bf_idx idx) noexcept final {
            _filter0.updateOnPointerSwizzling(idx);
            _filter1.updateOnPointerSwizzling(idx);
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details Releases the internal latches of the underlying buffer frame filters.
         */
        inline void releaseInternalLatches() noexcept final {
            _filter0.releaseInternalLatches();
            _filter1.releaseInternalLatches();
        };

    private:
        /*!\var     _filter0
         * \brief   The first underlying buffer frame filter
         */
        filter_class0 _filter0;

        /*!\var     _filter1
         * \brief   The second underlying buffer frame filter
         */
        filter_class1 _filter1;

    };

    /*!\class   PageEvictionerFilterLogicOR
     * \brief   Join two buffer frame filters using disjunction
     * \details This is a buffer frame filter for the _Select-and-Filter_ page evictioner that can be used to combine
     *          two buffer frame filters using a disjunction on the results.
     *
     * @tparam filter_class0 The first buffer frame filter of the disjunction.
     * @tparam filter_class1 The second buffer frame filter of the disjunction.
     */
    template <class filter_class0, class filter_class1>
    class PageEvictionerFilterLogicOR : public PageEvictionerFilter {
        static_assert(std::is_base_of_v<PageEvictionerFilter, filter_class0>,
                      "'filter_class0' is not of type 'PageEvictionerFilter'!");
        static_assert(std::is_base_of_v<PageEvictionerFilter, filter_class1>,
                      "'filter_class1' is not of type 'PageEvictionerFilter'!");

    public:
        /*!\fn      PageEvictionerFilterLogicOR(const BufferPool* bufferPool)
         * \brief   Constructs an ORed buffer frame filter
         *
         * @param bufferPool The buffer pool this ORed buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterLogicOR(const BufferPool* bufferPool) :
                _filter0(bufferPool),
                _filter1(bufferPool) {};

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would be filtered out by both of the underlying
         *          buffer frame filters.
         *
         * \warning This function does not update the reference statistics of the underlying buffer frame filters. For
         *          each buffer frame discovered evictable, \link filterAndUpdate() \endlink needs to be called exactly
         *          once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx .
         */
        inline bool filter(bf_idx idx) noexcept final {
            return _filter0.filter(idx) || _filter1.filter(idx);
        };

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if it would be filtered out by both of the underlying
         *          buffer frame filters and updates the reference statistics of the underlying buffer frame filters.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx before this
         *            function was called.
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            return _filter0.filterAndUpdate(idx) || _filter1.filterAndUpdate(idx);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics of the underlying buffer frame filters when a page hit occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            _filter0.updateOnPageHit(idx);
            _filter1.updateOnPageHit(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics of the underlying buffer frame filters when a page unfix occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            _filter0.updateOnPageUnfix(idx);
            _filter1.updateOnPageUnfix(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics of the underlying buffer frame filters when a page miss occurred.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _filter0.updateOnPageMiss(idx, pid);
            _filter1.updateOnPageMiss(idx, pid);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered fixed
         *          during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            _filter0.updateOnPageFixed(idx);
            _filter1.updateOnPageFixed(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered dirty
         *          during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            _filter0.updateOnPageDirty(idx);
            _filter1.updateOnPageDirty(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics of the underlying buffer frame filters when a page cannot be evicted at all.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            _filter0.updateOnPageBlocked(idx);
            _filter1.updateOnPageBlocked(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics of the underlying buffer frame filters when a page was discovered containing
         *          swizzled pointers during eviction.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _filter0.updateOnPageSwizzled(idx);
            _filter1.updateOnPageSwizzled(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics of the underlying buffer frame filters when a page got explicitly unbuffered.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _filter0.updateOnPageExplicitlyUnbuffered(idx);
            _filter1.updateOnPageExplicitlyUnbuffered(idx);
        };

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details Updates the statistics of the underlying buffer frame filters when the pointer of a page got
         *          swizzled in its parent page.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        inline void updateOnPointerSwizzling(bf_idx idx) noexcept final {
            _filter0.updateOnPointerSwizzling(idx);
            _filter1.updateOnPointerSwizzling(idx);
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details Releases the internal latches of the underlying buffer frame filters.
         */
        inline void releaseInternalLatches() noexcept final {
            _filter0.releaseInternalLatches();
            _filter1.releaseInternalLatches();
        };

    private:
        /*!\var     _filter0
         * \brief   The first underlying buffer frame filter
         */
        filter_class0 _filter0;

        /*!\var     _filter1
         * \brief   The second underlying buffer frame filter
         */
        filter_class1 _filter1;

    };

} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_FILTER_LOGIC_HPP
