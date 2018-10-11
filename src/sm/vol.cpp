/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

#include "w_defines.h"

/*  -- do not edit anything above this line --   </std-header>*/

#define SM_SOURCE
#define VOL_C

#include <boost/concept_check.hpp>
#include "sm_base.h"
#include "stnode_page.h"
#include "vol.h"
#include "log_core.h"
#include "sm_options.h"

#include "alloc_cache.h"
#include "backup_alloc_cache.h"
#include "logarchiver.h"
#include "restart.h"
#include "xct_logger.h"

// files and stuff
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <thread>

#include "sm.h"

// TODO proper exception mechanism
#define CHECK_ERRNO(n) \
    if (n == -1) { \
        W_FATAL_MSG(fcOS, << "Kernel errno code: " << errno); \
    }

/*
 * replacement for solaris gethrtime(), which is based in any case
 * on this clock:
 */
int64_t gethrtime()
{
    struct timespec tsp;
    long e = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tsp);
    w_assert0(e == 0);
    // tsp.tv_sec is time_t
    return (tsp.tv_sec * 1000* 1000 * 1000) + tsp.tv_nsec; // nanosecs
}

vol_t::vol_t(const sm_options& options)
             : _fd(-1),
               _fake_read_latency(options.get_int_option("sm_vol_simulate_read_latency", 0)),
               _fake_write_latency(options.get_int_option("sm_vol_simulate_write_latency", 0)),
               _alloc_cache(NULL), _stnode_cache(NULL),
               _backup_fd(-1),
               _current_backup_lsn(lsn_t::null), _backup_write_fd(-1),
               _log_page_reads(options.get_bool_option("sm_vol_log_reads", false)),
               _log_page_writes(options.get_bool_option("sm_vol_log_writes", false)),
               _prioritize_archive(true)
{
    string dbfile = options.get_string_option("sm_dbfile", "db");
    bool truncate = options.get_bool_option("sm_format", false);
    _use_o_sync = options.get_bool_option("sm_vol_o_sync", false);
    _use_o_direct = options.get_bool_option("sm_vol_o_direct", false);
    _readonly = options.get_bool_option("sm_vol_readonly", false);
    _prioritize_archive =
        options.get_bool_option("sm_recovery_prioritize_archive", false);
    _cluster_stores = options.get_bool_option("sm_vol_cluster_stores", true);

    _no_db_mode = options.get_bool_option("sm_no_db", false);
    if (_no_db_mode) {
        _readonly = true;
    }

    int open_flags = 0;
    open_flags |= _readonly ? O_RDONLY : O_RDWR;
    if (truncate) { open_flags |= O_TRUNC | O_CREAT; }
    if(_use_o_sync) { open_flags |= O_SYNC; }
    if(_use_o_direct) { open_flags |= O_DIRECT; }

    auto fd = open(dbfile.c_str(), open_flags, 0666 /*mode*/);
    CHECK_ERRNO(fd);
    _fd = fd;
}

vol_t::~vol_t()
{
    if (_alloc_cache) {
        delete _alloc_cache;
        _alloc_cache = NULL;
    }
    if (_stnode_cache) {
        delete _stnode_cache;
        _stnode_cache = NULL;
    }

    w_assert1(_fd == -1);
    w_assert1(_backup_fd == -1);
}

void vol_t::sync()
{
    auto ret = fsync(_fd);
    CHECK_ERRNO(ret);
}

void vol_t::build_caches(bool truncate, chkpt_t* chkpt_info)
{
    _stnode_cache = new stnode_cache_t(truncate);
    w_assert1(_stnode_cache);
    _stnode_cache->dump(cerr);

    _alloc_cache = new alloc_cache_t(*_stnode_cache, truncate, _cluster_stores);
    w_assert1(_alloc_cache);

    if (chkpt_info && !chkpt_info->bkp_path.empty()) {
        sx_add_backup(chkpt_info->bkp_path, chkpt_info->bkp_lsn, true);
        ERROUT(<< "Added backup: " << chkpt_info->bkp_path);
    }
}

