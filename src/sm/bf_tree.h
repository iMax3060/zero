/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#ifndef __BF_TREE_H
#define __BF_TREE_H

#include "w_defines.h"
#include "latch.h"
#include "vol.h"
#include "generic_page.h"
#include "bf_tree_cb.h"
#include <iosfwd>
#include "buffer_pool_free_list.hpp"
#include "page_cleaner.h"
#include "page_evictioner.h"
#include "restart.h"
#include "restore.h"

#include <array>
#include "bf_tree_hashtable.hpp"

class sm_options;
class lsn_t;
struct bf_tree_cb_t; // include bf_tree_cb.h in implementation codes

class test_bf_tree;
class page_evictioner_base;
class bf_tree_cleaner;
class btree_page_h;

/** a swizzled pointer (page ID) has this bit ON. */
constexpr uint32_t SWIZZLED_PID_BIT = 0x80000000;

/**
 * \brief The new buffer manager that exploits the tree structure of indexes.
 * \ingroup SSMBUFPOOL
 * \details
 * This is the new buffer manager in Foster B-tree which only deals with
 * tree-structured stores such as B-trees.
 * This class and bf_fixed_m effectively replace the old bf_core_m.
 *
 * \section buffer_swizzle Pointer-Swizzling
 * See bufferpool_design.docx.
 *
 * \section buffer_hierarchy Hierarchical-Bufferpool
 * This bufferpool assumes hierarchical data dastucture like B-trees.
 * fix() receives the already-latched parent pointer and uses it to find
 * the requested page. Especially when the pointer to the child is swizzled,
 *
 */
class bf_tree_m {
    friend class test_bf_tree; // for testcases
    friend class bf_tree_cleaner; // for page cleaning
    friend class page_evictioner_base;  // for page evictioning
    friend class page_cleaner_decoupled;
    friend class GenericPageIterator;
    friend class zero::buffer_pool::FreeListLowContention;
    friend class zero::buffer_pool::FreeListHighContention;

public:
    /** constructs the buffer pool. */
    bf_tree_m (const sm_options&);

    /** destructs the buffer pool.  */
    ~bf_tree_m ();

    void post_init();

    void shutdown();

    /** returns the total number of blocks in this bufferpool. */
    inline bf_idx get_block_cnt() const {return _block_cnt;}

    /** returns the control block corresponding to the given memory frame index */
    bf_tree_cb_t& get_cb(bf_idx idx) const;

    /** returns a pointer to the control block corresponding to the given memory frame index */
    bf_tree_cb_t* get_cbp(bf_idx idx) const;

    /** returns the control block corresponding to the given bufferpool page. mainly for debugging. */
    bf_tree_cb_t* get_cb(const generic_page *page);

    /** returns the memory-frame index corresponding to the given control block */
    bf_idx get_idx(const bf_tree_cb_t* cb) const;

    /** returns the bufferpool page corresponding to the given control block. mainly for debugging. */
    generic_page* get_page(const bf_tree_cb_t *cb);
    generic_page* get_page(const bf_idx& idx);

    /** returns the page ID of the root page (which is already loaded in this bufferpool) in given store. mainly for debugging or approximate purpose. */
    PageID get_root_page_id(StoreID store);

    /** returns the root-page index of the root page, which is always kept in the volume descriptor:*/
    bf_idx get_root_page_idx(StoreID store);

    static bool is_swizzled_pointer (PageID pid) {
        return (pid & SWIZZLED_PID_BIT) != 0;
    }

    // Used for debugging
    bool _is_frame_latched(generic_page* frame, latch_mode_t mode);

    void recover_if_needed(bf_tree_cb_t& cb, generic_page* page, bool only_if_dirty = true);

