#ifndef __ZERO_PAGE_EVICTIONER_OTHER_HPP
#define __ZERO_PAGE_EVICTIONER_OTHER_HPP

#include "page_evictioner.hpp"
#include "buffer_pool.hpp"
#include "multi_clock.hpp"
#include "hashtable_deque.hpp"
#include <mutex>
#include <cmath>

namespace zero::buffer_pool {


    /*!\class   PageEvictionerCAR
     * \brief   Page Eviction Algorithm CAR
     * \details Page replacement algorithm CAR as presented in
     *          <A HREF="http://www-cs.stanford.edu/~sbansal/pubs/fast04.pdf">"CAR: Clock with Adaptive Replacement"</A>
     *          by Sorav Bansal and Dharmendra S. Modha.
     *
     * @tparam on_page_unfix If set, the eviction statistics are updated on page unfix instead of page hit.
     *
     * \author  Max Gilbert
     */
    template <bool on_page_unfix/* = false*/>
    class PageEvictionerCAR : public PageEvictioner {
    public:
        /*!\fn      PageEvictionerCAR(const BufferPool* bufferPool)
         * \brief   Constructor for a _CAR_ page evictioner
         * \details This instantiates a page evictioner that uses the CAR algorithm to select victims for replacement.
         *          It will serve the specified \c bufferPool .
         *
         * @param bufferPool The buffer pool the constructed page evictioner is used to select pages for eviction for.
         */
        PageEvictionerCAR(const BufferPool* bufferPool) :
                PageEvictioner(bufferPool),
                _clocks(bufferPool->getBlockCount()),
                _b1(bufferPool->getBlockCount() - 1),
                _b2(bufferPool->getBlockCount() - 1),
                _p(0),
                _c(bufferPool->getBlockCount() - 1),
                _handMovement(0) {};

        /*!\fn      ~PageEvictionerCAR()
         * \brief   Destructor for a _CAR_ page evictioner
         */
        ~PageEvictionerCAR() final {};

