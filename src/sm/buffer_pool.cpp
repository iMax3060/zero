/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#include "w_defines.h"

#include "bf_tree_cb.h"
#include "bf_tree_cleaner.h"
#include "page_cleaner_decoupled.h"
#include "buffer_pool.hpp"

#include "smthread.h"
#include "generic_page.h"
#include <cstring>
#include <cstdlib>

#include "sm_base.h"
#include "sm.h"
#include "vol.h"
#include "alloc_cache.h"

#include <boost/static_assert.hpp>
#include <ostream>
#include <iomanip>
#include <limits>
#include <algorithm>

#include "sm_options.h"
#include "latch.h"
#include "btree_page_h.h"
#include "log_core.h"
#include "xct.h"
#include "xct_logger.h"

#include "page_evictioner.hpp"
#include "page_evictioner_select_and_filter.hpp"
#include "page_evictioner_selector.hpp"
#include "page_evictioner_filter.hpp"
#include "page_evictioner_other.hpp"
#include "page_evictioner_lean_store.hpp"

using namespace zero::buffer_pool;

thread_local unsigned BufferPool::_fixCount = 0;
thread_local unsigned BufferPool::_hitCount = 0;
thread_local SprIterator BufferPool::_localSprIter;

BufferPool::BufferPool() :
        _blockCount((ss_m::get_options().get_int_option("sm_bufpoolsize", 8192) * 1024 * 1024 - 1) / sizeof(generic_page) + 1),
        _controlBlocks(_blockCount),
        _buffer(nullptr),
        _hashtable(std::make_shared<Hashtable>(_blockCount)),
        _freeList(std::make_shared<FreeListLowContention>(this, ss_m::get_options())),
        _cleanerDecoupled(ss_m::get_options().get_bool_option("sm_cleaner_decoupled", false)),
        _evictioner(std::make_shared<PAGE_EVICTIONER>(this)),
        _asyncEviction(ss_m::get_options().get_bool_option("sm_async_eviction", false)),
        _maintainEMLSN(ss_m::get_options().get_bool_option("sm_bf_maintain_emlsn", false)),
        _useWriteElision(ss_m::get_options().get_bool_option("sm_write_elision", false)),
        _mediaFailurePID(0),
        _instantRestore(ss_m::get_options().get_bool_option("sm_restore_instant", true)),
        _noDBMode(ss_m::get_options().get_bool_option("sm_no_db", false)),
        _logFetches(ss_m::get_options().get_bool_option("sm_log_page_fetches", false)),
        _batchSegmentSize(ss_m::get_options().get_int_option("sm_batch_segment_size", 1)),
        _batchWarmup(_batchSegmentSize > 0),
        _warmupDone(false),
        _warmupHitRatio(ss_m::get_options().get_int_option("sm_bf_warmup_hit_ratio", 100)),
        _warmupMinFixes(ss_m::get_options().get_int_option("sm_bf_warmup_min_fixes", 1000000)) {

    if (_blockCount < 32)  {
        throw BufferPoolTooSmallException(_blockCount, 32);
    }
    _buffer = static_cast<generic_page*>(aligned_alloc(SM_PAGESIZE, static_cast<size_t>(SM_PAGESIZE) * _blockCount));
    if (_buffer == nullptr) {
        throw BufferPoolTooLargeException(_blockCount);
    }

    _rootPages.fill(0);

    for (bf_idx index = 0; index < _blockCount; index++) {
        bf_tree_cb_t& controlBlock = getControlBlock(index);
        controlBlock._pin_cnt = -1;
        controlBlock._used = false;
        controlBlock.clear_latch();
    }

    if (_asyncEviction) {
        _evictioner->fork();
    }
}

void BufferPool::postInitialize() {
    if (_noDBMode && _batchWarmup) {
        w_assert0(smlevel_0::vol->caches_ready());
        constexpr bool virginPages = true;
        auto volPages = smlevel_0::vol->num_used_pages();
        auto segCount = volPages  / _batchSegmentSize + (volPages % _batchSegmentSize ? 1 : 0);
        _restoreCoordinator = std::make_shared<RestoreCoord>(_batchSegmentSize, segCount, SegmentRestorer::bf_restore,
                                                             virginPages);
    }

    if(_cleanerDecoupled) {
        w_assert0(smlevel_0::logArchiver);
        _cleaner = std::make_shared<page_cleaner_decoupled>(ss_m::get_options());
    } else{
        _cleaner = std::make_shared<bf_tree_cleaner>(ss_m::get_options());
    }
    _cleaner->fork();
}

void BufferPool::shutdown() {
    // Order in which threads are destroyed is very important!
    if (_backgroundRestorer) {
        _backgroundRestorer->stop();
    }

    if(_asyncEviction && _evictioner) {
        _evictioner->stop();
    }

    if (_cleaner) {
        _cleaner->stop();
    }
}

BufferPool::~BufferPool() {
    std::free(_buffer);
}

bool BufferPool::hasDirtyFrames() {
    if (_noDBMode) {
        return false;
    }

    for (bf_idx i = 1; i < _blockCount; i++) {
        bf_tree_cb_t& controlBlock = getControlBlock(i);
        if (!controlBlock.pin()) {
            continue;
        }
        if (controlBlock.is_dirty() && controlBlock._used) {
            controlBlock.unpin();
            return true;
        }
        controlBlock.unpin();
    }

    return false;
}

