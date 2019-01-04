#include "page_evictioner.hpp"

#include "buffer_pool.hpp"
#include "log_core.h"
#include "xct_logger.h"
#include "btree_page_h.h"

using namespace zero::buffer_pool;

PageEvictioner::PageEvictioner(const BufferPool* bufferPool) :
        worker_thread_t(ss_m::get_options().get_int_option("sm_evictioner_interval_millisec", 1000)),
        _evictionBatchSize(static_cast<uint_fast32_t>(bufferPool->getBlockCount() * 0.000001 * ss_m::get_options().get_int_option("sm_evictioner_batch_ratio_ppm", 10000))),
        _enabledSwizzling(ss_m::get_options().get_bool_option("sm_bufferpool_swizzle", false)),
        _maintainEMLSN(ss_m::get_options().get_bool_option("sm_bf_maintain_emlsn", false)),
        _flushDirty(ss_m::get_options().get_bool_option("sm_bf_evictioner_flush_dirty_pages", false)),
        _logEvictions(ss_m::get_options().get_bool_option("sm_bf_evictioner_log_evictions", false)),
        _maxAttempts(1000 * bufferPool->getBlockCount()) {}

PageEvictioner::~PageEvictioner() {}

bool PageEvictioner::evictOne(bf_idx victim) {
    bf_tree_cb_t& victimControlBlock = smlevel_0::bf->getControlBlock(victim);
    w_assert1(victimControlBlock.latch().is_mine());

    // Try to unswizzle/update the parent of the victim.
    // Proceeding with this victim is not possible if this fails so another victim is tried.
    if (!unswizzleAndUpdateEMLSN(victim)) {
        victimControlBlock.latch().latch_release();
        return false;
    }

    // Try to atomically decrement the pin count of the page from 0 to -1.
    // Proceeding with this victim is not possible if this fails so another victim is tried.
    if (!victimControlBlock.prepare_for_eviction()) {
        victimControlBlock.latch().latch_release();
        return false;
    }

    // POINT OF NO RETURN: The eviction of the victim must happen -- no matter what!

    /* Check if the victim page needs to be flushed.
     * The function is_dirty() acquires the victim's latch in SH mode which always succeeds as this thread already holds
     * this latch in EX mode. */
    bool wasDirty = false;
    if (_flushDirty && victimControlBlock.is_dirty()) {
        flushDirtyPage(victimControlBlock);
        wasDirty = true;
    }
    w_assert1(victimControlBlock.latch().is_mine());

    /* If evictions should be logged, do this now. */
    if (_logEvictions) {
        Logger::log_sys<evict_page_log>(victimControlBlock._pid, wasDirty, victimControlBlock.get_page_lsn());
    }

    /* If NoDB is used, add the dirty pages  */
    if (smlevel_0::bf->isNoDBMode()) {
        smlevel_0::recovery->add_dirty_page(victimControlBlock._pid, victimControlBlock.get_page_lsn());
        if (victimControlBlock.get_page_lsn() == lsn_t::null) {
            // MG TODO: Throw Exception
        }
    }

    w_assert1(victimControlBlock._pin_cnt < 0);
    w_assert1(!victimControlBlock._used);

    /* Remove the page's entry from the hashtable. */
    smlevel_0::bf->getHashtable()->erase(victimControlBlock._pid);

    DBG2(<< "EVICTED page " << victimControlBlock._pid << " from bufferpool frame " << victim << ". "
         << "Log Tail: 0" << smlevel_0::log->curr_lsn());

    /* The eviction has completed and therefore the latch to the bufferpool frame can be released. */
    victimControlBlock.latch().latch_release();

//    if (smlevel_0::bf->is_no_db_mode()) {
//        lsn_t lsn = smlevel_0::recovery->get_dirty_page_emlsn(victimControlBlock._pid);
//        w_assert0(!lsn.is_null());
//    }

    INC_TSTAT(bf_evict);
    return true;
}

