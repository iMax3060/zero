/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#ifndef __SM_BUFFER_POOL_HPP
#define __SM_BUFFER_POOL_HPP

#include "w_defines.h"
#include "bf_tree_cb.h"
#include "latch.h"
#include "vol.h"
#include "generic_page.h"
#include <iosfwd>
#include "buffer_pool_free_list.hpp"
#include "page_cleaner.h"
#include "page_evictioner.hpp"
#include "restart.h"
#include "restore.h"

#include <array>
#include "buffer_pool_hashtable.hpp"
#include "page_evictioner_typedefs.hpp"
#include "buffer_pool_pointer_swizzling.hpp"

#include "boost/align/aligned_allocator.hpp"

class sm_options;
class lsn_t;
class test_bf_tree;
class bf_tree_cleaner;
class btree_page_h;
class GenericPageIterator;

namespace zero::buffer_pool {

    /*!\class   BufferPool
     * \brief   A buffer manager that exploits the tree structure of indexes.
     * \details This buffer manager only deals with tree-structured stores such as B-trees.
     */
    class BufferPool {
        friend class ::test_bf_tree; // for testcases
        friend class ::bf_tree_cleaner; // for page cleaning
        friend class page_cleaner_decoupled; // for log-based "decoubled" page cleaning
        friend class ::GenericPageIterator;
    friend class FreeListLowContention;
    friend class FreeListHighContention;
    friend class PageEvictioner;

        template<class selector_class, class filter_class, bool filter_early> friend
        class PageEvictionerSelectAndFilter;

    public:
        /*!\fn      BufferPool()
         * \brief   Default constructor for a buffer pool
         * \details Constructs an almost completely initialized buffer pool according to the settings set in
         *          \link ss_m::get_options() \endlink .
         *
         * \post    The buffer pool is partially initialized.
         *
         * \warning The buffer pool is only fully initialized after calling \link postInitialize \endlink .
         */
        BufferPool();

        /*!\fn      void postInitialize()
         * \brief   Completes the initialization of this buffer pool
         * \details Completes the initialization of this buffer pool which was started during its construction.
         *
         * \pre     \link vol_t::_alloc_cache \endlink needs to be initialized before.

         * \post    The buffer pool is fully initialized. All the required auxiliary threads are running.
         */
        void postInitialize();

        /*!\fn      void shutdown()
         * \brief   Shuts down this buffer pool
         * \details Stops the threads forked by this buffer pool to allow destruction without leaving those threads
         *          running.
         *
         * \post    Threads forked by the buffer pool are stopped. The buffer pool is not able to work properly anymore.
         */
        void shutdown();

        /*!\fn      ~BufferPool()
         * \brief   Default destructor for a buffer pool
         * \details Due to the usage of smart pointers, all this destructor does is implicit.
         *
         * \warning Due to forked threads, the buffer pool needs to be shut down before destruction using
         *          \link shutdown \endlink .
         */
        ~BufferPool();

        /*!\fn      bf_idx getBlockCount() const noexcept
         * \brief   Number of buffer frames of this buffer pool
         *
         * @return  The number of buffer pool frames (\link _blockCount \endlink) of this buffer pool.
         */
        inline bf_idx getBlockCount() const noexcept {
            return _blockCount;
        };

        /*!\fn      isNoDBMode() const noexcept
         * \brief   Whether this buffer pool is in NoDB mode
         *
         * @return  \c true if this buffer pool is in NoDB mode (\link _noDBMode \endlink ), \c false else.
         */
        inline bool isNoDBMode() const noexcept {
            return _noDBMode;
        }

        /*!\fn      usesWriteElision() const noexcept
         * \brief   Whether this buffer pool uses write elision
         *
         * @return  \c true if this buffer pool uses write elision (\link _useWriteElision \endlink ), \c false else.
         */
        inline bool usesWriteElision() const noexcept {
            return _useWriteElision;
        }

        /*!\fn      bool isWarmupDone() const noexcept
         * \brief   Return whether this buffer pool is already "warmed up"
         * \details Returns whether this buffer pool is already "warmed up". Further details on the meaning of "warmed
         *          up" can be found at \link _checkWarmupDone() \endlink .
         *
         * @return  False, if the warmup of this buffer pool is ongoing, true else.
         */
        inline bool isWarmupDone() const noexcept {
            return _warmupDone;
        }

        /*!\fn      getHashtable() const noexcept
         * \brief   Returns the hashtable of this buffer pool
         *
         * @return The hashtable mapping page IDs of pages buffered in this buffer pool to the corresponding buffer
         *         indexes of them and their parent pages.
         */
        inline const std::shared_ptr<zero::buffer_pool::Hashtable> getHashtable() const noexcept {
            return _hashtable;
        };

        /*!\fn      getFreeList() const noexcept
         * \brief   Returns the free list of this buffer pool
         *
         * @return The free list holding a queue of unoccupied buffer frames of this buffer pool.
         */
        inline const std::shared_ptr<zero::buffer_pool::FreeListLowContention> getFreeList() const noexcept {
            return _freeList;
        };

        /*!\fn      getPageCleaner() const noexcept
         * \brief   Returns the page cleaner of this buffer pool
         *
         * @return The page cleaner thread responsible to write-back dirty pages of this buffer pool.
         */
        inline const std::shared_ptr<page_cleaner_base> getPageCleaner() const noexcept {
            return _cleaner;
        };

        /*!
         *
         */
        inline void wakeupPageCleaner() const {
            if (_cleaner) {
                _cleaner->wakeup(true, 1);
            }
        };

        /*!\fn      getPageEvictioner() const noexcept
         * \brief   Returns the page evictioner of this buffer pool
         *
         * @return The page evictioner (thread) responsible evict pages from this buffer pool once its full.
         */
        const std::shared_ptr<PAGE_EVICTIONER> getPageEvictioner() const noexcept;

        /*!\fn      hasDirtyFrames() const
         * \brief   Whether this buffer pool has dirty buffer frames
         * \details Checks each frame of the buffer pool whether it contains a page and whether this page is dirty
         *          inside the buffer pool frame. In NoDB mode, pages cannot be dirty.
         *
         * @return  \c true if this buffer pool contains dirty buffer frames, \c false else.
         */
        bool hasDirtyFrames();

        /*!\fn      fixRoot(generic_page*& targetPage, StoreID store, latch_mode_t latchMode, bool conditional, bool virgin)
         * \brief   Fixes a root B-Tree page in this buffer pool
         * \details Fixes an existing (not virgin) root B-Tree page of a given \c store in this buffer pool.
         *
         * \post    The \c targetPage pointer points to the page which should have been fixed and it is latched in the
         *          wanted mode.
         *
         * @param[out] targetPage   This page should contain the fixed page.
         * @param[in]  store        Store ID of the store whose root B-Tree page is requested to be fixed.
         * @param[in]  latchMode    The wanted latch mode for the page that should be fixed (only
         *                          \link latch_mode_t::LATCH_SH \endlink and \link latch_mode_t::LATCH_EX \endlink
         *                          are allowed here).
         * @param[in]  conditional  Whether the fix is conditional (returns immediately even if failed).
         * @param[in]  virgin       Whether the page is a new page thus does not have to be read from disk.
         */
        void fixRoot(generic_page*& targetPage, StoreID store, latch_mode_t latchMode, bool conditional, bool virgin);