void BufferPool::fixRoot(generic_page*& targetPage, StoreID store, latch_mode_t latchMode, bool conditional,
                         bool virgin) {
    w_assert1(store != 0);

    bf_idx rootIndex = _rootPages[store];
    if (!isValidIndex(rootIndex)) {           // pointer not in _rootPages (usually a page miss)
        // Load root page from the database:
        PageID rootPID = smlevel_0::vol->get_store_root(store);
        _fix(nullptr, targetPage, rootPID, latchMode, conditional, virgin);

        rootIndex = getIndex(targetPage);

        w_assert1(!getControlBlock(rootIndex)._check_recovery);

        // Always swizzle the pointer to the root in the _rootPages array:
        bool oldValue = false;
        if (!std::atomic_compare_exchange_strong(&(getControlBlock(rootIndex)._swizzled), &oldValue, true)) {
            // CAS failed -- some other thread is swizzling
            return;
        } else {
            w_assert1(getControlBlock(targetPage)._swizzled);
            _rootPages[store] = rootIndex;
        }
    } else {                            // pointer in _rootPages (page hit)
        w_rc_t latchStatus =  getControlBlock(rootIndex).latch().latch_acquire(latchMode,
                                                                               conditional ? timeout_t::WAIT_IMMEDIATE
                                                                                           : timeout_t::WAIT_FOREVER);
        if (latchStatus.is_error()) {
            throw BufferPoolOldStyleException(latchStatus);
        }
        targetPage = getPage(rootIndex);
    }

    w_assert1(isValidIndex(rootIndex));
    w_assert1(isActiveIndex(rootIndex));
    w_assert1(getControlBlock(rootIndex)._used);
    // w_assert1(!get_cb(rootIndex)._check_recovery); // assertion fails with instant restore!
    w_assert1(getControlBlock(rootIndex)._pin_cnt >= 0);
    w_assert1(getControlBlock(rootIndex).latch().held_by_me());

    DBG(<< "Fixed root "
        << rootIndex
        << " with pin count "
        << getControlBlock(rootIndex)._pin_cnt);
}

w_rc_t BufferPool::fixRootOldStyleExceptions(generic_page*& targetPage, StoreID store, latch_mode_t latchMode,
                                             bool conditional, bool virgin) {
    try {
        fixRoot(targetPage, store, latchMode, conditional, virgin);
        return RCOK;
    } catch (const BufferPoolOldStyleException& ex) {
        return ex.getOldStyleException();
    }
}

bool BufferPool::fixNonRoot(generic_page*& targetPage, generic_page* parentPage, PageID pid, latch_mode_t latchMode,
                            bool conditional, bool virgin, bool onlyIfHit, bool doRecovery, lsn_t emlsn) {
    INC_TSTAT(bf_fix_nonroot_count);
    return _fix(parentPage, targetPage, pid, latchMode, conditional, virgin, onlyIfHit, doRecovery, emlsn);
}

w_rc_t BufferPool::fixNonRootOldStyleExceptions(generic_page *&targetPage, generic_page *parentPage, PageID pid,
                                                latch_mode_t latchMode, bool conditional, bool virgin, bool onlyIfHit,
                                                bool doRecovery, lsn_t emlsn) {
    INC_TSTAT(bf_fix_nonroot_count);
    try {
        if (!_fix(parentPage, targetPage, pid, latchMode, conditional, virgin, onlyIfHit, doRecovery, emlsn)) {
            return RC(stINUSE);
        } else {
            return RCOK;
        }
    } catch (const BufferPoolOldStyleException& ex) {
        return ex.getOldStyleException();
    }
}

bf_idx BufferPool::pinForRefix(const generic_page* pinPage) {
    w_assert1(pinPage != nullptr);
    w_assert1(getControlBlock(pinPage).latch().mode() != LATCH_NL);

    bf_idx pinIndex = getIndex(pinPage);
    w_assert1(isActiveIndex(pinIndex));

    w_assert1(getControlBlock(pinIndex)._pin_cnt >= 0);
    w_assert1(getControlBlock(pinIndex).latch().held_by_me());

    bool pinned = getControlBlock(pinIndex).pin();
    w_assert0(pinned);
    DBG(<< "Refix set pin cnt to "
        << getControlBlock(pinIndex)._pin_cnt);
    return pinIndex;
}

void BufferPool::refixDirect(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode, bool conditional) {
    bf_tree_cb_t& refixControlBlock = getControlBlock(refixIndex);

    w_rc_t latchAcquireStatus = refixControlBlock.latch().latch_acquire(latchMode,
                                                                        conditional ? timeout_t::WAIT_IMMEDIATE
                                                                                    : timeout_t::WAIT_FOREVER);
    if (latchAcquireStatus.is_error()) {
        throw BufferPoolOldStyleException(latchAcquireStatus);
    }

    w_assert1(refixControlBlock._pin_cnt > 0);
    // refixControlBlock.pin();

    DBG(<< "Refix direct of "
        << refixIndex
        << " set pin cnt to "
        << refixControlBlock._pin_cnt);

    refixControlBlock.inc_ref_count();
    if (latchMode == LATCH_EX) {
        refixControlBlock.inc_ref_count_ex();
    }

    _evictioner->updateOnPageHit(refixIndex);
    targetPage = getPage(refixIndex);
}

w_rc_t BufferPool::refixDirectOldSytleExceptions(generic_page*& targetPage, bf_idx refixIndex, latch_mode_t latchMode,
                                                 bool conditional) {
    try {
        refixDirect(targetPage, refixIndex, latchMode, conditional);
        return RCOK;
    } catch (const BufferPoolOldStyleException& ex) {
        return ex.getOldStyleException();
    }
}

void BufferPool::unpinForRefix(bf_idx unpinIndex) {
    w_assert1(isActiveIndex(unpinIndex));
    w_assert1(getControlBlock(unpinIndex)._pin_cnt > 0);

    // CS TODO: assertion below fails when btcursor is destructed.  Therefore,
    // we are violating the rule that pin count can only be updated when page
    // is latched. But it seems that the program logic avoids something bad
    // happened. Still, it's quite edgy at the moment. I should probaly study
    // the btcursor code in detail before taking further action on this.
    // w_assert1(getControlBlock(unpinIndex).latch().held_by_me());
    getControlBlock(unpinIndex).unpin();

    _evictioner->updateOnPageUnfix(unpinIndex);

    DBG(<< "Unpin for refix set pin cnt to "
        << getControlBlock(unpinIndex)._pin_cnt);
    w_assert1(getControlBlock(unpinIndex)._pin_cnt >= 0);
}

void BufferPool::unfix(const generic_page* unfixPage, bool evict) {
    w_assert1(unfixPage != nullptr);
    bf_idx unfixIndex = getIndex(unfixPage);
    w_assert1(isActiveIndex(unfixIndex));
    bf_tree_cb_t& cb = getControlBlock(unfixIndex);
    w_assert1(cb.latch().held_by_me());

    if (evict) {
        if (cb.prepare_for_eviction()) {
            w_assert0(cb.latch().is_mine());
            _hashtable->erase(unfixPage->pid);

            _evictioner->updateOnPageExplicitlyUnbuffered(unfixIndex);
            _freeList->addFreeBufferpoolFrame(unfixIndex);
        } else {
            return;
        }
    } else {
        w_assert1(cb._pin_cnt >= 0);
    }
    DBG(<< "Unfixed "
        << unfixIndex
        << " pin count "
        << cb._pin_cnt);
    _evictioner->updateOnPageUnfix(unfixIndex);
    cb.latch().latch_release();
}

