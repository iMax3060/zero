#ifndef __PAGE_EVICTIONER_H
#define __PAGE_EVICTIONER_H

#include "smthread.h"
#include "sm_options.h"
#include "lsn.h"
#include "bf_hashtable.h"
#include "allocator.h"
#include "generic_page.h"
#include "bf_tree_cb.h"

#include "worker_thread.h"

#include <random>

class bf_tree_m;
class generic_page;
struct bf_tree_cb_t;

/*!\class   page_evictioner_base
 * \brief   Basic Page Evictioner incl. RANDOM, LOOP, 0CLOCK and CLOCK strategies
 * \details Basic class for page eviction. Implements some common functionality
 *          required for every page evictioner (need to inherit from this class)
 *          and also implements the page eviction algorithms RANDOM, LOOP, 0CLOCK
 *          and CLOCK.
 *          - __RANDOM:__ The RANDOM page eviction strategy does not collect any
 *                        statistics about page references but just randomly
 *                        (independent and uniformly distributed) examines buffer
 *                        frames until it finds a frame which can be latched in
 *                        \link latch_mode_t::LATCH_EX \endlink mode without waiting
 *                        for another thread releasing the latch of the buffer frame.\n
 *                        To activate this page eviction strategy set the storage
 *                        manager option \c sm_evict_random and unset
 *                        \c sm_evict_use_clock.
 *          - __LOOP:__   The LOOP page eviction strategy does not collect any
 *                        statistics about page references but just loops over
 *                        the buffer frames (starting after the previously freed
 *                        buffer frame) until it finds a frame which can be latched
 *                        in \link latch_mode_t::LATCH_EX \endlink mode without
 *                        waiting for another thread releasing the latch of the
 *                        buffer frame.\n
 *                        To activate this page eviction strategy unset the storage
 *                        manager options \c sm_evict_random and
 *                        \c sm_evict_use_clock.
 *          - __0CLOCK:__ The 0CLOCK (_Zero-Handed CLOCK_ or _Zero-Bit CLOCK_) page
 *                        eviction strategy sets the referenced bit of the
 *                        corresponding buffer frame during each page reference and
 *                        unsets this bit when the frame was examined during eviction.
 *                        It randomly (independent and uniformly distributed) picks
 *                        buffer frames until it finds a frame whose referenced bit is
 *                        unset and which can be latched in
 *                        \link latch_mode_t::LATCH_EX \endlink mode without waiting
 *                        for another thread releasing the latch of the buffer frame.\n
 *                        To activate this page eviction strategy set the storage
 *                        manager option \c sm_evict_use_clock and unset
 *                        \c sm_evict_random.
 *          - __CLOCK:__  The CLOCK page eviction strategy sets the referenced bit of
 *                        the corresponding buffer pool frame during each page
 *                        reference and unsets this bit when the frame was examined
 *                        during eviction. It loops over the buffer frames (starting
 *                        after the previously freed buffer frame) until it finds a
 *                        frame whose referenced bit is unset and which can be latched
 *                        in \link latch_mode_t::LATCH_EX \endlink mode without
 *                        waiting for another thread releasing the latch of the buffer
 *                        frame.\n
 *                        To activate this page eviction strategy set the storage
 *                        manager options \c sm_evict_random and
 *                        \c sm_evict_use_clock.
 *
 * \remark  Every page evictioner implementing another page eviction strategy needs to
 *          inherit from this class.
 *
 * \author  Caetano Sauer
 */