        w_rc_t fixRootOldStyleExceptions(generic_page*& targetPage, StoreID store, latch_mode_t latchMode,
                                         bool conditional, bool virgin);

        /*!\fn      fixNonRoot(generic_page*& targetPage, generic_page* parentPage, PageID pid, latch_mode_t latchMode, bool conditional, bool virgin = false, bool onlyIfHit, bool doRecovery, lsn_t emlsn)
         * \brief   Fixes a non-root B-Tree page in this buffer pool
         * \details If pointer swizzling is enabled, this receives the requested page's parent page and efficiently
         *          fixes the requested page if the \c pid is already swizzled by inside the parent page. If it is not
         *          already swizzled, it will be swizzled after the execution of this. The optimization is transparent
         *          for most of the code because the page ID stored in the parent page is automatically (and atomically)
         *          changed to a swizzled pointer by the buffer pool.<br>
         *          If pointer swizzling is disabled, this fixes the requested page in this buffer pool.
         *
         * \pre     The \c parentPage set and latched. Otherwise use \link fixable_page_h.fix_direct(PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, bool do_recovery) \endlink .
         *
         * \post    The \c targetPage pointer points to the page which should have been fixed and it is latched in the
         *          wanted mode.
         *
         * @param[out] targetPage   This page should contain the fixed page.
         * @param[in]  parentPage   This is the parent of the page to be fixed. If the page is either represents a root
         *                          of a B-Tree or is not a B-Tree page this see the prerequisites.
         * @param[in]  pid          Page ID of the requested page to be fixed (or buffer pool index with
         *                          \link swizzledPIDBit \endlink set when swizzled).
         * @param[in]  latchMode    The wanted latch mode for the page that should be fixed (only
         *                          \link latch_mode_t::LATCH_SH \endlink and \link latch_mode_t::LATCH_EX \endlink
         *                          are allowed here).
         * @param[in]  conditional  Whether the fix is conditional (returns immediately even if failed).
         * @param[in]  virgin       Whether the page is a new page thus does not have to be read from disk.
         * @param[in]  onlyIfHit    The fix is only performed if the requested page is already buffered in this buffer
         *                          pool (i.e., a page hit occurs)
         * @param[in]  doRecovery   Whether recovery should be enabled for this page.
         * @param[in]  emlsn        The EMLSN of the requested page.
         * @return                  If \c onlyIfHit is set and it is a page miss \c false , else \c true .
         */
        bool fixNonRoot(generic_page*& targetPage, generic_page* parentPage, PageID pid, latch_mode_t latchMode,
                        bool conditional = false, bool virgin = false, bool onlyIfHit = false, bool doRecovery = true,
                        lsn_t emlsn = lsn_t::null);

        w_rc_t fixNonRootOldStyleExceptions(generic_page*& targetPage, generic_page* parentPage, PageID pid,
                                            latch_mode_t latchMode, bool conditional = false, bool virgin = false,
                                            bool onlyIfHit = false, bool doRecovery = true, lsn_t emlsn = lsn_t::null);

        /*!\fn      pinForRefix(const generic_page* pinPage)
         * \brief   Pin a page for a cheaper re-fix
         * \details Adds an additional pin count for the given \c pinPage . This is used to re-fix the page later
         *          without parent pointer. See \link refixDirect(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode, bool conditional) \endlink
         *          why we need this feature.
         *
         * \pre     The buffer frame corresponding to \c pinPage is already latched by this thread.
         * \post    The page cannot be evicted because it is pinned by this thread.
         *
         * @param[in] pinPage The page that is currently latched and will be re-fixed later.
         * @return            Buffer frame index of the \c pinPage in this buffer pool. Use this value for the
         *                    subsequent calls to \link refixDirect(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode, bool conditional) \endlink
         *                    and \link unpinForRefix(bf_idx unpinIndex) \endlink .
         *
         * \warning Never forget to call a corresponding \link unpinForRefix(bf_idx unpinIndex) \endlink because
         *          otherwise the \c pinPage will be in this buffer pool forever.
         */
        bf_idx pinForRefix(const generic_page* pinPage);

        /*!\fn      refixDirect(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode, bool conditional)
         * \brief   Re-fix a pinned page
         * \details Re-fixes a page without the parent pointer which was pinned for refix (\link pinForRefix(const generic_page* pinPage) \endlink )
         *          before.
         *
         * @param[out] targetPage This page should contain the re-fixed page.
         * @param[in] refixIndex  Buffer pool index of the requested page to be re-fixed.
         * @param[in] latchMode   The wanted latch mode for the page that should be re-fixed (only
         *                        \link latch_mode_t::LATCH_SH \endlink and \link latch_mode_t::LATCH_EX \endlink
         *                        are allowed here).
         * @param[in] conditional Whether the re-fix is conditional (returns immediately even if failed).
         */
        void refixDirect(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode, bool conditional);

        w_rc_t refixDirectOldSytleExceptions(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode,
                                             bool conditional);

        /*!\fn      unpinForRefix(bf_idx unpinIndex)
         * \brief   Unpin a page for re-fix
         * \details Removes the additional pin count added by the call to \link pinForRefix(const generic_page* pinPage) \endlink
         *          from the buffer frame corresponding to the given buffer frame index.
         *
         * \pre     The page cannot be evicted because it is pinned by this thread.
         * \post    If no other thread pinned this page, it can be evicted.
         *
         * @param[in] unpinIndex Buffer pool index of the page to be unpin for refix.
         */
        void unpinForRefix(bf_idx unpinIndex);

        /*!\fn      unfix(const generic_page* unfixPage, bool evict)
         * \brief   Unfixes a page in this buffer pool
         * \details Unfixes the given page in this buffer pool by releasing its latch, and, if requested, evicting the
         *          page.
         *
         * @param[in] unfixPage The page that should be unfixed.
         * @param[in] evict     Whether the unfixed page should be explicitly evicted if possible.
         */
        void unfix(const generic_page* unfixPage, bool evict = false);