const std::shared_ptr<PAGE_EVICTIONER> BufferPool::getPageEvictioner() const noexcept {
    return _evictioner;
}

bool BufferPool::upgradeLatchConditional(const generic_page* page) noexcept {
    w_assert1(isActiveIndex(getIndex(page)));

    bf_tree_cb_t& controlBlock = getControlBlock(page);
    w_assert1(controlBlock.latch().held_by_me());

    if (controlBlock.latch().mode() == LATCH_EX) {
        return true;
    }
    bool wouldBlock = false;
    controlBlock.latch().upgrade_if_not_block(wouldBlock);
    if (!wouldBlock) {
        w_assert1(controlBlock.latch().mode() == LATCH_EX);
        return true;
    } else {
        return false;
    }
}

void BufferPool::downgradeLatch(const generic_page* page) noexcept {
    w_assert1(isActiveIndex(getIndex(page)));

    bf_tree_cb_t& controlBlock = getControlBlock(page);
    w_assert1(controlBlock.latch().held_by_me());

    controlBlock.latch().downgrade();
}

bool BufferPool::unswizzlePagePointer(generic_page* parentPage, general_recordid_t childSlotInParentPage,
                                      PageID* childPageID) {
    if constexpr (POINTER_SWIZZLER::usesPointerSwizzling) {
        const bf_tree_cb_t &parentControlBlock = getControlBlock(getIndex(parentPage));
        // CS TODO: foster parent of a node created during a split will not have a swizzled pointer to the new node;
        // breaking the rule for now
        // if (!parentControlBlock._used || !parentControlBlock._swizzled) {
        w_assert1(parentControlBlock._used);
        w_assert1(parentControlBlock.latch().held_by_me());

        fixable_page_h fixedParentPage;
        fixedParentPage.fix_nonbufferpool_page(parentPage);
        w_assert1(childSlotInParentPage <= fixedParentPage.max_child_slot());

        PageID* childPIDInParent = fixedParentPage.child_slot_address(childSlotInParentPage);
        if (!POINTER_SWIZZLER::isSwizzledPointer(*childPIDInParent)) {
            return false;
        }

        bf_tree_cb_t& childControlBlock = getControlBlock(POINTER_SWIZZLER::makeBufferIndex(*childPIDInParent));
        w_assert1(childControlBlock._used);
        w_assert1(childControlBlock._swizzled);

        // Since we have EX latch, we can just set the _swizzled flag, otherwise there would be a race between swizzlers
        // and unswizzlers. Parent is updated without requiring EX latch. This is correct as long as fix call can deal
        // with swizzled pointers not being really swizzled.
        w_assert1(childControlBlock.latch().held_by_me());
        w_assert1(childControlBlock.latch().mode() == LATCH_EX);
        w_assert1(parentControlBlock.latch().held_by_me());
        w_assert1(parentControlBlock.latch().mode() == LATCH_EX);
        childControlBlock._swizzled = false;
        *childPIDInParent = childControlBlock._pid;
        w_assert1(!POINTER_SWIZZLER::isSwizzledPointer(*childPIDInParent));
#if W_DEBUG_LEVEL > 0
        general_recordid_t child_slotid = fixable_page_h::find_page_id_slot(parentPage, childControlBlock._pid);
        w_assert1(child_slotid != GeneralRecordIds::INVALID);
#endif

        if (childPageID) {
            *childPageID = childControlBlock._pid;
        }

        return true;
    } else {
        return false;
    }
}

bool BufferPool::checkEviction(const bf_idx indexToCheck, const bool doFlushIfDirty) noexcept {
    bool ignore_dirty = doFlushIfDirty || _noDBMode || _useWriteElision;

    bf_tree_cb_t& controlBlockToCheck = getControlBlock(indexToCheck);
    w_assert1(controlBlockToCheck.latch().held_by_me());
    w_assert1(controlBlockToCheck.latch().mode() == LATCH_EX);

    // We do not consider for eviction ...
    if (// ... unused buffer frames.
            !controlBlockToCheck._used) {
        DBG5(<< "Eviction failed on unused buffer frame " << indexToCheck);
        return false;
    }

    btree_page_h p;
    p.fix_nonbufferpool_page(&_buffer[indexToCheck]);

    // We do not consider for eviction ...
    if (// ... the stnode page
           p.tag() == t_stnode_p
        // ... B-tree root pages (note, single-node B-tree is both root and leaf)
        || (p.tag() == t_btree_p && p.pid() == p.root())) {
        _evictioner->updateOnPageBlocked(indexToCheck);
        DBG5(<< "Eviction failed on node type for " << indexToCheck);
        return false;
    }
    if (// ... B-tree inner (non-leaf) pages (requires unswizzling, which is not supported)
           (POINTER_SWIZZLER::usesPointerSwizzling && p.tag() == t_btree_p && !p.is_leaf())
        // ... B-tree pages that have a foster child (requires unswizzling, which is not supported)
        || (POINTER_SWIZZLER::usesPointerSwizzling && p.tag() == t_btree_p && p.get_foster() != 0)
       ) {
        _evictioner->updateOnPageSwizzled(indexToCheck);
        DBG5(<< "Eviction failed on swizzled for " << indexToCheck);
        return false;
    }

    if (// ... dirty pages, unless we're told to ignore them
           (!ignore_dirty && controlBlockToCheck.is_dirty())) {
        _evictioner->updateOnPageDirty(indexToCheck);
        DBG5(<< "Eviction failed on dirty for " << indexToCheck);
        return false;
    }
    if (// ... unused frames, which don't hold a valid page
           !controlBlockToCheck._used
        // ... frames prefetched by restore but not yet restored
        || controlBlockToCheck.is_pinned_for_restore()) {
        DBG5(<< "Eviction failed on unused for " << indexToCheck);
        return false;
    }
    if (// ... pinned frames, i.e., someone required it not be evicted
           controlBlockToCheck._pin_cnt != 0) {
        _evictioner->updateOnPageBlocked(indexToCheck);
        DBG5(<< "Eviction failed on pinned for " << indexToCheck);
        return false;
    }

    return true;
}