class page_evictioner_base : public worker_thread_t {
public:
    /*!\fn      page_evictioner_base(bf_tree_m* buffer pool, const sm_options& options)
     * \brief   Constructor for \c page_evictioner_base
     * \details This instantiates a page evictioner that uses the page eviction
     *          strategy according to the \c options to select victims for replacement.
     *          It will serve the specified \c bufferpool and uses the specified
     *          \c options to specify the behavior of the page evictioner.
     *
     * @param bufferpool The \link bf_tree_m \endlink the constructed page evictioner is
     *                   used to select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
	page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options);

    /*!\fn      ~page_evictioner_base()
     * \brief   Destructor for page_evictioner_base
     * \details Destroys this instance.
     */
    virtual                               ~page_evictioner_base();

    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details Selects a page to be evicted according to the selected page eviction
     *          strategy:
     *          - __RANDOM:__ Examines random (independent and uniformly distributed)
     *                        buffer frames until it finds a frame which can be latched
     *                        in \link latch_mode_t::LATCH_EX \endlink mode without
     *                        waiting for another thread releasing the latch of the
     *                        buffer frame.
     *          - __LOOP:__   Examines buffer frames in a loop (starting after the
     *                        previously freed buffer frame) until it finds a frame
     *                        which can be latched in
     *                        \link latch_mode_t::LATCH_EX \endlink mode without
     *                        waiting for another thread releasing the latch of the
     *                        buffer frame.
     *          - __0CLOCK:__ Examines random (independent and uniformly distributed)
     *                        buffer frames until it finds a frame whose referenced
     *                        bit is unset and which can be latched in
     *                        \link latch_mode_t::LATCH_EX \endlink mode without
     *                        waiting for another thread releasing the latch of the
     *                        buffer frame.
     *          - __CLOCK:__  Examines buffer frames in a loop (starting after the
     *                        previously freed buffer frame) until it finds a frame
     *                        whose referenced bit is unset and which can be latched in
     *                        \link latch_mode_t::LATCH_EX \endlink mode without
     *                        waiting for another thread releasing the latch of the
     *                        buffer frame.
     * \post    The picked victim is latched in \link latch_mode_t::LATCH_EX \endlink
     *          mode as the buffer pool frame will be changed during eviction (page
     *          will be removed).
     *
     * @return The buffer frame that can be freed or \c 0 if no eviction victim could
     *         be found.
     */
    virtual bf_idx                        pick_victim();

    /*!\fn      evict_one(bf_idx idx)
     * \brief   Evicts the page from the specified buffer frame
     * \details Evicts the page from the specified buffer frame corresponding to index
     *          \c idx. If enabled, it unswizzles the page, updates its EMLSN, takes
     *          care about the cleaning of the page and removes it from the hash table
     *          \link bf_tree_m::_hashtable \endlink.
     * \pre     The buffer frame corresponding to index \c idx is latched in
     *          \link latch_mode_t::LATCH_EX \endlink mode by this thread.
     *
     * @param idx          The index of the buffer frame that gets be freed.
     * @return             Returns \c true if the page could be evicted and \c false
     *                     if some property prevented the eviction of that page.
     */
    bool                                  evict_one(bf_idx);

protected:
    /*!\var     _bufferpool
     * \brief   This evictioner's corresponding buffer pool
     * \details The buffer pool for which this evictioner is responsible to evict pages
     *          from.
     */
    bf_tree_m*                            _bufferpool;

    /*!\var     _swizzling_enabled
     * \brief   Pointer swizzling used in the buffer pool
     * \details Set if the \link _bufferpool \endlink uses pointer swizzling for page
     *          references.
     */
    bool                                  _swizzling_enabled;

    /*!\var     _write_elision
     * \brief   TODO
     * \details TODO
     */
    bool                                  _write_elision;

    /*!\var     _no_db_mode
     * \brief   TODO
     * \details TODO
     */
    bool                                  _no_db_mode;

    /*!\var     _maintain_emlsn
     * \brief   Maintain parent's EMLSN
     * \details Set if the parent's EMLSN should be updated during eviction.
     */
    bool                                  _maintain_emlsn;

    /*!\var     _flush_dirty
     * \brief   Flush dirty pages
     * \details Set if dirty pages should be flushed when they get evicted from the
     *          buffer pool.
     */
    bool                                  _flush_dirty;

    /*!\var     _log_evictions
     * \brief   Log page evictions
     * \details Set if a log record should be created for each eviction of a page.
     */
    bool                                  _log_evictions;

