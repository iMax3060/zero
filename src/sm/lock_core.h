/*
 * (c) Copyright 2011-2013, Hewlett-Packard Development Company, LP
 */

#ifndef __LOCK_CORE_H
#define __LOCK_CORE_H

#include <cstdint>
#include "lsn.h"

struct RawLock;
struct RawLockQueue;
struct RawXct;
class RawLockBackgroundThread;

template<class T>
struct GcPoolForest;
class sm_options;
struct RawLockCleanerFunctor;
class lil_global_table;
class vtable_t;
class okvl_mode;

/**
* \brief Lock table implementation class.
* \ingroup SSMLOCK
* \details
* This is the gut of lock management in Foster B-trees.
* Most of the implementation has been moved to lock_raw.h/cpp.
*/
class lock_core_m {
public:
    lock_core_m(const sm_options& options);

    ~lock_core_m();

    int collect(vtable_t&, bool names_too);

    void assert_empty() const;

    void dump(std::ostream& o);

    lil_global_table* get_lil_global_table() {
        return _lil_global_table;
    }

public:
    /**
     * \brief Adds a new lock in the given mode to this queue, waiting until it is granted.
     * @param[in] xct the transaction to own the new lock
     * @param[in] hash precise hash of the resource to lock.
     * @param[in] mode requested lock mode
     * @param[in] check If true, this method doesn't wait at all \b and also it leaves
     * the inserted lock entry even if it wasn't granted immediately.
     * @param[in] wait if false, this method doesn't actually create a new lock object
     * but just checks if the requested lock mode can be granted or not.
     * @param[in] timeout int maximum length to wait in milliseconds.
     * negative number means forever. If conditional, this parameter is ignored.
     * @param[out] out out pointer to the \e successfully acquired lock. it returns NULL if
     * we couldn't get the lock \b except conditional==true case.
     * \details
     * \b check_only=true can give a false positive in concurrent unlock case, but
     * give no false negative \b assuming a conflicting lock is not concurrently taken for
     * the key. This assumption holds for our only check_only=true use case, which is the
     * tentative NX lock check before inserting a new key, \b because we then have an EX latch!
     * Thus, this is a safe and efficient check for B-tree insertion.
     *
     * \b conditional locking is the standard way to take a lock in DBMS without leaving
     * latches long time. B-tree first requests a lock without releasing latch (conditional).
     * If it fails, it releases latch and unconditionally lock, which needs re-check of LSN
     * after lock and re-latch. The purpose of this \e conditional parameter is that we don't
     * want to insert the same lock entry twice when the first conditional locking fails.
     * When conditional==true, we leave the lock entry and return it in \e out even if it
     * wasn't granted. The caller \e MUST be responsible to call retry_acquire() after the
     * failed acquire (which returns eCONDLOCKTIMEOUT if it failed) or release the lock.
     * It is anyway released at commit time, but waiting lock entry should be removed
     * before the transaction does anything else.
     *
     * @pre out != NULL
     */
    w_error_codes acquire_lock(RawXct* xct, uint32_t hash, const okvl_mode& mode,
                               bool check, bool wait, bool acquire, int32_t timeout, RawLock** out);

    /** @copydoc RawLockQueue::retry_acquire() */
    w_error_codes retry_acquire(RawLock** lock, bool check_only, int32_t timeout);

    void release_lock(RawLock* lock, lsn_t commit_lsn = lsn_t::null);

    void release_duration(bool read_lock_only = false, lsn_t commit_lsn = lsn_t::null);

    /**
     * Instantiate shadow transaction object for RAW-style lock manager for the current thread.
     */
    RawXct* allocate_xct();

    void deallocate_xct(RawXct* xct);

private:
    uint32_t _table_bucket(uint32_t id) const {
        return id % _htabsz;
    }

    GcPoolForest<RawLock>* _lock_pool;

    GcPoolForest<RawXct>* _xct_pool;

    RawLockCleanerFunctor* _raw_lock_cleaner_functor;

    RawLockBackgroundThread* _raw_lock_cleaner;

    RawLockQueue* _htab;

    uint32_t _htabsz;

    /** Global lock table for Light-weight Intent Lock. */
    lil_global_table* _lil_global_table;
};

// TODO to remove
// this is for experiments to compare deadlock detection/recovery methods.
#define SWITCH_DEADLOCK_IMPL
#ifdef SWITCH_DEADLOCK_IMPL

/** Whether to use the dreadlock sleep-backoff algorithm? */
extern bool g_deadlock_use_waitmap_obsolete;

/** How long to sleep between each dreadlock spin? */
extern int g_deadlock_dreadlock_interval_ms;

class xct_t;
class lock_request_t;

/** function pointer for the implementation of arbitrary _check_deadlock impl. */
extern w_error_codes (* g_check_deadlock_impl)(xct_t* xd, lock_request_t* myreq);

#endif // SWITCH_DEADLOCK_IMPL

#endif // __LOCK_CORE_H /*</std-footer>*/