        /*!\fn      bf_tree_cb_t& getControlBlock(bf_idx index) noexcept
         * \brief   Returns the control block corresponding to a buffer pool index
         * \details Returns the control block which corresponds to the buffer frame at the given buffer pool index. Due
         *          to the allocation scheme used in \link _controlBlocks \endlink , it is not trivial (but possible in
         *          constant time) to find the requested control block.
         *
         * @param index A buffer pool index corresponding to a buffer frame of this buffer pool.
         * @return      The control block corresponding to the buffer frame of the given buffer pool index.
         */
        bf_tree_cb_t& getControlBlock(bf_idx index) noexcept {
            return _controlBlocks[index];
        };

        //!@copydoc getControlBlock(bf_idx index) noexcept
        const bf_tree_cb_t& getControlBlock(bf_idx index) const noexcept {
            return _controlBlocks[index];
        };

        /*!\fn      bf_tree_cb_t& getControlBlock(const generic_page* page) noexcept
         * \brief   Returns the control block corresponding to a buffered page
         * \details Returns the control block which corresponds to the buffer frame of a given buffered page. Due to the
         *          allocation scheme used in \link _controlBlocks \endlink , it is not trivial (but possible in
         *          constant time) to find the requested control block.
         *
         * @param page A page buffered in this buffer pool.
         * @return     The control block corresponding to the buffer frame of the given buffered page.
         *
         * \warning Unwanted behaviour if the given \c page is not buffered in this buffer pool!
         */
        bf_tree_cb_t& getControlBlock(const generic_page* page) noexcept {
            return getControlBlock(getIndex(page));
        };

        /*!\fn      getIndex(const bf_tree_cb_t& controlBlock) const noexcept
         * \brief   The buffer pool index of a control block
         * \details Returns the buffer pool index to which the given control block corresponds to.
         *
         * @param controlBlock A control block of this buffer pool.
         * @return             The buffer pool index to which \c controlBlock corresponds to.
         */
        bf_idx getIndex(const bf_tree_cb_t& controlBlock) const noexcept {
            return std::distance(_controlBlocks.data(), &controlBlock);
        };

        /*!\fn      getIndex(const generic_page* page) const noexcept
         * \brief   The buffer pool index of a buffered page
         * \details Returns the buffer pool index in which the given buffered page is buffered in.
         *
         * @param page A page buffered in this buffer pool.
         * @return     The buffer pool index in which \c page is buffered in.
         */
        bf_idx getIndex(const generic_page* page) const noexcept {
            return static_cast<bf_idx>(page - _buffer);
        };

        /*!\fn      getRootIndex(StoreID store) const noexcept
         * \brief   The buffer pool index of a buffered B-Tree root page
         * \details Returns the buffer pool index in which a B-Tree root page specified by its Store ID is buffered in.
         *
         * @param store The Store ID of the requested B-Tree root page.
         * @return      The buffer pool index where the requested B-Tree root page is buffered in this buffer pool or
         *              \c 0 if it is currently not buffered in this buffer pool.
         */
        bf_idx getRootIndex(StoreID store) const noexcept {
            bf_idx rootIndex = _rootPages[store];
            if (!isValidIndex(rootIndex)) {
                return 0;
            } else {
                return rootIndex;
            }
        };

        /*!\fn      generic_page* getPage(const bf_idx& index) noexcept
         * \brief   A buffered page at a buffer pool index
         * \details Returns the page which is buffered at a given buffer pool index in this buffer pool.
         *
         * @param index A buffer index of this buffer pool.
         * @return      The page which is buffered in the buffer frame with buffer index \c index .
         *
         * \note Mainly for debugging!
         */
        generic_page* getPage(const bf_idx& index) noexcept {
            w_assert1(isValidIndex(index));
            return &_buffer[index];
        };

        //!\copydoc getPage(const bf_idx& index) noexcept
        const generic_page* getPage(const bf_idx& index) const noexcept {
            w_assert1(isValidIndex(index));
            return &_buffer[index];
        };

        /*!\fn      isValidIndex(bf_idx index) const noexcept
         * \brief   Checks the validity of a buffer pool index in this buffer pool
         * \details Whether the given buffer pool index is in the range of valid buffer pool indexes of this buffer pool
         *          (0 is invalid and too large indexes are invalid, too).
         *
         * @param index The buffer pool index to be checked.
         * @return      Whether \c index is a valid buffer pool index in this buffer pool.
         */
        bool isValidIndex(bf_idx index) const noexcept {
            return index > 0 && index < _blockCount;
        };

        /*!\fn      isActiveIndex(bf_idx index) const noexcept
         * \brief   Checks whether a buffer pool frame is used in this buffer pool
         * \details Whether the given buffer pool index is in the range of valid buffer pool indexes of this buffer pool
         *          (0 is invalid and too large indexes are invalid, too) and whether it is occupied by a page.
         *
         * \pre     Hold the latch corresponding to the buffer pool index in \link latch_mode_t::LATCH_SH \endlink or
         *          \link latch_mode_t::LATCH_EX \endlink .
         *
         * @param index The buffer pool index to be checked.
         * @return      Whether \c index is a valid buffer pool index in this buffer pool and whether a page is buffered
         *              in the corresponding buffer frame.
         */
        bool isActiveIndex(bf_idx index) const noexcept {
            return isValidIndex(index) && getControlBlock(index)._used;
        };

        /*!\fn      upgradeLatchConditional(const generic_page* page) noexcept
         * \brief   Upgrade the latch of a page
         * \details Upgrade this thread holding the latch of a page in \link latch_mode_t::LATCH_SH \endlink to one
         *          holding the latch in \link latch_mode_t::LATCH_EX \endlink without waiting.
         *
         * @param page The latch corresponding to this page should be upgraded.
         * @return     If the latch is already held in \link latch_mode_t::LATCH_EX \endlink or if the latch could be
         *             upgraded immediately \c true, else \c false .
         */
        bool upgradeLatchConditional(const generic_page* page) noexcept;

        /*!\fn      downgradeLatch(const generic_page* page) noexcept
         * \brief   Downgrade the latch of a page
         * \details Downgrade this thread holding the latch of a page in \link latch_mode_t::LATCH_EX \endlink to one
         *          holding the latch in \link latch_mode_t::LATCH_SH \endlink .
         *
         * @param page The latch corresponding to this page should be downgraded.
         */
        void downgradeLatch(const generic_page* page) noexcept;

        /*!\fn      unswizzlePagePointer(generic_page *parentPage, general_recordid_t childSlotInParentPage, bool doUnswizzle = true, PageID *childPageID = nullptr)
         * \brief   Unswizzle a page pointer in a page's parent page
         * \details Tries to unswizzle the given child page (\c childSlotInParentPage ) from the parent page
         *          (\c parentPage ). If, for some reason, unswizzling was impossible or troublesome, gives up and
         *          returns \c false .
         *
         * @pre The parent page is latched in any mode and the child page is latched in mode
         *      \link latch_mode_t::LATCH_EX \endlink.
         *
         * @param[in]     parentPage            The parent page where to unswizzle the child page.
         * @param[in]     childSlotInParentPage The slot within the parent page where to find the swizzled
         *                                      pointer to the child page.
         * @param[in,out] childPageID           If it wasn't set to the \c nullptr, the unswizzled
         *                                      \link PageID \endlink of the child page is returned.
         *
         * @return                              \c false if the unswizzling was not successful, else \c true.
         */
        bool unswizzlePagePointer(generic_page* parentPage, general_recordid_t childSlotInParentPage,
                                  PageID* childPageID = nullptr);

