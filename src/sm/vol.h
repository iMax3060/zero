/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

#ifndef __VOL_H
#define __VOL_H

#include "w_defines.h"
#include "stnode_page.h"

#include <list>
#include <cstdlib>
#include <chrono>

class alloc_cache_t;
class stnode_cache_t;
class RestoreMgr;
class sm_options;
class chkpt_t;

class vol_t
{
public:
    vol_t(const sm_options&);
    virtual ~vol_t();

    void finish_restore();
    void shutdown();

    size_t      num_used_pages() const;

    size_t  get_num_restored_pages() const;
    size_t  get_num_to_restore_pages() const;

    alloc_cache_t*           get_alloc_cache() {return _alloc_cache;}
    stnode_cache_t*          get_stnode_cache() {return _stnode_cache;}


    /**
     *
     * Thread safety: the underlying POSIX calls pwrite and pread are
     * guaranteed to be atomic, so no additional latching is required for these
     * methods. Mounting/dismounting during reads and writes causes the file
     * descriptor to change, resulting in the expected errors in the return
     * code.
     */
    rc_t                write_many_pages(
        PageID             first_page,
        const generic_page* buf,        //caller must align this buffer
        int                 cnt,
        bool ignoreRestore = false);

    rc_t write_page(PageID page, generic_page* buf) {
        return write_many_pages(page, buf, 1);
    }

    rc_t read_page(PageID page, generic_page* const buf);

    rc_t                read_many_pages(
        PageID             first_page,
        generic_page* const buf,        //caller must align this buffer
        int                 cnt,
        bool ignoreRestore = false);

    rc_t read_backup(PageID first, size_t count, void* buf);
    rc_t write_backup(PageID first, size_t count, void* buf);

    /** Add a backup file to be used for restore */
    rc_t sx_add_backup(const string& path, lsn_t backupLSN, bool redo = false);

    void list_backups(std::vector<string>& backups);

    void sync();

    rc_t            alloc_a_page(PageID& pid, StoreID stid = 0);
    rc_t            deallocate_page(const PageID& pid);

    bool                is_allocated_page(PageID pid) const;

    bool                is_valid_store(StoreID f) const;

    /**  Return true if the store "store" is allocated. false otherwise. */
    bool                is_alloc_store(StoreID f) const;

    /** Sets root page ID of the specified index. */
    rc_t            set_store_root(StoreID snum, PageID root);
    /** Returns root page ID of the specified index. */
    PageID         get_store_root(StoreID f) const;

    rc_t            create_store(PageID&, StoreID&);

    /** Mark device as failed and kick off Restore */
    rc_t            mark_failed(bool evict = false, bool redo = false,
            PageID lastUsedPID = 0);

    lsn_t get_backup_lsn();

    /** Turn on write elision (i.e., ignore all writes from now on) */
    void set_readonly(bool r)
    {
        spinlock_write_critical_section cs(&_mutex);
        _readonly = r;
    }

    /** Take a backup on the given file path. */
    rc_t take_backup(string path, bool forceArchive = false);

    bool is_failed() const
    {
        spinlock_read_critical_section cs(&_mutex);
        return _failed;
    }

    unsigned num_backups() const;

    bool check_restore_finished();

    /** Return largest PID allocated for this volume yet **/
    PageID get_last_allocated_pid() const;

    /** Method to create _alloc_cache and _stnode_cache */
    void build_caches(bool truncate, chkpt_t* = nullptr);

    bool caches_ready() { return _alloc_cache && _stnode_cache; }

private:
    // variables read from volume header -- remain constant after mount
    int              _fd;

    mutable srwlock_t _mutex;

    /**
     *  Impose a fake IO penalty. Assume that each batch of pages requires
     *  exactly one seek. A real system might perform better due to sequential
     *  access, or might be worse because the pages in the batch are not
     *  actually contiguous. Close enough...
     */
    std::chrono::high_resolution_clock::duration  _fake_read_latency;
    std::chrono::high_resolution_clock::duration  _fake_write_latency;

    alloc_cache_t*   _alloc_cache;
    stnode_cache_t*  _stnode_cache;

    /** Set to simulate a failed device for Restore **/
    bool             _failed;

    /** Writes are ignored and old page versions are kept.  This means that
     * clean status on buffer pool is invalid, and thus single-page recovery is
     * required when reading page back.  Due to a current bug on the page
     * cleaner, this is already the case anyway. I.e., write elision is already
     * taking place due to the bug. If readonly is set, all writes are elided.
     */
    bool             _readonly;

    bool _no_db_mode;

    /** Restore Manager is activated when volume has failed */
    RestoreMgr*      _restore_mgr;

    /** Paths to backup files, added with add_backup() */
    std::vector<string> _backups;
    std::vector<lsn_t> _backup_lsns;

    /** Currently opened backup (during restore only) */
    int _backup_fd;
    lsn_t _current_backup_lsn;
    size_t _backup_pages;

    /** Backup being currently taken */
    int _backup_write_fd;
    string _backup_write_path;

    /** Whether to generate page read log records */
    bool _log_page_reads;

    /** Whether to fetch as many log records as possible from archive
     * when performing single-page recovery */
    bool _prioritize_archive;

    /** Wheter to open file with O_SYNC */
    bool _use_o_sync;

    /** Whether to open file with O_DIRECT */
    bool _use_o_direct;

    /** Whether to cluster pages of the same store in extents */
    bool _cluster_stores;

    /** Open backup file descriptor for retore or taking new backup */
    void open_backup();

};

inline bool vol_t::is_valid_store(StoreID f) const
{
    return (f < stnode_page::max);
}

#endif // __VOL_H /*</std-footer>*/
