#ifndef __ZERO_PAGE_EVICTIONER_LEAN_STORE_HPP
#define __ZERO_PAGE_EVICTIONER_LEAN_STORE_HPP

#include "page_evictioner.hpp"
#include "buffer_pool_pointer_swizzling.hpp"
#include "hashtable_deque.hpp"
#include <random>
#include "splitmix/splitmix.hpp"
#include "uniform_int_distribution.hpp"

namespace zero::buffer_pool {

    /*!\class   PageEvictionerLeanStore
     * \brief   TODO
     * \details TODO
     *
     * @tparam cooling_stage_size_ppm TODO
     */
    template<uint32_t cooling_stage_size_ppm/* = 75000*/>
    class PageEvictionerLeanStore : public PageEvictioner {
    public:
        /*!\fn      PageEvictionerLeanStore(const BufferPool* bufferPool)
         * \brief   TODO
         * \details TODO
         *
         * @param bufferPool TODO
         */
        PageEvictionerLeanStore(const BufferPool* bufferPool) :
                PageEvictioner(bufferPool),
                _maxBufferpoolIndex(bufferPool->getBlockCount() - 1),
                _coolingStageSize(std::ceil(bufferPool->getBlockCount() * cooling_stage_size_ppm * 0.000001)),
                _coolingStage(_coolingStageSize),
                _randomNumberGenerator(std::random_device{}()),
                _randomDistribution(1, _maxBufferpoolIndex),
                _notEvictable(bufferPool->getBlockCount()) {
            static_assert(std::is_same_v<POINTER_SWIZZLER, SimpleSwizzling>,
                          "PageEvictionerLeanStore requires pointer swizzling in the buffer pool!");
        };

        /*!\fn      ~PageEvictionerLeanStore()
         * \brief   TODO
         */
        ~PageEvictionerLeanStore() final {};

        /*!\fn      pickVictim() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details TODO
         *
         * @return The buffer frame that can be freed or \c 0 if no eviction victim could be found.
         */
        bf_idx pickVictim() noexcept final {
            while (true) {
                if (should_exit()) {
                    return 0;
                } // the buffer index 0 has the semantics of null

                bf_idx victimIndex = 0;
                if (_coolingStage.length() * 2 < _coolingStageSize) {
                    fillCoolingStage();
                }
                try {
                    _coolingStageLock.lock();
                    _coolingStage.popFromFront(victimIndex);
                    _coolingStageLock.unlock();
                    return victimIndex;
                } catch (const hashtable_deque::HashtableDequeEmptyException<bf_idx, 0x80000001u>& ex) {
                    _coolingStageLock.unlock();
                    fillCoolingStage();
                    continue;
                }
            }
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page hit occurred.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page unfix occurred.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink on which a page miss occurred.
         * @param pid The \link PageID \endlink of the \link generic_page \endlink that was loaded into the buffer
         *             frame with index \c idx .
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            _notEvictable[idx] = false;
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of fixed (i.e. used) pages during eviction
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {
            try {
                _coolingStageLock.lock();
                _coolingStage.pushToBack(idx);
                _coolingStageLock.unlock();
            } catch (const hashtable_deque::HashtableDequeAlreadyContainsException<bf_idx, 0x80000001u>& ex) {}
        };

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a dirty page.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {
            try {
                _coolingStageLock.lock();
                _coolingStage.pushToBack(idx);
                _coolingStageLock.unlock();
            } catch (const hashtable_deque::HashtableDequeAlreadyContainsException<bf_idx, 0x80000001u>& ex) {}

            /*!\var     _dirtyCount
             * \brief   TODO
             * \details TODO
             */
            static thread_local bf_idx _dirtyCount;

            if (_dirtyCount % _coolingStageSize == 0) {
                smlevel_0::bf->wakeupPageCleaner();
            }
        };

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink which corresponding frame contains a page
         *            that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {
            _notEvictable[idx] = true;
        };

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink that was picked for eviction while the
         *            corresponding frame contained a page with swizzled pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {
            try {
                _coolingStageLock.lock();
                _coolingStage.pushToBack(idx);
                _coolingStageLock.unlock();
            } catch (const hashtable_deque::HashtableDequeAlreadyContainsException<bf_idx, 0x80000001u>& ex) {}
        };

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit unbuffer
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose corresponding frame is freed
         *            explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            try {
                _coolingStageLock.lock();
                _coolingStage.remove(idx);
                _coolingStageLock.unlock();
            } catch (const hashtable_deque::HashtableDequeNotContainedException<bf_idx, 0x80000001u>& ex) {}
            _notEvictable[idx] = true;
        };