bool PageEvictioner::unswizzleAndUpdateEMLSN(bf_idx victim) noexcept {
    // If this function got nothing to do, the work is done at this place.
    if (!_maintainEMLSN && !_enabledSwizzling) {
        return true;
    }

    // Get the control block of the latched victim buffer pool frame.
    bf_tree_cb_t& victimControlBlock = smlevel_0::bf->getControlBlock(victim);
    w_assert1(victimControlBlock.latch().is_mine());

    //==================================================================================================================
    // STEP 1: Lookup the parent page and the record slot corresponding to the victim.
    //==================================================================================================================
    // Lookup the victim's hashtable entry.
    PageID victimPageID = victimControlBlock._pid;
    bf_idx_pair victimPair = *(smlevel_0::bf->getHashtable()->lookupPair(victimPageID));
    w_assert1(victim == victimPair.first);

    /* The bufferpool frame index of the parent page is in the second element of the victim's hashtable entry. */
    bf_idx parent = victimPair.second;

    /* If write elision is disabled and if the page is not swizzled, having no parent page ís okay. */
    if (parent == 0) {
        return !smlevel_0::bf->usesWriteElision() && !victimControlBlock._swizzled;
    }

    /* Get the control block of the parent page and latch it in EX mode as a change of it is needed. The use of uncon-
     * ditional latching for the parent page's bufferpool frame is necessary to prevent deadlocks with other threads
     * waiting for the eviction mutex. */
    bf_tree_cb_t& parentControlBlock = smlevel_0::bf->getControlBlock(parent);
    rc_t parentLatchReturnCode = parentControlBlock.latch().latch_acquire(LATCH_EX, timeout_t::WAIT_IMMEDIATE);
    if (parentLatchReturnCode.is_error()) {
        return false;
    }
    w_assert1(parentControlBlock.latch().is_mine() && parentControlBlock.latch().mode() == LATCH_EX);

    /* Get the parent page. */
    w_assert1(smlevel_0::bf->isActiveIndex(parent));
    generic_page* parentPage = smlevel_0::bf->getPage(parent);

    /* Get the slot of the record slot ID of the victim page within its parent page which either contains the victim's
     * page ID or its swizzled bufferpool frame index. */
    general_recordid_t victimSlotID;
    if (_enabledSwizzling && victimControlBlock._swizzled) {
        PageID swizzledVictimPageID = parent | swizzledPIDBit;
        victimSlotID = fixable_page_h::find_page_id_slot(parentPage, swizzledVictimPageID);
    } else {
        victimSlotID = fixable_page_h::find_page_id_slot(parentPage, victimPageID);
    }
    w_assert1(victimSlotID != GeneralRecordIds::INVALID);

    //==================================================================================================================
    // STEP 2: Unswizzle pointer on parent before evicting.
    //==================================================================================================================
    if (_enabledSwizzling & victimControlBlock._swizzled) {
        bool successfullyUnswizzled = smlevel_0::bf->unswizzlePagePointer(parentPage, victimSlotID);
        if (!successfullyUnswizzled) {
            // MG TODO: Throw Exception
        } else {
            w_assert1(!victimControlBlock._swizzled);
        }
    }

    //==================================================================================================================
    // STEP 3: Page will be evicted -- update EMLSN on parent.
    //==================================================================================================================
    /* Get the B-Tree page handle of the parent page and the victim page. */
    btree_page_h parentPageHandle;
    parentPageHandle.fix_nonbufferpool_page(parentPage);
    generic_page* victimPage = smlevel_0::bf->getPage(victim);

    /* Update the LSN within the victim page. */
    lsn_t oldVictimLSN = parentPageHandle.get_emlsn_general(victimSlotID);
    lsn_t newVictimLSN = victimControlBlock.get_page_lsn();
    victimPage->lsn = newVictimLSN;

    /* Update the victim's EMLSN in the parent page. */
    if (_maintainEMLSN && oldVictimLSN < newVictimLSN) {
        DBG3(<< "Updated EMLSN on page " << parentPageHandle.pid()
                     << ": slot=" << victimSlotID
                     << ", (child pid=" << victimPID << ")"
                     << ", OldEMLSN=" << oldVictimLSN
                     << ", NewEMLSN=" << newVictimLSN);

        w_assert1(parentControlBlock.latch().is_mine());

        smlevel_0::bf->sxUpdateChildEMLSN(parentPageHandle, victimSlotID, newVictimLSN);

        w_assert1(parentPageHandle.get_emlsn_general(victimSlotID) == smlevel_0::bf->getPage(victim)->lsn);
    }

    /* The EMLSN in the parent page has been updated and the pointer in the parent's slot has been unswizzled and
     * therefore the latch to the parent's bufferpool frame can be released. */
    parentControlBlock.latch().latch_release();

    return true;
}

void PageEvictioner::flushDirtyPage(const bf_tree_cb_t& victimControlBlock) noexcept {
    /* Straight-forward write -- no need to do it asynchronously or worry about any race condition. We hold the latch in
     * EX mode and the entry in the hashtable has not been removed yet. Any thread attempting to fix the victim will be
     * waiting for the latch, after which it will notice that the control block's pin count is -1, which means it must
     * try the fix again. */
    generic_page* victimPage = smlevel_0::bf->getPage(smlevel_0::bf->getIndex(victimControlBlock));
    W_COERCE(smlevel_0::vol->write_page(victimControlBlock._pid, victimPage));
    smlevel_0::vol->sync();

    /* Log the write operation so that the log analysis recognizes the flushed page as clean. CleanVictimLSN cannot be
     * the victimLSN, otherwise the page is considered dirty, so simply use any LSN above that (cleanVictimLSN does not
     * have to be of a valid log record). */
    lsn_t cleanVictimLSN = victimPage->lsn + 1;
    Logger ::log_sys<page_write_log>(victimControlBlock._pid, cleanVictimLSN, 1);
}

void PageEvictioner::do_work() {
    while (smlevel_0::bf->getFreeList()->getCount() < _evictionBatchSize) {
        bf_idx victim = pickVictim();

        if (evictOne(victim)) {
            smlevel_0::bf->getFreeList()->addFreeBufferpoolFrame(victim);
        }

        notify_one();

        if (should_exit()) {
            break;
        }
    }
}