bool BufferPool::isEvictable(const bf_idx indexToCheck, const bool doFlushIfDirty) noexcept {
    bool ignore_dirty = doFlushIfDirty || _noDBMode || _useWriteElision;

    bf_tree_cb_t& controlBlockToCheck = getControlBlock(indexToCheck);
    w_assert1(controlBlockToCheck.latch().held_by_me());
    w_assert1(controlBlockToCheck.latch().mode() != LATCH_NL);

    // We do not consider for eviction ...
    if (// ... unused buffer frames.
            !controlBlockToCheck._used) {
        return false;
    }

    btree_page_h p;
    p.fix_nonbufferpool_page(&_buffer[indexToCheck]);

    // We do not consider for eviction ...
    if (// ... the stnode page
           p.tag() == t_stnode_p
        // ... B-tree root pages (note, single-node B-tree is both root and leaf)
        || (p.tag() == t_btree_p && p.pid() == p.root())
        // ... B-tree inner (non-leaf) pages (requires unswizzling, which is not supported)
        || (POINTER_SWIZZLER::usesPointerSwizzling && p.tag() == t_btree_p && !p.is_leaf())
        // ... B-tree pages that have a foster child (requires unswizzling, which is not supported)
        || (POINTER_SWIZZLER::usesPointerSwizzling && p.tag() == t_btree_p && p.get_foster() != 0)
        // ... dirty pages, unless we're told to ignore them
        || (!ignore_dirty && controlBlockToCheck.is_dirty())
        // ... unused frames, which don't hold a valid page
        || !controlBlockToCheck._used
        // ... frames prefetched by restore but not yet restored
        || controlBlockToCheck.is_pinned_for_restore()
        // ... pinned frames, i.e., someone required it not be evicted
        || controlBlockToCheck._pin_cnt != 0) {
        return false;
    }

    return true;
}

void BufferPool::batchPrefetch(PageID startPID, bf_idx numberOfPages) noexcept {
    std::vector<generic_page*> frames(numberOfPages);

    // First grab enough free frames to read into:
    for (bf_idx i = 0; i < numberOfPages; i++) {
        bf_idx freeFrameIndex;
        while (true) {
            if (!_freeList->grabFreeBufferpoolFrame(freeFrameIndex)) {
                // There're no free frames left. -> The warmup is done!
                _setWarmupDone();

                if (_asyncEviction) {
                    // Start the asynchronous eviction and block until a page was evicted:
                    _evictioner->wakeup(true);
                } else {
                    w_assert0(_evictioner->evictOne(freeFrameIndex));
                }
            }
            w_rc_t latchStatus = getControlBlock(freeFrameIndex).latch().latch_acquire(LATCH_EX,
                                                                                       timeout_t::WAIT_IMMEDIATE);
            if (latchStatus.is_error()) {
                _evictioner->updateOnPageExplicitlyUnbuffered(freeFrameIndex);
                _freeList->addFreeBufferpoolFrame(freeFrameIndex);
            } else {
                break;
            }
        }
        frames[i] = getPage(freeFrameIndex);
    }

    // Then read into them using iovec:
    smlevel_0::vol->read_vector(startPID, numberOfPages, frames, isMediaFailure());

    // Finally, add the frames to the hash table if not already there and initialize the control blocks:
    for (bf_idx i = 0; i < numberOfPages; i++) {
        PageID pid = startPID + i;
        bf_tree_cb_t& controlBlock = getControlBlock(frames[i]);
        bf_idx index = getIndex(frames[i]);

        constexpr bf_idx parentIndex = 0;
        atomic_bf_idx_pair* indexPair = new atomic_bf_idx_pair(index, parentIndex);
        bool registered = _hashtable->tryInsert(pid, indexPair);

        if (registered) {
            controlBlock.init(pid, frames[i]->lsn);
            // controlBlock.set_check_recovery(true);

            if (isMediaFailure()) {
                controlBlock.pin_for_restore();
            }

            _evictioner->updateOnPageMiss(index, pid);
        } else {
            delete indexPair;
            _evictioner->updateOnPageExplicitlyUnbuffered(index);
            _freeList->addFreeBufferpoolFrame(index);
        }

        controlBlock.latch().latch_release();
    }
}

void BufferPool::recoverIfNeeded(bf_tree_cb_t& controlBlock, generic_page* page, bool onlyIfDirty) noexcept {
    if (!controlBlock._check_recovery || !smlevel_0::recovery) {
        return;
    }

    w_assert1(controlBlock.latch().is_mine());
    w_assert1(controlBlock.get_page_lsn() == page->lsn);

    PageID pid = controlBlock._pid;
    lsn_t expectedLSN = smlevel_0::recovery->get_dirty_page_emlsn(pid);
    if (!onlyIfDirty || (!expectedLSN.is_null() && page->lsn < expectedLSN)) {
        btree_page_h fixedPage;
        fixedPage.fix_nonbufferpool_page(page);
        constexpr bool useArchive = true;
        // CS TODO: this is required to replay a btree_split correctly
        page->pid = pid;
        _localSprIter.open(pid, page->lsn, expectedLSN, useArchive);
        _localSprIter.apply(fixedPage);
        w_assert0(page->lsn >= expectedLSN);
    }

    w_assert1(page->pid == pid);
    w_assert1(controlBlock._pid == pid);
    w_assert1(page->lsn > lsn_t::null);
    controlBlock.set_check_recovery(false);

    if (_logFetches) {
        Logger::log_sys<fetch_page_log>(pid, page->lsn, page->store);
    }
}

void BufferPool::fuzzyCheckpoint(chkpt_t& checkpoint) const noexcept {
    if (_noDBMode) {
        return;
    }

    for (bf_idx i = 1; i < _blockCount; i++) {
        const bf_tree_cb_t& controlBlock = getControlBlock(i);
        /*
         * CS: We don't latch or pin because a fuzzy checkpoint doesn't care about false positives (i.e., pages marked
         * dirty that are actually clean).  Thus, if any of the controlBlock variables changes in between, the fuzzy
         * checkpoint is still correct, because LSN updates are atomic and monotonically increasing.
         */
        if (controlBlock.is_in_use() && controlBlock.is_dirty()) {
            // There's a small time window after page_lsn is updated for the first time and before rec_lsn is set, where
            // is_dirty() returns true but rec_lsn is still null. In that case, we can use the page_lsn instead, since
            // it is what rec_lsn will be eventually set to.
            lsn_t recoveryLSN = controlBlock.get_rec_lsn();
            if (recoveryLSN.is_null()) {
                recoveryLSN = controlBlock.get_page_lsn();
            }
            checkpoint.mark_page_dirty(controlBlock._pid, controlBlock.get_page_lsn(), recoveryLSN);
        }
    }
}

