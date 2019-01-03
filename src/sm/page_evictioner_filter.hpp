#ifndef __PAGE_EVICTIONER_FILTER_HPP
#define __PAGE_EVICTIONER_FILTER_HPP

#include "page_evictioner.hpp"

#include <vector>
#include <atomic>

namespace zero::buffer_pool {

    /*!\class   PageEvictionerFilter
     * \brief   Buffer frame filter for the _Select-and-Filter_ page evictioner
     * \details This class defines the interface for buffer frame filters (which represent different policies) used in
     *          the \link PageEvictionerSelectAndFilter \endlink. All those buffer frame filters have to inherit from
     *          this class.
     */
    class PageEvictionerFilter {
    public:
        /*!\fn      PageEvictionerFilter()
         * \brief   Constructs a buffer frame filter
         */
        PageEvictionerFilter();

        /*!\fn      ~PageEvictionerFilter()
         * \brief   Destructs a buffer frame filter
         */
        virtual ~PageEvictionerFilter() = 0;

        /*!\fn      preFilter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters the specified buffer frame to be evicted a page from according to the selected buffer frame
         *          filter. The \link PageEvictionerSelectAndFilter \endlink first selects a buffer frame using a
         *          \link PageEvictionerSelector \endlink and this function is responsible to filter the selected buffer
         *          frames according to a buffer frame filter policy.
         *
         * \note    This member function must be implemented by every specific buffer frame filter.
         *
         * \warning This function does not update the eviction statistics (e.g. resets the referenced bit of the
         *          buffer frame when the _CLOCK_-filter is used). For each buffer frame discovered evictable,
         *          \link filter() \endlink needs to be called exactly once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    If the page in the buffer frame \c idx should be evicted according the buffer frame filter policy,
         *            \c true , else \c false .
         */
        virtual bool preFilter(bf_idx idx) const noexcept = 0;

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters the specified buffer frame to be evicted a page from according to the selected buffer frame
         *          filter. The \link PageEvictionerSelectAndFilter \endlink first selects a buffer frame using a
         *          \link PageEvictionerSelector \endlink and this function is responsible to filter the selected buffer
         *          frames according to a buffer frame filter policy.
         *
         * \note    This member function must be implemented by every specific buffer frame filter.
         *
         * \warning This function potentially updates the eviction statistics (e.g. resets the referenced bit of the
         *          buffer frame when the _CLOCK_-filter is used). This function should only be called, when a page is
         *          actually evictable. Call \link preFilter() \endlink otherwise.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    If the page in the buffer frame \c idx should be evicted according the buffer frame filter policy,
         *            \c true , else \c false .
         */
        virtual bool filter(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics according to the selected buffer frame filter policy when a page hit
         *          occurred.
         *
         * \warning If a buffer frame filter policy is sensitive to recognizing the same page reference multiple times,
         *          implement this different from \link updateOnPageUnfix() \endlink.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        virtual void updateOnPageHit(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics according to the selected buffer frame filter policy when a page unfix
         *          occurred.
         *
         * \warning If a buffer frame filter policy is sensitive to recognizing the same page reference multiple times,
         *          implement this different from \link updateOnPageHit() \endlink.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        virtual void updateOnPageUnfix(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics according to the selected buffer frame filter policy when a page miss
         *          occurred.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        virtual void updateOnPageMiss(bf_idx idx, PageID pid) noexcept = 0;

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics according to the selected buffer frame filter policy when a page was
         *          discovered fixed during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        virtual void updateOnPageFixed(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics according to the selected buffer frame filter policy when a page was
         *          discovered dirty during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        virtual void updateOnPageDirty(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics according to the selected buffer frame filter policy when a page cannot be
         *          evicted at all.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        virtual void updateOnPageBlocked(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics according to the selected buffer frame filter policy when a page was
         *          discovered containing swizzled pointers during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        virtual void updateOnPageSwizzled(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics according to the selected buffer frame filter policy when a page got
         *          explicitly unbuffered.
         *
         * \note    This member function must be implemented by every specific buffer frame filter policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that is freed explicitly.
         */
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept = 0;

    };

    /*!\class   PageEvictionerFilterNone
     * \brief   None-filtering buffer frame filter
     * \details This is a buffer frame filter for the _Select-and-Filter_ page evictioner that is non-filtering. This
     *          policy does not filter out buffer frames for eviction.
     */
    class PageEvictionerFilterNone : public PageEvictionerFilter {
    public:
        /*!\fn      PageEvictionerFilterNone()
         * \brief   Constructs a non-filtering buffer frame filter
         */
        PageEvictionerFilterNone();

        /*!\fn      preFilter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details This buffer frame filter is non-filtering and therefore this does not filter out buffer frames.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Always \c true !
         */
        bool preFilter(bf_idx idx) const noexcept final;

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details This buffer frame filter is non-filtering and therefore this does not filter out buffer frames.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Always \c true !
         */
        bool filter(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final;

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that is freed explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final;

    };

    /*!\class   PageEvictionerFilterCLOCK
     * \brief   _CLOCK_ buffer frame filter
     * \details The _CLOCK_ buffer frame filter policy sets the referenced bit of a buffer pool frame during each page
     *          reference. The buffer frame is filtered out iff the corresponding referenced bit is set. This filtering
     *          also resets the corresponding referenced bit.
     *
     * @tparam on_hit      If set, the eviction statistics are updated on page hit.
     * @tparam on_unfix    If set, the eviction statistics are updated on page unfix.
     * @tparam on_miss     If set, the eviction statistics are updated on page miss.
     * @tparam on_fixed    If set, the eviction statistics are updated when a page is discovered fixed during eviction.
     * @tparam on_dirty    If set, the eviction statistics are updated when a page is discovered dirty during eviction.
     * @tparam on_blocked  If set, the eviction statistics are updated when a page is discovered to be unevictable at
     *                     all.
     * @tparam on_swizzled If set, the eviction statistics are updated when a page is discovered containing swizzled
     *                     pointers during eviction.
     */
    template <bool on_hit /*= true*/, bool on_unfix /*= false*/, bool on_miss /*= true*/, bool on_fixed /*= false*/, bool on_dirty /*= false*/, bool on_blocked /*= false*/, bool on_swizzled /*= false*/>
    class PageEvictionerFilterCLOCK : public PageEvictionerFilter {
    public:
        /*!\fn      PageEvictionerFilterCLOCK()
         * \brief   Constructs a _CLOCK_ buffer frame filter
         */
        PageEvictionerFilterCLOCK();

        /*!\fn      preFilter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced bit is set.
         *
         * \warning This function does not reset the referenced bit corresponding to the specified buffer frame. For
         *          each buffer frame discovered evictable, \link filter() \endlink needs to be called exactly once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx .
         */
        bool preFilter(bf_idx idx) const noexcept final;

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced bit is set and resets this
         *          referenced bit afterwards.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx before this
         *            function was called.
         */
        bool filter(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final;

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * \note    This behaviour is an optimization that saves one check for evictablity using the buffer pool.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that is freed explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final;

    private:
        /*!\var     _refBits
         * \brief   Referenced bits for the buffer frames
         * \details The index of the referenced bit corresponding to buffer frame \c n is \c n .
         */
        std::vector<bool> _refBits;

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_FILTER_HPP