    /**
     * Fixes a non-root page in the bufferpool. This method receives the parent page and efficiently
     * fixes the page if the pid (pointer) is already swizzled by the parent page.
     * The optimization is transparent for most of the code because the pid stored in the parent
     * page is automatically (and atomically) changed to a swizzled pointer by the bufferpool.
     *
     * @param[out] page         the fixed page.
     * @param[in]  parent       parent of the page to be fixed. has to be already latched. if you can't provide this,
     *                          use fix_direct() though it can't exploit pointer swizzling.
     * @param[in]  pid          ID of the page to fix (or bufferpool index when swizzled)
     * @param[in]  mode         latch mode.  has to be SH or EX.
     * @param[in]  conditional  whether the fix is conditional (returns immediately even if failed).
     * @param[in]  only_if_hit  fix is only successful if frame is already on buffer (i.e., hit)
     * @param[in]  virgin_page  whether the page is a new page thus doesn't have to be read from disk.
     * @param[in]  do_recovery  whether recovery should be enabled for this page.
     * @param[in]  emlsn        The emlsn of the requested page.
     *
     * To use this method, you need to include bf_tree_inline.h.
     */
    w_rc_t fix_nonroot (generic_page*& page, generic_page *parent, PageID pid,
                          latch_mode_t mode, bool conditional = false, bool virgin_page = false,
                          bool only_if_hit = false, bool do_recovery = true,
                          lsn_t emlsn = lsn_t::null);

    /**
     * Adds an additional pin count for the given page (which must be already latched).
     * This is used to re-fix the page later without parent pointer. See fix_direct() why we need this feature.
     * Never forget to call a corresponding unpin_for_refix() for this page. Otherwise, the page will be in the bufferpool forever.
     * @param[in] page the page that is currently latched and will be re-fixed later.
     * @return slot index of the page in this bufferpool. Use this value to the subsequent refix_direct() and unpin_for_refix() call.
     */
    bf_idx pin_for_refix(const generic_page* page);

    /**
     * Removes the additional pin count added by pin_for_refix().
     */
    void unpin_for_refix(bf_idx idx);

    /**
     * Fixes a page with the already known slot index, assuming the slot has at least one pin count.
     * Used with pin_for_refix() and unpin_for_refix().
     */
    w_rc_t refix_direct (generic_page*& page, bf_idx idx, latch_mode_t mode, bool conditional);

    /**
     * Fixes an existing (not virgin) root page for the given store.
     * This method doesn't receive page ID because it's already known by bufferpool.
     * To use this method, you need to include bf_tree_inline.h.
     */
    w_rc_t fix_root (generic_page*& page, StoreID store, latch_mode_t mode,
                     bool conditional, bool virgin);


    /** returns the current latch mode of the page. */
    latch_mode_t latch_mode(const generic_page* p);

    /** Prefetches pages into free frames using iovec */
    void prefetch_pages(PageID first, unsigned count);

    /**
     * upgrade SH-latch on the given page to EX-latch.
     * This method is always conditional, immediately returning if there is a conflicting latch.
     * Returns if successfully upgraded.
     */
    bool upgrade_latch_conditional(const generic_page* p);

    /** downgrade EX-latch on the given page to SH-latch. */
    void downgrade_latch(const generic_page* p);

    /**
     * Release the latch on the page.
     */
    void unfix(const generic_page* p, bool evict = false);

    /**
     * Returns if the page is already marked dirty.
     */
    bool is_dirty(const generic_page* p) const;

    /**
     * Returns if the page is already marked dirty.
     */
    bool is_dirty(const bf_idx idx) const;

    /*!\fn      isEvictable(const bf_idx indexToCheck, const bool doFlushIfDirty) const
     * \brief   Check if a page can be evicted
     * \details The following conditions make a page unevictable:
     *          - It is the store node page (\link generic_page_h::tag() \endlink \c ==
     *            \link page_tag_t::t_stnode_p \endlink).
     *          - It is the root page of a B-Tree (\link generic_page_h::tag() \endlink
     *            \c == \link page_tag_t::t_btree_p \endlink \c &&
     *            \link generic_page_h::pid() \endlink \c ==
     *            \link btree_page_h::root() \endlink).
     *          - It is an inner page of a B-Tree and swizzling is enabled
     *            (\link _enable_swizzling \endlink \c &&
     *            \link generic_page_h::tag() \endlink
     *            \c == \link page_tag_t::t_btree_p \endlink \c &&
     *            \c ! \link btree_page_h::is_leaf() \endlink).\n
     *            We exclude those as we do not support unswizzling and as we do not
     *            know if inner pages of a B-Tree might contain swizzled pointers.
     *          - It is a page of a B-Tree with a foster child
     *            (\link _enable_swizzling \endlink \c &&
     *            \link generic_page_h::tag() \endlink
     *            \c == \link page_tag_t::t_btree_p \endlink \c &&
     *            \c \link btree_page_h::get_foster() \endlink) \c != \c 0).\n
     *            We exclude those as we do not support unswizzling.
     *          - It is a dirty page that needs to be cleaned by the page cleaner
     *            (\c (\c flush_dirty \c || \link is_no_db_mode() \endlink
     *            \c || \link _write_elision \endlink\c ) \c &&
     *            \link bf_tree_cb_t::is_dirty() \endlink)\n
     *            If noDB or write elision is used, a page doesn't need to be flushed
     *            before eviction and if the evictioner flushes dirty pages, those can
     *            be evicted as well.
     *          - There is no page in the buffer pool frame - it is unused
     *            (\c !\link bf_tree_cb_t::_used \endlink).
     *          - It is pinned (\c !\link bf_tree_cb_t::_pin_cnt \endlink \c != \c 0).\n
     *            The page is either pinned or it gets currently evicted by another
     *            thread.
     *
     * \pre     The buffer frame with index \c indexToCheck is latched in
     *          \link latch_mode_t::LATCH_EX \endlink mode by this thread.
     *
     * \remark  The example implementations for the conditions are given in the
     *          documentation because it is hard to figure out which pages cannot be
     *          evicted.
     *
     * @param indexToCheck   The index of the buffer frame that is supposed to be freed
     *                       by evicting the contained page.
     * @param doFlushIfDirty \c true if the page gets flushed during the eviction,
     *                       \c false else.
     * @return               \c true if the page could be evicted, \c false else.
     */
    bool isEvictable(const bf_idx indexToCheck, const bool doFlushIfDirty) const;

