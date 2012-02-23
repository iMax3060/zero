#include "w_defines.h"

/**
 * Implementation of insert/remove/alloc functions in btree_impl.h.
 * Other functions are defined in btree_impl_xxx.cpp.
 */

#define SM_SOURCE
#define BTREE_C

#include "sm_int_2.h"
#include "btree_p.h"
#include "btree_impl.h"
#include "btcursor.h"
#include "crash.h"
#include "xct.h"
#include <vector>

rc_t
btree_impl::_ux_insert(
    const lpid_t&        root,
    const w_keystr_t&    key,
    const cvec_t&        el)
{
    FUNC(btree_impl::_ux_insert);
    INC_TSTAT(bt_insert_cnt);
    while (true) {
        rc_t rc = _ux_insert_core (root, key, el);
        if (rc.is_error() && rc.err_num() == eLOCKRETRY) {
            continue;
        }
        return rc;
    }
    return RCOK;
}
rc_t
btree_impl::_ux_insert_core(
    const lpid_t&        root,
    const w_keystr_t&    key,
    const cvec_t&        el)
{

    // find the leaf (potentially) containing the key
    btree_p       leaf;
    W_DO( _ux_traverse(root, key, t_fence_contain, LATCH_EX, leaf));
    w_assert1( leaf.is_fixed());
    w_assert1( leaf.is_leaf());
    w_assert1( leaf.latch_mode() == LATCH_EX);
    
    bool need_lock = g_xct_does_need_lock();
    
    // check if the same key already exists
    slotid_t slot;
    bool found;
    leaf.search_leaf(key, found, slot);
    bool alreay_took_XN = false;
    if (found) {
        // found! then we just lock the key (XN)
        if (need_lock) {
            W_DO (_ux_lock_key(leaf, key, LATCH_EX, XN, false));
            alreay_took_XN = true;
        }

        bool is_ghost = leaf.is_ghost(slot);
        
        //If the same key exists and non-ghost, exit with error (duplicate).
        if (!is_ghost) {
            return RC(eDUPLICATE);
        }

        // if the ghost record is enough spacious, we can just reuse it
        if (leaf._is_enough_spacious_ghost (key, slot, el)) {
            W_DO(leaf.replace_ghost(key, el));
            return RCOK;
        }
    }
    
    // then, we need to create (or expand) a ghost record for this key as a preparation to insert.
    // first, make sure this page is enough spacious (a bit conservative test).
    while (!leaf.check_space_for_insert_leaf(key, el)
        || (leaf.is_insertion_extremely_skewed_right() && leaf.check_chance_for_norecord_split(key))) {
        // See if there's room for the insert.
        // here, we start system transaction to split page
        lpid_t new_page_id;
        W_DO( _sx_split_blink(leaf, new_page_id, key) );
        
        // after split, should the old page contain the new tuple?
        if (!leaf.fence_contains(key)) {
            // if not, we should now insert to the new page.
            // because "leaf" is EX locked beforehand, no one
            // can have any latch on the new page, so we can always get this latch
            btree_p another_leaf; // latch coupling
            W_DO( another_leaf.fix(new_page_id, LATCH_EX) );
            w_assert1(another_leaf.is_fixed());                        
            w_assert2(another_leaf.fence_contains(key));
            leaf.unfix();
            leaf = another_leaf;
            w_assert2( leaf.is_fixed());
        }
    } // check for need to split

    // now we are sure the current page is enough spacious in any cases
    if (!found) {
        // corresponding ghost record didn't exist even before split.
        // so, it surely doesn't exist. we just create a new ghost record
        // by system transaction.
        
        if (need_lock) {
            W_DO(_ux_lock_range(leaf, key, -1, // search again because it might be split
                LATCH_EX, XN, NX, true)); // this lock "goes away" once it's taken
        }
        
        // so far deferring is disabled
        W_DO (_sx_reserve_ghost(leaf, key, el.size(), false));
    }
    
    // now we know the page has the desired ghost record. let's just replace it.
    if (need_lock && !alreay_took_XN) { // if "expand" case, do not need to get XN again
        W_DO (_ux_lock_key(leaf, key, LATCH_EX, XN, false));
    }
    W_DO(leaf.replace_ghost(key, el));

    return RCOK;
}