        /*!\fn      normalizePID(const PageID pid) const noexcept
         * \brief   Normalize a page ID
         * \details Normalizes the given page ID to an actual page ID (for \link vol_t \endlink ). If the page ID is a
         *          buffer pool index (in case of swizzling) then it returns the page ID of the page on disk, otherwise
         *          it returns the page ID as it is.
         *
         * \pre     Hold the latch corresponding to the parent page in \link latch_mode_t::LATCH_SH \endlink or
         *          \link latch_mode_t::LATCH_EX \endlink ?
         *
         * @param pid The page ID to normalize.
         * @return    The corresponding page ID to a \link vol_t \endlink page.
         */
        PageID normalizePID(const PageID pid) const noexcept {
            if constexpr (POINTER_SWIZZLER::usesPointerSwizzling) {
                if (POINTER_SWIZZLER::isSwizzledPointer(pid)) {
                    bf_idx index = POINTER_SWIZZLER::makeBufferIndex(pid);
                    w_assert1(isValidIndex(index));
                    const bf_tree_cb_t& controlBlock = getControlBlock(index);
                    w_assert1(!POINTER_SWIZZLER::isSwizzledPointer(controlBlock._pid));
                    return controlBlock._pid;
                } else {
                    return pid;
                }
            } else {
                return pid;
            }
        };

        /*!\fn      isEvictable(const bf_idx indexToCheck, const bool doFlushIfDirty) noexcept
         * \brief   Check if a page can be evicted
         * \details The following conditions make a page unevictable:
         *          - There is no page in the buffer frame (\link bf_tree_cb_t::_used \endlink \c == \c false).
         *          - It is the store node page (\link generic_page_h::tag() \endlink \c ==
         *            \link page_tag_t::t_stnode_p \endlink).
         *          - It is the root page of a B-Tree (\link generic_page_h::tag() \endlink \c ==
         *            \link page_tag_t::t_btree_p \endlink \c && \link generic_page_h::pid() \endlink \c ==
         *            \link btree_page_h::root() \endlink).
         *          - It is an inner page of a B-Tree and swizzling is enabled
         *            (\link POINTER_SWIZZLER::usesPointerSwizzling \endlink \c && \link generic_page_h::tag() \endlink
         *            \c == \link page_tag_t::t_btree_p \endlink \c && \c !\link btree_page_h::is_leaf() \endlink).\n
         *            We exclude those as we do not support unswizzling and as we do not know if inner pages of a B-Tree
         *            might contain swizzled pointers.
         *          - It is a page of a B-Tree with foster child (\link POINTER_SWIZZLER::usesPointerSwizzling \endlink
         *            \c && \link generic_page_h::tag() \endlink \c == \link page_tag_t::t_btree_p \endlink \c &&
         *            \link btree_page_h::get_foster() \endlink) \c != \c 0).\n
         *            We exclude those as we do not support unswizzling.
         *          - It is a dirty page that needs to be cleaned by the page cleaner (\c ( \c doFlushIfDirty \c ||
         *            \link _noDBMode \endlink \c || \link _useWriteElision \endlink \c ) \c &&
         *            \link bf_tree_cb_t::is_dirty() \endlink)\n
         *            If noDB or write elision is used, a page doesn't need to be flushed before eviction and if the
         *            evictioner flushes dirty pages, those can be evicted as well.
         *          - There is no page in the buffer pool frame - it is unused (\c !\link bf_tree_cb_t::_used \endlink).
         *          - It is pinned (\c !\link bf_tree_cb_t::_pin_cnt \endlink \c != \c 0).\n
         *            The page is either pinned or it gets currently evicted by another thread.
         *
         * \pre     The buffer frame with index \c indexToCheck is latched in \link latch_mode_t::LATCH_SH \endlink or
         *          \link latch_mode_t::LATCH_EX \endlink mode by this thread.
         *
         * \remark  The example implementations for the conditions are given in the documentation because it is hard to
         *          figure out which pages cannot be evicted.
         *
         * @param indexToCheck   The index of the buffer frame that is supposed to be freed by evicting the contained
         *                       page.
         * @param doFlushIfDirty \c true if the page gets flushed during the eviction, \c false else.
         * @return               \c true if the page could be evicted, \c false else.
         */
        bool isEvictable(const bf_idx indexToCheck, const bool doFlushIfDirty) noexcept;

        /*!\fn      batchPrefetch(PageID startPID, bf_idx numberOfPages) noexcept
         * \brief   Prefetches a batch of pages into this buffer pool
         * \details Prefetches a continuous batch of pages into this buffer pool using \c preadv . This allows a faster
         *          restore process without traversing the B-Tree.
         *
         * @param startPID      The page ID of the first page in the continuous batch of pages to prefetch.
         * @param numberOfPages The number of pages to prefetch (page ID of the last prefetched page: \c startPID \c +
         *                      \c numberOfPages \c - \c 1 ).
         *
         * \warning It is not guaranteed for a non-root B-Tree page that its parent page is also prefetched.
         * \warning The parent page information of non-root B-Tree pages is not properly set in the hashtable.
         */
        void batchPrefetch(PageID startPID, bf_idx numberOfPages) noexcept;

        /*!\fn      recoverIfNeeded(bf_tree_cb_t& controlBlock, generic_page* page, bool onlyIfDirty = true) noexcept
         * \brief   Recover buffered page if needed
         * \details Recovers a page---buffered in this buffer pool---if needed using the \link _localSprIter \endlink .
         *          It is not needed when recovery is disabled in general or when it is disabled for the particular
         *          page. It is possibly needed when it is explicitly requested (\c onlyIfTrue \c == \c false ) or when
         *          the page is dirty.
         *
         * \pre     Hold the latch corresponding to the control block in \link latch_mode_t::LATCH_EX \endlink .
         * \post    If recovery is enabled, the page is not dirty and a log record of type
         *          \link fetch_page_log \endlink was created if \link _logFetches \endlink is \c true .
         *
         * @param controlBlock The control block of the page to recover if needed.
         * @param page         The buffered page to recover if needed.
         * @param onlyIfDirty  Whether the page should be recovered even though it is not dirty.
         */
        void recoverIfNeeded(bf_tree_cb_t& controlBlock, generic_page* page, bool onlyIfDirty = true) noexcept;