    /**
     * Returns true if the page's _used flag is on
     */
    bool is_used (bf_idx idx) const;

    /**
     * Sets the page_lsn field on the control block. Used by every update
     * operation on a page, including redo.
     */
    void set_page_lsn(generic_page*, lsn_t);
    lsn_t get_page_lsn(generic_page*);

    /***
     * Marks the frame as in possible need of recovery
     * (used for prefetched pages during warmup)
     */
    void set_check_recovery(generic_page*, bool);

    void pin_for_restore(generic_page*);
    void unpin_for_restore(generic_page*);

    /**
     * Gets the value of the log volume counter in the control block.
     */
    uint32_t get_log_volume(generic_page*);
    void reset_log_volume(generic_page*);
    void increment_log_volume(generic_page*, uint32_t);

    /**
     * Whenever the parent of a page is changed (adoption or de-adoption),
     * this method must be called to switch it in bufferpool.
     * The caller must make sure the page itself, old and new parent pages
     * don't go away while this switch (i.e., latch them).
     */
    void switch_parent (PageID, generic_page*);

    /**
     * Search in the given page to find the slot that contains the page id as a child.
     * Returns >0 if a normal slot, 0 if pid0, -1 if foster, -2 if not found.
     */
    general_recordid_t find_page_id_slot (generic_page* page, PageID pid) const;

    /**
     * Returns if the page is swizzled by parent or the volume descriptor.
     * Do NOT call this method without a latch.
     */
    bool is_swizzled (const generic_page* page) const;

    /** Normalizes the page identifier to a disk page identifier.
      * If the page identifier is a memory frame index (in case of swizzling)
      * then it returns the disk page index, otherwise it returns the page
      * identifier as it is.
      * Do NOT call this method without a latch.
      */
    PageID normalize_pid(PageID pid) const;

    /**
     * Dumps all contents of this bufferpool.
     * this method is solely for debugging. It's slow and unsafe.
     */
    void  debug_dump (std::ostream &o) const;
    /**
     * Dumps the pointers in the given page, accounting for pointer swizzling.
     * this method is solely for debugging. It's slow and unsafe.
     */
    void  debug_dump_page_pointers (std::ostream &o, generic_page *page) const;
    void  debug_dump_pointer (std::ostream &o, PageID pid) const;

    /**
     * Returns the non-swizzled page-ID for the given pointer that might be swizzled.
     * This is NOT safe against concurrent eviction and should be used just for debugging.
     */
    PageID  debug_get_original_pageid (PageID pid) const;

    /**
     * Returns if the given page is managed by this bufferpool.
     */
    inline bool  is_bf_page (const generic_page *page) const {
        int32_t idx = page - _buffer;
        return _is_valid_idx(idx);
    }

    /**
     * Returns true if the node has any swizzled pointers to its children.
     * In constrast to the swizzled_ptr_cnt_hint counter, which is just a
     * a hint, this method is accurate as it scans the node * and counts
     * its swizzled pointers. It requires the caller to have the node latched.
     */
    bool has_swizzled_child(bf_idx node_idx);