    /*!\var     _rnd_gen
     * \brief   Pseudo-random number generator for RANDOM
     * \details Pseudo-random number generator used (together with
     *          \link _rnd_distr \endlink) by the RANDOM page eviction strategy to
     *          generate a random buffer frame index for eviction.
     */
    std::default_random_engine            _rnd_gen;

    /*!\var     _rnd_distr
     * \brief   Buffer frame index distribution for RANDOM
     * \details Produces uniformly distributed, random buffer frame indexes (together
     *          with \link _rnd_gen \endlink), used by the RANDOM page eviction
     *          strategy to pick a buffer frame for eviction.
     */
    std::uniform_int_distribution<bf_idx> _rnd_distr;

    /*!\fn      get_random_idx()
     * \brief   Get a random buffer frame index
     * \details Returns a uniformly distributed, random and valid buffer frame index
     *          greater than 0.
     *
     * @return A uniformly distributed, random and valid buffer frame index greater
     *         than 0.
     */
    bf_idx                                get_random_idx() {
        return _rnd_distr(_rnd_gen);
    }

    /*!\var     _max_attempts
     * \brief   Maximum number of unsuccessful picks of eviction victims
     * \details Maximum number of picks of pages in a row that cannot be evicted before
     *          an error (\"Eviction got stuck!\") is thrown.
     */
    unsigned                              _max_attempts;

    /*!\var     _wakeup_cleaner_attempts
     * \brief   Number of unsuccessful picks of eviction victims before waking up the
     *          page cleaner
     * \details Number of picks of pages in a row that cannot be evicted before the
     *          page cleaner of the \link _bufferpool \endlink gets woken up
     *          (\link bf_tree_m::wakeup_cleaner() \endlink).
     *
     * \remark  Dirty pages that need to get cleaned are a typical reason for stuck
     *          page evictions.
     */
    unsigned                              _wakeup_cleaner_attempts;

    // Dirty pages are flushed after this many eviction attempts
    /*!\var     _clean_only_attempts
     * \brief   TODO
     * \details TODO
     */
    unsigned                              _clean_only_attempts;

private:
    /*!\var     EVICT_BATCH_RATIO
     * \brief   Ratio of freed buffer frames during eviction
     * \details When eviction is triggered, _about_ this number of buffer frames will
     *          be freed at once. If this amount of frames is already free, the
     *          eviction does nothing and goes back to sleep. Given as a ratio of the
     *          buffer size (currently 1%).
     */
    static constexpr float                EVICT_BATCH_RATIO = 0.01;

    /*!\fn      unswizzle_and_update_emlsn(bf_idx idx)
     * \brief   Unswizzles the pointer in the parent page and updates the EMLSN of that
     *          page
     * \details In case swizzling is enabled, it will unswizzle the parent point.
	 *          Additionally, it will update the parent EMLSN.
	 *          This two operations are kept in a single method because both require
	 *          looking up the parent, latching, etc., so we save some work.
     *
     * @param idx The buffer frame index where the page that gets evicted can be found.
     * @return    \c true if the .
     */
	bool                                  unswizzle_and_update_emlsn(bf_idx idx);

    /*!\fn      flush_dirty_page(const bf_tree_cb_t& cb)
     * \brief   Flush specified page
     * \details Writes the specified page to the database file.
     *
     * \pre     Specified \link bf_tree_cb_t \endlink is latched in exclusive mode and
     *          corresponds to a used buffer frame.
     *
     * @param cb The control block that corresponds to the buffer frame whose contained
     *           page gets flushed.
     */
    void                                  flush_dirty_page(const bf_tree_cb_t& cb);

	/*!\fn      do_work()
	 * \brief   Function evicting pages in the eviction thread
	 * \details Runs in the eviction thread (executed when the eviction thread gets
	 *          woken up and when terminated it terminates the eviction thread) and
	 *          evicts pages as long as there are not that many buffer frames free as
	 *          defined in \link EVICT_BATCH_RATIO \endlink.
	 */
	virtual void                          do_work();
};

#endif // __PAGE_EVICTIONER_H
