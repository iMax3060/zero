#ifndef __PAGE_EVICTIONER_H
#define __PAGE_EVICTIONER_H

#include "smthread.h"
#include "sm_options.h"
#include "lsn.h"
#include "allocator.h"
#include "generic_page.h"
#include "bf_tree_cb.h"

#include "worker_thread.h"

#include <random>

namespace zero::buffer_pool {
    class BufferPool;
}
class generic_page;
struct bf_tree_cb_t;

using namespace zero::buffer_pool;

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
     * @param bufferpool The \link zero::buffer_pool::BufferPool \endlink the constructed page evictioner is
     *                   used to select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
	page_evictioner_base(BufferPool* bufferpool, const sm_options& options);

    /*!\fn      ~page_evictioner_base()
     * \brief   Destructor for page_evictioner_base
     * \details Destroys this instance.
     */
    virtual                               ~page_evictioner_base();

    // TODO: hit_ref, unfix_ref or both?
    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Does nothing as recognizing an unfix instead of
     *                               a fix optimizes the quality of a page evictioner.
     *                               At least according to
     *                               <A HREF="https://doi.org/10.1145/1994.2022">"Principles of Database Buffer Management"</A>
     *                               by W. Effelsberg and T. Härder.
     *
     * \warning If a page eviction strategy is sensitive to recognizing the same page
     *          reference multiple times, implement this different from
     *          \link unfix_ref(bf_idx) \endlink.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void                          hit_ref(bf_idx idx);

    /*!\fn      unfix_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page unfix
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the
     *                               buffer frame \c idx. This action is taken
     *                               according to the CLOCK page replacement strategy.
     *                               The recognition of unfixes instead of fixes as
     *                               page references is an optimization of a page
     *                               evictioner. At least according to
     *                               <A HREF="https://doi.org/10.1145/1994.2022">"Principles of Database Buffer Management"</A>
     *                               of W. Effelsberg and T. Härder.

     * \warning If a page eviction strategy is sensitive to recognizing the same page
     *          reference multiple times, implement this different from
     *          \link unfix_ref(bf_idx) \endlink.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was unfixed.
     */
    virtual void            unfix_ref(bf_idx idx);

    /*!\fn      miss_ref(bf_idx b_idx, PageID pid)
     * \brief   Updates the eviction statistics on page miss
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the
     *                               buffer frame \c idx. This action is taken
     *                               according to the CLOCK page replacement strategy.
     *
     * @param b_idx The frame of the \link _bufferpool \endlink that was fixed with a
     *              page miss.
     * @param pid   The \link PageID \endlink of the \link generic_page \endlink that
     *              was loaded into the buffer frame.
     */
    virtual void                          miss_ref(bf_idx b_idx, PageID pid);

    /*!\fn      used_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of used pages during eviction
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the
     *                               buffer frame \c idx to reduce the number of
     *                               control block examinations when a frame is fixed
     *                               (only once) for a long time.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while it was fixed.
     */
    virtual void                          used_ref(bf_idx idx);

    /*!\fn      dirty_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of dirty pages during eviction
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Does nothing as the evictioner periodically
     *                               runs the page cleaner and therefore it is
     *                               unlikely that the page won't be cleaned before
     *                               the next examination of the corresponding control
     *                               block.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while the contained page is dirty.
     */
    virtual void                          dirty_ref(bf_idx idx);

    /*!\fn      block_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages that cannot be evicted at all
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the
     *                               buffer frame \c idx to reduce the number of
     *                               control block examinations when it will not be
     *                               evictable at all.
     *
     * @param idx The frame of the \link _bufferpool \endlink that contains a page that
     *            cannot be evicted at all.
     */
    virtual void                          block_ref(bf_idx idx);

    /*!\fn      swizzle_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages containing swizzled pointers
     *          during eviction
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Sets the referenced bit corresponding to the
     *                               buffer frame \c idx to reduce the number of
     *                               control block examinations when it is very
     *                               unlikely that it will be evictable anytime soon.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while containing a page with swizzled pointers.
     */
    virtual void                          swizzle_ref(bf_idx idx);

    /*!\fn      unbuffered(bf_idx idx)
     * \brief   Updates the eviction statistics on explicit eviction
     * \details According to the selected page eviction strategy:
     *          - __RANDOM, LOOP:__  Does nothing.
     *          - __0CLOCK, CLOCK:__ Unsets the referenced bit corresponding to the
     *                               buffer frame \c idx to leave the referenced bit in
     *                               its empty frame state. There are three situations
     *                               leading to empty buffer frames that require an
     *                               initialized referenced bit when used the next
     *                               time:
     *                               -# Buffer frame wasn't used since the startup:
     *                                  Referenced bits are initialized with \c false
     *                                  when a \link page_evictioner_base \endlink is
     *                                  constructed.
     *                               -# Buffer frame was freed explicitly: Therefore
     *                                  the function
     *                                  \link zero::buffer_pool::FreeList::addFreeBufferpoolFrame(bf_idx idx) \endlink
     *                                  was called. If the function was called from
     *                                  within
     *                                  \link page_evictioner_base::do_work() \endlink
     *                                  it is redundant to initialize the referenced
     *                                  bit to \c false here (see last case) but if
     *                                  another method called it, it is required as the
     *                                  reference bit could have any value.
     *                               -# The Buffer frame was freed by the evictioner:
     *                                  This only happens when the referenced bit of
     *                                  the frame is unset.
     *
     * @param idx The frame of the \link _bufferpool \endlink that is freed explicitly.
     */
    virtual void                          unbuffered(bf_idx idx);

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
     *          \link zero::buffer_pool::BufferPool::_hashtable \endlink.
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
    BufferPool*                           _bufferpool;

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
     *          (\link zero::buffer_pool::BufferPool::wakeupPageCleaner() \endlink).
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