void BufferPool::sxUpdateChildEMLSN(btree_page_h &parentPage, general_recordid_t childSlotID, lsn_t childEMLSN) const {
    sys_xct_section_t sxs(true); // this transaction will output only one log!
    w_rc_t startStatus = sxs.check_error_on_start();
    if (startStatus.is_error()) {
        throw BufferPoolOldStyleException(startStatus);
    }

    w_assert1(parentPage.is_latched());

    Logger::log_p<update_emlsn_log>(&parentPage, childSlotID, childEMLSN);
    parentPage.set_emlsn_general(childSlotID, childEMLSN);

    w_rc_t endStatus = sxs.end_sys_xct(RCOK);
    if (endStatus.is_error()) {
        throw BufferPoolOldStyleException(endStatus);
    }
}

void BufferPool::switchParent(PageID childPID, generic_page* newParentPage) noexcept {
#if W_DEBUG_LEVEL > 0
    // Given PID must actually be an entry in the parent
    general_recordid_t childSlotID = fixable_page_h::find_page_id_slot(newParentPage, childPID);
    w_assert1(childSlotID != GeneralRecordIds::INVALID);
#endif

    childPID = normalizePID(childPID);
    w_assert1(!POINTER_SWIZZLER::isSwizzledPointer(childPID));

    atomic_bf_idx_pair* childIndexPair = _hashtable->lookupPair(childPID);
    // If the page is not cached, there is nothing to be done in the buffer pool:
    if (!childIndexPair) {
        return;
    }

    bf_idx newParentIndex = getIndex(newParentPage);
    // CS TODO: this assertion fails when using slot 1 sometimes
    // w_assert1(newParentIndex != childIndexPair->second);
    if (newParentIndex != childIndexPair->second) {
        childIndexPair->second = newParentIndex;

        DBG5(<< "Parent of "
             << childPID
             << " updated to "
             << newParentIndex
             << " from "
             << childIndexPair->second);
    }

    // The page cannot be evicted since the first lookup because the caller latched it.
    // CS Update: Yes it can be evicted, in adoption for example, where we don't hold latch on the foster child
    // w_assert0(found);
}

void BufferPool::setMediaFailure() noexcept {
    w_assert0(smlevel_0::logArchiver);

    auto volPages = smlevel_0::vol->num_used_pages();

    constexpr bool virginPages = false;
    constexpr bool startLocked = true;
    auto segmentCount = volPages  / _batchSegmentSize + (volPages % _batchSegmentSize ? 1 : 0);
    _restoreCoordinator = std::make_shared<RestoreCoord>(_batchSegmentSize, segmentCount, SegmentRestorer::bf_restore,
                                                         virginPages, _instantRestore, startLocked);

    smlevel_0::vol->open_backup();
    lsn_t backupLSN = smlevel_0::vol->get_backup_lsn();

    _mediaFailurePID = volPages;

    // Make sure log is archived until failureLSN
    lsn_t failureLSN = Logger::log_sys<restore_begin_log>(volPages);
    ERROUT(<< "Media failure injected! Waiting for log archiver to reach LSN "
           << failureLSN);
    stopwatch_t timer;
    smlevel_0::logArchiver->archiveUntilLSN(failureLSN);
    ERROUT(<< "Failure LSN reached in "
           << timer.time()
           << " seconds");

    _restoreCoordinator->set_lsns(backupLSN, failureLSN);
    _restoreCoordinator->start();

    _backgroundRestorer = std::make_shared<BgRestorer>(_restoreCoordinator, [this] { unsetMediaFailure(); });
    _backgroundRestorer->fork();
    _backgroundRestorer->wakeup();
}

void BufferPool::unsetMediaFailure() noexcept {
    _mediaFailurePID = 0;
    // Background restorer cannot be destroyed here because it is the caller of this method via a callback. For now,
    // well just let it linger as a "zombie" thread
    // _backgroundRestorer = nullptr;
    Logger::log_sys<restore_end_log>();
    smlevel_0::vol->close_backup();
    ERROUT(<< "Restore done!");
    _restoreCoordinator = nullptr;
}

void BufferPool::debugDump(std::ostream& o) const {
    std::ios::fmtflags initialOFlags = o.flags();
    o << std::boolalpha;
    o << std::dec << std::noshowbase << std::noshowpos;
    o << std::left << std::setfill(' ') << std::noskipws << std::nouppercase;

    o << "Buffer Pool (at "
      << static_cast<const void*>(this)
      << ") Debug Dump:"
      << std::endl;
    o << "Number of buffer pool frames: "
      << _blockCount
      << std::endl;
    o << "Number of unoccupied buffer pool frames: "
      << _freeList.get()->getCount()
      << std::endl;

    o << "Buffer indexes of root pages buffered in this buffer pool (Buffer Index <- Store ID):"
      << std::endl;
    std::string listSeparator = "";
    for (StoreID store = 1; store < stnode_page::max; store++) {
        if (_rootPages[store] != 0) {
            o << listSeparator
              << std::setw(std::numeric_limits<bf_idx>::digits10 + 1)
              << _rootPages[store]
              << " <- "
              << std::setw(std::numeric_limits<StoreID>::digits10 + 1)
              << store;
            listSeparator = ", ";
        }
    }
    o << std::endl;

    o << "Buffer pool frames:"
      << std::endl;
    for (bf_idx index = 1; index < _blockCount && index < 1000; index++) {
        o << std::setw(std::numeric_limits<bf_idx>::digits10 + 1)
          << index
          << ": ";
        const bf_tree_cb_t& controlBlock = getControlBlock(index);
        if (controlBlock._used) {
            o << "PID("
              << std::setw(std::numeric_limits<PageID>::digits10 + 1)
              << controlBlock._pid
              << ")";
            o << ", dirty("
              << std::setw(5)
              << controlBlock.is_dirty()
              << ")";
            o << ", swizzled("
              << std::setw(5)
              << controlBlock._swizzled
              << ")";
            o << ", pinCount("
              << std::setw(std::numeric_limits<decltype(controlBlock._pin_cnt)>::digits10 + 1)
              << controlBlock._pin_cnt
              << ")";
            o << ", refCount("
              << std::setw(std::numeric_limits<decltype(controlBlock._ref_count)>::digits10 + 1)
              << controlBlock._ref_count
              << ")";
            o << ", refCountExclusive("
              << std::setw(std::numeric_limits<decltype(controlBlock._ref_count)>::digits10 + 1)
              << controlBlock._ref_count_ex
              << ")";
            o << ", latch(";
            controlBlock.latch().print(o);
            o << ")";
        } else {
            o << "UNUSED";
        }
        o << std::endl;
    }

    if (_blockCount >= 1000) {
        o << "  ..."
          << std::endl;
    }

    o.flags(initialOFlags);
}

