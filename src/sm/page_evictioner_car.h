#ifndef ZERO_PAGE_EVICTIONER_CAR_H
#define ZERO_PAGE_EVICTIONER_CAR_H

#include "page_evictioner.h"
#include "multi_clock.h"
#include "hashtable_queue.h"

/*!\class   page_evictioner_car
 * \brief   Page Eviction Algorithm CAR
 *
 * \details Page replacement algorithm CAR as presented in
 *          <A HREF="http://www-cs.stanford.edu/~sbansal/pubs/fast04.pdf">
 *          "CAR: Clock with Adaptive Replacement"</A> by Sorav Bansal and
 *          Dharmendra S. Modha.
 *          To use this page eviction algorithm, the only thing to do is
 *          to set the parameter \c sm_evict_policy to \c car when
 *          starting the \c zapps. Other parameters aren't needed as this
 *          page replacement algorithm is self-tuning.
 *          On construction, this page evictioner needs to be connected to
 *          a bufferpool bf_tree_m for which this will serve. The bufferpool
 *          needs to call ref(bf_idx idx) on every page hit,
 *          miss_ref(bf_idx b_idx, PageID pid) on every page miss and
 *          pick_victim() needs to be called to get a page to evict from the
 *          bufferpool.
 *
 * \author  Max Gilbert
 */
class page_evictioner_car : public page_evictioner_base {
public:
    /*!\fn      page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options)
     * \brief   Constructor for page_evictioner_car
     * \details This instantiates a page evictioner that uses the CAR algorithm
     *          to select victims for replacement. It will serve the specified
     *          \c bufferpool but won't use the specified \c options as this
     *          page replacement strategy doesn't need any "magic" parameters.
     *
     * @param bufferpool The bf_tree_m the constructed page evictioner is used to
     *                   select pages for eviction for.
     * @param options    The options passed to the program on startup.
     */
    page_evictioner_car(bf_tree_m *bufferpool, const sm_options &options);
    
    /*!\fn      ~page_evictioner_car()
     * \brief   Destructor for page_evictioner_car
     * \details Destroys this instance including the \link _lock \endlink.
     */
    virtual             ~page_evictioner_car();
    
    /*!\fn      hit_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page hit
     * \details As a page currently fixed cannot be evicted, setting the referenced bit of
     *          the corresponding buffer frame is not required to prevent its eviction. We
     *          therefore set the referenced bit not during the page fix but during the
     *          page unfix.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was fixed with a
     *            page hit.
     */
    virtual void        hit_ref(bf_idx idx);
    
    /*!\fn      unfix_ref(bf_idx idx)
     * \brief   Updates the eviction statistics on page unfix
     * \details Sets the referenced bit of the specified buffer frame. This prevents
     *          the evictioner to evict this page during the next circulation of the
     *          corresponding clock.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was unfixed.
     */
    virtual void            unfix_ref(bf_idx idx);
    
    /*!\fn      miss_ref(bf_idx b_idx, PageID pid)
     * \brief   Updates the eviction statistics on page miss
     * \details Classifies the specified buffer frame to be in clock \f$T_1\f$ or
     *          \f$T_2\f$ based on the membership of the referenced page in either
     *          \f$B_1\f$, \f$B_2\f$ of none of the LRU-lists. It also removes entries
     *          from the LRU-lists \f$B_1\f$ or \f$B_2\f$ if needed. The referenced
     *          bit of the specified buffer frame will be unset.
     *
     * @param b_idx The frame of the \link _bufferpool \endlink where the fixed page
     *              is cached in.
     * @param pid   The \c PageID of the fixed page.
     */
    virtual void        miss_ref(bf_idx b_idx, PageID pid);
    
    /*!\fn      used_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of used pages during eviction
     * \details As CAR logs page fixes in specific time intervals, a page fixed for
     *          a longer timespan must not set the corresponding referenced bit as this
     *          would be recognized as repeated usage and therefore the page would be
     *          promoted to \f$T_2\f$.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while it was fixed.
     */
    virtual void        used_ref(bf_idx idx);
    
    /*!\fn      dirty_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of dirty pages during eviction
     * \details As a dirty page shouldn't be picked for eviction until it is cleaned, it
     *          should be excluded from the eviction to increase the performance of the
     *          eviction but that is not implemented yet.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while the contained page is dirty.
     */
    virtual void        dirty_ref(bf_idx idx);
    
    /*!\fn      block_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages that cannot be evicted at all
     * \details As some pages are not allowed to be evicted at all (will never be allowed),
     *          those should be excluded from the eviction but that is not implemented
     *          yet.
     *
     * @param idx The frame of the \link _bufferpool \endlink that contains a page that
     *            cannot be evicted at all.
     */
    virtual void        block_ref(bf_idx idx);
    
