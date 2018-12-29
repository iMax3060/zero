/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#include "fixable_page_h.h"

#define SM_SOURCE
#include "sm_base.h"

#include "btree_logrec.h"
#include "restart.h"
#include "logrec.h"
#include "buffer_pool.hpp"
#include "xct_logger.h"
#include "alloc_cache.h"


int fixable_page_h::force_Q_fixing = 0;  // <<<>>>

void check_page_tags(generic_page* s)
{
    // CS: these fail during restore, which might fix pages with garbage content
    // from the backup before replaying a page-image format log record
    // w_assert1(s->tag != t_alloc_p  || (s->pid % alloc_cache_t::extent_size == 0));
    // w_assert1(s->tag != t_stnode_p || s->pid == stnode_page::stpid);
    // w_assert1(s->tag != t_btree_p  || (s->pid != stnode_page::stpid &&
    //             (s->pid % alloc_cache_t::extent_size > 0)));
    // w_assert1(s->tag == t_alloc_p || s->tag == t_stnode_p || s->tag == t_btree_p);
}

void fixable_page_h::unfix(bool evict)
{
    if (_pp) {
        check_page_tags(_pp);
        if (_bufferpool_managed) {
            smlevel_0::bf->unfix(_pp, evict);
        }
        _pp   = nullptr;
        _mode = LATCH_NL;
    }
}

w_rc_t fixable_page_h::fix_nonroot(const fixable_page_h &parent,
                                   PageID shpid, latch_mode_t mode,
                                   bool conditional, bool virgin_page, bool only_if_hit)
{
    w_assert1(parent.is_fixed());
    w_assert1(mode != LATCH_NL);

    unfix();
    W_DO(smlevel_0::bf->fixNonRootOldStyleExceptions(_pp, parent._pp, shpid, mode, conditional, virgin_page,
                                                     only_if_hit));
    w_assert1(smlevel_0::bf->isSwizzledPointer(shpid) || smlevel_0::bf->getControlBlock(_pp)._pid == shpid);
    if (!virgin_page) { check_page_tags(_pp); }
    _bufferpool_managed = true;
    _mode               = mode;

    return RCOK;
}

w_rc_t fixable_page_h::fix_direct(PageID shpid, latch_mode_t mode,
                                   bool conditional, bool virgin_page,
                                   bool only_if_hit, bool do_recovery)
{
    w_assert1(mode != LATCH_NL);

    unfix();

    W_DO(smlevel_0::bf->fixNonRootOldStyleExceptions(_pp, nullptr, shpid, mode, conditional, virgin_page, only_if_hit,
                                                     do_recovery));

    w_assert1(smlevel_0::bf->isSwizzledPointer(shpid) || smlevel_0::bf->getControlBlock(_pp)._pid == shpid);
    if (!virgin_page) { check_page_tags(_pp); }

    _bufferpool_managed = true;
    _mode               = mode;

    return RCOK;
}

bf_idx fixable_page_h::pin_for_refix() {
    w_assert1(_bufferpool_managed);
    w_assert1(is_latched());
    return smlevel_0::bf->pinForRefix(_pp);
}

w_rc_t fixable_page_h::refix_direct (bf_idx idx, latch_mode_t mode, bool conditional) {
    w_assert1(idx != 0);
    w_assert1(mode != LATCH_NL);

    unfix();
    W_DO(smlevel_0::bf->refixDirectOldSytleExceptions(_pp, idx, mode, conditional));
    check_page_tags(_pp);
    _bufferpool_managed = true;
    _mode               = mode;
    return RCOK;
}

w_rc_t fixable_page_h::fix_root (StoreID store, latch_mode_t mode,
        bool conditional, bool virgin)
{
    w_assert1(mode != LATCH_NL);

    unfix();
    W_DO(smlevel_0::bf->fixRootOldStyleExceptions(_pp, store, mode, conditional, virgin));
    if (!virgin) { check_page_tags(_pp); }

    _bufferpool_managed = true;
    _mode               = mode;
    return RCOK;
}

void fixable_page_h::fix_nonbufferpool_page(generic_page* s)
{
    w_assert1(s != nullptr);
    // This method is used to recover pages with single-page recovery, which
    // means we might be fixing a garbage page that will be formatted.
    // check_page_tags(s);

    unfix();
    _pp                 = s;
    _bufferpool_managed = false;
    _mode               = LATCH_EX;
}

bool fixable_page_h::is_dirty() const {

    if (_bufferpool_managed) {
        return smlevel_0::bf->getControlBlock(_pp).is_dirty();
    } else {
        return false;
    }
}

lsn_t fixable_page_h::get_page_lsn() const
{
    w_assert1(_pp);
    return smlevel_0::bf->getControlBlock(_pp).get_page_lsn();
}

void fixable_page_h::update_page_lsn(const lsn_t & lsn) const
{
    w_assert1(_pp);
    smlevel_0::bf->getControlBlock(_pp).set_page_lsn(lsn);
}