void BufferPool::debugDumpPagePointers(std::ostream& o, generic_page* page) const {
    std::ios::fmtflags initialOFlags = o.flags();
    o << std::boolalpha;
    o << std::dec << std::noshowbase << std::noshowpos;
    o << std::left << std::setfill(' ') << std::noskipws << std::nouppercase;

    bf_idx pageIndex = getIndex(page);
    w_assert1(pageIndex > 0);
    w_assert1(pageIndex < _blockCount);

    o << "Page Pointer Dump of Page "
      << std::setw(std::numeric_limits<PageID>::digits10 + 1)
      << page->pid
      << " at Buffer Pool Index "
      << std::setw(std::numeric_limits<bf_idx>::digits10 + 1)
      << pageIndex
      << ": ";

    fixable_page_h fixedPage;
    fixedPage.fix_nonbufferpool_page(page);
    std::string listSeparator = "";
    for (general_recordid_t childSlot = -1; childSlot <= fixedPage.max_child_slot(); childSlot++) {
        o << listSeparator
          << std::setw(std::numeric_limits<general_recordid_t>::digits10 + 1)
          << childSlot
          << "(";
        POINTER_SWIZZLER::debugDumpPointer(o, *fixedPage.child_slot_address(childSlot));
        o << ")";
        listSeparator = ", ";
    }

    o << std::endl;

    o.flags(initialOFlags);
}

