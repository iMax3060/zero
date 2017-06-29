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

typedef uint32_t clk_idx;

class bf_tree_m;
class generic_page;
struct bf_tree_cb_t;

/*!\class   page_evictioner_base
 * \brief   Page Eviction Algorithm RANDOM (latched)
 *
 * \details Basic class for page eviction. Implements some functionality
 *          required for every page evictioner (need to inherit from this
 *          class) and implements a RANDOM page eviction.
 *          The RANDOM page eviction strategy does not collect any statistics
 *          about page references but just iterates over the buffer frames
 *          until it finds a page which can be latched in exclusive mode
 *          without waiting for another thread releasing the frame's latch.
 *          Therefore this replacement strategy is called "latched".
 *
 * \author  Caetano Sauer
 */
class page_evictioner_base : public worker_thread_t {
public:
    /*!\fn      page_evictioner_base(bf_tree_m* bufferpool, const sm_options& options)
     * \brief   Constructor for page_evictioner_base
     * \details This instantiates a page evictioner that uses the RANDOM algorithm
     *          to select victims for replacement. It will serve the specified
     *          \c bufferpool but won't use the specified \c options as this
     *          page replacement strategy doesn't need any further parameters.
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
    virtual ~page_evictioner_base();
    
    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void            hit_ref(bf_idx idx);
    
    /*!\fn      unfix_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page unfix
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was unfixed.
     */
    virtual void            unfix_ref(bf_idx idx);
    
    /*!\fn      miss_ref(bf_idx b_idx, PageID pid)
     * \brief   Updates the eviction statistics on page miss
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param b_idx The frame of the \link _bufferpool \endlink that was fixed with a
     *              page miss.
     * @param pid   The \link PageID \endlink of the \link generic_page \endlink that was
     *              loaded into the buffer frame.
     */
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    
    /*!\fn      used_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of used pages during eviction
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while it was fixed.
     */
    virtual void            used_ref(bf_idx idx);
    
    /*!\fn      dirty_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of dirty pages during eviction
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while the contained page is dirty.
     */
    virtual void            dirty_ref(bf_idx idx);
    
    /*!\fn      block_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages that cannot be evicted at all
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that contains a page that
     *            cannot be evicted at all.
     */
    virtual void            block_ref(bf_idx idx);
    
    /*!\fn      swizzle_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while containing a page with swizzled pointers.
     */
    virtual void            swizzle_ref(bf_idx idx);
    
    /*!\fn      unbuffered(bf_idx idx)
     * \brief   Updates the eviction statistics on explicit eviction
     * \details As RANDOM page eviction doesn't require any statistics, this function
     *          does nothing.
     *
     * @param idx The frame of the \link _bufferpool \endlink that is freed explicitly.
     */
    virtual void            unbuffered(bf_idx idx);


protected:
    /*!\var     _bufferpool
     * \bried   The bufferpool for which this evictioner is responsible for
     * \details This evictioner expects to be used for the eviction of pages from
     *          the bufferpool referenced here.
     */
    bf_tree_m*                  _bufferpool;
    
    /*!\var     _swizzling_enabled
     * \brief   Pointer Swizzling in the bufferpool
     * \details Set if the \link _bufferpool \endlink uses pointer swizzling for page references.
     */
    bool                        _swizzling_enabled;
    
    /*!\var     _maintain_emlsn
     * \brief   EMLSN-update by evictioner
     * \details Set if the parent's EMLSN should be updated during eviction.
     */
    bool                        _maintain_emlsn;
    
    
    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details This method uses the RANDOM algorithm to select one buffer frame which
     *          is expected to be used the furthest in the future (with the currently
     *          cached page). It acquires a LATCH_EX to prohibit the usage of the
     *          frame as the content of the buffer frame will definitely change.
     *
     * @return The buffer frame that can be freed or \c 0 if no eviction victim could
     *         be found.
     */
    virtual bf_idx          pick_victim();
    
    /*!\fn      evict_page(bf_idx idx, PageID &evicted_page)
     * \brief   Prepares a page for eviction
     * \details Checks a buffer frame if it can be freed (in use, contained page
     *          not pinned, etc.). If it can be freed, the checked buffer frame is
     *          latched in exclusive mode after the execution of this function.
     *
     * @param idx          The buffer frame that should be freed. If it can be freed,
     *                     it gets latched exclusively by this function.
     * @param evicted_page Returns the page in the selected buffer frame \c idx.
     * @return             Returns \c true if the page can be evicted and \c false
     *                     if some property prevents the eviction of that page.
     */
    bool evict_page(bf_idx idx, PageID &evicted_page);

    /*!\var     EVICT_BATCH_RATIO
     * \brief   Ratio of buffer frames freed as batch
     * \details When eviction is triggered, _about_ this number of buffer frames will
     *          be freed at once. If this amount of frames is already free, the
     *          eviction does nothing and goes back to sleep. Given as a ratio of the
     *          buffer size (currently 1%).
     */
    const float EVICT_BATCH_RATIO = 0.01;
    

private:
    /*!\var     _current_frame
     * \brief   Last buffer frame examined
     * \details The buffer frame index of the last frame that was checked by RANDOM
     *          page replacement.
     */
    bf_idx                      _current_frame;
    
    
    /*!\fn      unswizzle_and_update_emlsn(bf_idx idx)
     * \brief   Unswizzles the pointer in the parent page and updates the EMLSN of that
     *          page
     * \details In case swizziling is enabled, it will unswizzle the parent point.
     *          Additionally, it will update the parent EMLSN.
     *          This two operations are kept in a single method because both require
     *          looking up the parent, latching, etc., so we save some work.
     *
     * @param idx The buffer frame index where the page that gets evicted can be found.
     * @return    \c true if the .
     */
    bool unswizzle_and_update_emlsn(bf_idx idx);
    
    /*!\fn      do_work()
     * \brief   Function evicting pages in the eviciton thread
     * \details Runs in the eviction thread (executed when the eviction thread gets woken
     *          up and when terminated it terminates the eviction thread) and evicts pages
     *          as long as there are not that many buffer frames free as defined in
     *          \link EVICT_BATCH_RATIO \endlink.
     */
    virtual void do_work();
};

#endif
