#ifndef __PAGE_EVICTIONER_SELECTOR_LOOP_HPP
#define __PAGE_EVICTIONER_SELECTOR_LOOP_HPP

#include "page_evictioner_selector.hpp"

#include <mutex>
#include <atomic>

namespace zero::buffer_pool {

    /*!\class   PageEvictionerSelectorLOOPMutex
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. The absolute accurate
     *          version of the _LOOP_ policy always selects the buffer frames exactly this order.
     */
    class PageEvictionerSelectorLOOPMutex : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPMutex(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPMutex(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _lastFrame(_maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            w_assert1(_lastFrame > 0 && _lastFrame <= _maxBufferpoolIndex);

            std::lock_guard<std::mutex> guard(_lastFrameLock);
            if (_lastFrame >= _maxBufferpoolIndex) {
                _lastFrame = 1;
            } else {
                _lastFrame++;
            }
            return _lastFrame;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _lastFrame
         * \brief   The buffer frame index last picked for eviction
         * \details This is the buffer frame index that was picked for eviction the last time \link select() \endlink
         *          was called.
         */
        volatile bf_idx _lastFrame;

        /*!\var     _lastFrameLock
         * \brief   Protects the buffer frame index last picked for eviction from concurrent manipulations
         * \details This lock protects the \link _lastFrame \endlink from hazards caused by multiple threads
         *          concurrently evicting pages from the \link BufferPool \endlink and therefore, concurrently
         *          manipulating the \link _lastFrame \endlink.
         */
        std::mutex _lastFrameLock;
    };

    /*!\class   PageEvictionerSelectorLOOPSpinlock
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. The absolute accurate
     *          version of the _LOOP_ policy always selects the buffer frames exactly this order.
     */
    class PageEvictionerSelectorLOOPSpinlock : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPSpinlock(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPSpinlock(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _lastFrame(_maxBufferpoolIndex),
                _lastFrameLock(ATOMIC_FLAG_INIT) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            w_assert1(_lastFrame > 0 && _lastFrame <= _maxBufferpoolIndex);

            bf_idx newFrame;
            while (_lastFrameLock.test_and_set(std::memory_order_acquire)) {}
            if (_lastFrame >= _maxBufferpoolIndex) {
                _lastFrame = 1;
            } else {
                _lastFrame++;
            }
            newFrame = _lastFrame;
            _lastFrameLock.clear(std::memory_order_acquire);
            return newFrame;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _lastFrame
         * \brief   The buffer frame index last picked for eviction
         * \details This is the buffer frame index that was picked for eviction the last time \link select() \endlink
         *          was called.
         */
        volatile bf_idx _lastFrame;

        /*!\var     _lastFrameLock
         * \brief   Protects the buffer frame index last picked for eviction from concurrent manipulations
         * \details This lock protects the \link _lastFrame \endlink from hazards caused by multiple threads
         *          concurrently evicting pages from the \link BufferPool \endlink and therefore, concurrently
         *          manipulating the \link _lastFrame \endlink.
         */
        std::atomic_flag _lastFrameLock;
    };

    /*!\class   PageEvictionerSelectorLOOPModulo
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. The practically accurate
     *          version of the _LOOP_ policy does not always selects the buffer frames exactly in this order---it is off
     *          once every 18446744073709551616 buffer frame selects.
     */
    class PageEvictionerSelectorLOOPModulo : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPModulo(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPModulo(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _lastFrame(0) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {// Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
            return (_lastFrame++ % _maxBufferpoolIndex) + 1;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _lastFrame
         * \brief   Last control block examined
         * \details Represents the clock hand pointing to the control block that was examined last during the most
         *          recent execution of \link select() \endlink (evicted last).
         *
         * \remark  Only used by __LOOP__ and __CLOCK__.
         */
        std::atomic<uint_fast64_t> _lastFrame;
    };

    /*!\class   PageEvictionerSelectorLOOPLockFree
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. TODO.
     */
    class PageEvictionerSelectorLOOPLockFree : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPLockFree(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPLockFree(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _newFrame(1) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            uint32_t pickedFrame = _newFrame;
            if (pickedFrame <= _maxBufferpoolIndex) {
                _newFrame++;
                w_assert1(pickedFrame > 0 && pickedFrame <= _maxBufferpoolIndex);
                return pickedFrame;
            } else {
                return _newFrame = 1;
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _newFrame
         * \brief   Last control block examined
         * \details Represents the clock hand pointing to the control block that was examined last during the most
         *          recent execution of \link select() \endlink (evicted last).
         *
         * \remark  Only used by __LOOP__ and __CLOCK__.
         */
        std::atomic<uint32_t> _newFrame;
    };

    /*!\class   PageEvictionerSelectorLOOPThreadLocal
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. When only one thread is
     *          used for eviction, the buffer frames are selected in perfect _LOOP_ order but if multiple threads are
     *          used for eviction, the _LOOP_ order of buffer frame selects is only thread-local when this
     *          thread-locally accurate version of the _LOOP_ policy is used.
     */
    class PageEvictionerSelectorLOOPThreadLocal : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPModulo(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPThreadLocal(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _lastFrame
             * \brief   Last control block examined (per evicting thread)
             * \details Represents the clock hand pointing to the control block that was examined last during the most
             *          recent execution of \link select() \endlink (evicted last) on this thread.
             *
             * \remark  Only used by __LOOP__ and __CLOCK__.
             */
            static thread_local bf_idx _lastFrame;

            if (_lastFrame >= _maxBufferpoolIndex) {
                _lastFrame = 1;
            } else {
                _lastFrame++;
            }
            w_assert1(_lastFrame > 0 && _lastFrame <= _maxBufferpoolIndex);
            return _lastFrame;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};
    };

    /*!\class   PageEvictionerSelectorLOOPThreadLocalModulo
     * \brief   _LOOP_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LOOP_
     *          policy. The _LOOP_ policy selects buffer frames by looping over the buffer frame IDs from \c 1 to
     *          \link _maxBufferpoolIndex \endlink and afterwards restarting from \c 1 again. When only one thread is
     *          used for eviction, the buffer frames are selected in perfect _LOOP_ order but if multiple threads are
     *          used for eviction, the _LOOP_ order of buffer frame selects is only thread-local when this
     *          thread-locally accurate version of the _LOOP_ policy is used.
     */
    class PageEvictionerSelectorLOOPThreadLocalModulo : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLOOPThreadLocalModulo(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorLOOPThreadLocalModulo(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {// Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!

            /*!\var     _lastFrame
             * \brief   Last control block examined (per evicting thread)
             * \details Represents the clock hand pointing to the control block that was examined last during the most
             *          recent execution of \link select() \endlink (evicted last) on this thread.
             *
             * \remark  Only used by __LOOP__ and __CLOCK__.
             */
            static thread_local uint64_t _lastFrame;

            return (_lastFrame++ % _maxBufferpoolIndex) + 1;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details This buffer frame selector does not require any statistics and therefore, this function does
         *          nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore, this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};
    };
} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_SELECTOR_LOOP_HPP