rc_t btree_impl::_sx_reserve_ghost(btree_p &leaf, const w_keystr_t &key, int elem_len, bool defer_apply)
{
    FUNC(btree_impl::_sx_reserve_ghost);
    sys_xct_section_t sxs (true); // this transaction will output only one log!
    W_DO(sxs.check_error_on_start());
    rc_t ret = _ux_reserve_ghost_core(leaf, key, elem_len, defer_apply);
    W_DO (sxs.end_sys_xct (ret));
    return ret;
}

rc_t btree_impl::_ux_reserve_ghost_core(btree_p &leaf, const w_keystr_t &key, int elem_len, bool defer_apply)
{
    w_assert1 (xct()->is_sys_xct());
    w_assert1 (leaf.fence_contains(key));
    size_t rec_size = key.get_length_as_keystr() - leaf.get_prefix_length()
        + elem_len + sizeof(int16_t) * 2;
    w_assert1 (leaf.usable_space() >= btree_p::slot_sz + rec_size);

    if (defer_apply) {
        W_DO (log_btree_ghost_reserve (leaf, key, rec_size));
    } else {
        // so far deferring is disabled
        // ssx_defer_section_t ssx_defer (&leaf); // auto-commit for deferred ssx log on leaf
        W_DO (log_btree_ghost_reserve (leaf, key, rec_size));
        leaf.reserve_ghost(key, rec_size);
    }
    return RCOK;
}

rc_t
btree_impl::_ux_update(const lpid_t &root, const w_keystr_t &key, const cvec_t &el)
{
    while (true) {
        rc_t rc = _ux_update_core (root, key, el);
        if (rc.is_error() && rc.err_num() == eLOCKRETRY) {
            continue;
        }
        return rc;
    }
    return RCOK;
}

rc_t
btree_impl::_ux_update_core(const lpid_t &root, const w_keystr_t &key, const cvec_t &el)
{
    bool need_lock = g_xct_does_need_lock();
    btree_p         leaf;

    // find the leaf (potentially) containing the key
    W_DO( _ux_traverse(root, key, t_fence_contain, LATCH_EX, leaf));

    w_assert3(leaf.is_fixed());
    w_assert3(leaf.is_leaf());

    slotid_t       slot = -1;
    bool            found = false;
    leaf.search(key, found, slot);

    if(!found) {
        if (need_lock) {
            // re-latch mode is SH because this is "not-found" case.
            W_DO(_ux_lock_range(leaf, key, slot,
                        LATCH_SH, XN, NS, false));
        }
        return RC(eNOTFOUND);
    }
    
    // it's found (whether it's ghost or not)! so, let's just
    // lock the key.
    if (need_lock) {
        // only the key is locked (XN)
        W_DO (_ux_lock_key(leaf, key, LATCH_EX, XN, false));
    }

    // get the old data and log
    bool ghost;
    const char *old_el;
    smsize_t old_elen;
    leaf.dat_leaf_ref(slot, old_el, old_elen, ghost);
    // it might be ghost..
    if (ghost) {
        return RC(eNOTFOUND);
    }
    
    // are we expanding?
    if (old_elen < el.size()) {
        if (!leaf.check_space_for_insert_leaf(key, el)) {
            // this page needs split. As this is a rare case,
            // we just call remove and then insert to simplify the code
            W_DO(_ux_remove(root, key));
            W_DO(_ux_insert(root, key, el));
            return RCOK;
        }
    }
    
    W_DO(log_btree_update (leaf, key, old_el, old_elen, el));
    
    W_DO(leaf.replace_el_nolog(slot, el));
    return RCOK;
}

rc_t btree_impl::_ux_overwrite(
        const lpid_t&                     root,
        const w_keystr_t&                 key,
        const char *el, smsize_t offset, smsize_t elen)
{
    while (true) {
        rc_t rc = _ux_overwrite_core (root, key, el, offset, elen);
        if (rc.is_error() && rc.err_num() == eLOCKRETRY) {
            continue;
        }
        return rc;
    }
    return RCOK;
}