bool vol_t::open_backup()
{
    bool useBackup = _backups.size() > 0;
    if (!useBackup || _backup_fd >= 0) { return false; }

    // mutex held by caller -- no concurrent backup being added
    string backupFile = _backups.back();
    // Using direct I/O
    int open_flags = O_RDONLY | O_SYNC;
    if (_use_o_direct) { open_flags |= O_DIRECT; }

    auto fd = open(backupFile.c_str(), open_flags, 0666 /*mode*/);
    CHECK_ERRNO(fd);
    _backup_fd = fd;
    _current_backup_lsn = _backup_lsns.back();

    struct stat stat;
    auto ret = ::fstat(fd, &stat);
    CHECK_ERRNO(ret);
    auto backup_pages = (stat.st_size / sizeof(generic_page));
    w_assert0(stat.st_size % sizeof(generic_page) == 0);

    _backup_alloc_cache = std::make_unique<backup_alloc_cache_t>(backup_pages);

    return true;
}

lsn_t vol_t::get_backup_lsn()
{
    spinlock_read_critical_section cs(&_mutex);
    return _current_backup_lsn;
}

unsigned vol_t::num_backups() const
{
    spinlock_read_critical_section cs(&_mutex);
    return _backups.size();
}

void vol_t::list_backups(
    std::vector<string>& backups)
{
    spinlock_read_critical_section cs(&_mutex);

    for (size_t k = 0; k < _backups.size(); k++) {
        backups.push_back(_backups[k]);
    }
}

rc_t vol_t::sx_add_backup(const string& path, lsn_t backupLSN, bool redo)
{
    spinlock_write_critical_section cs(&_mutex);

    _backups.push_back(path);
    _backup_lsns.push_back(backupLSN);
    w_assert1(_backups.size() == _backup_lsns.size());

    if (!redo) {
        sys_xct_section_t ssx(true);
        Logger::log_sys<add_backup_log>(path, backupLSN);
        W_DO(ssx.end_sys_xct(RCOK));
    }

    return RCOK;
}

void vol_t::shutdown()
{
    spinlock_write_critical_section cs(&_mutex);

    DBG(<<" vol_t::dismount flush=" << flush);

    // INC_TSTAT(vol_cache_clears);

    w_assert1(_fd >= 0);

    auto ret = close(_fd);
    CHECK_ERRNO(ret);
    _fd = -1;
}

void vol_t::close_backup()
{
    if (_backup_fd > 0) {
        auto ret = close(_backup_fd);
        CHECK_ERRNO(ret);
        _backup_fd = -1;
        _current_backup_lsn = lsn_t::null;
        _backup_alloc_cache = nullptr;
    }
}

rc_t vol_t::alloc_a_page(PageID& shpid, StoreID stid)
{
    if (!_cluster_stores) { stid = 0; }
    w_assert1(_alloc_cache);
    W_DO(_alloc_cache->sx_allocate_page(shpid, stid));
    INC_TSTAT(page_alloc_cnt);

    return RCOK;
}

rc_t vol_t::deallocate_page(const PageID& pid)
{
    w_assert1(_alloc_cache);
    W_DO(_alloc_cache->sx_deallocate_page(pid));
    INC_TSTAT(page_dealloc_cnt);

    return RCOK;
}

size_t vol_t::num_used_pages() const
{
    return _alloc_cache->get_last_allocated_pid() + 1;
}

rc_t vol_t::create_store(PageID& root_pid, StoreID& snum)
{
    W_DO(_alloc_cache->sx_allocate_page(root_pid));
    W_DO(_stnode_cache->sx_create_store(root_pid, snum));
    return RCOK;
}

bool vol_t::is_alloc_store(StoreID f) const
{
    return _stnode_cache->is_allocated(f);
}

PageID vol_t::get_store_root(StoreID f) const
{
    return _stnode_cache->get_root_pid(f);
}

/*********************************************************************
 *
 *  vol_t::read_page(pnum, page)
 *
 *  Read the page at "pnum" of the volume to the buffer "page".
 *
 *********************************************************************/
rc_t vol_t::read_page(PageID pnum, generic_page* const buf)
{
    return read_many_pages(pnum, buf, 1);
}

