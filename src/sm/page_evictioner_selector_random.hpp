#ifndef __PAGE_EVICTIONER_SELECTOR_RANDOM_HPP
#define __PAGE_EVICTIONER_SELECTOR_RANDOM_HPP

#include "page_evictioner_selector.hpp"

#include <random>
#include <cstdlib>
#include <ctime>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/additive_combine.hpp>
#include <boost/random/shuffle_order.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/inversive_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/ranlux.hpp>

namespace zero::buffer_pool {

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
        explicit PageEvictionerSelectorRANDOMDefault(const BufferPool *bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the default random number generator of the C++ Standard
         *          Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_standardRandomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _standardRandomEngine
         * \brief   The used pseudo-random number generator
         */
        std::default_random_engine _standardRandomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<bf_idx> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMFastRand
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
        explicit PageEvictionerSelectorRANDOMFastRand(const BufferPool *bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a very fast linear congruential generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

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
            static thread_local bf_idx _randomState;

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
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMMinstdRand0
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::minstd_rand0 random number generator of the C++ Standard Library over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMMinstdRand0 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMMinstdRand0(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMMinstdRand0(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::minstd_rand0 random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::minstd_rand0 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMMinstdRand
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::minstd_rand random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMMinstdRand : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMMinstdRand(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMMinstdRand(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::minstd_rand random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::minstd_rand _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMMT19937
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::mt19937 random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMMT19937 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMMT19937(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMMT19937(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::mt19937 random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::mt19937 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMMT19937_64
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::mt19937_64 random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMMT19937_64 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMMT19937_64(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMMT19937_64(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::mt19937_64 random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::mt19937_64 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMRanlux24Base
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::ranlux24_base random number generator of the C++ Standard Library over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMRanlux24Base : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMRanlux24Base(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMRanlux24Base(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::ranlux24_base random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::ranlux24_base _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMRanlux48Base
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::ranlux48_base random number generator of the C++ Standard Library over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMRanlux48Base : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMRanlux48Base(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMRanlux48Base(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::ranlux48_base random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::ranlux48_base _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMRanlux24
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::ranlux24 random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMRanlux24 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMRanlux24(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMRanlux24(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::ranlux24 random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::ranlux24 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMRanlux48
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::ranlux48 random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMRanlux48 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMRanlux48(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMRanlux48(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::ranlux48 random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::ranlux48 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMKnuthB
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::knuth_b random number generator of the C++ Standard Library over a uniform distribution.
     */
    class PageEvictionerSelectorRANDOMKnuthB : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMKnuthB(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMKnuthB(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::knuth_b random number generator of the C++
         *          Standard Library over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        std::knuth_b _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        std::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMCRand
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c std::rand random number generator of the C Standard Library.
     */
    class PageEvictionerSelectorRANDOMCRand : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMCRand(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMCRand(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {
            std::srand(std::time(nullptr));
        };

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c std::rand random number generator of the C Standard
         *          Library.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return 1 + std::rand() / ((RAND_MAX + 1u) / _maxBufferpoolIndex);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostMinstdRand0
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::minstd_rand0 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostMinstdRand0 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostMinstdRand0(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostMinstdRand0(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::minstd_rand0 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::minstd_rand0 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostMinstdRand
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::minstd random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostMinstdRand : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostMinstdRand(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostMinstdRand(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::minstd random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::minstd_rand _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRand48
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::rand48 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRand48 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRand48(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRand48(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::rand48 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::rand48 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostEcuyer1988
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ecuyer1988 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostEcuyer1988 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostEcuyer1988(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostEcuyer1988(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ecuyer1988 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ecuyer1988 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostKnuthB
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::knuth_b random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostKnuthB : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostKnuthB(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostKnuthB(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::knuth_b random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::knuth_b _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostKreutzer1986
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::kreutzer1986 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostKreutzer1986 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostKreutzer1986(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostKreutzer1986(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::kreutzer1986 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::kreutzer1986 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostTaus88
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::taus88 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostTaus88 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostTaus88(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostTaus88(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::taus88 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::taus88 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostHellekalek1995
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::hellekalek1995 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostHellekalek1995 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostHellekalek1995(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostHellekalek1995(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::hellekalek1995 random number generator
         *          of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::hellekalek1995 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostMT11213b
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::mt11213b random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostMT11213b : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostMT11213b(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostMT11213b(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::mt11213b random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::mt11213b _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostMT19937
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::mt19937 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostMT19937 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostMT19937(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostMT19937(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::mt19937 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::mt19937 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostMT19937_64
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::mt19937_64 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostMT19937_64 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostMT19937_64(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostMT19937_64(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::mt19937_64 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::mt19937_64 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci607
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci607 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci607 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci607(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci607(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci607 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci607 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci1279 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci1279 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci1279 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci2281 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci2281 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci2281 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci3217 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci3217 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci3217 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci4423 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci4423 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci4423 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci9689 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci9689 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci9689 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci19937 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci19937 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci19937 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci23209 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci23209 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci23209 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::lagged_fibonacci44497 random number generator of the Boost C++ Libraries over a
     *          uniform distribution.
     */
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::lagged_fibonacci44497 random number
         *          generator of the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::lagged_fibonacci44497 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux3
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux3 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux3 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux3(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux3(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux3 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux3 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux4
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux4 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux4 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux4(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux4(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux4 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux4 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux64_3
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux64_3 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux64_3 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux64_3(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux64_3(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux64_3 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux64_3 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux64_4
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux64_4 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux64_4 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux64_4(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux64_4(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux64_4 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux64_4 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux3_01
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux3_01 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux3_01 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux3_01(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux3_01(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux3_01 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux3_01 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux4_01
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux4_01 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux4_01 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux4_01(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux4_01(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux4_01 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux64_3_01 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux64_4_01
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux64_4_01 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux64_4_01 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux64_4_01(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux64_4_01(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux64_4_01 random number generator of
         *          the Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux64_4_01 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux24
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux24 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux24 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux24(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux24(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux24 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux24 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMBoostRanlux48
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          the \c boost::random::ranlux48 random number generator of the Boost C++ Libraries over a uniform
     *          distribution.
     */
    class PageEvictionerSelectorRANDOMBoostRanlux48 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMBoostRanlux48(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMBoostRanlux48(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool),
                _randomDistribution(1, _maxBufferpoolIndex) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using the \c boost::random::ranlux48 random number generator of the
         *          Boost C++ Libraries over a uniform distribution.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {
            return _randomDistribution(_randomEngine);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    private:
        /*!\var     _randomEngine
         * \brief   The used pseudo-random number generator
         */
        boost::random::ranlux48 _randomEngine;

        /*!\var     _randomDistribution
         * \brief   The uniform distribution and range for the pseudo-random number generator (post-processor)
         */
        boost::random::uniform_int_distribution<uint_fast32_t> _randomDistribution;

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift32
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift pseudo-random number generator and a 32-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift32 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift32(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        PageEvictionerSelectorRANDOMXORShift32(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed;

            if (!_seedInitialized) {
                _seed = std::random_device{}();
                _seedInitialized = true;
            }
            _seed ^= _seed << 13;
            _seed ^= _seed >> 17;
            _seed ^= _seed << 5;
            return (_seed % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift64
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift pseudo-random number generator and a 64-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift64 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift64(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORShift64(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed
             * \brief   The state of the Xorshift
             */
            static thread_local uint64_t _seed;

            if (!_seedInitialized) {
                _seed = std::random_device{}();
                _seedInitialized = true;
            }
            _seed ^= _seed << 13;
            _seed ^= _seed >> 7;
            _seed ^= _seed << 17;
            return (_seed % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift96
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift pseudo-random number generator and a 96-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift96 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift96(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORShift96(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed0
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed0;

            /*!\var     _seed1
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed1;

            /*!\var     _seed2
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed2;

            if (!_seedInitialized) {
                _seed0 = std::random_device{}();
                _seed1 = std::random_device{}();
                _seed2 = std::random_device{}();
                _seedInitialized = true;
            }
            uint32_t t;
            _seed0 ^= _seed0 << 16;
            _seed0 ^= _seed0 >> 5;
            _seed0 ^= _seed0 << 1;

            t = _seed0;
            _seed0 = _seed1;
            _seed1 = _seed2;
            _seed2 = t ^ _seed0 ^ _seed1;

            return (_seed2 % (_maxBufferpoolIndex - 1)) + 1;
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift128
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift pseudo-random number generator and a 128-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift128 : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift128(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORShift128(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed0
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed0;

            /*!\var     _seed1
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed1;

            /*!\var     _seed2
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed2;

            /*!\var     _seed3
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _seed3;

            if (!_seedInitialized) {
                _seed0 = std::random_device{}();
                _seed1 = std::random_device{}();
                _seed2 = std::random_device{}();
                _seed3 = std::random_device{}();
                _seedInitialized = true;
            }
            uint32_t t = _seed0 ^ (_seed0 << 11);
            _seed0 = _seed1;
            _seed1 = _seed2;
            _seed2 = _seed3;

            _seed3 ^= (_seed3 >> 19) ^ t ^ (t >> 8);

            return (_seed3 % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORWow
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorwow pseudo-random number generator and a 192-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORWow : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORWow(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORWow(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorwow pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _x
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _x;

            /*!\var     _y
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _y;

            /*!\var     _z
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _z;

            /*!\var     _w
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _w;

            /*!\var     _v
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _v;

            /*!\var     _d
             * \brief   The state of the Xorshift
             */
            static thread_local uint32_t _d;

            if (!_seedInitialized) {
                _x = std::random_device{}();
                _y = std::random_device{}();
                _z = std::random_device{}();
                _w = std::random_device{}();
                _v = std::random_device{}();
                _d = std::random_device{}();
                _seedInitialized = true;
            }
            uint32_t t = _x ^ (_x >> 2);
            _x = _y;
            _y = _z;
            _z = _w;
            _w = _v;
            _v = (_v ^ (_v << 4)) ^ (t ^ (t << 1));

            return (((_d += 362437) + _v) % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift64Star
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift* pseudo-random number generator and a 64-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift64Star : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift64Star(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORShift64Star(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift* pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed
             * \brief   The state of the Xorshift
             */
            static thread_local uint64_t _seed;

            if (!_seedInitialized) {
                _seed = std::random_device{}();
                _seedInitialized = true;
            }
            uint64_t x = _seed;
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;
            _seed = x;
            return ((x * 0x2545F4914F6CDD1D) % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

    /*!\class   PageEvictionerSelectorRANDOMXORShift128Plus
     * \brief   _RANDOM_ buffer frame selector
     * \details This is a buffer frame selector for the _Select-and-Filter_ page evictioner that implements the _RANDOM_
     *          policy. The _RANDOM_ policy selects buffer frames randomly. This version of the _RANDOM_ policy uses
     *          uses a Xorshift+ pseudo-random number generator and a 128-bit thread-local state.
     */
    class PageEvictionerSelectorRANDOMXORShift128Plus : public PageEvictionerSelector {
    public:
        /*!\fn      PageEvictionerSelectorRANDOMXORShift128Plus(const BufferPool* bufferPool)
         * \brief   Constructs a _RANDOM_ buffer frame selector
         *
         * @param bufferPool The buffer pool this _RANDOM_ buffer frame selector is responsible for.
         */
        explicit PageEvictionerSelectorRANDOMXORShift128Plus(const BufferPool* bufferPool) :
                PageEvictionerSelector(bufferPool) {};

        /*!\fn      select() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details Selects a buffer frame randomly using a Xorshift+ pseudo-random number generator.
         *
         * @return The selected buffer frame.
         */
        inline bf_idx select() noexcept final {

            /*!\var     _seedInitialized
             * \brief   Whether the state of the Xorshift is initialized on this thread
             * \details The first time \link select() \endlink runs on a particular thread, the state of the Xorshift
             *          needs to be initialized. If this is set on a thread, the state is already initialized on this
             *          thread.
             */
            static thread_local bool _seedInitialized;

            /*!\var     _seed0
             * \brief   The state of the Xorshift
             */
            static thread_local uint64_t _seed0;

            /*!\var     _seed1
             * \brief   The state of the Xorshift
             */
            static thread_local uint64_t _seed1;

            if (!_seedInitialized) {
                _seed0 = std::random_device{}();
                _seed1 = std::random_device{}();
                _seedInitialized = true;
            }
            uint64_t x = _seed0;
            uint64_t const y = _seed1;
            _seed0 = y;
            x ^= x << 23;
            _seed1 = x ^ y ^ (x >> 17) ^ (y >> 26);
            return ((_seed1 + y) % (_maxBufferpoolIndex - 1) + 1);
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        inline void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        inline void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        inline void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {};

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        inline void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        inline void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        inline void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        inline void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details This buffer frame selector does not require any statistics and therefore this function does nothing.
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        inline void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {};

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this buffer frame selector
         * \details This buffer frame selector does not use locking and therefore this function does nothing.
         */
        inline void releaseInternalLatches() noexcept final {};

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_SELECTOR_RANDOM_HPP