    /*!\fn      swizzle_ref(bf_idx idx)
     * \brief   Updates the eviction statistics of pages containing swizzled pointers during eviction
     * \details As a page containing swizzled pointers shouldn't be picked for eviction until the
     *          pointers are unswizzled, it should be excluded from the eviction to increase the
     *          performance of the eviction but that is not implemented yet.
     *
     * @param idx The frame of the \link _bufferpool \endlink that was picked for
     *            eviction while containing a page with swizzled pointers.
     */
    virtual void        swizzle_ref(bf_idx idx);
    
    /*!\fn      unbuffered(bf_idx idx)
     * \brief   Updates the eviction statistics on explicit eviction
     * \details When a page is evicted explicitly, the corresponding buffer frame index
     *          is removed from the clock \f$T_1\f$ or \f$T_2\f$.
     *
     * @param idx The frame of the \link _bufferpool \endlink that is freed explicitly.
     */
    virtual void        unbuffered(bf_idx idx);


protected:
    /*!\fn      pick_victim()
     * \brief   Selects a page to be evicted from the \link _bufferpool \endlink
     * \details This method uses the CAR algorithm to select one buffer frame which
     *          is expected to be used the furthest in the future (with the currently
     *          cached page). It acquires a LATCH_EX to prohibit the usage of the
     *          frame as the content of the buffer frame will definitely change.
     *
     * @return The buffer frame that can be freed.
     */
    bf_idx              pick_victim();


protected:
    /*!\var     _clocks
     * \brief   Clocks f$T_1\f$ and \f$T_2\f$
     * \details Represents the clocks \f$T_1\f$ and \f$T_2\f$ which contain
     *          eviction-specific metadata of the pages that are inside the bufferpool.
     *          Therefore there needs to be two clocks in the multi_clock and the size
     *          of the clock equals the size of the bufferpool. As the CAR algorithm
     *          only stores a referenced bit, the value stored for each index is of
     *          Boolean type. And as the internal operation of multi_clock needs an
     *          invalid index (as well as a range of indexes starting from 0), the used
     *          invalid index is 0 which isn't used in the bufferpool as well.
     */
    multi_clock<bf_idx, bool>*      _clocks;
    
    /*!\var     _b1
     * \brief   LRU-list \f$B_1\f$
     * \details Represents the LRU-list \f$B_1\f$ which contains the PageIDs of pages
     *          evicted from \f$T_1\f$.
     */
    hashtable_queue<PageID>*        _b1;
    
    /*!\var     _b2
     * \brief   LRU-list \f$B_2\f$
     * \details Represents the LRU-list \f$B_2\f$ which contains the PageIDs of pages
     *          evicted from \f$T_2\f$.
     */
    hashtable_queue<PageID>*        _b2;
    
    /*!\var     _p
     * \brief   Parameter \f$p\f$
     * \details Represents the parameter \f$p\f$ which acts as a target size of \f$T_1\f$.
     */
    u_int32_t                       _p;
    
    /*!\var     _c
     * \brief   Parameter \f$c\f$
     * \details The number of buffer frames in the bufferpool \link _bufferpool \endlink.
     */
    u_int32_t                       _c;
    
    /*!\var     _hand_movement
     * \brief   Clock hand movements in current circulation
     * \details The combined number of movements of the clock hands of \f$T_1\f$ and
     *          \f$T_2\f$. Is reset after \link _c \endlink movements.
     */
    bf_idx                          _hand_movement;
    
    /*!\var     _lock
     * \brief   Latch of \link _clocks \endlink, \link _b1 \endlink and \link _b2 \endlink
     * \details As the data structures \link _clocks \endlink, \link _b1 \endlink and
     *          \link _b2 \endlink aren't thread-safe and as the
     *          \link pick_victim() \endlink and the \link miss_ref(bf_idx, PageID) \endlink
     *          methods might change those data structures concurrently, this lock needs
     *          to be acquired by those methods. The \link ref() \endlink method is only
     *          called with the corresponding buffer frame latched and the access is also
     *          only atomic and therefore this method doesn't need to acquire this lock
     *          for its changes.
     */
    pthread_mutex_t                 _lock;
    
    /*!\enum    clock_index
     * \brief   Clock names
     * \details Contains constants that map the names of the clocks used by the CAR
     *          algorithm to the indexes used by the \link _clocks \endlink data structure.
     */
    enum clock_index {
        T_1 = 0,
        T_2 = 1
    };
};

#endif //ZERO_PAGE_EVICTIONER_CAR_H