void vol_t::read_vector(PageID first_pid, unsigned count,
        std::vector<generic_page*>& frames, bool from_backup)
{
    w_assert1(frames.size() >= count);

    w_assert0(count <= IOV_MAX);

    // Backup reads must guarantee that unallocated pages are zeroed out
    // (see comment in read_backup)
    auto backup_pages = _backup_alloc_cache->get_end_pid();
    if (from_backup && first_pid >= backup_pages) {
        for (size_t i = 0; i < count; i++) {
            memset(frames[i], 0, sizeof(generic_page));
        }
        return;
    }

    static thread_local std::vector<struct iovec> iov;
    iov.resize(count);
    for (unsigned i = 0; i < count; i++) {
        iov[i].iov_base = frames[i];
        iov[i].iov_len = sizeof(generic_page);
    }

    size_t offset = size_t(first_pid) * sizeof(generic_page);
    auto fd = from_backup ? _backup_fd : _fd;
    int read_count = preadv(fd, &iov[0], count, offset);
    CHECK_ERRNO(read_count);

    if (from_backup) {
        w_assert0(_backup_alloc_cache);
        for (size_t i = 0; i < count; i++) {
            if (!_backup_alloc_cache->is_allocated(first_pid + i)) {
                memset(frames[i], 0, sizeof(generic_page));
            }
        }
    }
}

/*********************************************************************
 *
 *  vol_t::read_many_pages(first_page, buf, cnt)
 *
 *  Read "cnt" buffers in "buf" from pages starting at "first_page"
 *  of the volume.
 *
 *********************************************************************/
rc_t vol_t::read_many_pages(PageID first_page, generic_page* const buf, int cnt)
{
    DBG(<< "Page read: from " << first_page << " to " << first_page + cnt);
    ADD_TSTAT(vol_reads, cnt);

    std::chrono::high_resolution_clock::time_point sleep_until;
    if(_fake_read_latency.count()) {
        sleep_until = std::chrono::high_resolution_clock::now() + _fake_read_latency;
    }

    w_assert1(cnt > 0);
    size_t offset = size_t(first_page) * sizeof(generic_page);
    memset(buf, '\0', cnt * sizeof(generic_page));
    int read_count = pread(_fd, (char *) buf, cnt * sizeof(generic_page), offset);
    CHECK_ERRNO(read_count);

    std::this_thread::sleep_until(sleep_until);

    if (_log_page_reads) {
        Logger::log_sys<page_read_log>(first_page, cnt);
    }

    return RCOK;
}

void vol_t::read_backup(PageID first, size_t count, void* buf)
{
    if (_backup_fd < 0) {
        W_FATAL_MSG(eINTERNAL,
                << "Cannot read from backup because it is not active");
    }
    w_assert0(_backup_alloc_cache);

    // Backup reads must guarantee that unallocated pages are zeroed out,
    // otherwise restore will not work (I tried to make it work, but it was
    // just too complicated -- this is much easier)
    auto backup_pages = _backup_alloc_cache->get_end_pid();
    if (first >= backup_pages) {
        memset(buf, 0, sizeof(generic_page) * count);
        return;
    }

    // adjust count to avoid short I/O
    auto actual_count = count;
    if (first + count > backup_pages) {
        actual_count = backup_pages - first;
    }

    size_t offset = size_t(first) * sizeof(generic_page);
    size_t bytes = actual_count * sizeof(generic_page);
    int read_count = pread(_backup_fd, buf, bytes, offset);
    CHECK_ERRNO(read_count);

    // Short I/O is still possible because backup is only taken until last used
    // page, i.e., the file may be smaller than the total quota.
    if (read_count < (int) actual_count) {
        // Actual short I/O only happens if we are not reading past last page
        w_assert0(first + count <= num_used_pages());
    }

    for (size_t i = 0; i < count; i++) {
        if (!_backup_alloc_cache->is_allocated(first + i)) {
            void* addr = &(reinterpret_cast<generic_page*>(buf)[i]);
            memset(addr, 0, sizeof(generic_page));
        }
    }
}