        /*!\fn      fuzzyCheckpoint(chkpt_t& checkpoint) const noexcept
         * \brief   Creates a fuzzy checkpoint for this buffer pool
         * \details Adds the information required for a fuzzy checkpoint of this buffer pool to the given checkpoint.
         *          A fuzzy checkpoint of this buffer pool contains all the page IDs of potentially dirty pages buffered
         *          in this buffer pool together with their LSNs.
         *
         * \post    The \c checkpoint contains a fuzzy checkpoint of this buffer pool.
         *
         * @param[in,out] checkpoint A fuzzy checkpoint.
         */
        void fuzzyCheckpoint(chkpt_t& checkpoint) const noexcept;

        /*!\fn      sxUpdateChildEMLSN(btree_page_h &parentPage, general_recordid_t childSlotID, lsn_t childEMLSN) const
         * \brief   Update the EMLSN of a B-Tree child inside its parent page
         * \details Sets the EMLSN of a B-Tree child in the corresponding slot of its parent page. This uses a system
         *          transaction for the purpose.
         *
         * \pre     Hold the latch corresponding to the parent page in either \link latch_mode_t::LATCH_SH \endlink
         *          or  \link latch_mode_t::LATCH_EX \endlink .
         * \post    A log record of type \link update_emlsn_log \endlink was created.
         *
         * @param parentPage  The parent page buffered in this buffer pool.
         * @param childSlotID The corresponding slot ID of the child within \c parentPage .
         * @param childEMLSN  The new EMLSN of the child page requested to be written in the parent page.
         */
        void sxUpdateChildEMLSN(btree_page_h& parentPage, general_recordid_t childSlotID, lsn_t childEMLSN) const;

        /*!\fn      switchParent(PageID childPID, generic_page* newParentPage) noexcept
         * \brief   Switch the parent page of a buffered B-Tree page in this buffer pool
         * \details For each buffered non-root B-Tree page, the buffer pool also stores the buffer pool index of the
         *          buffer frame where the corresponding parent page is buffered. Therefore, this information needs to
         *          be updated once the parent page of such a page is changed. That is what this function does.
         *
         * \pre     Hold the latch corresponding to the old and new parent page in either
         *          \link latch_mode_t::LATCH_SH \endlink or  \link latch_mode_t::LATCH_EX \endlink .
         *
         * @param childPID      The page ID of the B-Tree page whose parent is switched.
         * @param newParentPage The new parent page.
         */
        void switchParent(PageID childPID, generic_page* newParentPage) noexcept;

        /*!\fn      void setMediaFailure()
         * \brief   MG TODO
         * \details MG TODO
         *
         * \pre     MG TODO
         * \post    MG TODO
         */
        void setMediaFailure() noexcept;

        /*!\fn      void unsetMediaFailure()
         * \brief   MG TODO
         * \details MG TODO
         *
         * \pre     MG TODO
         * \post    MG TODO
         */
        void unsetMediaFailure() noexcept;

        /*!\fn      getMediaFailurePID() const
         * \brief   MG TODO
         * \details MG TODO
         *
         * @return MG TODO
         */
        PageID getMediaFailurePID() const noexcept {
            return _mediaFailurePID;
        };

        /*!\fn      void unsetMediaFailure() const
         * \brief   MG TODO
         * \details MG TODO
         *
         * @return MG TODO
         */
        bool isMediaFailure() const noexcept {
            return _mediaFailurePID > 0;
        };

        /*!\fn      bool isMediaFailure(PageID pid) const
         * \brief   MG TODO
         * \details MG TODO
         *
         * @param pid MG TODO
         * @return    MG TODO
         */
        bool isMediaFailure(PageID pid) const noexcept {
            return _mediaFailurePID > 0 && pid < _mediaFailurePID;
        };

        /*!\fn      debugDump(std::ostream& o) const
         * \brief   Dumps some meta data of this buffer pool to an output stream
         * \details Dumps some general meta data of this buffer pool and of the first 1000 buffer pool frames to the
         *          specified output stream. See the implementation for more details! The data is multi-lined and cannot
         *          be manipulated using stream manipulators.
         *
         * @param o The output stream to which the meta data are printed.
         *
         * \warning This function is only for debug purposes because it is slow and unsafe.
         */
        void debugDump(std::ostream& o) const;

        /*!\fn      debugDumpPagePointers(std::ostream& o, generic_page* page) const
         * \brief   Dumps the pointers stored inside a page to an output stream
         * \details Dumps the pointers (accounting for swizzling) of a given page to the specified output stream. See
         *          the implementation for more details! The data is single-lined and cannot be manipulated using stream
         *          manipulators.
         *
         * @param o    The output stream to which the pointers are printed.
         * @param page The page whose contained pointers are printed.
         *
         * \warning This function is only for debug purposes because it is slow and unsafe.
         */
        void debugDumpPagePointers(std::ostream& o, generic_page* page) const;

    private:
        /*!\var     _blockCount
         * \brief   Maximum number of pages in this buffer pool
         * \details Number of buffer pool frames available to hold pages in this buffer pool.
         */
        bf_idx _blockCount;

        // CS TODO: concurrency???
        /*!\var     _rootPages
         * \brief   Buffer indexes of root pages
         * \details This holds for each \link stnode_t \endlink the buffer index of its root page or \c 0 if it is
         *          currently not buffered in this buffer pool.
         */
        std::array<bf_idx, stnode_page::max> _rootPages;

        /*!\var     _controlBlocks
         * \brief   Array of control blocks
         * \details A C-array containing \link _blockCount \endlink control blocks, one for each buffer pool frame of
         *          this buffer pool. The array index represents the buffer frame index.
         */
        std::vector<bf_tree_cb_t, boost::alignment::aligned_allocator<bf_tree_cb_t, sizeof(bf_tree_cb_t)>>
                _controlBlocks;

        /*!\var     _buffer
         * \brief   Array of buffered pages
         * \details A C-array containing up to \link _blockCount \endlink buffered pages, each in a buffer pool frame of
         *          this buffer pool. The array index represents the buffer frame index.
         */
        generic_page* _buffer;

        /*!\var     _hashtable
         * \brief   Allows to locate pages by \link PageID \endlink
         * \details A hashtable which maps the \link PageID \endlink of page buffered in this buffer pool to the buffer
         *          frame index where it is located in. For each buffered page, it also contains the buffer frame index
         *          of its parent page (if it's a non-root B-Tree page).
         */
        std::shared_ptr<Hashtable> _hashtable;

        /*!\var     _freeList
         * \brief   List of unused buffer frames
         * \details A queue containing the indexes of currently unoccupied buffer frames of this buffer pool.
         *
         * \note    It could be any synchronized data structure but a queue is required for some of the page eviction
         *          strategies like CLOCK.
         */
        std::shared_ptr<FreeListLowContention> _freeList;

