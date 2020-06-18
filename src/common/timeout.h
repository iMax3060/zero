#ifndef __TIMEOUT_T
#define __TIMEOUT_T

/**\struct timeout_t
 * \brief  Special values for timeouts (int-values).
 * \details sthreads package recognizes 2 \c WAIT_* values:
 *            - \c == \link WAIT_IMMEDIATE \endlink
 *            - \c != \link WAIT_IMMEDIATE \endlink
 *          If it's not \link WAIT_IMMEDIATE \endlink, it's assumed to be a
 *          positive integer (milliseconds) used for the select timeout.\n
 *          The user of the thread (e.g., sm) had better convert timeout that
 *          are negative values (\c WAIT_* below) to something \c >= \c 0
 *          before calling \c block().\n
 *          All other \c WAIT_* values other than \link WAIT_IMMEDIATE \endlink
 *          are handled by sm layer.
 *
 * @param WAIT_IMMEDIATE           Don't wait.
 * @param WAIT_FOREVER             May block indefinitely.
 * @param WAIT_SPECIFIED_BY_THREAD Pick up a timeout from the smthread.
 * @param WAIT_SPECIFIED_BY_XCT    Pick up a timeout from the transaction.
 * @param WAIT_NOT_USED            Indicates the negative number used by
 *                                 sthreads.
 *
 * \see int
 */
struct timeout_t {
    static constexpr int WAIT_IMMEDIATE = 0;
    static constexpr int WAIT_FOREVER = -1;
    static constexpr int WAIT_SPECIFIED_BY_THREAD = -4; // used by lock manager
    static constexpr int WAIT_SPECIFIED_BY_XCT = -5; // used by lock manager
    // CS: I guess the NOT_USED value is only for threads that never acquire
    // any locks? And neither latches?
    static constexpr int WAIT_NOT_USED = -6; // indicates last negative number used by sthreads
};

#endif // __TIMEOUT_T