    size_t get_size() { return _block_cnt; }

    std::shared_ptr<page_cleaner_base> get_cleaner();

    template <typename... Args>
    void wakeup_cleaner(Args... args)
    {
        auto cleaner = get_cleaner();
        if (cleaner) { cleaner->wakeup(args...); }
    }

    bool is_no_db_mode() const { return _no_db_mode; }

    bool is_warmup_done() const { return _warmup_done; }

    bool has_dirty_frames() const;

    void fuzzy_checkpoint(chkpt_t& chkpt) const;

    void set_media_failure();
    void unset_media_failure();

    PageID get_media_failure_pid() { return _media_failure_pid; }
    bool is_media_failure() { return _media_failure_pid > 0; }
    bool is_media_failure(PageID pid) { return _media_failure_pid > 0 &&
        pid < _media_failure_pid; }

    // Used for decoupled cleaning
    void notify_archived_lsn(lsn_t);

    /*!\fn      unswizzlePagePointer(generic_page *parentPage, general_recordid_t childSlotInParentPage, bool doUnswizzle = true, PageID *childPageID = nullptr)
     * \brief   Unswizzle a page pointer in a page's parent page
     * \details Tries to unswizzle the given child page (\c childSlotInParentPage) from the parent
     *          page (\c parentPage). If, for some reason, unswizzling was impossible or troublesome,
     *          gives up and returns \c false.
     *
     * @pre The parent page is latched in any mode and the child page is latched in mode
     *      \link latch_mode_t::LATCH_EX \endlink (if \c doUnswizzle \c == \c true).
     *
     * @param[in]     parentPage            The parent page where to unswizzle the child page.
     * @param[in]     childSlotInParentPage The slot within the parent page where to find the swizzled
     *                                      pointer to the child page.
     * @param[in]     doUnswizzle           If \c true, pointer is actually unswizzled in parent,
     *                                      otherwise just return what the unswizzled pointer would be
     *                                      (i.e., the \c childPageID).
     * @param[in,out] childPageID           If it wasn't set to the \c nullptr, the unswizzled
     *                                      \link PageID \endlink of the child page is returned.
     *
     * @return                              \c false if the unswizzling was not successful, else \c true.
     */
    bool unswizzlePagePointer(generic_page* parentPage, general_recordid_t childSlotInParentPage,
                              bool doUnswizzle = true,
                              PageID* childPageID = nullptr);

    // Used for debugging
    void print_page(PageID pid);

private:

    /** fixes a non-swizzled page. */
    w_rc_t fix(generic_page* parent, generic_page*& page, PageID pid,
                               latch_mode_t mode, bool conditional, bool virgin_page,
                               bool only_if_hit = false, bool do_recovery = true,
                               lsn_t emlsn = lsn_t::null);

    /**
     * Given an image of page which might have swizzled pointers,
     * convert it to a disk page without swizzled pointers.
     * Used to write out dirty pages.
     * NOTE this method assumes the swizzled pointers in the page are
     * not being unswizzled concurrently.
     * Take SH latch on this page (not pointed pages) or make sure
     * there aren't such concurrent threads by other means.
     */
    void   _convert_to_disk_page (generic_page* page) const;

    /**
     * try to evict a given block.
     * @return whether evicted the page or not
     */
    bool _try_evict_block(bf_idx parent_idx, bf_idx idx);

    /**
     * Subroutine of _try_evict_block() called after the CAS on pin_cnt.
     * @pre cb.pin_cnt() == -1
     */
    bool _try_evict_block_pinned(bf_tree_cb_t &parent_cb, bf_tree_cb_t &cb,
        bf_idx parent_idx, bf_idx idx);
    /**
     * Subroutine of _try_evict_block_pinned() to update parent's EMLSN.
     * @pre cb.pin_cnt() == -1
     * @pre parent_cb.latch().is_latched()
     */
    bool _try_evict_block_update_emlsn(bf_tree_cb_t &parent_cb, bf_tree_cb_t &cb,
        bf_idx parent_idx, bf_idx idx, general_recordid_t child_slotid);

    /// returns true iff idx is in the valid range.  for assertion.
    bool   _is_valid_idx (bf_idx idx) const;

    /// Called by fix to read a page from the database (or the backup)
    rc_t _read_page(PageID pid, bf_tree_cb_t& cb, bool from_backup = false);