        /*!\var     _cleaner
         * \brief   Cleans dirty pages
         * \details This is responsible to clean dirty pages (write-back changed pages) buffered in this buffer pool.
         */
        std::shared_ptr<page_cleaner_base> _cleaner;

        /*!\var     _cleanerDecoupled
         * \brief   Use log-based "decoupled" cleaner
         * \details Set if the log-based "decoupled" cleaner should be used instead of one that is based on data from
         *          the buffer pool.
         */
        bool _cleanerDecoupled;

        /*!\var     _evictioner
         * \brief   Evicts pages
         * \details This is responsible to evict buffered pages from this buffer pool once there are (almost) no more
         *          unoccupied buffer frames in this buffer pool while currently not buffered pages should be added to
         *          this buffer pool.
         */
        std::shared_ptr<PAGE_EVICTIONER> _evictioner;

        /*!\var     _asyncEviction
         * \brief   Use a dedicated thread for eviction
         * \details Set if a dedicated thread should be used for page eviction. If a dedicated thread is used, threads
         *          encountering a full buffer pool wakeup this page eviction thread and wait until there is a buffer
         *          index of an unoccupied buffer frame available for them in the \link _freeList \endlink . If no
         *          dedicated thread is used, the first thread encountering a full buffer pool runs the eviction before
         *          it continues with its transaction.
         *
         * \note    One run of the evictioner might evict many pages (batch eviction) therefore such a run might take a
         *          long time.
         */
        bool _asyncEviction;

        /*!\var     _maintainEMLSN
         * \brief   Maintain EMLSN of B-Tree pages
         * \details MG TODO
         */
        bool _maintainEMLSN;

        /*!\var     _localSprIter
         * \brief   Single-page-recovery iterator used for instant restart redo
         * \details MG TODO
         *
         * \see     recoverIfNeeded
         */
        static thread_local SprIterator _localSprIter;

        using RestoreCoord = RestoreCoordinator<std::function<decltype(SegmentRestorer::bf_restore)>>;

        /*!\var     _restoreCoordinator
         * \brief   MG TODO
         * \details MG TODO
         */
        std::shared_ptr<RestoreCoord> _restoreCoordinator;

        using BgRestorer = BackgroundRestorer<RestoreCoord, std::function<void(void)>>;

        /*!\var     _backgroundRestorer
         * \brief   MG TODO
         * \details MG TODO
         */
        std::shared_ptr<BgRestorer> _backgroundRestorer;

        /*!\var     _useWriteElision
         * \brief   Use write elision
         * \details Set if write elision should be used by this buffer pool. With write elision, a page can be evicted
         *          from the buffer pool even though it is dirty because once the page is read again, the changes are
         *          retrieved from the log.
         */
        bool _useWriteElision;

        /*!\var     _mediaFailurePID
         * \brief   MG TODO
         * \details MG TODO
         */
        std::atomic<PageID> _mediaFailurePID;

        /*!\var     _instantRestore
         * \brief   Use instant restore
         * \details Set if instant restore should be used by this buffer pool. MG TODO
         */
        bool _instantRestore;

        /*!\var     _noDBMode
         * \brief   Use NoDB
         * \details Set if this buffer pool should be used for NoDB. MG TODO
         */
        bool _noDBMode;

        /*!\var     _logFetches
         * \brief   Log page fetches
         * \details Set if page fetches from database or from the used recovery mechansim should be logged in the
         *          transactional log using log entries of type \link fetch_page_log \endlink .
         */
        bool _logFetches;

        /*!\var     _batchSegmentSize
         * \brief   MG TODO
         * \details MG TODO
         */
        size_t _batchSegmentSize;

        /*!\var     _batchWarmup
         * \brief   MG TODO
         * \details MG TODO
         */
        bool _batchWarmup;

        /*!\var     _warmupDone
         * \brief   MG TODO
         * \details MG TODO
         */
        bool _warmupDone;

        /*!\var     _warmupHitRatio
         * \brief   Hit rate of a "warm" buffer pool
         * \details At least this hit rate needs to be achieved by this buffer pool to be considered warm.
         */
        double _warmupHitRatio;

        /*!\var     _warmupMinFixes
         * \brief   Minimum number of fixes of a "warm" buffer pool
         * \details At least this many page fixes need to be performed by this buffer pool to be considered warm.
         */
        unsigned _warmupMinFixes;

        /*!\var     _fixCount
         * \brief   MG TODO
         * \details MG TODO
         */
        static thread_local unsigned _fixCount;

        /*!\var     _hitCount
         * \brief   MG TODO
         * \details MG TODO
         */
        static thread_local unsigned _hitCount;

        /*!\fn      _fix(generic_page* parentPage, generic_page*& targetPage, PageID pid, latch_mode_t latchMode, bool conditional, bool virgin, bool onlyIfHit, bool doRecovery, lsn_t emlsn)
         * \brief   Fixes a page in this buffer pool
         * \details If pointer swizzling is enabled, this receives the requested page's parent page and efficiently
         *          fixes the requested page if the \c pid is already swizzled by inside the parent page. If it is not
         *          already swizzled, it will be swizzled after the execution of this. The optimization is transparent
         *          for most of the code because the page ID stored in the parent page is automatically (and atomically)
         *          changed to a swizzled pointer by the buffer pool.<br>
         *          If pointer swizzling is disabled, this fixes the requested page in this buffer pool.
         *
         * \pre     The \c parentPage set and latched. Otherwise use \link fixable_page_h.fix_direct(PageID pid, latch_mode_t mode, bool conditional, bool virgin_page, bool only_if_hit, bool do_recovery) \endlink .
         *
         * \post    The \c targetPage pointer points to the page which should have been fixed and it is latched in the
         *          wanted mode.
         *
         * @param[in]  parentPage   This is the parent of the page to be fixed. If the page is either represents a root
         *                          of a B-Tree or is not a B-Tree page this see the prerequisites.
         * @param[out] targetPage   This page should contain the fixed page.
         * @param[in]  pid          Page ID of the requested page to be fixed (or buffer pool index with
         *                          \link swizzledPIDBit \endlink set when swizzled).
         * @param[in]  latchMode    The wanted latch mode for the page that should be fixed (only
         *                          \link latch_mode_t::LATCH_SH \endlink and \link latch_mode_t::LATCH_EX \endlink
         *                          are allowed here).
         * @param[in]  conditional  Whether the fix is conditional (returns immediately even if failed).
         * @param[in]  virgin       Whether the page is a new page thus does not have to be read from disk.
         * @param[in]  onlyIfHit    The fix is only performed if the requested page is already buffered in this buffer
         *                          pool (i.e., a page hit occurs)
         * @param[in]  doRecovery   Whether recovery should be enabled for this page.
         * @param[in]  emlsn        The EMLSN of the requested page.
         * @return                  If \c onlyIfHit is set and it is a page miss \c false , else \c true .
         */
        bool _fix(generic_page* parentPage, generic_page*& targetPage, PageID pid, latch_mode_t latchMode,
                  bool conditional, bool virgin, bool onlyIfHit = false, bool doRecovery = true,
                  lsn_t emlsn = lsn_t::null);