void fixable_page_h::set_img_page_lsn(const lsn_t & lsn)
{
    if (_pp) { _pp->lsn = lsn; }
}

void fixable_page_h::set_check_recovery(bool chk)
{
    w_assert1(_pp);
    smlevel_0::bf->getControlBlock(_pp).set_check_recovery(chk);
}

uint32_t fixable_page_h::get_log_volume()
{
    if (!_pp) { return 0; }
    return smlevel_0::bf->getControlBlock(_pp).get_log_volume();
}

void fixable_page_h::increment_log_volume(uint32_t v)
{
    w_assert1(_pp);
    smlevel_0::bf->getControlBlock(_pp).increment_log_volume(v);
}

void fixable_page_h::reset_log_volume()
{
    w_assert1(_pp);
    smlevel_0::bf->getControlBlock(_pp).set_log_volume(0);
}

bool fixable_page_h::has_check_recovery()
{
    w_assert1(_pp);
    bf_tree_cb_t& cb = smlevel_0::bf->getControlBlock(_pp);
    return cb._check_recovery;
}

bool fixable_page_h::is_to_be_deleted() {
    return (_pp->page_flags&t_to_be_deleted) != 0;
}

rc_t fixable_page_h::set_to_be_deleted (bool /*log_it*/) {
    // CS TODO: implement page deletion properly
    //
    w_assert0(false);
    // w_assert1(is_latched());
    // if ((_pp->page_flags & t_to_be_deleted) == 0) {
    //     if (log_it) {
    //         W_DO(Logger::log<page_set_to_be_deleted_log> (this));
    //     }
    //     _pp->page_flags ^= t_to_be_deleted;
    // }
    return RCOK;
}

void fixable_page_h::unset_to_be_deleted() {
    w_assert1(is_latched());
    if ((_pp->page_flags & t_to_be_deleted) != 0) {
        _pp->page_flags ^= t_to_be_deleted;
        // we don't need set_dirty() as it's always dirty if this is ever called
        // (UNDOing this means the page wasn't deleted yet by bufferpool, so it's dirty)
    }
}


bool fixable_page_h::change_possible_after_fix() const {
    w_assert1(is_fixed());
    return false; // for now assume no interference <<<>>>
}


bool fixable_page_h::upgrade_latch_conditional(latch_mode_t mode) {
    w_assert1(_pp != nullptr);
    w_assert1(mode >= LATCH_SH);

    if (_mode >= mode) {
        return true;
    };
    if (!_bufferpool_managed) {
        _mode = mode;
        return true;
    }

    if (_mode == LATCH_SH) {
        w_assert1(mode == LATCH_EX);
        bool success = smlevel_0::bf->upgradeLatchConditional(_pp);
        if (success) {
            _mode = LATCH_EX;
        }
        return success;

    } else {
        return false;
    }
}

void fixable_page_h::setup_for_restore(generic_page* pp)
{
    // w_assert1(!is_bufferpool_managed());

    // make assertions happy, even though this is not a buffer pool page
    _mode = LATCH_EX;

    _pp = pp;
    // Restore can read trash from backup file, which is not an error
    // if it is about to replay a page-image logrec, but in that case,
    // this assertion will fail.
    // check_page_tags(_pp);
}

bool fixable_page_h::is_pinned_for_restore()
{
    if (!_pp) { return false; }
    bf_tree_cb_t& cb = smlevel_0::bf->getControlBlock(_pp);
    return cb.is_pinned_for_restore();
}

// <<<>>>

#include "btree_page_h.h"

bool fixable_page_h::has_children() const {
    btree_page_h downcast;
    downcast.fix_nonbufferpool_page(get_generic_page());

    return !downcast.is_leaf();
}

int fixable_page_h::max_child_slot() const {
    btree_page_h downcast;
    downcast.fix_nonbufferpool_page(get_generic_page());

    if (downcast.level()<=1)
        return -1;  // if a leaf page, foster is the only pointer
    return downcast.nrecs();
}

PageID* fixable_page_h::child_slot_address(int child_slot) const {
    btree_page_h downcast;
    downcast.fix_nonbufferpool_page(get_generic_page());
    return downcast.page_pointer_address(child_slot -1);
}

general_recordid_t fixable_page_h::find_page_id_slot(generic_page* page, PageID pid) {
    fixable_page_h p;
    p.fix_nonbufferpool_page(page);
    int max_slot = p.max_child_slot();

    for (general_recordid_t i = GeneralRecordIds::FOSTER_CHILD; i <= max_slot; ++i) {
        // ERROUT(<< "Looking for child " << pid << " on " <<
        //         *p.child_slot_address(i));
        if (*p.child_slot_address(i) == pid) {
            // ERROUT(<< "OK");
            return i;
        }
    }
    return GeneralRecordIds::INVALID;
}

PageID fixable_page_h::root() const {
    btree_page_h downcast;
    downcast.fix_nonbufferpool_page(get_generic_page());
    return downcast.root();
}