rc_t btree_impl::_ux_overwrite_core(
        const lpid_t&                     root,
        const w_keystr_t&                 key,
        const char *el, smsize_t offset, smsize_t elen)
{
    // basically same as ux_update
    bool need_lock = g_xct_does_need_lock();
    btree_p         leaf;

    W_DO( _ux_traverse(root, key, t_fence_contain, LATCH_EX, leaf));

    w_assert3(leaf.is_fixed());
    w_assert3(leaf.is_leaf());

    slotid_t       slot = -1;
    bool            found = false;
    leaf.search(key, found, slot);

    if(!found) {
        if (need_lock) {
            W_DO(_ux_lock_range(leaf, key, slot,
                        LATCH_SH, XN, NS, false));
        }
        return RC(eNOTFOUND);
    }
    
    if (need_lock) {
        W_DO (_ux_lock_key(leaf, key, LATCH_EX, XN, false));
    }

    // get the old data and log
    bool ghost;
    const char *old_el;
    smsize_t old_elen;
    leaf.dat_leaf_ref(slot, old_el, old_elen, ghost);
    if (ghost) {
        return RC(eNOTFOUND);
    }
    if (old_elen < offset + elen) {
        return RC(eRECWONTFIT);
    }
    
    W_DO(log_btree_overwrite (leaf, key, old_el, el, offset, elen));    
    leaf.overwrite_el_nolog(slot, offset, el, elen);
    return RCOK;
}

rc_t
btree_impl::_ux_remove(const lpid_t &root, const w_keystr_t &key)
{
    FUNC(btree_impl::_ux_remove);
    INC_TSTAT(bt_remove_cnt);
    while (true) {
        rc_t rc = _ux_remove_core (root, key);
        if (rc.is_error() && rc.err_num() == eLOCKRETRY) {
            continue;
        }
        return rc;
    }
    return RCOK;
}

rc_t
btree_impl::_ux_remove_core(const lpid_t &root, const w_keystr_t &key)
{
    bool need_lock = g_xct_does_need_lock();
    btree_p         leaf;

    // find the leaf (potentially) containing the key
    W_DO( _ux_traverse(root, key, t_fence_contain, LATCH_EX, leaf));

    w_assert3(leaf.is_fixed());
    w_assert3(leaf.is_leaf());

    slotid_t       slot = -1;
    bool            found = false;
    leaf.search(key, found, slot);

    if(!found) {
        if (need_lock) {
            // re-latch mode is SH because this is "not-found" case.
            W_DO(_ux_lock_range(leaf, key, slot,
                        LATCH_SH, XN, NS, false));
        }
        return RC(eNOTFOUND);
    }
    
    // it's found (whether it's ghost or not)! so, let's just
    // lock the key.
    if (need_lock) {
        // only the key is locked (XN)
        W_DO (_ux_lock_key(leaf, key, LATCH_EX, XN, false));
    }
    
    // it might be already ghost..
    if (leaf.is_ghost(slot)) {
        return RC(eNOTFOUND);
    }

    // log first
    vector<slotid_t> slots;
    slots.push_back(slot);
    W_DO(log_btree_ghost_mark (leaf, slots));
    
    // then mark it as ghost
    leaf.mark_ghost (slot);
    return RCOK;
}

rc_t
btree_impl::_ux_undo_ghost_mark(const lpid_t &root, const w_keystr_t &key)
{
    FUNC(btree_impl::_ux_undo_ghost_mark);
    w_assert1(key.is_regular());
    btree_p         leaf;
    W_DO( _ux_traverse(root, key, t_fence_contain, LATCH_EX, leaf));
    w_assert3(leaf.is_fixed());
    w_assert3(leaf.is_leaf());

    slotid_t       slot = -1;
    bool            found = false;
    leaf.search(key, found, slot);

    if(!found) {
        return RC(eNOTFOUND);
    }
    leaf.unmark_ghost (slot);
    return RCOK;
}

uint8_t btree_impl::s_ex_need_counts[1 << btree_impl::GAC_HASH_BITS];
uint8_t btree_impl::s_foster_children_counts[1 << btree_impl::GAC_HASH_BITS];
queue_based_lock_t btree_impl::s_ex_need_mutex[1 << GAC_HASH_BITS];