        /*!\fn      _convertToDiskPage(generic_page* page) const noexcept
         * \brief   Converts an image of a page with swizzled pointers to one without
         * \details Converts an image of a page (buffered in this buffer pool) with swizzled pointers to a disk page
         *          without swizzled pointers. This is mainly used to write out dirty pages.
         *
         * \pre     Hold the latch corresponding to the page in either \link latch_mode_t::LATCH_SH \endlink or
         *          \link latch_mode_t::LATCH_EX \endlink .
         *
         * @param page The page with swizzled pointers to convert to a disk page.
         */
        void _convertToDiskPage(generic_page* page) const noexcept;

        /*!\fn      _readPage(PageID pid, generic_page* targetPage, bool fromBackup)
         * \brief   Reads a page from database or backup
         * \details Reads a page from database or backup into a buffer frame of this buffer pool. The source of the page
         *          needs to be selected explicitly.
         *
         * \pre     Hold the latch corresponding to the \c targetPage in \link latch_mode_t::LATCH_EX \endlink .
         *
         * @param pid        The page ID of the page to read.
         * @param targetPage The page should be read into this page image.
         * @param fromBackup Whether the page should be read from the backup or from the database.
         */
        void _readPage(PageID pid, generic_page* targetPage, bool fromBackup = false);

        /*!\fn      _deletePage(bf_idx index) noexcept
         * \brief   Deletes a page from this buffer pool
         * \details Makes the buffer frame which corresponds to the specified buffer index unoccupied.
         *
         * \pre     The buffer frame corresponding the buffer index is occupied, it is not fixed by any thread and if it
         *          contains a non-root B-Tree page, the corresponding pointer is not swizzled in its parent page (the
         *          page is not pinned).
         *
         * @param index The buffer index of the buffer pool frame to be freed.
         */
        void _deletePage(bf_idx index) noexcept;

        /*!\fn      _checkWarmupDone() const noexcept
         * \brief   Decides if this buffer pool is "warmed up"
         * \details Decides if this buffer pool is "warmed up" by looking at the hit ratio observed for the current
         *          thread and the total number of page fixes done by this thread. This is for cases where the dataset
         *          fits in main memory and the buffer pool never (or just takes really long to) fill up.
         */
        void _checkWarmupDone() noexcept;

        /*!\fn      _setWarmupDone() const noexcept
         * \brief   Sets this buffer pool to "warmed up"
         */
        void _setWarmupDone() noexcept;
    };

