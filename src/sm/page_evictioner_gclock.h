//
// Created by Max Gilbert on 20.06.17.
//

#ifndef ZERO_PAGE_EVICTIONER_GCLOCK_H
#define ZERO_PAGE_EVICTIONER_GCLOCK_H

#include "page_evictioner.h"

/*!\class   page_evictioner_gclock
 * \brief   Page Eviction Algorithm GCLOCK
 *
 * \details Page replacement algorithm GCLOCK as presented in
 *          <A HREF="http://doi.org/10.1145/320263.320276">"Sequentiality and
 *          Prefetching in Database Systems"</A> by Alan Jay Smith.
 *          To use this page eviction algorithm, the only thing to do is
 *          to set the parameter \c sm_evict_policy to \c gclock when
 *          starting the \c zapps. To set the k-parameter (i in the original
 *          paper), the parameter \c sm_bufferpool_gclock_k is offered by
 *          \c zapps. The default value is 10.
 *          On construction, this page evictioner needs to be connected to
 *          a bufferpool bf_tree_m for which this will serve. The bufferpool
 *          needs to call hit_ref(bf_idx idx) on every page hit and
 *          pick_victim() needs to be called to get a page to evict from the
 *          bufferpool.
 *
 * \author  Lucas Lersch
 */
class page_evictioner_gclock : public page_evictioner_base {
public:
    /*!\fn      page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options)
     * \brief   Constructor for page_evictioner_gclock
     * \details This instantiates a page evictioner that uses the GCLOCK algorithm
     *          to select victims for replacement. It will serve the specified
     *          \c bufferpool and it will use the \c sm_bufferpool_gclock_k
     *          parameter from the \c options to specify \link _k \endlink (default
     *          value is 10).
     *          It also initializes the \link _counts \endlink array of referenced
     *          counters and it initializes the clock hand \link _current_frame \endlink
     *          to the invalid frame 0 which gets fixed during the first execution of
     *          \link pick_victim() \endlink.
     *
     * @param bufferpool The bf_tree_m the constructed page evictioner is used to
     *                   select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
    page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options);
    
    /*!\fn      ~page_evictioner_gclock()
     * \brief   Destructor for page_evictioner_gclock
     * \details Destroys this instance and its \link _counts \endlink array of
     *          referenced counters.
     */
    virtual ~page_evictioner_gclock();
    
    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details Sets the referenced counter of the specified buffer frame \c idx to
     *          the value specified in \link _k \endlink.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void            hit_ref(bf_idx idx);
    
    /*!\fn      unfix_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page unfix
     * \details Sets the referenced counter of the specified buffer frame \c idx to
     *          the value specified in \link _k \endlink as this page was still used
     *          until this point in time.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was unfixed.
     */
    virtual void            unfix_ref(bf_idx idx);
    
    /*!\fn      miss_ref(bf_idx b_idx, PageID pid)
     * \brief   Updates the eviction statistics on page miss
     * \details There are three situations leading to empty buffer frames that require
     *          an initialized referenced counter when used the next time:
     *            - Buffer frame wasn't used since the startup: Referenced counters
     *              are initialized with 0 when a \link page_evictioner_gclock \endlink
     *              is constructed.
     *            - Buffer frame was freed explicitly: Therefore the function
     *              \link bf_tree_m::_add_free_block(bf_idx idx) \endlink was called. If
     *              the function was called from within \link page_evictioner_base::do_work() \endlink
     *              it is redundant to initialize the referenced counter here (see last
     *              case) but if another method called it, it is required as the reference
     *              counter could have any value.
     *            - The Buffer frame was freed by the evictioner: This only happens when
     *              the referenced counter of the frame is 0.
     *          Therefore, no action is required during a page miss as the initial value
     *          of the referenced counter is always already set.
     *
     * @param b_idx The frame of the \link _bufferpool \endlink that was fixed with a
     *              page miss.
     * @param pid   The \link PageID \endlink of the \link generic_page \endlink that was
     *              loaded into the buffer frame.
     */
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    
    /*!\fn      used_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of used pages during eviction
     * \details As GCLOCK logs page usage in its statistics, the referenced counter of a page
     *          which is encountered used needs to be handled like page hits.
     *          When a page is fixed while its referenced counter is 0, it is picked for
     *          eviction during each circulation of the clock hand. But the eviction fails
     *          as long as it is fixed and therefore the incrementing of the referenced
     *          counter delays the next time this page is picked for eviction and therefore
     *          this probably speeds up the eviction.
     *
     * @param idx The frame of the \link _bufferpool \endlink that had a referenced counter
     *            of 0 while it was fixed.
     */
    virtual void            used_ref(bf_idx idx);
    
    /*!\fn      dirty_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of dirty pages during eviction
     * \details As a dirty page shouldn't be picked for eviction until it is cleaned, it
     *          should be excluded from the eviction to increase the performance of the
     *          eviction but that is not implemented yet.
     *
     * @param idx The frame of the \link _bufferpool \endlink that had a referenced counter
     *            of 0 while the contained page is dirty.
     */
    virtual void            dirty_ref(bf_idx idx);
    
    /*!\fn      block_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages that cannot be evicted at all
     * \details As some pages are not allowed to be evicted at all (will never be allowed),
     *          those are excluded from the eviction by setting the referenced value to
     *          a large value.
     *
     * @param idx The frame of the \link _bufferpool \endlink that contains a page that
     *            cannot be evicted at all.
     */
    virtual void            block_ref(bf_idx idx);
    
    /*!\fn      swizzle_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
     * \details As a page containing swizzled pointers shouldn't be picked for eviction until the
     *          pointers are unswizzled, it should be excluded from the eviction to increase the
     *          performance of the eviction but that is not implemented yet.
     *
     * @param idx The frame of the \link _bufferpool \endlink that had a referenced counter
     *            of 0 while containing a page with swizzled pointers.
     */
    virtual void            swizzle_ref(bf_idx idx);
    
    /*!\fn      unbuffered(bf_idx idx)
     * \brief   Updates the eviction statistics on explicit eviction
     * \details When a page is evicted explicitly, the referenced counter of the corresponding frame
     *          might be greater than 0 and therefore this function initializes the counter for
     *          this case.
     *
     * @param idx The frame of the \link _bufferpool \endlink that is freed explicitly.
     */
    virtual void            unbuffered(bf_idx idx);


protected:
    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details This method uses the GCLOCK algorithm to select one buffer frame which
     *          is expected to be used the furthest in the future (with the currently
     *          cached page). It acquires a LATCH_EX to prohibit the usage of the
     *          frame as the content of the buffer frame will definitely change.
     *
     * @return The buffer frame that can be freed or \c 0 if no eviction victim could
     *         be found.
     */
    virtual bf_idx          pick_victim();


private:
    /*!\var     _k
     * \brief   k-parameter (set referenced counter to that value)
     * \details The k-parameter (i in the original paper) of the algorithm. When a
     *          page is referenced, its referenced counter is set to this value.
     */
    uint16_t            _k;
    
    /*!\var     _counts
     * \brief   Referenced counters per buffer frame
     * \details One referenced counter per buffer frame set to \link _k \endlink on
     *          page hits and decremented during the execution of
     *          \link pick_victim() \endlink.
     */
    uint16_t*           _counts;
    
    /*!\var     _current_frame
     * \brief   Clock hand into \link _counts \endlink
     * \details Represents the clock hand pointing to the buffer frame that was
     *          checked last during the most recent execution of
     *          \link pick_victim() \endlink (evicted last).
     */
    bf_idx              _current_frame;
};

#endif //ZERO_PAGE_EVICTIONER_GCLOCK_H