rc_t vol_t::take_backup(string path, bool flushArchive)
{
    // CS TODO -- use new RestoreCoordinator!
    return RC(eNOTIMPLEMENTED);

    // Open old backup file, if available
    bool useBackup = false;
    {
        spinlock_write_critical_section cs(&_mutex);

        if (_backup_write_fd >= 0) {
            return RC(eBACKUPBUSY);
        }

        _backup_write_path = path;
        int flags = O_SYNC | O_WRONLY | O_TRUNC | O_CREAT;
        auto fd = open(path.c_str(), flags, 0666 /*mode*/);
        CHECK_ERRNO(fd);
        _backup_write_fd = fd;

        useBackup = open_backup();
    }

    // No need to hold latch here -- mutual exclusion is guaranteed because
    // only one thread may set _backup_write_fd (i.e., open file) above.

    // Maximum LSN which is guaranteed to be reflected in the backup
    lsn_t backupLSN = ss_m::logArchiver->getIndex()->getLastLSN();
    DBG1(<< "Taking backup until LSN " << backupLSN);

    // CS TODO -- use new RestoreCoordinator!
    // // Instantiate special restore manager for taking backup
    // RestoreMgr restore(ss_m::get_options(), ss_m::logArchiver->getIndex().get(),
    //         this, useBackup, true /* takeBackup */);

    // restore.setInstant(false);
    // if (flushArchive) {
    //     lsn_t currLSN = smlevel_0::log->durable_lsn();
    //     restore.setFailureLSN(currLSN);
    //     DBGTHRD(<< "Taking sharp backup until " << currLSN);
    //     backupLSN = currLSN;
    // }

    // restore.start();
    // restore.shutdown();
    // TODO -- do we have to catch errors from restore thread?

    // Write volume header and metadata to new backup
    // (must be done after restore so that alloc pages are correct)
    // CS TODO
    // volhdr_t vhdr(_vid, _num_pages, backupLSN);
    // W_DO(vhdr.write(_backup_write_fd));

    // At this point, new backup is fully written
    W_DO(sx_add_backup(path, backupLSN));
    {
        // critical section to guarantee visibility of the fd update
        spinlock_write_critical_section cs(&_mutex);
        auto ret = close(_backup_write_fd);
        CHECK_ERRNO(ret);
        _backup_write_fd = -1;
    }

    DBG1(<< "Finished taking backup");

    return RCOK;
}

rc_t vol_t::write_backup(PageID first, size_t count, void* buf)
{
    w_assert0(_backup_write_fd > 0);
    w_assert1(count > 0);
    size_t offset = size_t(first) * sizeof(generic_page);

    auto ret = pwrite(_backup_write_fd, buf, sizeof(generic_page) * count, offset);
    CHECK_ERRNO(ret);

    DBG(<< "Wrote out " << count << " pages into backup offset " << offset);

    return RCOK;
}


/*********************************************************************
 *
 *  vol_t::write_many_pages(pnum, pages, cnt)
 *
 *  Write "cnt" buffers in "pages" to pages starting at "pnum"
 *  of the volume.
 *
 *********************************************************************/
rc_t vol_t::write_many_pages(PageID first_page, const generic_page* const buf, int cnt)
{
    if (_readonly) {
        // Write elision!
        return RCOK;
    }

    w_assert1(cnt > 0);
    size_t offset = size_t(first_page) * sizeof(generic_page);

#if W_DEBUG_LEVEL>0
    // Doesnt work for decoupled cleaner
    // for (int i = 0; i < cnt; i++) {
    //     w_assert1(buf[i].pid == first_page + i);
    // }
#endif

    std::chrono::high_resolution_clock::time_point sleep_until;
    if(_fake_write_latency.count()) {
        sleep_until = std::chrono::high_resolution_clock::now() + _fake_write_latency;
    }

    // do the actual write now
    auto ret = pwrite(_fd, buf, sizeof(generic_page)*cnt, offset);
    CHECK_ERRNO(ret);

    std::this_thread::sleep_until(sleep_until);

    ADD_TSTAT(vol_blks_written, cnt);
    INC_TSTAT(vol_writes);

    if (_log_page_writes) {
        Logger::log_sys<page_write_log>(first_page, cnt);
    }

    return RCOK;
}

uint32_t vol_t::get_last_allocated_pid() const
{
    w_assert1(_alloc_cache);
    return _alloc_cache->get_last_allocated_pid();
}

bool vol_t::is_allocated_page(PageID pid) const
{
    w_assert1(_alloc_cache);
    return _alloc_cache->is_allocated(pid);
}