    class BufferPoolException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Some error happened in this buffer pool!";
        };
    };

    class BufferPoolSizeException : public BufferPoolException {
    public:
        BufferPoolSizeException(const bf_idx& blockCount) :
                _blockCount(blockCount) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The set buffer pool size of %zdMB causes an error in this buffer pool!",
                                            (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The set buffer pool size of %zdMB causes an error in this buffer pool!",
                     (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            return whatSentence;
        };

    protected:
        bf_idx _blockCount;
    };

    class BufferPoolTooSmallException : public BufferPoolSizeException {
    public:
        BufferPoolTooSmallException(const bf_idx& blockCount, const bf_idx& minimumBlockCount) :
                BufferPoolSizeException(blockCount),
                _minimumBlockCount(minimumBlockCount) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The set buffer pool size of %zdMB is too small! At least %zdMB are required.",
                                            (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024,
                                            (_minimumBlockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The set buffer pool size of %zdMB is too small! At least %zdMB are required.",
                     (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024,
                     (_minimumBlockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            return whatSentence;
        };

    protected:
        bf_idx _minimumBlockCount;
    };

    class BufferPoolTooLargeException : public BufferPoolSizeException {
    public:
        BufferPoolTooLargeException(const bf_idx& blockCount) :
                BufferPoolSizeException(blockCount) {};

        const char* what() const noexcept override {
            int whatStringLength = snprintf(nullptr, 0,
                                            "The set buffer pool size of %zdMB cannot be allocated!",
                                            (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            char* whatSentence = new char[whatStringLength + 1];
            snprintf(whatSentence, whatStringLength,
                     "The set buffer pool size of %zdMB cannot be allocated!",
                     (_blockCount * sizeof(generic_page) + 33) / 1024 / 1024);
            return whatSentence;
        };
    };

    class BufferPoolOldStyleException : public BufferPoolException {
    public:
        BufferPoolOldStyleException(const w_rc_t& oldStyleException) :
                _oldStyleException(oldStyleException) {};

        const char* what() const noexcept override {
            if (_oldStyleException.err_num() == w_error_ok) {
                return "No error";
            } else {
                char* whatStringFirstLine;
                int whatStringFirstLineLength;
                if (_oldStyleException.get_custom_message() != nullptr) {
                    whatStringFirstLineLength = snprintf(nullptr, 0, "%s(%d):%s:%s",
                                                         w_error_name(_oldStyleException.err_num()),
                                                         _oldStyleException.err_num(),
                                                         _oldStyleException.get_message(),
                                                         _oldStyleException.get_custom_message());
                    whatStringFirstLine = new char[whatStringFirstLineLength + 1];
                    snprintf(whatStringFirstLine, whatStringFirstLineLength, "%s(%d):%s:%s",
                             w_error_name(_oldStyleException.err_num()),
                             _oldStyleException.err_num(),
                             _oldStyleException.get_message(),
                             _oldStyleException.get_custom_message());
                } else {
                    whatStringFirstLineLength = snprintf(nullptr, 0, "%s(%d):%s:%s",
                                                         w_error_name(_oldStyleException.err_num()),
                                                         _oldStyleException.err_num(),
                                                         _oldStyleException.get_message(),
                                                         _oldStyleException.get_custom_message());
                    whatStringFirstLine = new char[whatStringFirstLineLength + 1];
                    snprintf(whatStringFirstLine, whatStringFirstLineLength, "%s(%d):%s",
                             w_error_name(_oldStyleException.err_num()),
                             _oldStyleException.err_num(),
                             _oldStyleException.get_message());
                }

                char** whatStringStack = new char* [_oldStyleException.get_stack_depth()];
                int* whatStringStackLength = new int[_oldStyleException.get_stack_depth()];
                int whatStringStackTotalLength = 0;
                for (int stackIndex = 0; stackIndex < _oldStyleException.get_stack_depth(); stackIndex++) {
                    whatStringStackLength[stackIndex] = snprintf(nullptr, 0, "  %s:%d",
                                                                 _oldStyleException.get_filename(stackIndex),
                                                                 _oldStyleException.get_linenum(stackIndex));
                    whatStringStack[stackIndex] = new char[whatStringStackLength[stackIndex] + 1];
                    snprintf(whatStringStack[stackIndex], whatStringStackLength[stackIndex], "  %s:%d",
                             _oldStyleException.get_filename(stackIndex),
                             _oldStyleException.get_linenum(stackIndex));
                    whatStringStackTotalLength += whatStringStackLength[stackIndex];
                }

                char* whatStringLastLine;
                int whatStringLastLineLength = 0;
                if (_oldStyleException.get_stack_depth() >= MAX_RCT_STACK_DEPTH) {
                    whatStringLastLine = "  .. and more. Increase MAX_RCT_STACK_DEPTH to see full stacktraces";
                    whatStringLastLineLength = strlen(whatStringLastLine);
                }

                char* whatString = new char[whatStringFirstLineLength + 1
                                            + whatStringStackTotalLength + _oldStyleException.get_stack_depth() - 1
                                            + whatStringLastLineLength + (whatStringLastLineLength ? 1 : 0)
                                            + 1];
                strcpy(whatString, whatStringFirstLine);
                for (int stackLine = 0; stackLine < sizeof(whatStringStack) / sizeof(whatStringStack[0]); stackLine++) {
                    strcat(whatString, "\n");
                    strcat(whatString, whatStringStack[stackLine]);
                }
                if (whatStringLastLineLength > 0) {
                    strcat(whatString, "\n");
                    strcat(whatString, whatStringLastLine);
                }

                return whatString;
            }
        }

        const w_rc_t& getOldStyleException() const noexcept {
            return _oldStyleException;
        };

    protected:
        w_rc_t _oldStyleException;
    };
} // zero::buffer_pool

/**
 * Holds the buffer slot index of additionally pinned page and
 * releases the pin count when it's destructed.
 * @see zero::buffer_pool::BufferPool::pinForRefix(), zero::buffer_pool::BufferPool::unpinForRefix(),
 *      zero::buffer_pool::BufferPool::refixDirect().
 */
class pin_for_refix_holder {
public:
    pin_for_refix_holder() :
            _idx(0) {};

    pin_for_refix_holder(bf_idx idx) :
            _idx(idx) {};

    pin_for_refix_holder(pin_for_refix_holder& h) {
        steal_ownership(h);
    };

    ~pin_for_refix_holder() {
        if (_idx != 0) {
            release();
        }
    };

    pin_for_refix_holder& operator=(pin_for_refix_holder& h) {
        steal_ownership(h);
        return *this;
    };

    void steal_ownership(pin_for_refix_holder& h) {
        if (_idx != 0) {
            release();
        }
        _idx = h._idx;
        h._idx = 0;
    };

    void set(bf_idx idx) {
        if (_idx != 0) {
            release();
        }
        _idx = idx;
    };

    void release();

    bf_idx _idx;
};

class GenericPageIterator {
public:
    /// This essentially yields an "end" iterator
    GenericPageIterator() :
            _first(0),
            _count(0),
            _virgin(false),
            _current(nullptr),
            _current_pid(0),
            _fix_depth(0) {};

    /// This essentially yields a "begin" iterator
    GenericPageIterator(PageID first, PageID count, bool virgin) :
            _first(first),
            _count(count),
            _virgin(virgin),
            _current(nullptr),
            _fix_depth(0) {
        if (count > 0) {
            _current_pid = _first - 1;
            operator++();
        }
    };

    GenericPageIterator(const GenericPageIterator& other) :
            _first(other._first),
            _count(other._count),
            _virgin(other._virgin),
            _current(nullptr),
            _fix_depth(0) {
        if (_count > 0) {
            _current_pid = _first - 1;
            operator++();
        }
    };

    ~GenericPageIterator() {
        unfix_current();
        w_assert1(_fix_depth == 0);
    };

    generic_page* operator*() {
        if (!_current || _current_pid >= end_pid()) {
            return nullptr;
        }
        return _current;
    };

    /*
     * WARNING: this operator may skip a PID (e.g., jump from 42 to 44) if
     * fix_current() returns false! I couldn't find a quick, elegant way to
     * work around this problem yet.
     */
    GenericPageIterator& operator++() {
        unfix_current();

        bool success = false;
        while (!success) {
            _current_pid++;
            if (_current_pid >= end_pid()) {
                return *this;
            }
            success = fix_current();
        }

        return *this;
    };

    GenericPageIterator& operator=(GenericPageIterator other) {
        std::swap(*this, other);
        return *this;
    };

    friend void swap(GenericPageIterator& a, GenericPageIterator& b) {
        std::swap(a._first, b._first);
        std::swap(a._count, b._count);
        std::swap(a._virgin, b._virgin);
        std::swap(a._current, b._current);
        std::swap(a._current_pid, b._current_pid);
        std::swap(a._fix_depth, b._fix_depth);
    };

    bool operator==(GenericPageIterator& other) {
        if (!_current && !other._current) {
            return true;
        }
        return other._current_pid == _current_pid && _current && other._current;
    };

    bool operator!=(GenericPageIterator& other) {
        return !(*this == other);
    };

    PageID begin_pid() const {
        return _first;
    };

    PageID end_pid() const {
        return _first + _count;
    };

    PageID _current_pid;

private:
    PageID _first;

    PageID _count;

    bool _virgin;

    generic_page* _current;

    int _fix_depth;

    bool fix_current() {
        w_assert1(_fix_depth == 0);
        constexpr bool conditional = true;
        constexpr bool do_recovery = false;
        constexpr bool only_if_hit = false;
        try {
            smlevel_0::bf->_fix(nullptr, _current, _current_pid, LATCH_EX, conditional, _virgin, only_if_hit,
                                do_recovery);
        } catch (const zero::buffer_pool::BufferPoolOldStyleException& exception) {
            /*
             * The latch is already held, either by this thread (in SH mode, which
             * results in stINUSE to avoid deadlock) or by another thread. In that
             * case, we assume that if the frame is latched, the thread that holds
             * that latch already made sure that the page is consistent, and thus
             * log replay is not required.
             */
            if (exception.getOldStyleException().err_num() == stINUSE
                || exception.getOldStyleException().err_num() == stTIMEOUT) {
                ERROUT(<< "failed to fix "
                               << _current_pid);
                _current = nullptr;
                return false;
            }
            W_COERCE(exception.getOldStyleException());
        }

        _fix_depth++;

        return true;
    };

    void unfix_current() {
        if (_current) {
            w_assert1(_fix_depth == 1);
            bf_tree_cb_t& currentControlBlock = smlevel_0::bf->getControlBlock(_current);
            currentControlBlock.unpin_for_restore();
            currentControlBlock.set_check_recovery(true);
            smlevel_0::bf->unfix(_current);
            _current = nullptr;
            _fix_depth--;
        }
    };
};

#endif // __SM_BUFFER_POOL_HPP