    /**
     * returns true if idx is in the valid range and also the block is used.  for assertion.
     *
     * @pre hold get_cb(idx).latch() in read or write mode
     */
    bool   _is_active_idx (bf_idx idx) const;

    /**
     * Deletes the given block from this buffer pool. This method must be called when
     *  1. there is no concurrent accesses on the page (thus no latch)
     *  2. the page's _used is true
     *  3. the page's _pin_cnt is 0 (so, it must not be swizzled, nor being evicted)
     * Used from the dirty page cleaner to delete a page with "tobedeleted" flag.
     */
    void   _delete_block (bf_idx idx);

    /**
     * \brief System transaction for upadting child EMLSN in parent
     * \ingroup SPR
     * @param[in,out] parent parent page
     * @param[in] child_slotid slot id of child
     * @param[in] child_emlsn new emlsn to store in parent
     * @pre parent.is_latched()
     * \note parent must be latched, but does not have to be EX-latched.
     * This is because EMLSN are not viewed/updated by multi threads (only accessed during
     * page eviction or cache miss of the particular page).
     */
    w_rc_t _sx_update_child_emlsn(btree_page_h &parent,
                                  general_recordid_t child_slotid, lsn_t child_emlsn);

    /**
     * Check if the buffer pool is warmed up by looking at the hit ratio
     * observed for the current thread. This is for cases where the dataset
     * fits in main memory and the buffer pool never (or just takes really
     * long to) fill up.
     */
    void check_warmup_done();

    void set_warmup_done();

    /// Buffer is considered warm when hit ratio goes above this
    double _warmup_hit_ratio;

    /// Only check after this number of fixes
    unsigned _warmup_min_fixes;


private:
    /** count of blocks (pages) in this bufferpool. */
    bf_idx               _block_cnt;

    // CS TODO: concurrency???
    std::array<bf_idx, stnode_page::max> _root_pages;

    /** Array of control blocks. array size is _block_cnt. index 0 is never used (means NULL). */
    bf_tree_cb_t*        _control_blocks;

    /** Array of page contents. array size is _block_cnt. index 0 is never used (means NULL). */
    generic_page*              _buffer;

    /** hashtable to locate a page in this bufferpool. swizzled pages are removed from bufferpool. */
    std::shared_ptr<zero::buffer_pool::Hashtable> _hashtable;

    /* free list containing the indexes of the unused buffer frames. */
    std::shared_ptr<zero::buffer_pool::FreeListLowContention> _freeList;

    /** the dirty page cleaner. */
    std::shared_ptr<page_cleaner_base>   _cleaner;

    /** worker thread responsible for evicting pages. */
    std::shared_ptr<page_evictioner_base> _evictioner;

    /** Perform eviction on dedicated thread; fixing threads just wait until a free
     * frame is available */
    bool _async_eviction;

    /** whether to swizzle non-root pages. */
    bool                 _enable_swizzling;

    /** whether to update emlsn on parent upon eviction */
    bool                 _maintain_emlsn;

    bool _write_elision;

    std::atomic<PageID> _media_failure_pid;

    bool _cleaner_decoupled;

    bool _instant_restore;

    bool _no_db_mode;

    bool _batch_warmup;
    size_t _batch_segment_size;
    bool _warmup_done;

    bool _log_fetches;

    static thread_local unsigned _fix_cnt;
    static thread_local unsigned _hit_cnt;

    /// Single-page-recovery iterator used for instant restart redo
    /// (see bf_tree_m::recover_if_needed)
    static thread_local SprIterator _localSprIter;

    using RestoreCoord = RestoreCoordinator<
        std::function<decltype(SegmentRestorer::bf_restore)>>;
    std::shared_ptr<RestoreCoord> _restore_coord;

    using BgRestorer = BackgroundRestorer<RestoreCoord, std::function<void(void)>>;
    std::shared_ptr<BgRestorer> _background_restorer;
};

/**
 * Holds the buffer slot index of additionally pinned page and
 * releases the pin count when it's destructed.
 * @see bf_tree_m::pin_for_refix(), bf_tree_m::unpin_for_refix(), bf_tree_m::refix_direct().
 */
