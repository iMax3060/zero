#ifndef __PAGE_EVICTIONER_FILTER_HPP
#define __PAGE_EVICTIONER_FILTER_HPP

#include <vector>
#include <atomic>

#include "buffer_pool.hpp"
#include "btree_page_h.h"

namespace zero::buffer_pool {

    /*!\class   PageEvictionerFilter
     * \brief   Buffer frame filter for the _Select-and-Filter_ page evictioner
     * \details This class defines the interface for buffer frame filters (which represent different policies) used in
     *          the \link PageEvictionerSelectAndFilter \endlink. All those buffer frame filters have to inherit from
     *          this class.
     */
    class PageEvictionerFilter {
    protected:
        /*!\fn      PageEvictionerFilter(const BufferPool* bufferPool)
         * \brief   Constructs a buffer frame filter
         *
         * @param bufferPool The buffer pool this buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilter(const BufferPool* bufferPool) {};

    public:
        /*!\fn      ~PageEvictionerFilter()
         * \brief   Destructs a buffer frame filter
         */
        virtual ~PageEvictionerFilter() {};

        /*!\fn      filter(bf_idx idx) noexcept
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
         *          \link filterAndUpdate() \endlink needs to be called exactly once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    If the page in the buffer frame \c idx should be evicted according the buffer frame filter policy,
         *            \c true , else \c false .
         */
        virtual bool filter(bf_idx idx) const noexcept = 0;

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
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
         *          actually evictable. Call \link filter() \endlink otherwise.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    If the page in the buffer frame \c idx should be evicted according the buffer frame filter policy,
         *            \c true , else \c false .
         */
        virtual bool filterAndUpdate(bf_idx idx) noexcept = 0;

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
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept = 0;

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of the buffer frame filter
         * \details Some methods of buffer frame filters hold internal latches beyond the invocation of one method but
         *          expect another method to be called later to release those internal latches. This should be used to
         *          explicitly release those latches.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         */
        virtual void releaseInternalLatches() noexcept = 0;

    };

    /*!\class   PageEvictionerFilterNone
     * \brief   None-filtering buffer frame filter
     * \details This is a buffer frame filter for the _Select-and-Filter_ page evictioner that is non-filtering. This
     *          policy does not filter out buffer frames for eviction.
     */
    class PageEvictionerFilterNone : public PageEvictionerFilter {
    public:
        /*!\fn      PageEvictionerFilterNone(const BufferPool* bufferPool)
         * \brief   Constructs a non-filtering buffer frame filter
         *
         * @param bufferPool The buffer pool this non-filtering buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterNone(const BufferPool* bufferPool) :
                PageEvictionerFilter(bufferPool) {};

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details This buffer frame filter is non-filtering and therefore this does not filter out buffer frames.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Always \c true !
         */
        inline bool filter(bf_idx idx) const noexcept final {
            return true;
        };

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details This buffer frame filter is non-filtering and therefore this does not filter out buffer frames.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Always \c true !
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            return true;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame filter does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details This buffer frame filter does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

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
        /*!\fn      PageEvictionerFilterCLOCK(const BufferPool* bufferPool)
         * \brief   Constructs a _CLOCK_ buffer frame filter
         *
         * @param bufferPool The buffer pool this _CLOCK_ buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterCLOCK(const BufferPool* bufferPool) :
                PageEvictionerFilter(bufferPool),
                _refBits(bufferPool->getBlockCount()) {};

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced bit is set.
         *
         * \warning This function does not reset the referenced bit corresponding to the specified buffer frame. For
         *          each buffer frame discovered evictable, \link filterAndUpdate() \endlink needs to be called exactly
         *          once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx .
         */
        inline bool filter(bf_idx idx) const noexcept final {
            if (_refBits[idx]) {
                return false;
            } else {
                return true;
            }
        };