bool BufferPool::_fix(generic_page* parentPage, generic_page*& targetPage, PageID pid, latch_mode_t latchMode,
                      bool conditional, bool virgin, bool onlyIfHit, bool doRecovery, lsn_t emlsn) {
    if constexpr (POINTER_SWIZZLER::usesPointerSwizzling) {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////// The pid is Swizzled: /////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (POINTER_SWIZZLER::isSwizzledPointer(pid)) {
            w_assert1(!virgin);
            // Swizzled pointer traversal only valid with latch coupling (i.e., parent must also have been fixed)
            w_assert1(parentPage);

            bf_idx pageIndex = POINTER_SWIZZLER::makeBufferIndex(pid);
            w_assert1(isValidIndex(pageIndex));
            bf_tree_cb_t& pageControlBlock = getControlBlock(pageIndex);

            w_rc_t latchStatus = pageControlBlock.latch().latch_acquire(latchMode,
                                                                        conditional ? timeout_t::WAIT_IMMEDIATE
                                                                                    : timeout_t::WAIT_FOREVER);
            if (latchStatus.is_error()) {
                throw BufferPoolOldStyleException(latchStatus);
            }

            // CS: Normally, we must always check if pageControlBlock is still valid after latching, because page might
            // have been evicted while we were waiting for the latch. In the case of following a swizzled pointer,
            // however,that is not necessary because of latch coupling: The thread calling fix here *must* have the
            // parent latched in shared mode. Eviction, on the other hand, will only select a victim if it can acquire
            // an exclusive latch on its parent. Thus, the mere fact that we are following a swizzled pointer already
            // gives us the guarantee that the control block cannot be invalidated.

            w_assert1(pageControlBlock.is_in_use());
            w_assert1(pageControlBlock._swizzled);
            w_assert1(pageControlBlock._pid == _buffer[pageIndex].pid);

            pageControlBlock.inc_ref_count();
            _evictioner->updateOnPageHit(pageIndex);
            if (latchMode == LATCH_EX) {
                pageControlBlock.inc_ref_count_ex();
            }

            targetPage = getPage(pageIndex);

            INC_TSTAT(bf_fix_cnt);
            INC_TSTAT(bf_hit_cnt);
            _fixCount++;
            _hitCount++;

            return true;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////// The pid is not Swizzled: /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Wait for log replay before attempting to fix anything ->  noDB mode only! (for instant restore see below):
    bool noDBUsedRestore = false;
    if (_noDBMode && doRecovery && !virgin && _restoreCoordinator && !_warmupDone) {
        // Copy into local variable to avoid race condition with setting member to null:
        auto restore = _restoreCoordinator;
        if (restore) {
            restore->fetch(pid);
            noDBUsedRestore = true;
        }
    }

    while (true) {
        atomic_bf_idx_pair* pageIndexPair = _hashtable->lookupPair(pid);
        bf_idx pageIndex = 0;
        bf_idx parentIndex = 0;
        if (parentPage) {
            parentIndex = getIndex(parentPage);
        }
        if (pageIndexPair) {
            pageIndex = pageIndexPair->first;
            if (pageIndexPair->second != parentIndex) {
                // Updating the parent pointer in the hashtable is required:
                pageIndexPair->second = parentIndex;
                INC_TSTAT(bf_fix_adjusted_parent);
            }
        }
        bf_tree_cb_t* pageControlBlock = nullptr;

        // The result of calling this function determines if we'll behave in normal mode or in failure mode below, and
        // it does that in an atomic way, i.e., we can't switch from one mode to another in the middle of a fix. Note
        // that we can operate in normal mode even if a failure occurred and we missed it here, because vol_t still
        // operates normally even during the failure (remember, we just simulate a media failure)
        bool mediaFailure = isMediaFailure(pid);

        if (pageIndex == 0) {                       // page miss
            if (onlyIfHit) {
                return false;
            }

            // Wait for instant restore to restore this segment
            if (doRecovery && !virgin && mediaFailure) {
                // Copy into local variable to avoid race condition with setting member to null:
                auto restore = _restoreCoordinator;
                if (restore) {
                    restore->fetch(pid);
                }
            }

            /*
             * STEP 1: Grab a free frame to read into
             */
            if (!_freeList->grabFreeBufferpoolFrame(pageIndex)) {
                // There're no free frames left. -> The warmup is done!
                _setWarmupDone();

                if (_asyncEviction) {
                    // Start the asynchronous eviction and block until a page was evicted:
                    _evictioner->wakeup(true);
                } else {
                    w_assert0(_evictioner->evictOne(pageIndex));
                }
            }
            pageControlBlock = &getControlBlock(pageIndex);

            /*
             * STEP 2: Acquire EX latch before hashtable insert, to make sure nobody will access this page until we are
             *         done
             */
            w_rc_t latchStatus = pageControlBlock->latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
            if (latchStatus.is_error()) {
                _evictioner->updateOnPageExplicitlyUnbuffered(pageIndex);
                _freeList->addFreeBufferpoolFrame(pageIndex);
                continue;
            }

            /*
             * STEP 3: Register the page on the hash table atomically to guarantee that only one thread attempts to
             *         read the page
             */
            atomic_bf_idx_pair* pageIndexPair = new atomic_bf_idx_pair(pageIndex, parentIndex);
            bool registered = _hashtable->tryInsert(pid, pageIndexPair);
            if (!registered) {
                delete pageIndexPair;
                pageControlBlock->latch().latch_release();
                _evictioner->updateOnPageExplicitlyUnbuffered(pageIndex);
                _freeList->addFreeBufferpoolFrame(pageIndex);
                continue;
            }

            w_assert1(pageIndex != parentIndex);

            /*
             * STEP 4: Read the page from disk
             */
            targetPage = getPage(pageIndex);

            if (!virgin && !_noDBMode) {
                INC_TSTAT(bf_fix_nonroot_miss_count);

                if (parentPage && emlsn.is_null() && _maintainEMLSN) {
                    // Get EMLSN from the parent page
                    general_recordid_t recordID = fixable_page_h::find_page_id_slot(parentPage, pid);
                    btree_page_h fixedParent;
                    fixedParent.fix_nonbufferpool_page(parentPage);
                    emlsn = fixedParent.get_emlsn_general(recordID);
                }

                bool fromBackup = mediaFailure && !doRecovery;
                _readPage(pid, targetPage, fromBackup);
                pageControlBlock->init(pid, targetPage->lsn);
                if (fromBackup) {
                    pageControlBlock->pin_for_restore();
                }
            } else {
                // Initialize contents of virgin page:
                pageControlBlock->init(pid, lsn_t::null);
                std::memset(targetPage, 0, sizeof(generic_page));
                targetPage->pid = pid;

                // Only way I could think of to destroy background restorer:
                static std::atomic<bool> iShallDestroy{false};
                if (!isMediaFailure() && !_restoreCoordinator && _backgroundRestorer) {
                    bool expected = false;
                    if (iShallDestroy.compare_exchange_strong(expected, true)) {
                        _backgroundRestorer->join();
                        _backgroundRestorer = nullptr;
                    }
                }
            }

            // When a page is first fetched from storage, we always check if recovery is needed (we might not recover it
            // right now, because doRecovery might be false, i.e., due to bulk fetch with GenericPageIterator or when
            // prefetching pages).
            pageControlBlock->set_check_recovery(true);

            w_assert1(isActiveIndex(pageIndex));

            /*
             * STEP 5: Register the page in the page evictioner
             */
            _evictioner->updateOnPageMiss(pageIndex, pid);

            w_assert1(pageControlBlock->latch().is_mine());
            DBG(<< "Fixed page "
                << pid
                << " (miss) to frame "
                << pageIndex);
        } else {                                    // page hit
            pageControlBlock = &getControlBlock(pageIndex);

            // Wait for instant restore to restore this segment:
            if (doRecovery && pageControlBlock->is_pinned_for_restore()) {
                // Copy into local variable to avoid race condition with setting member to null:
                auto restore = _restoreCoordinator;
                if (restore) {
                    restore->fetch(pid);
                }
            }

            /*
             * STEP 1: Acquire latch in mode requested (or in EX if we might have to recover this page)
             */
            latch_mode_t temporaryLatchMode = pageControlBlock->_check_recovery ? LATCH_EX : latchMode;
            w_rc_t latchStatus = pageControlBlock->latch().latch_acquire(temporaryLatchMode,
                                                                         conditional ? timeout_t::WAIT_IMMEDIATE
                                                                                     : timeout_t::WAIT_FOREVER);
            if (latchStatus.is_error()) {
                throw BufferPoolOldStyleException(latchStatus);
            }

            /*
             * STEP 2: Check the control block for changes that happened while we were waiting for the latch
             */
            bool checkRecoveryChanged = pageControlBlock->_check_recovery && temporaryLatchMode == LATCH_SH;
            bool waitForRestore = doRecovery && pageControlBlock->is_pinned_for_restore();
            bool pageWasEvicted = !pageControlBlock->is_in_use() || pageControlBlock->_pid != pid;
            if (pageWasEvicted || checkRecoveryChanged || waitForRestore) {
                pageControlBlock->latch().latch_release();
                continue;
            }

            targetPage = getPage(pageIndex);

            _evictioner->updateOnPageHit(pageIndex);

            w_assert1(pageControlBlock->latch().held_by_me());
            w_assert1(!doRecovery || !pageControlBlock->is_pinned_for_restore());
            w_assert1(!pageControlBlock->_check_recovery || pageControlBlock->latch().is_mine());
            DBG(<< "Fixed page "
                << pid
                << " (hit) to frame "
                << pageIndex);

            INC_TSTAT(bf_hit_cnt);
            _hitCount++;
        }

        INC_TSTAT(bf_fix_cnt);
        _fixCount++;

        _checkWarmupDone();

        // Pin the page:
        w_assert1(isActiveIndex(pageIndex));
        pageControlBlock->inc_ref_count();
        if (latchMode == LATCH_EX) {
            pageControlBlock->inc_ref_count_ex();
        }

        // w_assert1(pageControlBlock->_pid == pid);
        // w_assert1(targetPage->pid == pid);
        // w_assert1(targetPage->lsn == pageControlBlock->get_page_lsn());

        if (doRecovery) {
            if (virgin) {
                pageControlBlock->set_check_recovery(false);
            } else {
                recoverIfNeeded(*pageControlBlock, targetPage, !noDBUsedRestore);
            }
        }
        w_assert1(pageControlBlock->_pin_cnt >= 0);

        // Downgrade the latch if necessary:
        if (pageControlBlock->latch().mode() != latchMode) {
            w_assert1(latchMode == LATCH_SH && pageControlBlock->latch().mode() == LATCH_EX);
            pageControlBlock->latch().downgrade();
        }

        // Swizzle the pointer inside the parent page if necessary:
        if constexpr (POINTER_SWIZZLER::usesPointerSwizzling) {
            if (!pageControlBlock->_swizzled && parentPage) {
                bf_tree_cb_t &parentControlBlock = getControlBlock(parentIndex);
                if (!parentControlBlock._swizzled) {
                    return true;
                }
                w_assert1(!pageControlBlock->_check_recovery);
                w_assert1(!parentControlBlock._check_recovery);

                // Get slot on parent page:
                w_assert1(isActiveIndex(parentIndex));
                w_assert1(parentControlBlock.latch().mode() != LATCH_NL);
                fixable_page_h fixedParentPage;
                fixedParentPage.fix_nonbufferpool_page(parentPage);
                general_recordid_t childSlot = fixable_page_h::find_page_id_slot(parentPage, pid);

                // Either a virgin page which has not been linked yet, or some other thread won the race and already
                // swizzled the pointer:
                if (childSlot == GeneralRecordIds::INVALID) {
                    return true;
                }
                // Not worth swizzling foster children, since they will soon be adopted (an thus unswizzled):
                if (childSlot == GeneralRecordIds::FOSTER_CHILD) {
                    return true;
                }
                w_assert1(childSlot > GeneralRecordIds::FOSTER_CHILD);
                w_assert1(childSlot <= fixedParentPage.max_child_slot());

                // Update _swizzled flag atomically:
                bool old_value = false;
                if (!std::atomic_compare_exchange_strong(&pageControlBlock->_swizzled, &old_value, true)) {
                    // CAS failed -- some other thread is swizzling
                    return true;
                }
                w_assert1(pageControlBlock->_swizzled);

                // Replace pointer with swizzled version:
                PageID *childPID = fixedParentPage.child_slot_address(childSlot);
                *childPID = POINTER_SWIZZLER::makeSwizzledPointer(pageIndex);
                _evictioner->updateOnPointerSwizzling(pageIndex);
                w_assert1(isActiveIndex(pageIndex));
                w_assert1(fixable_page_h::find_page_id_slot(parentPage,
                                                            POINTER_SWIZZLER::makeSwizzledPointer(pageIndex))
                       != GeneralRecordIds::INVALID);
            }
        }

        return true;
    }
}

void BufferPool::_convertToDiskPage(generic_page* page) const noexcept {
    if constexpr (POINTER_SWIZZLER::usesPointerSwizzling) {
        fixable_page_h fixedPage;
        fixedPage.fix_nonbufferpool_page(page);

        for (general_recordid_t recordID = GeneralRecordIds::FOSTER_CHILD; recordID <= fixedPage.max_child_slot(); recordID++) {
            PageID* pid = fixedPage.child_slot_address(recordID);
            if (POINTER_SWIZZLER::isSwizzledPointer(*pid)) {
                // CS TODO: Slot 1 (which is actually 0 in the internal page representation) is not used sometimes (I
                // think when a page is first created?) so we must skip it manually here to avoid getting an invalid
                // page below.
                if (recordID == 1 && !isActiveIndex(POINTER_SWIZZLER::makeBufferIndex(*pid))) {
                    continue;
                } else {
                    *pid = normalizePID(*pid);
                }
            }
        }
    }
}

void BufferPool::_readPage(PageID pid, generic_page* targetPage, bool fromBackup) {
    bf_tree_cb_t& targetControlBlock = getControlBlock(getIndex(targetPage));
    w_assert1(targetControlBlock.latch().is_mine());

    if (fromBackup) {
        smlevel_0::vol->read_backup(pid, 1, targetPage);
    } else {
        w_rc_t readStatus = smlevel_0::vol->read_page(pid, targetPage);

        if (readStatus.is_error()) {
            _hashtable->erase(pid);
            targetControlBlock.latch().latch_release();
            _evictioner->updateOnPageExplicitlyUnbuffered(getIndex(targetPage));
            _freeList->addFreeBufferpoolFrame(getIndex(targetPage));
            throw BufferPoolOldStyleException(readStatus);
        }
    }
}

void BufferPool::_deletePage(bf_idx index) noexcept {
    w_assert1(isActiveIndex(index));
    bf_tree_cb_t& controlBlock = getControlBlock(index);
    w_assert1(controlBlock._pin_cnt == 0);
    w_assert1(!controlBlock.latch().is_latched());
    w_assert1(!controlBlock._swizzled);
    controlBlock._used = false; // clear _used BEFORE _dirty so that eviction thread will ignore this block.

    DBGOUT1(<<"delete block: remove page pid = "
            << controlBlock._pid);
    _hashtable->erase(controlBlock._pid);

    _evictioner->updateOnPageExplicitlyUnbuffered(index);
    _freeList->addFreeBufferpoolFrame(index);
}

void BufferPool::_checkWarmupDone() noexcept {
    // if warm-up hit ratio is 100%, we don't even bother
    if (!_warmupDone && _warmupHitRatio < 1.0 && _fixCount > _warmupMinFixes) {
        double hitRatio = static_cast<double>(_hitCount) / _fixCount;
        if (hitRatio > _warmupHitRatio) {
            _setWarmupDone();
        }
    }
}

void BufferPool::_setWarmupDone() noexcept {
    // CS: no CC needed, threads can race on blind updates and visibility not an issue
    if (!_warmupDone) {
        _warmupDone = true;
        _restoreCoordinator = nullptr;
        Logger::log_sys<warmup_done_log>();

        // Start background recovery after warm-up, in order to not interfere with on-demand recovery.
        if (smlevel_0::recovery && smlevel_0::recovery->isInstant()) {
            smlevel_0::recovery->wakeup();
        }
    }
}

void pin_for_refix_holder::release() {
    if (_idx != 0) {
        smlevel_0::bf->unpinForRefix(_idx);
        _idx = 0;
    }
}
