/*
 * (c) Copyright 2011-2014, Hewlett-Packard Development Company, LP
 */

#ifndef __LOCK_H
#define __LOCK_H

#include "w_defines.h"

#include "sm_base.h"
#include "w_okvl.h"
#include "w_okvl_inl.h"
#include "lock_s.h"

class xct_lock_info_t;
class lock_core_m;
class lil_global_table;
struct RawXct;
struct RawLock;
class sm_options;

/**
 * \brief Lock Manager API.
 * \ingroup SSMLOCK
 * See \ref OKVL and \ref LIL.
 */
class lock_m : public smlevel_0 {
public:
    // initialize/takedown functions for thread-local state
    static void on_thread_init();
    static void on_thread_destroy();

    NORET                        lock_m(const sm_options &options);
    NORET                        ~lock_m();

    /**
    * \brief Unsafely check that the lock table is empty for debugging
    *  and assertions at shutdown, when MT-safety shouldn't be an issue.
    */
    void                         assert_empty() const;

    /**
     * \brief Unsafely dump the lock hash table (for debugging).
     * \details Doesn't acquire the mutexes it should for safety, but
     * allows you dump the table while inside the lock manager core.
     */
    void                         dump(ostream &o);

    void                         stats(
                                    u_long & buckets_used,
                                    u_long & max_bucket_len,
                                    u_long & min_bucket_len,
                                    u_long & mode_bucket_len,
                                    float & avg_bucket_len,
                                    float & var_bucket_len,
                                    float & std_bucket_len
                                    ) const;

    lil_global_table*            get_lil_global_table();

    /**
     * \brief Returns the lock granted to the given transaction for this lock.
     * @return the lock mode the transaction has for this lock. ALL_N_GAP_N if not any.
     * \details
     * This method returns very quickly because it only checks transaction-private data.
     * @pre the current thread is the only thread running the current transaction
     */
    okvl_mode                   get_granted_mode(uint32_t hash, xct_t* xd = NULL);

    /**
     * \brief Acquires a lock of the given mode (or stronger)
     * @param[in] hash precise hash of the resource to lock.
     * @param[in] m requested lock mode
     * @param[in] check if true, this method doesn't actually create a new lock object
     * but just checks if the requested lock mode can be granted or not.
     * @param[in] wait If false, this method doesn't wait at all \b and also it leaves
     * the inserted lock entry even if it wasn't granted immediately.
     * @param[in] xd the transaction to own the new lock
     * @param[in] timeout maximum length to wait in milliseconds.
     * negative number means forever. If conditional, this parameter is ignored.
     * @param[out] out pointer to the \e successfully acquired lock. it returns NULL if
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
    rc_t lock(uint32_t hash, const okvl_mode &m,
            bool check, bool wait, bool acquire,
            xct_t* = NULL,
            int timeout = timeout_t::WAIT_SPECIFIED_BY_XCT,
            RawLock** out = NULL);

    /** @copydoc RawLockQueue::retry_acquire() */
    rc_t                        retry_lock(RawLock** lock, bool check_only,
                                           int timeout = timeout_t::WAIT_SPECIFIED_BY_XCT);

    /**
     * Take an intent lock on the given store.
     */
    rc_t                        intent_store_lock(StoreID stid, okvl_mode::element_lock_mode m);

    void                        unlock(RawLock* lock, lsn_t commit_lsn = lsn_t::null);

    rc_t                        unlock_duration(bool read_lock_only = false, lsn_t commit_lsn = lsn_t::null);

    void                        give_permission_to_violate(lsn_t commit_lsn = lsn_t::null);

    static void                 lock_stats(
        u_long&                      locks,
        u_long&                      acquires,
        u_long&                      cache_hits,
        u_long&                      unlocks,
        bool                         reset);

    RawXct*     allocate_xct();
    void        deallocate_xct(RawXct* xct);

private:
    int               _convert_timeout(int timeout);
    int               _convert_timeout(int timeout, xct_t* xd);
    lock_core_m*                core() const { return _core; }

    lock_core_m*                _core;
};

#endif // __LOCK_H