class pin_for_refix_holder {
public:
    pin_for_refix_holder() : _idx(0) {}
    pin_for_refix_holder(bf_idx idx) : _idx(idx) {}
    pin_for_refix_holder(pin_for_refix_holder &h) {
        steal_ownership(h);
    }
    ~pin_for_refix_holder () {
        if (_idx != 0) {
            release();
        }
    }
    pin_for_refix_holder& operator=(pin_for_refix_holder& h) {
        steal_ownership(h);
        return *this;
    }
    void steal_ownership (pin_for_refix_holder& h) {
        if (_idx != 0) {
            release();
        }
        _idx = h._idx;
        h._idx = 0;
    }

    void set(bf_idx idx) {
        if (_idx != 0) {
            release();
        }
        _idx = idx;
    }
    bf_idx idx() const { return _idx;}
    void release ();

private:
    bf_idx _idx;
};

class GenericPageIterator
{
public:
    generic_page* operator*()
    {
        if (!_current || _current_pid >= end_pid()) { return nullptr; }
        return _current;
    }

    /*
     * WARNING: this operator may skip a PID (e.g., jump from 42 to 44) if
     * fix_current() returns false! I couldn't find a quick, elegant way to
     * work around this problem yet.
     */
    GenericPageIterator& operator++()
    {
        unfix_current();

        bool success = false;
        while (!success) {
            _current_pid++;
            if (_current_pid >= end_pid()) { return *this; }
            success = fix_current();
        }

        return *this;
    }

    /// This essentially yields an "end" iterator
    GenericPageIterator()
        : _first(0), _count(0), _virgin(false), _current(nullptr),
        _current_pid(0), fix_depth(0)
    {
    }

    /// This essentially yields a "begin" iterator
    GenericPageIterator(PageID first, PageID count, bool virgin)
        : _first(first), _count(count), _virgin(virgin), _current(nullptr),
        fix_depth(0)
    {
        if (count > 0) {
            _current_pid = _first - 1;
            operator++();
        }
    }

    GenericPageIterator(const GenericPageIterator& other)
        : _first(other._first), _count(other._count),
        _virgin(other._virgin), _current(nullptr), fix_depth(0)
    {
        if (_count > 0) {
            _current_pid = _first - 1;
            operator++();
        }
    }

    ~GenericPageIterator()
    {
        unfix_current();
        w_assert1(fix_depth == 0);
    }

    GenericPageIterator& operator=(GenericPageIterator other)
    {
        swap(*this, other);
        return *this;
    }

    friend void swap(GenericPageIterator& a, GenericPageIterator& b)
    {
        swap(a._first, b._first);
        swap(a._count, b._count);
        swap(a._virgin, b._virgin);
        swap(a._current, b._current);
        swap(a._current_pid, b._current_pid);
        swap(a.fix_depth, b.fix_depth);
    }

    bool operator==(GenericPageIterator& other)
    {
        if (!_current && !other._current) { return true; }
        return other._current_pid == _current_pid && _current && other._current;
    }

    bool operator!=(GenericPageIterator& other)
    {
        return !(*this == other);
    }

    PageID current_pid() const { return _current_pid; }

    PageID begin_pid() const { return _first; }
    PageID end_pid() const { return _first + _count; }

private:
    PageID _first;
    PageID _count;
    bool _virgin;

    generic_page* _current;
    PageID _current_pid;

    int fix_depth;

    bool fix_current()
    {
        w_assert1(fix_depth == 0);
        constexpr bool conditional = true;
        constexpr bool do_recovery = false;
        constexpr bool only_if_hit = false;
        auto rc = smlevel_0::bf->fix(nullptr, _current, _current_pid, LATCH_EX,
                conditional, _virgin, only_if_hit, do_recovery);

        /*
         * The latch is already held, either by this thread (in SH mode, which
         * results in stINUSE to avoid deadlock) or by another thread. In that
         * case, we assume that if the frame is latched, the thread that holds
         * that latch already made sure that the page is consistent, and thus
         * log replay is not required.
         */
        if (rc.err_num() == stINUSE || rc.err_num() == stTIMEOUT) {
            ERROUT(<< "failed to fix " << _current_pid);
            _current = nullptr;
            return false;
        }
        W_COERCE(rc);

        fix_depth++;

        return true;
    }

    void unfix_current()
    {
        if (_current) {
            w_assert1(fix_depth == 1);
            smlevel_0::bf->unpin_for_restore(_current);
            smlevel_0::bf->set_check_recovery(_current, true);
            smlevel_0::bf->unfix(_current);
            _current = nullptr;
            fix_depth--;
        }
    }
};

#endif // __BF_TREE_H