        /*!\fn      filterAndUpdate(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced bit is set and resets this
         *          referenced bit afterwards.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    The value of the referenced bit corresponding to the buffer frame with index \c idx before this
         *            function was called.
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            if (_refBits[idx]) {
                _refBits[idx] = false;
                return false;
            } else {
                return true;
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            if constexpr (on_hit) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            if constexpr (on_unfix) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {
            if constexpr (on_miss) {
                _refBits[b_idx] = true;
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            if constexpr (on_fixed) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            if constexpr (on_dirty) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            if constexpr (on_blocked) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            if constexpr (on_swizzled) {
                _refBits[idx] = true;
            }
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * \note    This behaviour is an optimization that saves one check for evictablity using the buffer pool.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _refBits[idx] = true;
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details This buffer frame filter does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _refBits
         * \brief   Referenced bits for the buffer frames
         * \details The index of the referenced bit corresponding to buffer frame \c n is \c n .
         */
        std::vector<bool> _refBits;

    };

    /*!\class   PageEvictionerFilterGCLOCK
     * \brief   _GCLOCK_ buffer frame filter
     * \details The _GCLOCK_ buffer frame filter policy is an extremely flexible implementation of the _GCLOCK_ cache
     *          eviction strategy. It combines the variations _GCLOCK-V1_, _GCLOCK-V2_ and (partially) _DGCLOCK_ in one
     *          class. It sets (or increments) the referenced integer of a buffer pool frame to a specified value (by a
     *          specified value) during each page reference. Those values can also be selected dynamically by the level
     *          of the contained page inside the b-tree index structure. The buffer frame is filtered out iff the
     *          corresponding referenced integer is greater than 0. This filtering also decrements the corresponding
     *          referenced integer by a specified value. See the specification of the template parameters for more
     *          details about the customizability of this class.
     *
     * @tparam decrement          The value by which a referenced integer is decremented when the corresponding buffer
     *                            frame index is encountered during eviction.
     * @tparam discriminate_pages If set, the values \c level0_on_* , \c level1_on_* and \c level2_on_* are used for the
     *                            updates of the referenced integers depending on the page contained in the
     *                            corresponding buffer frame. Otherwise only the \c level0_on_* values are used.
     * @tparam on_hit             If set, the eviction statistics are updated on page hit.
     * @tparam set_on_hit         If set, the referenced integer is set to the value of \c level_*_on_hit on page hit.
     *                            Otherwise, the referenced integer is incremented by this value.
     * @tparam level0_on_hit      The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page hit when either \c discriminate_pages is \c false or
     *                            when the page contained in the corresponding buffer frame is either a non-b-tree page
     *                            or a b-tree root page.
     * @tparam level1_on_hit      The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page hit when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is child of a b-tree root page.
     * @tparam level2_on_hit      The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page hit when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is a b-tree page but neither a root,
     *                            nor the child of a root b-tree page.
     * @tparam on_unfix           If set, the eviction statistics are updated on page unfix.
     * @tparam set_on_unfix       If set, the referenced integer is set to the value of \c level_*_on_unfix on page
     *                            unfix. Otherwise, the referenced integer is incremented by this value.
     * @tparam level0_on_unfix    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page unfix when either \c discriminate_pages is \c false or
     *                            when the page contained in the corresponding buffer frame is either a non-b-tree page
     *                            or a b-tree root page.
     * @tparam level1_on_unfix    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page unfix when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is child of a b-tree root page.
     * @tparam level2_on_unfix    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page unfix when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is a b-tree page but neither a root,
     *                            nor the child of a root b-tree page.
     * @tparam on_miss            If set, the eviction statistics are updated on page miss.
     * @tparam set_on_miss        If set, the referenced integer is set to the value of \c level_*_on_miss on page miss.
     *                            Otherwise, the referenced integer is incremented by this value.
     * @tparam level0_on_miss     The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page miss when either \c discriminate_pages is \c false or
     *                            when the page contained in the corresponding buffer frame is either a non-b-tree page
     *                            or a b-tree root page.
     * @tparam level1_on_miss     The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page miss when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is child of a b-tree root page.
     * @tparam level2_on_miss     The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) on page miss when \c discriminate_pages is \c true and when the
     *                            page contained in the corresponding buffer frame is a b-tree page but neither a root,
     *                            nor the child of a root b-tree page.
     * @tparam on_fixed           If set, the eviction statistics are updated when a page is discovered fixed during
     *                            eviction.
     * @tparam set_on_fixed       If set, the referenced integer is set to the value of \c level_*_on_fixed when a page
     *                            is discovered fixed during eviction. Otherwise, the referenced integer is incremented
     *                            by this value.
     * @tparam level0_on_fixed    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered fixed during eviction and when either
     *                            \c discriminate_pages is \c false or when the page contained in the corresponding
     *                            buffer frame is either a non-b-tree page or a b-tree root page.
     * @tparam level1_on_fixed    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered fixed during eviction, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is child of a b-tree root page.
     * @tparam level2_on_fixed    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered fixed during eviction, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is a b-tree page but neither a root, nor the child of a root b-tree page.
     * @tparam on_dirty           If set, the eviction statistics are updated when a page is discovered dirty during
     *                            eviction.
     * @tparam set_on_dirty       If set, the referenced integer is set to the value of \c level_*_on_dirty when a page
     *                            is discovered dirty during eviction. Otherwise, the referenced integer is incremented
     *                            by this value.
     * @tparam level0_on_dirty    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered dirty during eviction and when either
     *                            \c discriminate_pages is \c false or when the page contained in the corresponding
     *                            buffer frame is either a non-b-tree page or a b-tree root page.
     * @tparam level1_on_dirty    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered dirty during eviction, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is child of a b-tree root page.
     * @tparam level2_on_dirty    The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered dirty during eviction, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is a b-tree page but neither a root, nor the child of a root b-tree page.
     * @tparam on_blocked         If set, the eviction statistics are updated when a page is discovered to be
     *                            unevictable at all.
     * @tparam set_on_blocked     If set, the referenced integer is set to the value of \c level_*_on_blocked when a
     *                            page is discovered unevictable at all. Otherwise, the referenced integer is
     *                            incremented by this value.
     * @tparam level0_on_blocked  The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered unevictable at all during eviction and
     *                            when either \c discriminate_pages is \c false or when the page contained in the
     *                            corresponding buffer frame is either a non-b-tree page or a b-tree root page.
     * @tparam level1_on_blocked  The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered unevictable at all, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is child of a b-tree root page.
     * @tparam level2_on_blocked  The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered unevictable at all, when
     *                            \c discriminate_pages is \c true and when the page contained in the corresponding
     *                            buffer frame is a b-tree page but neither a root, nor the child of a root b-tree page.
     * @tparam on_swizzled        If set, the eviction statistics are updated when a page is discovered containing
     *                            swizzled pointers during eviction.
     * @tparam set_on_swizzled    If set, the referenced integer is set to the value of \c level_*_on_swizzled when a
     *                            page is discovered containing swizzled pointers during eviction. Otherwise, the
     *                            referenced integer is incremented by this value.
     * @tparam level0_on_swizzled The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered containing swizzled pointers during
     *                            eviction and when either \c discriminate_pages is \c false or when the page contained
     *                            in the corresponding buffer frame is either a non-b-tree page or a b-tree root page.
     * @tparam level1_on_swizzled The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered containing swizzled pointers during
     *                            eviction, when \c discriminate_pages is \c true and when the page contained in the
     *                            corresponding buffer frame is child of a b-tree root page.
     * @tparam level2_on_swizzled The value by which a referenced integer is incremented (or to which it is set when
     *                            \c set_on_hit is set) when a page is discovered containing swizzled pointers during
     *                            eviction, when \c discriminate_pages is \c true and when the page contained in the
     *                            corresponding buffer frame is a b-tree page but neither a root, nor the child of a
     *                            root b-tree page.
     */
    template <uint16_t decrement, bool discriminate_pages,
            bool on_hit, bool set_on_hit, uint16_t level0_on_hit, uint16_t level1_on_hit, uint16_t level2_on_hit,
            bool on_unfix, bool set_on_unfix, uint16_t level0_on_unfix, uint16_t level1_on_unfix, uint16_t level2_on_unfix,
            bool on_miss, bool set_on_miss, uint16_t level0_on_miss, uint16_t level1_on_miss, uint16_t level2_on_miss,
            bool on_fixed, bool set_on_fixed, uint16_t level0_on_fixed, uint16_t level1_on_fixed, uint16_t level2_on_fixed,
            bool on_dirty, bool set_on_dirty, uint16_t level0_on_dirty, uint16_t level1_on_dirty, uint16_t level2_on_dirty,
            bool on_blocked, bool set_on_blocked, uint16_t level0_on_blocked, uint16_t level1_on_blocked, uint16_t level2_on_blocked,
            bool on_swizzled, bool set_on_swizzled, uint16_t level0_on_swizzled, uint16_t level1_on_swizzled, uint16_t level2_on_swizzled>
    class PageEvictionerFilterGCLOCK : PageEvictionerFilter {
    public:
        /*!\fn      PageEvictionerFilterGCLOCK(const BufferPool* bufferPool)
         * \brief   Constructs a _GCLOCK_ buffer frame filter
         *
         * @param bufferPool The buffer pool this _GCLOCK_ buffer frame filter is responsible for.
         */
        explicit PageEvictionerFilterGCLOCK(const BufferPool* bufferPool) :
                PageEvictionerFilter(bufferPool),
                _refInts(bufferPool->getBlockCount()) {};

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced integer greater than 0.
         *
         * \warning This function does not decrement the referenced integer corresponding to the specified buffer frame.
         *          For each buffer frame discovered evictable, \link filterAndUpdate() \endlink needs to be called
         *          exactly once.
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Whether the value of the referenced integer corresponding to the buffer frame with index \c idx is
         *            greater than 0.
         */
        inline bool filter(bf_idx idx) const noexcept final {
            if (_refInts[idx] > 0) {
                return false;
            } else {
                return true;
            }
        };

