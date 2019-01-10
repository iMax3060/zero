#ifndef __PAGE_EVICTIONER_SELECTOR_HPP
#define __PAGE_EVICTIONER_SELECTOR_HPP

#include <mutex>
#include <random>

#include <atomic>

#include "cds/container/fcqueue.h"
#include "cds/container/fcstack.h"
#include "MPMCQueue/MPMCQueue.h"
#include <vector>

#include "hashtable_deque.hpp"

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
        /*!\fn      PageEvictionerSelector(const BufferPool* bufferPool)
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
        /*!\fn      PageEvictionerSelectorLOOPAbsolutelyAccurate(const BufferPool* bufferPool)
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
        /*!\fn      PageEvictionerSelectorLOOPPracticallyAccurate(const BufferPool* bufferPool)
         * \brief   Constructs a _LOOP_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LOOP_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorLOOPPracticallyAccurate(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _currentFrame(1) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If it selected the buffer frame \c n the last time this function was called, then it selects
         *          \c n \c + \c 1 if \c n \c + \c 1 \c <= \link _maxBufferpoolIndex \endlink or \c 1 if \c n \c +
         *          \c 1 \c > \link _maxBufferpoolIndex \endlink.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {// Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
            w_assert1(_currentFrame > 0 && _currentFrame <= _maxBufferpoolIndex);

            while (true) {
                uint_fast32_t this_frame = _currentFrame++ % (_maxBufferpoolIndex + 1);
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
         * \brief   Last control block examined
         * \details Represents the clock hand pointing to the control block that was examined last during the most
         *          recent execution of \link select() \endlink (evicted last).
         *
         * \remark  Only used by __LOOP__ and __CLOCK__.
         */
        std::atomic<uint_fast64_t> _currentFrame;

    };

    /*!\class   PageEvictionerSelectorRANDOMDefault
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. The default version of the _RANDOM_ policy
     *          uses the random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMDefault : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMDefault(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorRANDOMDefault(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the random number generator of the C++ Standard Library over a
         *          uniform distribution.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            return _randomDistribution(_standardRandomEngine);
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
        /*!\var     _standardRandomEngine
         * \brief   The used pseudo-random number generator
         */
        std::default_random_engine _standardRandomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMDefault
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. The fast version of the _RANDOM_ policy
     *          uses a very fast linear congruential generator and a thread-local state.
     */
    class PageEvictionerSelectorRANDOMFastRand : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMFastRand(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorRANDOMFastRand(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a very fast linear congruential generator.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            if (!_randomStateInitialized) {
                _randomState = std::random_device{}();
                _randomStateInitialized = true;
            }
            _randomState = 214013 * _randomState + 2531011;
            return ((_randomState >> 16 & 0x7FFF) % _maxBufferpoolIndex) + 1;
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
        /*!\var     _randomStateInitialized
         * \brief   Whether the state of the LCG is initialized on this thread
         * \details The first time \link select() \endlink runs on a particular thread, the state of the linear
         *          congruential generator needs to be initialized. If this is set on a thread, the state is already
         *          initialized on this thread.
         */
        static thread_local bool _randomStateInitialized;

        /*!\var     _randomState
         * \brief   The state of the linear congruential generator
         */
        static thread_local uint_fast32_t _randomState;

    };

    /*!\class   PageEvictionerSelectorQuasiFIFOLowContention
     * \brief   _FIFO_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _FIFO_
     *          policy. The _FIFO_ policy selects the buffer frame containing the page that is the longest in the
     *          buffer. But due to the fact that currently fixed or dirty pages or pages containing swizzled pointers
     *          cannot (always) be evicted until their state changes w.r.t. that, this buffer frame selector does not
     *          implement a perfect _FIFO_ policy. This selector does not manage one, but two queues---one for the
     *          frames which were not found not evictable since last time a page was loaded into them
     *          (\link _initialList \endlink) and another one for pages which could not be evicted the last time they
     *          were selected by this (\link _retryList \endlink). A thread evicting alternately selects buffer frames
     *          from both of the queues. The template parameters of this selector specify the number of buffer frames
     *          to select before changing the list. The low contention version of this buffer frame selector uses
     *          queues which work better under low contention.
     *
     * @tparam retry_list_check_ppm   The fraction (in PPM) of the \link _retryList \endlink queue selected by a thread
     *                                before selecting buffer frames from the \link _initialList \endlink.
     * @tparam initial_list_check_ppm The fraction (in PPM) of the \link _initialList \endlink queue selected by a
     *                                thread before selecting buffer frames from the \link _retryList \endlink.
     *
     * \note See the function specifications for more details about the implementation.
     */
    template <uint32_t retry_list_check_ppm/* = 1000000*/, uint32_t initial_list_check_ppm/* = 10000*/>
    class PageEvictionerSelectorQuasiFIFOLowContention : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorQuasiFIFOLowContention(const BufferPool* bufferPool)
         * \brief   Constructs a _FIFO_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _FIFO_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorQuasiFIFOLowContention(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _notExplicitlyEvictedList(bufferPool->getBlockCount()) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If a thread is currently selecting pages from the \link _initialList \endlink, if it should continue
         *          with that according to the template parameter \c initial_list_check_ppm and if the
         *          \link _initialList \endlink is not empty, this selects the front of the \link _initialList \endlink.
         *          Otherwise, it selects the front of the \link _retryList \endlink.
         *
         *          Explicitly evicted buffer frames are removed from the queue.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            static thread_local bool currentlyCheckingRetryList;
            static thread_local size_t retriedBufferIndexes;

            bf_idx selected;
            while (true) {
                if (currentlyCheckingRetryList) {   // This thread checked the front of the _retryList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(retry_list_check_ppm * 0.000001 * _retryList.size())
                     || _initialList.empty()) {     // Check again the front of the _retryList:
                        _retryList.pop(selected);
                        retriedBufferIndexes++;
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        return selected;
                    } else {                        // Change to checking entries from the front of the _initialList
                        _initialList.pop(selected);
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = false;
                        return selected;
                    }
                } else {                            // This thread checked the front of the _initialList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(initial_list_check_ppm * 0.000001 * _initialList.size())
                     || _retryList.empty()) {       // Check again the front of the _initialList:
                        _initialList.pop(selected);
                        retriedBufferIndexes++;
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        return selected;
                    } else {                        // Change to checking entries from the front of the _retryList
                        _retryList.pop(selected);
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = true;
                        return selected;
                    }
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the back of the \link _initialList \endlink queue if the respective
         *          buffer frame index was not explicitly evicted previously. Then, the buffer frame index stays in the
         *          list it was before it was explicitly evicted.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            if (!_notExplicitlyEvictedList[idx].test_and_set()) {
                _initialList.push(idx);
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets this buffer frame explicitly evicted so it is ignored by this page evictioner. But it is
         *          removed from the \link _initialList \endlink or from the \link _retryList \endlink once it is found
         *          by \link select() \endlink. If the buffer frame is not found by \link select() \endlink before it is
         *          used next, it will be in its old position in the respective queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _notExplicitlyEvictedList[idx].clear();
        };

    private:
        /*!\var     _initialList
         * \brief   Queue of buffer frames currently used but not selected for eviction
         * \details Contains all the buffer frame indexes of currently used buffer frames. The front of the queue is the
         *          buffer frame where the contained page is in the buffer pool for the longest time. Once a buffer
         *          frame was found not evictable during eviction it is not in this queue anymore.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        cds::container::FCQueue<bf_idx> _initialList;

        /*!\var     _retryList
         * \brief   Queue of buffer frames currently last selected for eviction
         * \details Contains the buffer frame indexes of currently used buffer frames which were already found not
         *          evictable during eviction. The front of the queue is the buffer frame index that was the last found
         *          to be not evictable during eviction.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        cds::container::FCQueue<bf_idx> _retryList;

        /*!\var     _notExplicitlyEvictedList
         * \brief   Flags not explicitly buffer frames
         * \details Explicitly evicted buffer frames still in either \link _initialList \endlink or
         *          \link _retryList \endlink have this flag unset.
         */
        std::vector<std::atomic_flag>   _notExplicitlyEvictedList;

    };

    /*!\class   PageEvictionerSelectorQuasiFIFOHighContention
     * \brief   _FIFO_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _FIFO_
     *          policy. The _FIFO_ policy selects the buffer frame containing the page that is the longest in the
     *          buffer. But due to the fact that currently fixed or dirty pages or pages containing swizzled pointers
     *          cannot (always) be evicted until their state changes w.r.t. that, this buffer frame selector does not
     *          implement a perfect _FIFO_ policy. This selector does not manage one, but two queues---one for the
     *          frames which were not found not evictable since last time a page was loaded into them
     *          (\link _initialList \endlink) and another one for pages which could not be evicted the last time they
     *          were selected by this (\link _retryList \endlink). A thread evicting alternately selects buffer frames
     *          from both of the queues. The template parameters of this selector specify the number of buffer frames
     *          to select before changing the list. The high contention version of this buffer frame selector uses
     *          queues which work better under high contention.
     *
     * @tparam retry_list_check_ppm   The fraction (in PPM) of the \link _retryList \endlink queue selected by a thread
     *                                before selecting buffer frames from the \link _initialList \endlink.
     * @tparam initial_list_check_ppm The fraction (in PPM) of the \link _initialList \endlink queue selected by a
     *                                thread before selecting buffer frames from the \link _retryList \endlink.
     *
     * \note See the function specifications for more details about the implementation.
     */
    template <uint32_t retry_list_check_ppm/* = 1000000*/, uint32_t initial_list_check_ppm/* = 10000*/>
    class PageEvictionerSelectorQuasiFIFOHighContention : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorQuasiFIFOHighContention(const BufferPool* bufferPool)
         * \brief   Constructs a _FIFO_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _FIFO_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorQuasiFIFOHighContention(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _approximateInitialListLength(0),
                _approximateRetryListLength(0),
                _initialList(bufferPool->getBlockCount()),
                _retryList(bufferPool->getBlockCount()),
                _notExplicitlyEvictedList(bufferPool->getBlockCount()) {};

        bf_idx select() noexcept final {
            static thread_local bool currentlyCheckingRetryList;
            static thread_local size_t retriedBufferIndexes;

            bf_idx selected;
            while (true) {
                if (currentlyCheckingRetryList) {               // This thread checked the front of the _retryList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(retry_list_check_ppm * 0.000001 * _approximateRetryListLength)) {
                        if (!_retryList.try_pop(selected)) {    // Check again the front of the _retryList
                            currentlyCheckingRetryList = false;
                            continue;
                        } else {
                            _approximateRetryListLength--;
                            retriedBufferIndexes++;
                            if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                                continue;                       // This buffer frame was explicitly evicted
                            }
                            return selected;
                        }
                    } else {                                    // Change to checking entries from the front of the _initialList
                        if (!_initialList.try_pop(selected)) {
                            currentlyCheckingRetryList = true;
                            continue;
                        } else {
                            _approximateInitialListLength--;
                            retriedBufferIndexes++;
                            if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                                continue;                       // This buffer frame was explicitly evicted
                            }
                            return selected;
                        }
                    }
                } else {                                        // This thread checked the front of the _initialList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(initial_list_check_ppm * 0.000001 * _approximateInitialListLength)) {     // Check again the front of the _initialList:
                        if (!_initialList.try_pop(selected)) {  // Check again the front of the _initialList
                            currentlyCheckingRetryList = true;
                            continue;
                        } else {
                            _approximateInitialListLength--;
                            retriedBufferIndexes++;
                            if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                                continue;                       // This buffer frame was explicitly evicted
                            }
                            return selected;
                        }
                    } else {                                    // Change to checking entries from the front of the _retryList
                        if (!_retryList.try_pop(selected)) {
                            currentlyCheckingRetryList = false;
                            continue;
                        } else {
                            _approximateRetryListLength--;
                            retriedBufferIndexes++;
                            if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                                continue;                       // This buffer frame was explicitly evicted
                            }
                            return selected;
                        }
                    }
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the back of the \link _initialList \endlink queue if the respective
         *          buffer frame index was not explicitly evicted previously. Then, the buffer frame index stays in the
         *          list it was before it was explicitly evicted.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            if (!_notExplicitlyEvictedList[idx].test_and_set()) {
                _initialList.push(idx);
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            _retryList.push(idx);
            _approximateRetryListLength++;
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            _retryList.push(idx);
            _approximateRetryListLength++;
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            _retryList.push(idx);
            _approximateRetryListLength++;
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _retryList.push(idx);
            _approximateRetryListLength++;
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets this buffer frame explicitly evicted so it is ignored by this page evictioner. But it is
         *          removed from the \link _initialList \endlink or from the \link _retryList \endlink once it is found
         *          by \link select() \endlink. If the buffer frame is not found by \link select() \endlink before it is
         *          used next, it will be in its old position in the respective queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _notExplicitlyEvictedList[idx].clear();
        };

    private:
        /*!\var     _initialList
         * \brief   Queue of buffer frames currently used but not selected for eviction
         * \details Contains all the buffer frame indexes of currently used buffer frames. The front of the queue is the
         *          buffer frame where the contained page is in the buffer pool for the longest time. Once a buffer
         *          frame was found not evictable during eviction it is not in this queue anymore.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        rigtorp::MPMCQueue<bf_idx>      _initialList;

        /*!\var     _approximateInitialListLength
         * \brief   Approximate length of the \link _initialList \endlink (not synchronized)
         */
        atomic_bf_idx                   _approximateInitialListLength;

        /*!\var     _retryList
         * \brief   Queue of buffer frames currently last selected for eviction
         * \details Contains the buffer frame indexes of currently used buffer frames which were already found not
         *          evictable during eviction. The front of the queue is the buffer frame index that was the last found
         *          to be not evictable during eviction.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        rigtorp::MPMCQueue<bf_idx>      _retryList;

        /*!\var     _approximateRetryListLength
         * \brief   Approximate length of the \link _initialList \endlink (not synchronized)
         */
        atomic_bf_idx                   _approximateRetryListLength;

        /*!\var     _notExplicitlyEvictedList
         * \brief   Flags not explicitly buffer frames
         * \details Explicitly evicted buffer frames still in either \link _initialList \endlink or
         *          \link _retryList \endlink have this flag unset.
         */
        std::vector<std::atomic_flag>   _notExplicitlyEvictedList;

    };

    /*!\class   PageEvictionerSelectorQuasiFILOLowContention
     * \brief   _FIFO_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _FILO_
     *          policy. The _FILO_ policy selects the buffer frame containing the page that is the shortest in the
     *          buffer. But due to the fact that currently fixed or dirty pages or pages containing swizzled pointers
     *          cannot (always) be evicted until their state changes w.r.t. that, this buffer frame selector does not
     *          implement a perfect _FILO_ policy. This selector does manage a stack and an additional queue---the stack
     *          for the frames which were not found not evictable since last time a page was loaded into them
     *          (\link _initialList \endlink) and the queue for pages which could not be evicted the last time they
     *          were selected by this (\link _retryList \endlink). A thread evicting alternately selects buffer frames
     *          from the stack and the queue. The template parameters of this selector specify the number of buffer
     *          frames to select before changing the list.
     *
     * @tparam retry_list_check_ppm   The fraction (in PPM) of the \link _retryList \endlink stack selected by a thread
     *                                before selecting buffer frames from the \link _initialList \endlink.
     * @tparam initial_list_check_ppm The fraction (in PPM) of the \link _initialList \endlink queue selected by a
     *                                thread before selecting buffer frames from the \link _retryList \endlink.
     *
     * \note See the function specifications for more details about the implementation.
     */
    template <uint32_t retry_list_check_ppm/* = 1000000*/, uint32_t initial_list_check_ppm/* = 10000*/>
    class PageEvictionerSelectorQuasiFILOLowContention : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorQuasiFILOLowContention(const BufferPool* bufferPool)
         * \brief   Constructs a _FILO_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _FILO_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorQuasiFILOLowContention(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _notExplicitlyEvictedList(bufferPool->getBlockCount()) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If a thread is currently selecting pages from the \link _initialList \endlink, if it should continue
         *          with that according to the template parameter \c initial_list_check_ppm and if the
         *          \link _initialList \endlink is not empty, this selects the front of the \link _initialList \endlink.
         *          Otherwise, it selects the front of the \link _retryList \endlink.
         *
         *          Explicitly evicted buffer frames are removed from the queue.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            static thread_local bool currentlyCheckingRetryList;
            static thread_local size_t retriedBufferIndexes;

            bf_idx selected;
            while (true) {
                if (currentlyCheckingRetryList) {   // This thread checked the front of the _retryList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(retry_list_check_ppm * 0.000001 * _retryList.size())
                     || _initialList.empty()) {     // Check again the front of the _retryList:
                        _retryList.pop(selected);
                        retriedBufferIndexes++;
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        return selected;
                    } else {                        // Change to checking entries from the front of the _initialList
                        _initialList.pop(selected);
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = false;
                        return selected;
                    }
                } else {                            // This thread checked the front of the _initialList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(initial_list_check_ppm * 0.000001 * _initialList.size())
                     || _retryList.empty()) {       // Check again the front of the _initialList:
                        _initialList.pop(selected);
                        retriedBufferIndexes++;
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        return selected;
                    } else {                        // Change to checking entries from the front of the _retryList
                        _retryList.pop(selected);
                        if (!_notExplicitlyEvictedList[selected].test_and_set()) {
                            continue;               // This buffer frame was explicitly evicted
                        }
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = true;
                        return selected;
                    }
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics about page references and therefore this
         *          function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the top of the \link _initialList \endlink stack if the respective
         *          buffer frame index was not explicitly evicted previously. Then, the buffer frame index stays in the
         *          list it was before it was explicitly evicted.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            if (!_notExplicitlyEvictedList[idx].test_and_set()) {
                _initialList.push(idx);
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Adds the buffer frame index to the back of the \link _retryList \endlink queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _retryList.push(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets this buffer frame explicitly evicted so it is ignored by this page evictioner. But it is
         *          removed from the \link _initialList \endlink or from the \link _retryList \endlink once it is found
         *          by \link select() \endlink. If the buffer frame is not found by \link select() \endlink before it is
         *          used next, it will be in its old position in the respective queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _notExplicitlyEvictedList[idx].clear();
        };

    private:
        /*!\var     _initialList
         * \brief   Queue of buffer frames currently used but not selected for eviction
         * \details Contains all the buffer frame indexes of currently used buffer frames. The to of the stack is the
         *          buffer frame where the contained page is in the buffer pool for the shortest time. Once a buffer
         *          frame was found not evictable during eviction it is not in this queue anymore.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        cds::container::FCStack<bf_idx> _initialList;

        /*!\var     _retryList
         * \brief   Queue of buffer frames currently last selected for eviction
         * \details Contains the buffer frame indexes of currently used buffer frames which were already found not
         *          evictable during eviction. The front of the queue is the buffer frame index that was the last found
         *          to be not evictable during eviction.
         *
         * \note    This might also contain explicitly evicted buffer frames (currently not in use or reused) at
         *          arbitrary positions.
         */
        cds::container::FCQueue<bf_idx> _retryList;

        /*!\var     _notExplicitlyEvictedList
         * \brief   Flags not explicitly buffer frames
         * \details Explicitly evicted buffer frames still in either \link _initialList \endlink or
         *          \link _retryList \endlink have this flag unset.
         */
        std::vector<std::atomic_flag>   _notExplicitlyEvictedList;

    };

    /*!\class   PageEvictionerSelectorLRU
     * \brief   _LRU_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LRU_
     *          policy. The _LRU_ policy selects the buffer frame that was not used for the longest time. This is
     *          implemented using a queue which allows to move referenced buffer frames from some place within the queue
     *          up. This requires latching during each page reference to prevent races.
     */
    class PageEvictionerSelectorLRU : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLRU(const BufferPool* bufferPool)
         * \brief   Constructs a _LRU_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LRU_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorLRU(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _lruList(bufferPool->getBlockCount()) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects the buffer frame from the buffer pool that was not used for the longest time.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            bf_idx selected;
            std::lock_guard<std::mutex> lock(_lruListLock);
            _lruList.popFromFront(selected);
            return selected;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the back of the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Moves the buffer frame index from some place within the queue to the back.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
            _lruList.pushToBack(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Removes the buffer frame index from the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {}
        };

    private:
        /*!\var     _lruList
         * \brief   Queue of recently referenced buffer frame indexes (most recent in the back)
         */
        zero::hashtable_dequeu::HashtableDeque<bf_idx, 0>   _lruList;

        /*!\var     _lruListLock
         * \brief   Latch protecting the \link _lruList \endlink from concurrent manipulations
         */
        std::mutex                                          _lruListLock;

    };

    /*!\class   PageEvictionerSelectorSLRU
     * \brief   _SLRU_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the
     *          _Segmented LRU_ policy as proposed in _Caching Strategies to Improve Disk System Performance_ by
     *          R. Karedla, J.S. Love and B.G. Wherry. The _SLRU_ policy is an extension of the _LRU_ policy. This
     *          requires latching during each page reference to prevent races.
     *
     * @tparam protected_block_ppm The fraction (in PPM) of buffer frames that fit into the protected segment.
     */
    template <bf_idx protected_block_ppm/* = 10000*/>
    class PageEvictionerSelectorSLRU : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorSLRU(const BufferPool* bufferPool)
         * \brief   Constructs a _Segmented LRU_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _SLRU_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorSLRU(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _protectedBlockCount(static_cast<bf_idx>(protected_block_ppm * 0.000001 * smlevel_0::bf->getBlockCount())),
                _protectedLRUList(_protectedBlockCount),
                _probationaryLRUList(bufferPool->getBlockCount()) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects the buffer frame from the probationary segment that was not used for the longest time.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            bf_idx selected;
            _probationaryLRUList.popFromFront(selected);
            return selected;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            _probationaryLRUList.pushToBack(b_idx);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Moves the buffer frame index from some segment to the back of the protected segment. If the
         *          protected segment is greater than supposed, the least recently used buffer frame is moved from the
         *          protected segment to the back of the probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
            if (_protectedLRUList.length() >= _protectedBlockCount - 1) {
                bf_idx downgradeIndex;
                _protectedLRUList.popFromFront(downgradeIndex);
                _probationaryLRUList.pushToBack(downgradeIndex);
            }
            _protectedLRUList.pushToBack(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Removes the buffer frame index from the protected or probationary segment.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _protectedLRUList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _probationaryLRUList.remove(idx);
            }
        };

    private:
        /*!\var     _protectedLRUList
         * \brief   The protected segment sorted according to reference recency
         */
        zero::hashtable_dequeu::HashtableDeque<bf_idx, 0>   _protectedLRUList;

        /*!\var     _probationaryLRUList
         * \brief   The probationary segment sorted according to reference recency
         */
        zero::hashtable_dequeu::HashtableDeque<bf_idx, 0>   _probationaryLRUList;

        /*!\var     _protectedBlockCount
         * \brief   Maximum number of buffer frame indexes in the protected segment
         */
        const bf_idx                                        _protectedBlockCount;

        /*!\var     _lruListLock
         * \brief   Protects the protected and probationary segments from concurrent manupulations
         */
        std::mutex                                          _lruListLock;

    };

    /*!\class   PageEvictionerSelectorLRUK
     * \brief   _LRU-k_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LRU-k_
     *          policy. The _LRU-k_ policy selects the buffer frame that was not used _k_-times for the longest time.
     *          This is implemented using a queue which allows to move referenced buffer frames from some place within
     *          the queue up. This requires latching during each page reference to prevent races.
     *
     * @tparam k             The number of last references considered per buffer frame.
     * @tparam on_page_unfix Count the unfix of a page as a reference instead of the fix.
     */
    template <size_t k/* = 2*/, bool on_page_unfix/* = false*/>
    class PageEvictionerSelectorLRUK : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorLRU(const BufferPool* bufferPool)
         * \brief   Constructs a _LRU-k_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LRU-k_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorLRUK(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _lruList(bufferPool->getBlockCount()),
                _frameReferences(bufferPool->getBlockCount(), 0) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects the buffer frame from the buffer pool whose _k_-th page reference is furthest in the past.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            uint64_t selected;
            _lruList.popFromFront(selected);
            return static_cast<bf_idx>(selected / k);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one if the template parameter \c on_page_unfix is not set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            if constexpr (!on_page_unfix) {
                std::lock_guard<std::mutex> lock(_lruListLock);
                try {
                    _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
                } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
                _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
            }
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one if the template parameter \c on_page_unfix is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            if constexpr (on_page_unfix) {
                std::lock_guard<std::mutex> lock(_lruListLock);
                try {
                    _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
                } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
                _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
            }
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds a page reference of the buffer frame index to the page reference statistics.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            _frameReferences[b_idx] = 0;
            _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[b_idx]++ % k) + k * b_idx));
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
            _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
            _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
            _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Replaces the stored page reference of the buffer frame index which is the furthest in the past with
         *          a new one.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            try {
                _lruList.remove(static_cast<uint64_t>(((_frameReferences[idx] - 1) % k) + k * idx));
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
            _lruList.pushToBack(static_cast<uint64_t>((_frameReferences[idx]++ % k) + k * idx));
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Removes all the stored page references of the buffer frame index from the page reference statistics.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_lruListLock);
            _frameReferences[idx] = 0;
            try {
                for (size_t i; i < k; i++) {
                    _lruList.remove(static_cast<uint64_t>((i % k) + k * idx));
                }
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<uint64_t, 0>& ex) {}
        };

    private:
        /*!\var     _lruList
         * \brief   At most _k_ page references per buffer frame index (most recent in the back)
         */
        zero::hashtable_dequeu::HashtableDeque<uint64_t, 0> _lruList;

        /*!\var     _frameReferences
         * \brief   Last used page reference numbers per buffer frame index
         */
        std::vector<size_t>                                 _frameReferences;

        /*!\var     _lruListLock
         * \brief   Latch protecting the \link _lruList \endlink and \link _frameReferences \endlink from concurrent
         *          manipulations
         */
        std::mutex                                          _lruListLock;

    };

    /*!\class   PageEvictionerSelectorQuasiMRU
     * \brief   _MRU_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _MRU_
     *          policy. The _MRU_ policy selects the buffer frame that was not used for the shortest time. But due to
     *          the fact that currently fixed or dirty pages or pages containing swizzled pointers cannot (always) be
     *          evicted until their state changes w.r.t. that, this buffer frame selector does not implement a perfect
     *          _MRU_ policy. This selector does manage a stack and an additional queue---the stack for the frames which
     *          were not found not evictable since last time a page was loaded into them (\link _mruList \endlink) and
     *          the queue for pages which could not be evicted the last time they were selected by this
     *          (\link _retryList \endlink). A thread evicting alternately selects buffer frames from the stack and the
     *          queue. The template parameters of this selector specify the number of buffer frames to select before
     *          changing the list.
     *
     * @tparam retry_list_check_ppm The fraction (in PPM) of the \link _retryList \endlink queue selected by a thread
     *                              before selecting buffer frames from the \link _mruList \endlink.
     * @tparam mru_list_check_ppm   The fraction (in PPM) of the \link _mruList \endlink queue selected by a thread
     *                              before selecting buffer frames from the \link _retryList \endlink.
     *
     * \note See the function specifications for more details about the implementation.
     */
    template <uint32_t retry_list_check_ppm/* = 1000000*/, uint32_t mru_list_check_ppm/* = 10000*/>
    class PageEvictionerSelectorQuasiMRU : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorQuasiMRU(const BufferPool* bufferPool)
         * \brief   Constructs a _MRU_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _MRU_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorQuasiMRU(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _mruList(bufferPool->getBlockCount()) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details If a thread is currently selecting pages from the \link _mruList \endlink, if it should continue
         *          with that according to the template parameter \c mru_list_check_ppm and if the
         *          \link _mruList \endlink is not empty, this selects the front of the \link _mruList \endlink.
         *          Otherwise, it selects the front of the \link _retryList \endlink.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            static thread_local bool currentlyCheckingRetryList;
            static thread_local size_t retriedBufferIndexes;

            bf_idx selected;
            while (true) {
                if (currentlyCheckingRetryList) {       // This thread checked the front of the _retryList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(retry_list_check_ppm * 0.000001 * _retryList.length())
                        || _mruList.length() == 0) {    // Check again the front of the _retryList:
                        _retryListLock.lock();
                        _retryList.popFromFront(selected);
                        _retryListLock.unlock();
                        retriedBufferIndexes++;
                        return selected;
                    } else {                            // Change to checking entries from the front of the _initialList
                        _mruListLock.lock();
                        _mruList.popFromFront(selected);
                        _mruListLock.unlock();
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = false;
                        return selected;
                    }
                } else {                                // This thread checked the front of the _initialList last:
                    if (retriedBufferIndexes < static_cast<bf_idx>(mru_list_check_ppm * 0.000001 * _mruList.length())
                        || _retryList.length() == 0) {  // Check again the front of the _initialList:
                        _mruListLock.lock();
                        _mruList.popFromFront(selected);
                        _mruListLock.unlock();
                        retriedBufferIndexes++;
                        return selected;
                    } else {                            // Change to checking entries from the front of the _retryList
                        _retryListLock.lock();
                        _retryList.popFromFront(selected);
                        _retryListLock.unlock();
                        retriedBufferIndexes = 0;
                        currentlyCheckingRetryList = true;
                        return selected;
                    }
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Recently fixed buffer frames are usually not evictable and therefore the respective buffer frame
         *          index is only moved to the top of the \link _mruList \endlink when it is unfixed.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Moves the buffer frame index from some place within the stack or queue to the top of the stack.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
            _mruList.pushToFront(idx);
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Adds the buffer frame index to the top of the stack.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            _mruList.pushToFront(idx);
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Moves the buffer frame index from some place within the stack or the queue to the back of the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
            _retryList.pushToBack(idx);
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Moves the buffer frame index from some place within the stack or the queue to the back of the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
            _retryList.pushToBack(idx);
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Moves the buffer frame index from some place within the stack or the queue to the back of the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
            _retryList.pushToBack(idx);
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Moves the buffer frame index from some place within the stack or the queue to the back of the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
            _retryList.pushToBack(idx);
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Removes the buffer frame index from the stack or the queue.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            std::lock_guard<std::mutex> lock(_mruListLock);
            try {
                _mruList.remove(idx);
            } catch (const zero::hashtable_dequeu::HashtableDequeNotContainedException<bf_idx, 0>& ex) {
                _retryList.remove(idx);
            }
        };

    private:
        /*!\var     _mruList
         * \brief   Stack of recently referenced buffer frame indexes (most recent at the top)
         * \details Stack of the recently referenced buffer frame indexes without those already found not evictable
         *          during eviction.
         */
        zero::hashtable_dequeu::HashtableDeque<bf_idx, 0>   _mruList;

        /*!\var     _mruListLock
         * \brief   Latch protecting the \link _mruList \endlink from concurrent manipulations
         */
        std::mutex                                          _mruListLock;

        /*!\var     _retryList
         * \brief   Queue of buffer frames currently last selected for eviction
         * \details Contains the buffer frame indexes of currently used buffer frames which were already found not
         *          evictable during eviction. The front of the queue is the buffer frame index that was the last found
         *          to be not evictable during eviction.
         */
        zero::hashtable_dequeu::HashtableDeque<bf_idx, 0>   _retryList;

        /*!\var     _retryListLock
         * \brief   Latch protecting the \link _retryList \endlink from concurrent manipulations
         */
        std::mutex                                          _retryListLock;

    };

    /*!\class   PageEvictionerSelectorTimestampLRU
     * \brief   _LRU_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LRU_
     *          policy. The _LRU_ policy selects the buffer frame that was not used for the longest time. This is
     *          implemented using timestamps and (cached) sorting.
     *
     * @tparam resort_threshold_ppm The fraction (in PPM) of buffer frames checked in the currently used sorted list of
     *                              buffer frames before one thread starts the process of resorting the other list based
     *                              on the most recent timestamps of page references.
     */
    template <bf_idx resort_threshold_ppm/* = 750000*/>
    class PageEvictionerSelectorTimestampLRU : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorTimestampLRU(const BufferPool* bufferPool)
         * \brief   Constructs a _LRU_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LRU_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorTimestampLRU(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _timestampsLive(bufferPool->getBlockCount()),
                _lruList0(bufferPool->getBlockCount()),
                _lruList1(bufferPool->getBlockCount()),
                _lastChecked(0),
                _useLRUList0(false),
                _useLRUList1(false) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects the buffer frame from the buffer pool that was not used for the longest time. This is done
         *          using one of the lists of buffer frames sorted by last page reference (\link _lruList0 \endlink or
         *          \link _lruList1 \endlink). If \c resort_threshold_ppm of the buffer frames of the currently used
         *          list were already checked and if no other thread currently re-sorts the list which is currently not
         *          used based on the most recent timestamps of page references (from \link _timestampsLive \endlink),
         *          this thread sorts the buffer frames according to the timestamps from \link _timestampsLive \endlink
         *          into the list which is currently not used. If the currently sorted list was completely checked and
         *          if the sorting is currently in progress, this thread waits for the new list to be sorted.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            while (true) {
                if (_useLRUList0) {
                    if (_lastChecked > static_cast<bf_idx>(resort_threshold_ppm * 0.000001 * smlevel_0::bf->getBlockCount())
                     && !_sortingInProgress.test_and_set()) {
                        _waitForSorted.lock();
                        sort(_lruList1);
                        _useLRUList0 = true;
                        _lastChecked = 0;
                        _waitForSorted.unlock();
                        _useLRUList1 = false;
                        _sortingInProgress.clear();
                        continue;
                    } else {
                        bf_idx checkThis = ++_lastChecked;
                        if (checkThis > _maxBufferpoolIndex) {
                            _waitForSorted.lock();
                            _waitForSorted.unlock();
                            continue;
                        } else {
                            if (std::get<1>(_lruList0[checkThis]) == _timestampsLive[std::get<0>(_lruList0[checkThis])]) {
                                return std::get<0>(_lruList0[checkThis]);
                            } else {
                                continue;
                            }
                        }
                    }
                } else if (_useLRUList1) {
                    if (_lastChecked > static_cast<bf_idx>(resort_threshold_ppm * 0.000001 * smlevel_0::bf->getBlockCount())
                     && !_sortingInProgress.test_and_set()) {
                        _waitForSorted.lock();
                        sort(_lruList0);
                        _useLRUList1 = true;
                        _lastChecked = 0;
                        _waitForSorted.unlock();
                        _useLRUList0 = false;
                        _sortingInProgress.clear();
                        continue;
                    } else {
                        bf_idx checkThis = ++_lastChecked;
                        if (checkThis > _maxBufferpoolIndex) {
                            _waitForSorted.lock();
                            _waitForSorted.unlock();
                            continue;
                        } else {
                            if (std::get<1>(_lruList1[checkThis]) == _timestampsLive[std::get<0>(_lruList1[checkThis])]) {
                                return std::get<0>(_lruList1[checkThis]);
                            } else {
                                continue;
                            }
                        }
                    }
                } else if (!_sortingInProgress.test_and_set()) {
                    _waitForSorted.lock();
                    sort(_lruList0);
                    _useLRUList0 = true;
                    _lastChecked = 0;
                    _waitForSorted.unlock();
                    _useLRUList1 = false;
                    _sortingInProgress.clear();
                } else {

                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets the timestamp of the last page reference of the respective buffer frame to the highest possible
         *          time to prevent further unsuccessful evictions until this buffer frame is reused.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _timestampsLive[idx] = std::chrono::high_resolution_clock::duration::max().count();
        };

    private:
        /*!\var     _timestampsLive
         * \brief   Most recent page reference timestamps for the buffer frames
         * \details This array contains for each buffer frame the timestamp of the most recent page reference of the
         *          contained page. The index of the page reference timestamps corresponding to buffer frame \c n is
         *          \c n .
         */
        std::vector<std::atomic<std::chrono::high_resolution_clock::duration::rep>>                     _timestampsLive;

        /*!\var     _lruList0
         * \brief   List of buffer frame indexes sorted by page reference recency
         * \details If \link _useLRUList0 \endlink is set, this contains a list of buffer frames sorted according to the
         *          timestamps of most recent references at the time when this list was created. Each entry also
         *          contains the timestamp that was used for sorting.
         */
        std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>              _lruList0;

        /*!\var     _lruList1
         * \brief   List of buffer frame indexes sorted by page reference recency
         * \details If \link _useLRUList1 \endlink is set, this contains a list of buffer frames sorted according to the
         *          timestamps of most recent references at the time when this list was created. Each entry also
         *          contains the timestamp that was used for sorting.
         */
        std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>              _lruList1;

        /*!\var     _lastChecked
         * \brief   The last checked index of the currently sorted list of buffer frames
         */
        atomic_bf_idx                                                                                   _lastChecked;

        /*!\var     _useLRUList0
         * \brief   \link _lruList0 \endlink is most recently sorted list if set
         */
        std::atomic<bool>                                                                               _useLRUList0;

        /*!\var     _useLRUList1
         * \brief   \link _lruList1 \endlink is most recently sorted list if set
         */
        std::atomic<bool>                                                                               _useLRUList1;

        /*!\var     _sortingInProgress
         * \brief   One thread currently sorts one of the lists if set
         */
        std::atomic_flag                                                                                _sortingInProgress;

        /*!\var     _waitForSorted
         * \brief   Manages a queue for the sorting process
         * \details If the currently sorted list was completely checked and if the sorting is currently in progress,
         *          threads wait for the new list to be sorted. This is used to notify those threads.
         */
        std::mutex                                                                                      _waitForSorted;

        /*!\fn      sort(std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>& into) noexcept
         * \brief   Sorts the buffer frames according to timestamps
         * \details Sorts the buffer frame indexes according to the timestamps in \link _timestampsLive \endlink into
         *          the specified list of buffer frame indexes.
         *
         * @param into The list of buffer frame indexes to sort into.
         */
        void sort(std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>& into) noexcept {
            for (bf_idx index = 0; index < _timestampsLive.size(); index++) {
                into[index] = std::make_tuple(index, _timestampsLive[index]);
            }

            std::sort(/*std::parallel::par,*/
                    into.begin(),
                    into.end(),
                    [](const std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>& a,
                       const std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>& b) {
                        return std::get<1>(a) < std::get<1>(b);
                    });
        };

    };

    /*!\class   PageEvictionerSelectorTimestampLRUK
     * \brief   _LRU-k_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _LRU-k_
     *          policy. The _LRU-k_ policy selects the buffer frame that was not used _k_-times for the longest time.
     *          This is implemented using timestamps and (cached) sorting.
     *
     * @tparam k                    The number of last references considered per buffer frame.
     * @tparam resort_threshold_ppm The fraction (in PPM) of buffer frames checked in the currently used sorted list of
     *                              buffer frames before one thread starts the process of resorting the other list based
     *                              on the most recent timestamps of page references.
     * @tparam on_page_unfix        Count the unfix of a page as a reference instead of the fix.
     */
    template <size_t k/* = 2*/, bf_idx resort_threshold_ppm/* = 750000*/, bool on_page_unfix/* = false*/>
    class PageEvictionerSelectorTimestampLRUK : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorTimestampLRUK(const BufferPool* bufferPool)
         * \brief   Constructs a _LRU-k_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _LRU-k_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorTimestampLRUK(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _timestampsLive(bufferPool->getBlockCount(), std::make_tuple(0, std::vector<std::atomic<std::chrono::high_resolution_clock::duration::rep>>(k, std::atomic<std::chrono::high_resolution_clock::duration::rep>(0)))),
                _lruList0(bufferPool->getBlockCount()),
                _lruList1(bufferPool->getBlockCount()),
                _lastChecked(0),
                _useLRUList0(false),
                _useLRUList1(false) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects the buffer frame from the buffer pool whose _k_-th page reference is furthest in the past.
         *          This is done using one of the lists of buffer frames sorted by _k_-th last page reference
         *          (\link _lruList0 \endlink or \link _lruList1 \endlink). If \c resort_threshold_ppm of the buffer
         *          frames of the currently used list were already checked and if no other thread currently re-sorts the
         *          list which is currently not used based on the _k_ most recent timestamps of page references (from
         *          \link _timestampsLive \endlink), this thread sorts the buffer frames according to the timestamps
         *          from \link _timestampsLive \endlink into the list which is currently not used. If the currently
         *          sorted list was completely checked and if the sorting is currently in progress, this thread waits for the new list to be sorted.
         *
         * @return The selected buffer frame.
         */
        bf_idx select() noexcept final {
            while (true) {
                if (_useLRUList0) {
                    if (_lastChecked
                      > static_cast<bf_idx>(resort_threshold_ppm * 0.000001 * smlevel_0::bf->getBlockCount())
                     && !_sortingInProgress.test_and_set()) {
                        _waitForSorted.lock();
                        sort(_lruList1);
                        _useLRUList0 = true;
                        _lastChecked = 0;
                        _waitForSorted.unlock();
                        _useLRUList1 = false;
                        _sortingInProgress.clear();
                        continue;
                    } else {
                        bf_idx checkThis = ++_lastChecked;
                        if (checkThis > _maxBufferpoolIndex) {
                            _waitForSorted.lock();
                            _waitForSorted.unlock();
                            continue;
                        } else {
                            if (std::get<1>(_lruList0[checkThis])
                             == std::get<1>(_timestampsLive[checkThis])[std::get<0>(_timestampsLive[checkThis]) % k]) {
                                return std::get<0>(_lruList0[checkThis]);
                            } else {
                                continue;
                            }
                        }
                    }
                } else if (_useLRUList1) {
                    if (_lastChecked
                      > static_cast<bf_idx>(resort_threshold_ppm * 0.000001 * smlevel_0::bf->getBlockCount())
                     && !_sortingInProgress.test_and_set()) {
                        _waitForSorted.lock();
                        sort(_lruList0);
                        _useLRUList1 = true;
                        _lastChecked = 0;
                        _waitForSorted.unlock();
                        _useLRUList0 = false;
                        _sortingInProgress.clear();
                        continue;
                    } else {
                        bf_idx checkThis = ++_lastChecked;
                        if (checkThis > _maxBufferpoolIndex) {
                            _waitForSorted.lock();
                            _waitForSorted.unlock();
                            continue;
                        } else {
                            if (std::get<1>(_lruList1[checkThis])
                             == std::get<1>(_timestampsLive[checkThis])[std::get<0>(_timestampsLive[checkThis]) % k]) {
                                return std::get<0>(_lruList1[checkThis]);
                            } else {
                                continue;
                            }
                        }
                    }
                } else if (!_sortingInProgress.test_and_set()) {
                    _waitForSorted.lock();
                    sort(_lruList0);
                    _useLRUList0 = true;
                    _lastChecked = 0;
                    _waitForSorted.unlock();
                    _useLRUList1 = false;
                    _sortingInProgress.clear();
                } else {

                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time if the template
         *          parameter \c on_page_unfix is not set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            if constexpr (!on_page_unfix) {
                std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                        = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            }
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time if the template
         *          parameter \c on_page_unfix is set.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            if constexpr (on_page_unfix) {
                std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                        = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            }
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Sets all timestamps in the second component of \link _timestampsLive \endlink to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx b_idx, PageID pid) noexcept final {
            for (size_t i = 0; i < k; i++) {
                std::get<1>(_timestampsLive[b_idx])[std::get<0>(_timestampsLive[b_idx])++ % k]
                        = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            }
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                    = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                    = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                    = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details Sets the next timestamp to update according to the first component of \link _timestampsLive \endlink
         *          in the second component of \link _timestampsLive \endlink to the current time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            std::get<1>(_timestampsLive[idx])[std::get<0>(_timestampsLive[idx])++ % k]
                    = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details Sets all timestamps in the second component of \link _timestampsLive \endlink to the highest
         *          possible time.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            _timestampsLive[idx]
                    = std::make_tuple(0, std::vector<std::atomic<std::chrono::high_resolution_clock::duration::rep>>(k, std::atomic<std::chrono::high_resolution_clock::duration::rep>(std::chrono::high_resolution_clock::duration::max().count())));
        };

    private:
        /*!\var     _timestampsLive
         * \brief   _k_ most recent page reference timestamps for the buffer frames
         * \details This array contains for each buffer frame the pointer to the oldest timestamp in the first component
         *          and the _k_ page reference timestamps of the contained page. The index of the page reference
         *          timestamps corresponding to buffer frame \c n is \c n .
         */
        std::vector<std::tuple<size_t, std::vector<std::atomic<std::chrono::high_resolution_clock::duration::rep>>>>    _timestampsLive;

        /*!\var     _lruList0
         * \brief   List of buffer frame indexes sorted by page reference recency
         * \details If \link _useLRUList0 \endlink is set, this contains a list of buffer frames sorted according to the
         *          _k_-th timestamps of furthest in the past at the time when this list was created. Each entry also
         *          contains the timestamp that was used for sorting.
         */
        std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>                              _lruList0;

        /*!\var     _lruList1
         * \brief   List of buffer frame indexes sorted by page reference recency
         * \details If \link _useLRUList1 \endlink is set, this contains a list of buffer frames sorted according to the
         *          _k_-th timestamps of furthest in the past at the time when this list was created. Each entry also
         *          contains the timestamp that was used for sorting.
         */
        std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>                              _lruList1;

        /*!\var     _lastChecked
         * \brief   The last checked index of the currently sorted list of buffer frames
         */
        atomic_bf_idx                                                                                                   _lastChecked;

        /*!\var     _useLRUList0
         * \brief   \link _lruList0 \endlink is most recently sorted list if set
         */
        std::atomic<bool>                                                                                               _useLRUList0;

        /*!\var     _useLRUList1
         * \brief   \link _lruList1 \endlink is most recently sorted list if set
         */
        std::atomic<bool>                                                                                               _useLRUList1;

        /*!\var     _sortingInProgress
         * \brief   One thread currently sorts one of the lists if set
         */
        std::atomic_flag                                                                                                _sortingInProgress;

        /*!\var     _waitForSorted
         * \brief   Manages a queue for the sorting process
         * \details If the currently sorted list was completely checked and if the sorting is currently in progress,
         *          threads wait for the new list to be sorted. This is used to notify those threads.
         */
        std::mutex                                                                                                      _waitForSorted;

        /*!\fn      sort(std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>& into) noexcept
         * \brief   Sorts the buffer frames according to timestamps
         * \details Sorts the buffer frame indexes according to the most recent timestamps (defined in the first
         *          component of \link _timestampsLive \endlink) in the second component \link _timestampsLive \endlink
         *          into the specified list of buffer frame indexes.
         *
         * @param into The list of buffer frame indexes to sort into.
         */
        void sort(std::vector<std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>>& into) noexcept {
            for (bf_idx index = 0; index < _timestampsLive.size(); index++) {
                into[index] = std::make_tuple(index, std::get<1>(_timestampsLive[index])[std::get<0>(_timestampsLive[index]) % k]);
            }

            std::sort(/*std::parallel::par,*/
                    into.begin(),
                    into.end(),
                    [](const std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>& a,
                       const std::tuple<bf_idx, std::chrono::high_resolution_clock::duration::rep>& b) {
                        return std::get<1>(a) < std::get<1>(b);
                    });
        };

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_SELECTOR_HPP