        /*!\fn      updateOnPointerSwizzling(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages when its pointer got swizzled in its parent page
         * \details TODO
         *
         * @param idx The buffer frame index of the \link BufferPool \endlink whose pointer got swizzled in its
         *            corresponding parent page.
         */
        void updateOnPointerSwizzling(bf_idx idx) noexcept final {
            if (_coolingStage.length() > 0) {
                _coolingStageLock.lock();
                try {
                    _coolingStage.remove(idx);
                } catch (const hashtable_deque::HashtableDequeNotContainedException<bf_idx, 0x80000001u>& ex) {}
                _coolingStageLock.unlock();
            }
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of
         * \details TODO
         */
        void releaseInternalLatches() noexcept final {
            _coolingStageLock.unlock();
        };

    private:
        /*!\fn      fillCoolingStage()
         * \brief   TODO
         * \details TODO
         */
        void fillCoolingStage() {
            while (_coolingStage.length() < _coolingStageSize) {
                // Randomly select a buffer frame as a candidate for the cooling stage:
                bf_idx coolingCandidate = _randomDistribution(_randomNumberGenerator);
                // Unlock the cooling stage between iterations to give page hits a chance to move pages from the cooling
                // stage to the hot state:
                _coolingStageLock.lock();
                if (_coolingStage.length() >= _coolingStageSize) {
                    _coolingStageLock.unlock();
                    return;
                }
                // The selected buffer frame is already in the cooling stage or permanently not evictable:
                if (_coolingStage.contains(coolingCandidate) || _notEvictable[coolingCandidate]) {
                    _coolingStageLock.unlock();
                    continue;
                    // The selected buffer frame is not already in the cooling stage (it is hot):
                } else {
                    _coolingStageLock.unlock();

                    bf_tree_cb_t& coolingCandidateControlBlock = smlevel_0::bf->getControlBlock(coolingCandidate);
                    // If the pointer to the page is not swizzled or if it is currently latched, it cannot go into the
                    // cooling stage:
                    if (!coolingCandidateControlBlock._swizzled || coolingCandidateControlBlock.latch().is_latched()) {
                        continue;
                    }
                    // Acquire the candidate's latch to allow further checks:
                    rc_t coolingCandidateLatchReturnCode
                            = coolingCandidateControlBlock.latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
                    if (coolingCandidateLatchReturnCode.is_error()) {
                        continue;
                    }
                    w_assert1(coolingCandidateControlBlock.latch().is_mine());
                    // If the page in the candidate buffer frame cannot be evicted, there is no reason to put it into
                    // the cooling stage:
                    if (!smlevel_0::bf->isEvictable(coolingCandidate, _flushDirty)) {
                        coolingCandidateControlBlock.latch().latch_release();
                        continue;
                    }
                    // Find the buffer frame of the candidate's parent because the flag inside the pointer to the
                    // candidate needs to be changed there:
                    bf_idx_pair coolingCandidatePair
                            = *(smlevel_0::bf->getHashtable()->lookupPair(coolingCandidateControlBlock._pid));
                    w_assert1(coolingCandidate == coolingCandidatePair.first);
                    bf_idx coolingCandidateParent = coolingCandidatePair.second;
                    // If there is no parent page, the candidate buffer frame cannot be put into the cooling stage:
                    if (coolingCandidateParent == 0 || !smlevel_0::bf->isActiveIndex(coolingCandidateParent)) {
                        coolingCandidateControlBlock.latch().latch_release();
                        continue;
                    }
                    // Acquire the latch of the candidate's parent to allow unswizzling:
                    bf_tree_cb_t& coolingCandidateParentControlBlock
                            = smlevel_0::bf->getControlBlock(coolingCandidateParent);
                    rc_t coolingCandidateParentLatchReturnCode
                            = coolingCandidateParentControlBlock.latch().latch_acquire(LATCH_SH,
                                                                                       timeout_t::WAIT_IMMEDIATE);
                    if (coolingCandidateParentLatchReturnCode.is_error()) {
                        coolingCandidateControlBlock.latch().latch_release();
                        continue;
                    }
                    w_assert1(coolingCandidateParentControlBlock.latch().held_by_me());

                    // Get the parent page from the buffer pool and find the slot with the candidate's pointer in it:
                    w_assert1(smlevel_0::bf->isActiveIndex(coolingCandidateParent));
                    generic_page* coolingCandidateParentPage = smlevel_0::bf->getPage(coolingCandidateParent);
                    general_recordid_t coolingCandidateSlotID;
                    coolingCandidateSlotID
                            = fixable_page_h::find_page_id_slot(coolingCandidateParentPage,
                                                                POINTER_SWIZZLER::makeSwizzledPointer(
                                                                        coolingCandidate));
                    w_assert1(coolingCandidateSlotID != GeneralRecordIds::INVALID);

                    // Unswizzle the candidate's pointer inside its parent page:
                    if (smlevel_0::bf->unswizzlePagePointer(coolingCandidateParentPage, coolingCandidateSlotID)) {
                        // Add the candidate's buffer index to the cooling stage queue and release all latches:
                        _coolingStageLock.lock();
                        _coolingStage.pushToBack(coolingCandidate);
                        _coolingStageLock.unlock();
                        coolingCandidateParentControlBlock.latch().latch_release();
                        coolingCandidateControlBlock.latch().latch_release();
                    } else {
                        coolingCandidateParentControlBlock.latch().latch_release();
                        coolingCandidateControlBlock.latch().latch_release();
                        continue;
                    }
                }
            }
        };

        /*!\var     _coolingStage
         * \brief   TODO
         * \details TODO
         */
        hashtable_deque::HashtableDeque<bf_idx, 0x80000001u> _coolingStage;

        /*!\var     _coolingStageSize
         * \brief   TODO
         */
        bf_idx _coolingStageSize;

        /*!\var     _coolingStageLock
         * \brief   TODO
         */
        std::recursive_mutex _coolingStageLock;

        /*!\var     _notEvictable
         * \brief   TODO
         * \details TODO
         */
        std::vector<std::atomic<bool>> _notEvictable;

        /*!\var     _maxBufferpoolIndex
         * \brief   TODO
         */
        bf_idx _maxBufferpoolIndex;

        /*!\var     _randomNumberGenerator
         * \brief   TODO
         * \details TODO
         */
        splitmix32 _randomNumberGenerator;

        /*!\var     _randomDistribution
         * \brief   TODO
         * \details TODO
         */
        uniform_int_distribution::biased_uniform_int_distribution<bf_idx> _randomDistribution;
    };
} // zero::buffer_pool

#endif // __ZERO_PAGE_EVICTIONER_LEAN_STORE_HPP
