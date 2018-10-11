#ifndef __PAGE_EVICTIONER_SELECTOR_HPP
#define __PAGE_EVICTIONER_SELECTOR_HPP

#include <mutex>
#include <random>

/*!
 *
 */
class page_evictioner_selector {
public:
    page_evictioner_selector(bf_tree_m& bufferpool, const sm_options& options);

    virtual                               ~page_evictioner_selector();

    virtual                               bf_idx select() = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
    virtual void                          updateOnPageHit(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
    virtual void                          updateOnPageUnfix(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
    virtual void                          updateOnPageFixed(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
    virtual void                          updateOnPageDirty(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
    virtual void                          updateOnPageBlocked(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
    virtual void                          updateOnPageSwizzled(bf_idx idx) = 0;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) = 0;

private:
    bf_idx                                _bufferpool_size;

};

/*!
 *
 * @tparam _loop_accuracy
 */
template <loop_accuracy _loop_accuracy = loop_accuracy::PRACTICALLY_ACCURATE, bool excludeBlocked>
class page_evictioner_selector_loop : public page_evictioner_selector {
private:
    /*!\var     _practically_accurate_current_frame
     * \brief   Last control block examined
     * \details Represents the clock hand pointing to the control block that was
     *          examined last during the most recent execution of
     *          \link pickVictim() \endlink (evicted last).
     *
     * \remark  Only used by __LOOP__ and __CLOCK__.
     */
    std::atomic<uint_fast64_t>            _practically_accurate_current_frame;

    volatile uint_fast32_t                _absolutely_accurate_current_frame;

    std::mutex                            _current_frame_lock;

public:
    page_evictioner_selector_loop(bf_tree_m& bufferpool, const sm_options& options);

    virtual bf_idx                        select() final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
    virtual void                          updateOnPageHit(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
    virtual void                          updateOnPageUnfix(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
    virtual void                          updateOnPageMiss(bf_idx b_idx, PageID pid) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
    virtual void                          updateOnPageFixed(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
    virtual void                          updateOnPageDirty(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
    virtual void                          updateOnPageBlocked(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
    virtual void                          updateOnPageSwizzled(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
    virtual void                          updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    enum class loop_accuracy : bool {
        ABSOLUTELY_ACCURATE,
        PRACTICALLY_ACCURATE
    };

};

/*!
 *
 * @tparam _random_numbers_generator
 */
template <random_numbers_generator _random_numbers_generator = random_numbers_generator::FAST_RAND>
class page_evictioner_selector_random : public page_evictioner_selector {
private:
    thread_local bool                                             _random_state_initialized;

    thread_local uint_fast32_t                                    _random_state;

    thread_local std::default_random_engine                       _standard_random_engine;

    thread_local std::uniform_int_distribution<uint_fast32_t>     _random_distribution;

public:
    page_evictioner_selector_random(bf_tree_m& bufferpool, const sm_options& options);

    virtual bf_idx                                                select() final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageHit(bf_idx idx)
    virtual void                                                  updateOnPageHit(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageUnfix(bf_idx idx)
    virtual void                                                  updateOnPageUnfix(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageMiss(bf_idx b_idx, PageID pid)
    virtual void                                                  updateOnPageMiss(bf_idx b_idx, PageID pid) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageFixed(bf_idx idx)
    virtual void                                                  updateOnPageFixed(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageDirty(bf_idx idx)
    virtual void                                                  updateOnPageDirty(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageBlocked(bf_idx idx)
    virtual void                                                  updateOnPageBlocked(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageSwizzled(bf_idx idx)
    virtual void                                                  updateOnPageSwizzled(bf_idx idx) final;

    //!@copydoc PageEvictionerSFSkeleton::updateOnPageExplicitlyUnbuffered(bf_idx idx)
    virtual void                                                  updateOnPageExplicitlyUnbuffered(bf_idx idx) final;

    enum class random_numbers_generator : uint_fast8_t {
        FAST_RAND,
//        rand_sse,
        DEFAULT
    };

};

#endif // __PAGE_EVICTIONER_SELECTOR_HPP
