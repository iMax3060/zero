#ifndef __PAGE_EVICTIONER_SELECTOR_HPP
#define __PAGE_EVICTIONER_SELECTOR_HPP

#include <mutex>
#include <random>

#include <atomic>

#include "buffer_pool.hpp"

namespace zero::buffer_pool {

    /*!\class   PageEvictionerSelector
     * \brief   Buffer frame selector for the _Select-and-Filter_ page evictioner
     * \details This class defines the interface for buffer frame selectors (which represent different policies) used in
     *          the \link PageEvictionerSelectAndFilter \endlink. All those buffer frame selectors have to inherit from
     *          this class.
     */
    class PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelector()
         * \brief   Constructs a buffer frame selector
         *
         * @param bufferPool The buffer pool this buffer frame selector is responsible for.
         */
        PageEvictionerSelector(const BufferPool* bufferPool) :
                _maxBufferpoolIndex(bufferPool->getBlockCount() - 1) {};

        /*!\fn      ~PageEvictionerSelector()
         * \brief   Destructs a buffer frame selector
         */
        virtual ~PageEvictionerSelector() {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a page to be evicted according to the selected buffer frame selector. The
         *          \link PageEvictionerSelectAndFilter \endlink will filter the selected buffer frame using a
         *          \link PageEvictionerFilter \endlink.
         *
         * \note    This member function must be implemented by every specific buffer frame selector.
         *
         * @return The selected buffer frame.
         */
        virtual bf_idx select() noexcept = 0;

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Updates the statistics according to the selected buffer frame selection policy when a page hit
         *          occurred.
         *
         * \warning If a buffer frame selection policy is sensitive to recognizing the same page reference multiple
         *          times, implement this different from \link updateOnPageUnfix() \endlink.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        virtual void updateOnPageHit(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Updates the statistics according to the selected buffer frame selection policy when a page unfix
         *          occurred.
         *
         * \warning If a buffer frame selection policy is sensitive to recognizing the same page reference multiple
         *          times, implement this different from \link updateOnPageHit() \endlink.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        virtual void updateOnPageUnfix(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Updates the statistics according to the selected buffer frame selection policy when a page miss
         *          occurred.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        virtual void updateOnPageMiss(bf_idx idx, PageID pid) noexcept = 0;

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Updates the statistics according to the selected buffer frame selection policy when a page was
         *          discovered fixed during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        virtual void updateOnPageFixed(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Updates the statistics according to the selected buffer frame selection policy when a page was
         *          discovered dirty during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        virtual void updateOnPageDirty(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Updates the statistics according to the selected buffer frame selection policy when a page cannot be
         *          evicted at all.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        virtual void updateOnPageBlocked(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Updates the statistics according to the selected buffer frame selection policy when a page was
         *          discovered containing swizzled pointers during eviction.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        virtual void updateOnPageSwizzled(bf_idx idx) noexcept = 0;

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Updates the statistics according to the selected buffer frame selection policy when a page got
         *          explicitly unbuffered.
         *
         * \note    This member function must be implemented by every specific buffer frame selection policy.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept = 0;

    protected:
        /*!\var     _maxBufferpoolIndex
         * \brief   The maximum buffer frame index
         */
        bf_idx          _maxBufferpoolIndex;

    };

    /*!\class   PageEvictionerSelectorLOOPAbsolutelyAccurate
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. The absolute accurate
     *          version of the _LOOP_ policy always selects the buffer frames exactly this order.
     */
    class PageEvictionerSelectorLOOPAbsolutelyAccurate : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPAbsolutelyAccurate()
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorLOOPAbsolutelyAccurate(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _currentFrame(_maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            w_assert1(_currentFrame > 0 && _currentFrame <= _maxBufferpoolIndex);

            std::lock_guard<std::mutex> guard(_currentFrameLock);
            if (_currentFrame >= _maxBufferpoolIndex) {
                _currentFrame = 1;
            } else {
                _currentFrame++;
            }
            return _currentFrame;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

    private:
        /*!\var     _currentFrame
         * \brief   The buffer frame index last picked for eviction
         * \details This is the buffer frame index that was picked for eviction the last time \link select() \endlink
         *          was called.
         */
        volatile bf_idx _currentFrame;

        /*!\var     _currentFrameLock
         * \brief   Protects the buffer frame index last picked for eviction from concurrent manipulations
         * \details This lock protects the \link _currentFrame \endlink from hazards caused by multiple threads
         *          concurrently evicting pages from the \link BufferPool \endlink and therefore concurrently
         *          manipulating the \link _currentFrame \endlink.
         */
        std::mutex _currentFrameLock;

    };

    /*!\class   PageEvictionerSelectorLOOPPracticallyAccurate
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. The practically accurate
     *          version of the _LOOP_ policy does not always selects the buffer frames exactly in this order---it is off
     *          once every 18446744073709551616 buffer frame selects.
     */
    class PageEvictionerSelectorLOOPPracticallyAccurate : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPPracticallyAccurate()
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorLOOPPracticallyAccurate(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _current_frame(1) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {// Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
            w_assert1(_current_frame > 0 && _current_frame <= _maxBufferpoolIndex);

            while (true) {
                uint_fast32_t this_frame = _current_frame++ % (_maxBufferpoolIndex + 1);
                if (this_frame == 0) {
                    continue;
                } else {
                    return this_frame;
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

    private:
        /*!\var     _current_frame
         * \brief   Last control block examined
         * \details Represents the clock hand pointing to the control block that was
         *          examined last during the most recent execution of
         *          \link pickVictim() \endlink (evicted last).
         *
         * \remark  Only used by __LOOP__ and __CLOCK__.
         */
        std::atomic<uint_fast64_t> _current_frame;

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_SELECTOR_HPP
