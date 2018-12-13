#ifndef __PAGE_EVICTIONER_SELECTOR_HPP
#define __PAGE_EVICTIONER_SELECTOR_HPP

#include "page_evictioner.hpp"

#include <mutex>
#include <random>

namespace zero::buffer_pool {

    /*!
     *
     */
    class PageEvictionerSelector {
    public:
        PageEvictionerSelector(BufferPool* bufferPool, const sm_options& options);

        virtual                               ~PageEvictionerSelector();

        virtual bf_idx select() = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        virtual void updateOnPageHit(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        virtual void updateOnPageUnfix(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        virtual void updateOnPageMiss(bf_idx b_idx, PageID pid) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        virtual void updateOnPageFixed(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        virtual void updateOnPageDirty(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        virtual void updateOnPageBlocked(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        virtual void updateOnPageSwizzled(bf_idx idx) = 0;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        virtual void updateOnPageExplicitlyUnbuffered(bf_idx idx) = 0;

    protected:
        bf_idx _max_bufferpool_index;

    };

    class PageEvictionerSelectorLOOPAbsolutelyAccurate : public PageEvictionerSelector {
    public:
        PageEvictionerSelectorLOOPAbsolutelyAccurate(BufferPool* bufferPool, const sm_options& options);

        bf_idx select() final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        volatile bf_idx _current_frame;

        std::mutex _current_frame_lock;

    };

    class PageEvictionerSelectorLOOPPracticallyAccurate : public PageEvictionerSelector {
    public:
        PageEvictionerSelectorLOOPPracticallyAccurate(BufferPool* bufferPool, const sm_options& options);

        bf_idx select() final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

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

    class PageEvictionerSelectorRANDOMDefault : public PageEvictionerSelector {
    public:
        PageEvictionerSelectorRANDOMDefault(BufferPool* bufferPool, const sm_options& options);

        bf_idx select() final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        std::default_random_engine _standard_random_engine;

        std::uniform_int_distribution<uint_fast32_t> _random_distribution;

    };

    class PageEvictionerSelectorRANDOMFastRand : public PageEvictionerSelector {
    public:
        PageEvictionerSelectorRANDOMFastRand(BufferPool* bufferPool, const sm_options& options);

        bf_idx select() final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
        void updateOnPageHit(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
        void updateOnPageUnfix(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
        void updateOnPageMiss(bf_idx b_idx, PageID pid) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
        void updateOnPageFixed(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
        void updateOnPageDirty(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
        void updateOnPageBlocked(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
        void updateOnPageSwizzled(bf_idx idx) final;

        //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    private:
        static thread_local bool _random_state_initialized;

        static thread_local uint_fast32_t _random_state;

    };

} // zero::buffer_pool

#endif // __PAGE_EVICTIONER_SELECTOR_HPP