        /*!\fn      pickVictim() noexcept
         * \brief   Selects a page to be evicted from the buffer pool
         * \details This method uses the CAR algorithm to select one buffer frame which is expected to be used the
         *          furthest in the future (with the currently cached page).
         *
         * \post    The picked victim is latched in \link latch_mode_t::LATCH_EX \endlink mode as the buffer pool frame
         *          will be changed during eviction (page will be removed).
         *
         * @return The buffer frame that can be freed.
         */
        bf_idx pickVictim() noexcept final {
            bool evictedPage = false;
            bf_idx blockedT1 = 0;
            bf_idx blockedT2 = 0;

            while (!evictedPage) {
                if (should_exit()) return 0; // the buffer index 0 has the semantics of null

                if (_handMovement >= _c) {
                    smlevel_0::bf->getPageCleaner()->wakeup(false);
                    DBG3(<< "Run Page_Cleaner ...");
                    _handMovement = 0;
                }
                uint32_t iterations = (blockedT1 + blockedT2) / _c;
                if ((blockedT1 + blockedT2) % _c == 0 && (blockedT1 + blockedT2) > 0) {
                    DBG1(<< "Iterated " << iterations << "-times in CAR's pick_victim().");
                }
                w_assert1(iterations < 3);
                DBG3(<< "p = " << _p);
                std::lock_guard<std::mutex> guard(_latch);
                if ((_clocks.sizeOf<T_1>() >= std::max<uint32_t>(uint32_t(1), _p) || blockedT2 >= _clocks.sizeOf<T_2>()) && blockedT1 < _clocks.sizeOf<T_1>()) {
                    bool t1Head;
                    _clocks.getHead<T_1>(t1Head);
                    bf_idx t1HeadIndex;
                    _clocks.getHeadIndex<T_1>(t1HeadIndex);
                    w_assert1(t1HeadIndex != 0);

                    if (!t1Head) {
                        evictedPage = evictOne(t1HeadIndex);
                        PageID evictedPID = smlevel_0::bf->getControlBlock(t1HeadIndex)._pid;

                        if (evictedPage) {
                            _clocks.removeHead<T_1>(t1HeadIndex);
                            _b1.pushToBack(evictedPID);
                            DBG5(<< "Removed from T_1: " << t1HeadIndex << "; New size: " << _clocks.sizeOf<T_1>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                            return t1HeadIndex;
                        } else {
                            _clocks.moveHead<T_1>();
                            blockedT1++;
                            _handMovement++;
                            continue;
                        }
                    } else {
                        _clocks.setHead<T_1>(false);

                        _clocks.switchHeadToTail<T_1, T_2>(t1HeadIndex);
                        DBG5(<< "Removed from T_1: " << t1HeadIndex << "; New size: " << _clocks.sizeOf<T_1>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                        DBG5(<< "Added to T_2: " << t1HeadIndex << "; New size: " << _clocks.sizeOf<T_2>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                        continue;
                    }
                } else if (blockedT2 < _clocks.sizeOf<T_2>()) {
                    bool t2Head;
                    _clocks.getHead<T_2>(t2Head);
                    bf_idx t2HeadIndex;
                    _clocks.getHeadIndex<T_2>(t2HeadIndex);
                    w_assert1(t2HeadIndex != 0);

                    if (!t2Head) {
                        evictedPage = evictOne(t2HeadIndex);
                        PageID evictedPID = smlevel_0::bf->getControlBlock(t2HeadIndex)._pid;

                        if (evictedPage) {
                            _clocks.removeHead<T_2>(t2HeadIndex);
                            _b2.pushToBack(evictedPID);
                            DBG5(<< "Removed from T_2: " << t2HeadIndex << "; New size: " << _clocks.sizeOf<T_2>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());

                            return t2HeadIndex;
                        } else {
                            _clocks.moveHead<T_2>();
                            blockedT2++;
                            _handMovement++;
                            continue;
                        }
                    } else {
                        _clocks.setHead<T_2>(false);

                        _clocks.moveHead<T_2>();
                        _handMovement++;
                        continue;
                    }
                } else {
                    return 0;
                }
            }
            return 0;           // Suppress compiler warning about missing return statement!
        };

        /*!\fn      updateOnPageHit(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page hit
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The frame of the buffer pool that was fixed with a page hit.
         */
        void updateOnPageHit(bf_idx idx) noexcept final {
            if constexpr (!on_page_unfix) {
                _clocks.set(idx, true);
            }
        };

        /*!\fn      updateOnPageUnfix(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on page unfix
         * \details Sets the referenced bit of the specified buffer frame (inside \link _refBits \endlink) if the
         *          template parameter \c on_hit is set.
         *
         * @param idx The frame of the buffer pool that was unfixed.
         */
        void updateOnPageUnfix(bf_idx idx) noexcept final {
            if constexpr (on_page_unfix) {
                _clocks.set(idx, true);
            }
        };

        /*!\fn      updateOnPageMiss(bf_idx idx, PageID pid) noexcept
         * \brief   Updates the eviction statistics on page miss
         * \details Classifies the specified buffer frame to be in clock \f$T_1\f$ or \f$T_2\f$ based on the membership
         *          of the referenced page in either \f$B_1\f$, \f$B_2\f$ of none of the LRU-lists. It also removes
         *          entries from the LRU-lists \f$B_1\f$ or \f$B_2\f$ if needed. The referenced bit of the specified
         *          buffer frame will be unset.
         *
         * @param idx The frame of the buffer pool where the fixed page is cached in.
         * @param pid The \c PageID of the fixed page.
         */
        void updateOnPageMiss(bf_idx idx, PageID pid) noexcept final {
            std::lock_guard<std::mutex> guard(_latch);
            if (!_b1.contains(pid) && !_b2.contains(pid)) {
                if (_clocks.sizeOf<T_1>() + _b1.length() >= _c) {
                    _b1.popFromFront();
                } else if (_clocks.sizeOf<T_1>() + _clocks.sizeOf<T_2>() + _b1.length() + _b2.length() >= 2 * (_c)) {
                    _b2.popFromFront();
                }
                _clocks.addTail<T_1>(idx);
                DBG5(<< "Added to T_1: " << idx << "; New size: " << _clocks.sizeOf<T_1>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                _clocks.set(idx, false);
            } else if (_b1.contains(pid)) {
                _p = std::min(_p + std::max<uint32_t>(uint32_t(1), (_b2.length() / _b1.length())), _c);
                _b1.remove(pid);
                _clocks.addTail<T_2>(idx);
                DBG5(<< "Added to T_2: " << idx << "; New size: " << _clocks.sizeOf<T_2>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                _clocks.set(idx, false);
            } else {
                _p = std::max<int32_t>(int32_t(_p) - std::max<int32_t>(1, (_b1.length() / _b2.length())), 0);
                _b2.remove(pid);
                _clocks.addTail<T_2>(idx);
                DBG5(<< "Added to T_2: " << idx << "; New size: " << _clocks.sizeOf<T_2>() << "; Free frames: " << smlevel_0::bf->getFreeList()->getCount());
                _clocks.set(idx, false);
            }
            w_assert1(0 <= _clocks.sizeOf<T_1>() + _clocks.sizeOf<T_2>() && _clocks.sizeOf<T_1>() + _clocks.sizeOf<T_2>() <= _c);
            w_assert1(0 <= _clocks.sizeOf<T_1>() + _b1.length() && _clocks.sizeOf<T_1>() + _b1.length() <= _c);
            w_assert1(0 <= _clocks.sizeOf<T_2>() + _b2.length() && _clocks.sizeOf<T_2>() + _b2.length() <= 2 * (_c));
            w_assert1(0 <= _clocks.sizeOf<T_1>() + _clocks.sizeOf<T_2>() + _b1.length() + _b2.length() && _clocks.sizeOf<T_1>() + _clocks.sizeOf<T_2>() + _b1.length() + _b2.length() <= 2 * (_c));
        };

        /*!\fn      updateOnPageFixed(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of used pages during eviction
         * \details As CAR logs page fixes in specific time intervals, a page fixed for a longer timespan must not set
         *          the corresponding referenced bit as this would be recognized as repeated usage and therefore the
         *          page would be promoted to \f$T_2\f$.
         *
         * @param idx The frame of the buffer pool that was picked for eviction while it was fixed.
         */
        void updateOnPageFixed(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageDirty(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of dirty pages during eviction
         * \details As a dirty page shouldn't be picked for eviction until it is cleaned, it should be excluded from the
         *          eviction to increase the performance of the eviction but that is not implemented yet.
         *
         * @param idx The frame of the buffer pool that was picked for eviction while the contained page is dirty.
         */
        void updateOnPageDirty(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageBlocked(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages that cannot be evicted at all
         * \details As some pages are not allowed to be evicted at all (will never be allowed), those should be excluded
         *          from the eviction but that is not implemented yet.
         *
         * @param idx The frame of the buffer pool that contains a page that cannot be evicted at all.
         */
        void updateOnPageBlocked(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageSwizzled(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
         * \details As a page containing swizzled pointers shouldn't be picked for eviction until the pointers are
         *          unswizzled, it should be excluded from the eviction to increase the performance of the eviction but
         *          that is not implemented yet.
         *
         * @param idx The frame of the buffer pool that was picked for eviction while containing a page with swizzled
         *            pointers.
         */
        void updateOnPageSwizzled(bf_idx idx) noexcept final {};

        /*!\fn      updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept
         * \brief   Updates the eviction statistics on explicit eviction
         * \details When a page is evicted explicitly, the corresponding buffer frame index is removed from the clock
         *          \f$T_1\f$ or \f$T_2\f$.
         *
         * @param idx The frame of the buffer pool that is freed explicitly.
         */
        void updateOnPageExplicitlyUnbuffered(bf_idx idx) noexcept final {
            try {
                _clocks.remove(idx);
            } catch (const multi_clock::MultiHandedClockNotContainedException<bf_idx, bool, 2, 0>& ex) {}
        };

        /*!\fn      releaseInternalLatches() noexcept
         * \brief   Releases the internal latches of this page evictioner
         * \details Releases the latch acquired and released in \link select() \endlink and
         *          \link updateOnPageMiss() \endlink.
         *
         * \pre     The \link _latch \endlink might be acquired by this thread.
         * \post    The \link _latch \endlink is not acquired by this thread.
         *
         * \warning The behavior of this method is undefined if the implementation of \c std::mutex::unlock() does not
         *          allow too many invocations of it.
         */
        virtual void releaseInternalLatches() noexcept final {
            _latch.unlock();
        };

    protected:
        /*!\var     _clocks
         * \brief   Clocks \f$T_1\f$ and \f$T_2\f$
         * \details Represents the clocks \f$T_1\f$ and \f$T_2\f$ which contain eviction-specific metadata of the pages
         *          that are inside the buffer pool. Therefore there needs to be two clocks in the
         *          \link multi_clock::MultiHandedClock \endlink and the size of the clock equals the size of the buffer
         *          pool. As the CAR algorithm only stores a referenced bit, the value stored for each index is of
         *          Boolean type. And as the internal operation of multi_clock needs an invalid index (as well as a
         *          range of indexes starting from 0), the used invalid index is 0 which isn't used in the buffer pool
         *          as well.
         */
        multi_clock::MultiHandedClock<bf_idx, bool, 2, 0>           _clocks;

        /*!\var     _b1
         * \brief   LRU-list \f$B_1\f$
         * \details Represents the LRU-list \f$B_1\f$ which contains the \link PageID \endlink s of pages evicted from
         *          \f$T_1\f$.
         */
        hashtable_deque::HashtableDeque<PageID, 1 | 0x80000000>     _b1;

        /*!\var     _b2
         * \brief   LRU-list \f$B_2\f$
         * \details Represents the LRU-list \f$B_2\f$ which contains the \link PageID \endlink s of pages evicted from
         *          \f$T_2\f$.
         */
        hashtable_deque::HashtableDeque<PageID, 1 | 0x80000000>     _b2;

        /*!\var     _p
         * \brief   Parameter \f$p\f$
         * \details Represents the parameter \f$p\f$ which acts as a target size of \f$T_1\f$.
         */
        u_int32_t                                                   _p;

        /*!\var     _c
         * \brief   Parameter \f$c\f$
         * \details The number of buffer frames in the buffer pool.
         */
        u_int32_t                                                   _c;

        /*!\var     _handMovement
         * \brief   Clock hand movements in current circulation
         * \details The combined number of movements of the clock hands of \f$T_1\f$ and \f$T_2\f$. Is reset after
         *          \link _c \endlink movements.
         */
        bf_idx                                                      _handMovement;

        /*!\var     _latch
         * \brief   Latch of \link _clocks \endlink, \link _b1 \endlink and \link _b2 \endlink
         * \details As the data structures \link _clocks \endlink, \link _b1 \endlink and \link _b2 \endlink are not
         *          thread-safe and as the \link pickVictim() \endlink and the \link updateOnPageMiss() \endlink methods
         *          might change those data structures concurrently, this lock needs to be acquired by those methods.
         *          The \link updateOnPageHit() \endlink and \link updateOnPageUnfix() \endlink functions are only
         *          called with the corresponding buffer frame latched and the access is also only atomic and therefore
         *          this method does not need to acquire this lock for its changes.
         */
        std::mutex                                                  _latch;

        /*!\enum    clock_index
         * \brief   Clock names
         * \details Contains constants that map the names of the clocks used by the CAR algorithm to the indexes used by
         *          the \link _clocks \endlink data structure.
         */
        enum clock_index {
            T_1 = 0,
            T_2 = 1
        };

    };

} // zero::buffer_pool
#endif // __ZERO_PAGE_EVICTIONER_OTHER_HPP
