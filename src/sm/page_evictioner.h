#ifndef PAGE_EVICTIONER_H
#define PAGE_EVICTIONER_H

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
     * \brief   Constructor for page_evictioner_base
     * \details This instantiates a page evictioner that uses the page eviction strategy
     *          according to the \c options to select victims for replacement. It will
     *          serve the specified \c bufferpool and uses the specified \c options to
     *          specify the behavior of the page evictioner.
     *
     * @param bufferpool The bf_tree_m the constructed page evictioner is used to
     *                   select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
	page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options);

    /*!\fn      ~page_evictioner_base()
     * \brief   Destructor for page_evictioner_base
     * \details Destroys this instance.
     */
    virtual                               ~page_evictioner_base();

    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the buffer
     *                               frame \c idx.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void                          hit_ref(bf_idx idx);

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
     * @return The buffer frame that can be freed or \c 0 \c if no eviction victim could
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

    /*!\var     _random_pick
     * \brief   Use RANDOM page eviction strategy
     * \details Set if each buffer frame picked for eviction is picked randomly and
     *          independent from previously picked buffer frames.
     */
    bool                                  _random_pick;

    /*!\var     _use_clock
     * \brief   Use CLOCK page eviction strategy
     * \details Set if the pages picked for eviction are picked using the simple CLOCK
     *          page replacement algorithm.
     */
    bool                                  _use_clock;

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

private:
    /*!\const   EVICT_BATCH_RATIO
     * \brief   Ratio of freed buffer frames during eviction
     * \details When eviction is triggered, _about_ this number of buffer frames will
     *          be freed at once. If this amount of frames is already free, the
     *          eviction does nothing and goes back to sleep. Given as a ratio of the
     *          buffer size (currently 1%).
     */
    constexpr float                       EVICT_BATCH_RATIO = 0.01;

    /*!\var     _current_frame
     * \brief   Last control block examined
     * \details Represents the clock hand pointing to the control block that was
     *          examined last during the most recent execution of
     *          \link pick_victim() \endlink (evicted last).
     *
     * \remark  Only used by __LOOP__ and __CLOCK__.
     */
    std::atomic<bf_idx>                   _current_frame;

    /*!\var     _clock_ref_bits
     * \brief   Referenced bit for the buffer frames
     * \details One referenced bit per buffer frame set to \c true on page hit and set
     *          to \c false during the execution of \link pick_victim() \endlink.
     *
     * \remark  Only used by __0CLOCK__ and __CLOCK__.
     */
    std::vector<bool>                     _clock_ref_bits;

    /*!\fn      unswizzle_and_update_emlsn(bf_idx idx)
     * \brief   Unswizzles the pointer in the parent page and updates the EMLSN of that
     *          page
     * \details In case swizzling is enabled, it will unswizzle the parent point.
	 *          Additionally, it will update the parent EMLSN.
	 *          This two operations are kept in a single method because both require
	 *          looking up the parent, latching, etc., so we save some work.
     *
     * @param idx The buffer frame index where the page that gets evicted can be found.
     * @return    \c true \c if the .
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

/*!\class   page_evictioner_gclock
 * \brief   Page Eviction Algorithm GCLOCK
 *
 * \details Page replacement algorithm GCLOCK as presented in
 *          <A HREF="http://doi.org/10.1145/320263.320276">"Sequentiality and
 *          Prefetching in Database Systems"</A> by Alan Jay Smith. To use this page
 *          eviction algorithm, the only thing to do is to set the parameter
 *          \c sm_evict_policy to \c gclock when starting the \c zapps. To set the
 *          \f$ k \f$-parameter (\f$ i \f$ in the original paper), the parameter
 *          \c sm_bufferpool_gclock_k is offered by \c zapps. The default value is 10.
 *
 * \author  Lucas Lersch
 */
class page_evictioner_gclock : public page_evictioner_base {
public:
    /*!\fn      page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options)
     * \brief   Constructor for page_evictioner_gclock
     * \details This instantiates a page evictioner that uses the GCLOCK page eviction
     *          strategy to select victims for replacement. It will serve the specified
     *          \c bufferpool and it will use the \c sm_bufferpool_gclock_k
     *          parameter from the \c options to specify \link _k \endlink member
     *          variable (default value is 10). It also initializes the
     *          \link _counts \endlink array of referenced counters and it initializes
     *          the clock hand \link _current_frame \endlink to the invalid frame 0
     *          which gets examined during the first execution of
     *          \link pick_victim() \endlink.
     *
     * @param bufferpool The buffer pool the constructed page evictioner is used to
     *                   select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
    page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options);

    /*!\fn      ~page_evictioner_gclock()
     * \brief   Destructor for page_evictioner_gclock
     * \details Destroys this instance and its \link _counts \endlink array of
     *          referenced counters.
     */
    virtual        ~page_evictioner_gclock();

    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details Sets the referenced counter of the specified buffer frame \c idx to
     *          the value specified in \link _k \endlink.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void   hit_ref(bf_idx idx);

protected:
    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details This method uses the GCLOCK algorithm to select one buffer frame which
     *          is expected to be used the furthest in the future (with the currently
     *          cached page).
     *
     * \post    The picked victim is latched in \link latch_mode_t::LATCH_EX \endlink
     *          mode as the buffer pool frame will be changed during eviction (page
     *          will be removed).
     *
     * @return The buffer frame that can be freed or \c 0 if no eviction victim could
     *         be found.
     */
    virtual bf_idx pick_victim();

private:
    /*!\var     _k
     * \brief   \f$ k \f$-parameter
     * \details The \f$ k \f$-parameter (\f$ i \f$ in the original paper) of the
     *          algorithm. When a page is referenced, its referenced counter is set to
     *          this value.
     */
    uint16_t       _k;

    /*!\var     _counts
     * \brief   Referenced counter for the buffer frames
     * \details One referenced counter per buffer frame set to \link _k \endlink on
     *          page hits and decremented during the execution of
     *          \link pick_victim() \endlink.
     */
    uint16_t*      _counts;

    /*!\var     _current_frame
     * \brief   Last buffer frame examined
     * \details Represents the clock hand pointing to the buffer frame that was checked
     *          last during the most recent execution of \link pick_victim() \endlink
     *          (evicted last).
     */
    bf_idx         _current_frame;
};

#endif
