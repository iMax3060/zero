#include "buffer_pool_pointer_swizzling.hpp"

#include "buffer_pool.hpp"
#include "btree_page_h.h"

using namespace zero::buffer_pool;

void NoSwizzling::debugDumpPointer(std::ostream& o, PageID pid) {
    o << "normal(page("
      << pid
      << "))";
}

general_recordid_t SimpleSwizzling::findSwizzledPageIDSlot(generic_page* page, bf_idx index) {
    return fixable_page_h::find_page_id_slot(page, makeSwizzledPointer(index));
}

void SimpleSwizzling::debugDumpPointer(std::ostream& o, PageID pid) {
    if (isSwizzledPointer(pid)) {
        bf_idx index = makeBufferIndex(pid);
        o << "swizzled(bf_idx("
          << index;
        o << "), page("
          << smlevel_0::bf->getControlBlock(index)._pid
          << "))";
    } else {
        NoSwizzling::debugDumpPointer(o, pid);
    }
}

bf_idx SimpleSwizzling::makeBufferIndexForFix(generic_page *parentPage, generic_page *&targetPage, PageID pid) {
    return makeBufferIndex(pid);
}

general_recordid_t LeanStoreSwizzling::findSwizzledPageIDSlot(generic_page* page, bf_idx index) {
    general_recordid_t pageSlotID = fixable_page_h::find_page_id_slot(page, makeSwizzledPointer(index));
    if (pageSlotID == GeneralRecordIds::INVALID) {
        pageSlotID = fixable_page_h::find_page_id_slot(page, makeCoolingPointerFromBufferIndex(index));
    }
    return pageSlotID;
}

void LeanStoreSwizzling::debugDumpPointer(std::ostream& o, PageID pid) {
    if (isHotPointer(pid)) {
        bf_idx index = makeBufferIndex(pid);
        o << "hot(bf_idx("
          << index;
        o << "), page("
          << smlevel_0::bf->getControlBlock(index)._pid
          << "))";
    } else if (isSwizzledPointer(pid)) {
        bf_idx index = makeBufferIndex(pid);
        o << "cooling(bf_idx("
          << index;
        o << "), page("
          << smlevel_0::bf->getControlBlock(index)._pid
          << "))";
    } else {
        NoSwizzling::debugDumpPointer(o, pid);
    }
}

bf_idx LeanStoreSwizzling::makeBufferIndexForFix(generic_page *parentPage, generic_page *&targetPage, PageID pid) {
    if (isHotPointer(pid)) {
        return makeBufferIndex(pid);
    } else {
        /*
         * STEP 1: Set the hot Bit in the PageID inside the parent page
         */
        bf_idx pageIndex = makeBufferIndex(pid);
        w_assert1(smlevel_0::bf->isValidIndex(pageIndex));
        bf_tree_cb_t& pageControlBlock = smlevel_0::bf->getControlBlock(pageIndex);
        bf_idx parentIndex = smlevel_0::bf->getIndex(parentPage);
        bf_tree_cb_t& parentControlBlock = smlevel_0::bf->getControlBlock(parentIndex);
        w_assert1(!pageControlBlock._check_recovery);
        w_assert1(!parentControlBlock._check_recovery);

        // Get slot on parent page:
        w_assert1(smlevel_0::bf->isActiveIndex(parentIndex));
        w_assert1(parentControlBlock.latch().mode() != LATCH_NL);
        fixable_page_h fixedParentPage;
        fixedParentPage.fix_nonbufferpool_page(parentPage);
        general_recordid_t childSlot = fixable_page_h::find_page_id_slot(parentPage, pid);

        w_assert1(childSlot <= fixedParentPage.max_child_slot());

        // Replace pointer with swizzled version:
        PageID* childPID = fixedParentPage.child_slot_address(childSlot);
        *childPID = makeSwizzledPointer(pageIndex);
        w_assert1(smlevel_0::bf->isActiveIndex(pageIndex));
        w_assert1(fixable_page_h::find_page_id_slot(parentPage, makeSwizzledPointer(pageIndex))
               != GeneralRecordIds::INVALID);

        /*
         * STEP 2: Remove the requested page from cooling stage
         */
        // TODO

        return pageIndex;
    }
}