        /*!\fn      filter(bf_idx idx) noexcept
         * \brief   Filters a buffer frame for eviction
         * \details Filters out the specified buffer frame if its corresponding referenced integer greater than 0 and
         *          decrements this by the value of the template parameter \c decrement .
         *
         * @param idx The selected buffer frame where the contained page should be evicted from.
         * @return    Whether the value of the referenced integer corresponding to the buffer frame with index \c idx is
         *            greater than 0.
         */
        inline bool filterAndUpdate(bf_idx idx) noexcept final {
            if (_refInts[idx] > 0) {
                _refInts[idx] = std::max(_refInts[idx] - decrement, 0);
                return false;
            } else {
                return true;
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_hit is set. If the template parameter \c discriminate_pages is not set, the
         *          value of the template parameter \c level0_on_hit will be used for the update, otherwise the values
         *          of the template parameters \c level0_on_hit , \c level1_on_hit and \c level2_on_hit will be used
         *          depending on the level of the contained page inside the B-tree. If the template parameter
         *          \c set_on_hit is not set, the referenced integer is decremented by the appropriate value, if it is
         *          set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {
            if constexpr (on_hit) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_hit) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_hit;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_hit;
                        } else {
                            _refInts[idx] = level2_on_hit;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_hit;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_hit;
                        } else {
                            _refInts[idx] += level2_on_hit;
                        }
                    }
                } else {
                    if constexpr (set_on_hit) {
                        _refInts[idx] = level2_on_hit;
                    } else {
                        _refInts[idx] += level2_on_hit;
                    }
                }
            }
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_unfix is set. If the template parameter \c discriminate_pages is not set,
         *          the value of the template parameter \c level0_on_unfix will be used for the update, otherwise the
         *          values of the template parameters \c level0_on_unfix , \c level1_on_unfix and \c level2_on_unfix
         *          will be used depending on the level of the contained page inside the B-tree. If the template
         *          parameter \c set_on_unfix is not set, the referenced integer is decremented by the appropriate
         *          value, if it is set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {
            if constexpr (on_unfix) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_unfix) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_unfix;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_unfix;
                        } else {
                            _refInts[idx] = level2_on_unfix;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_unfix;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_unfix;
                        } else {
                            _refInts[idx] += level2_on_unfix;
                        }
                    }
                } else {
                    if constexpr (set_on_unfix) {
                        _refInts[idx] = level2_on_unfix;
                    } else {
                        _refInts[idx] += level2_on_unfix;
                    }
                }
            }
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_miss is set. If the template parameter \c discriminate_pages is not set,
         *          the value of the template parameter \c level0_on_miss will be used for the update, otherwise the
         *          values of the template parameters \c level0_on_miss , \c level1_on_miss and \c level2_on_miss will
         *          be used depending on the level of the contained page inside the B-tree. If the template parameter
         *          \c set_on_miss is not set, the referenced integer is decremented by the appropriate value, if it is
         *          set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {
            if constexpr (on_miss) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(b_idx);
                    if constexpr (set_on_miss) {
                        if (page_level == 0) {
                            _refInts[b_idx] = level0_on_miss;
                        } else if (page_level == 1) {
                            _refInts[b_idx] = level1_on_miss;
                        } else {
                            _refInts[b_idx] = level2_on_miss;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[b_idx] += level0_on_miss;
                        } else if (page_level == 1) {
                            _refInts[b_idx] += level1_on_miss;
                        } else {
                            _refInts[b_idx] += level2_on_miss;
                        }
                    }
                } else {
                    if constexpr (set_on_miss) {
                        _refInts[b_idx] = level2_on_miss;
                    } else {
                        _refInts[b_idx] += level2_on_miss;
                    }
                }
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_fixed is set. If the template parameter \c discriminate_pages is not set,
         *          the value of the template parameter \c level0_on_fixed will be used for the update, otherwise the
         *          values of the template parameters \c level0_on_fixed , \c level1_on_fixed and \c level2_on_fixed
         *          will be used depending on the level of the contained page inside the B-tree. If the template
         *          parameter \c set_on_fixed is not set, the referenced integer is decremented by the appropriate
         *          value, if it is set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {
            if constexpr (on_fixed) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_fixed) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_fixed;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_fixed;
                        } else {
                            _refInts[idx] = level2_on_fixed;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_fixed;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_fixed;
                        } else {
                            _refInts[idx] += level2_on_fixed;
                        }
                    }
                } else {
                    if constexpr (set_on_fixed) {
                        _refInts[idx] = level2_on_fixed;
                    } else {
                        _refInts[idx] += level2_on_fixed;
                    }
                }
            }
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_dirty is set. If the template parameter \c discriminate_pages is not set,
         *          the value of the template parameter \c level0_on_dirty will be used for the update, otherwise the
         *          values of the template parameters \c level0_on_dirty , \c level1_on_dirty and \c level2_on_dirty
         *          will be used depending on the level of the contained page inside the B-tree. If the template
         *          parameter \c set_on_dirty is not set, the referenced integer is decremented by the appropriate
         *          value, if it is set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {
            if constexpr (on_dirty) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_dirty) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_dirty;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_dirty;
                        } else {
                            _refInts[idx] = level2_on_dirty;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_dirty;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_dirty;
                        } else {
                            _refInts[idx] += level2_on_dirty;
                        }
                    }
                } else {
                    if constexpr (set_on_dirty) {
                        _refInts[idx] = level2_on_dirty;
                    } else {
                        _refInts[idx] += level2_on_dirty;
                    }
                }
            }
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_blocked is set. If the template parameter \c discriminate_pages is not set,
         *          the value of the template parameter \c level0_on_blocked will be used for the update, otherwise the
         *          values of the template parameters \c level0_on_blocked , \c level1_on_blocked and
         *          \c level2_on_blocked will be used depending on the level of the contained page inside the B-tree. If
         *          the template parameter \c set_on_blocked is not set, the referenced integer is decremented by the
         *          appropriate value, if it is set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {
            if constexpr (on_blocked) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_blocked) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_blocked;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_blocked;
                        } else {
                            _refInts[idx] = level2_on_blocked;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_blocked;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_blocked;
                        } else {
                            _refInts[idx] += level2_on_blocked;
                        }
                    }
                } else {
                    if constexpr (set_on_blocked) {
                        _refInts[idx] = level2_on_blocked;
                    } else {
                        _refInts[idx] += level2_on_blocked;
                    }
                }
            }
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the referenced integer of the specified buffer frame (inside \link _refInts \endlink) if the
         *          template parameter \c on_swizzled is set. If the template parameter \c discriminate_pages is not
         *          set, the value of the template parameter \c level0_on_swizzled will be used for the update,
         *          otherwise the values of the template parameters \c level0_on_swizzled , \c level1_on_swizzled and
         *          \c level2_on_swizzled will be used depending on the level of the contained page inside the B-tree.
         *          If the template parameter \c set_on_swizzled is not set, the referenced integer is decremented by
         *          the appropriate value, if it is set, the referenced integer is set to this value.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {
            if constexpr (on_swizzled) {
                if constexpr (discriminate_pages) {
                    uint8_t page_level = getLevel(idx);
                    if constexpr (set_on_swizzled) {
                        if (page_level == 0) {
                            _refInts[idx] = level0_on_swizzled;
                        } else if (page_level == 1) {
                            _refInts[idx] = level1_on_swizzled;
                        } else {
                            _refInts[idx] = level2_on_swizzled;
                        }
                    } else {
                        if (page_level == 0) {
                            _refInts[idx] += level0_on_swizzled;
                        } else if (page_level == 1) {
                            _refInts[idx] += level1_on_swizzled;
                        } else {
                            _refInts[idx] += level2_on_swizzled;
                        }
                    }
                } else {
                    if constexpr (set_on_swizzled) {
                        _refInts[idx] = level2_on_swizzled;
                    } else {
                        _refInts[idx] += level2_on_swizzled;
                    }
                }
            }
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets the referenced integer of the specified buffer frame (inside \link _refInts \endlink) to the
         *          highest possible value.
         *
         * \note    This behaviour is an optimization that saves many checks for evictablity using the buffer pool.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _refInts[idx] = std::numeric_limits<uint16_t>::max();
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame filter
         * \details This buffer frame filter does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\fn      getLevel(const bf_idx& idx) const
         * \brief   B-tree depth of the contained page
         * \details Calculates the depth of the page contained in the buffer frame with index \c idx . If the page is a
         *          B-tree root page, the depth is \c 0 , if it is child of a root, the depth is \c 1 . If it is any
         *          other B-tree page, the depth is \c 2 . Non-b-tree pages are treated like B-tree root pages.
         *
         * \pre     The buffer frame with index \c idx contains a page.
         *
         * \note    This function is uses to discriminate the pages when the template parameter \c discriminate_pages is
         *          set. The values of the template parameter \c level0_on_* is used, when this returns \c 0 etc.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is contains the
         *            page to check.
         * @return    \c o if the page is a non-b-tree page or a b-tree root page, \c 1 if it is a child page of a root
         *            page and \c 2 if it is another b-tree page.
         */
        inline uint8_t getLevel(const bf_idx& idx) const {
            const generic_page* page = smlevel_0::bf->getPage(idx);
            w_assert1(page != nullptr);
            if (page->tag == t_btree_p) {
                btree_page_h fixedPage;
                fixedPage.fix_nonbufferpool_page(const_cast<generic_page*>(page));
                if (fixedPage.pid() == fixedPage.btree_root() || fixedPage.level() > 2) {
                    return 0;
                } else if (fixedPage.level() == 2) {
                    return 1;
                } else if (fixedPage.level() == 1) {
                    return 2;
                }
            } else {     // Non-B-Tree pages are interpreted as B-Tree root pages!
                return 0;
            }
        };

        /*!\var     _refInts
         * \brief   Referenced integers for the buffer frames
         * \details The index of the referenced integer corresponding to buffer frame \c n is \c n .
         */
        std::vector<std::atomic<uint16_t>> _refInts;

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_FILTER_HPP
