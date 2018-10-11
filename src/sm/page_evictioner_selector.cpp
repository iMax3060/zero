#include "page_evictioner_selector.hpp"

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// page_evictioner_selector ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

page_evictioner_selector::page_evictioner_selector(bf_tree_m &bufferpool, const sm_options &options) :
        _bufferpool_size(bufferpool.get_block_cnt()) {}

page_evictioner_selector::~page_evictioner_selector() {}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// page_evictioner_selector_loop /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <loop_accuracy _loop_accuracy>
page_evictioner_selector_loop<_loop_accuracy>::page_evictioner_selector_loop(bf_tree_m &bufferpool, const sm_options &options) :
        page_evictioner_selector(bufferpool, options),
        _practically_accurate_current_frame(1),
        _absolutely_accurate_current_frame(1) {}

template <>
bf_idx page_evictioner_selector_loop<loop_accuracy::ABSOLUTELY_ACCURATE>::select() {
    w_assert1(_current_frame > 0 && _current_frame <= _max_bufferpool_index);

    uint_fast32_t this_frame;
    std::lock_guard<std::mutex> guard(_current_frame_lock);
    if (_absolutely_accurate_current_frame >= _bufferpool_size - 1) {
        _absolutely_accurate_current_frame = 1;
    } else {
        _absolutely_accurate_current_frame++;
    }
    _absolutely_accurate_current_frame = used_idx;
    return this_frame;
}

template <>
bf_idx page_evictioner_selector_loop<loop_accuracy::PRACTICALLY_ACCURATE>::select() { // Not exact after 18446744073709551616 (1 per ns -> once in 585 years) incrementations!
    w_assert1(_current_frame > 0 && _current_frame <= _max_bufferpool_index);

    while (true) {
        uint_fast32_t this_frame = _practically_accurate_current_frame++ % _bufferpool_size;
        if (this_frame == 0) {
            continue;
        } else {
            return this_frame;
        }
    }
}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageHit(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageUnfix(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageFixed(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageDirty(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageBlocked(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageSwizzled(bf_idx idx) {}

template <loop_accuracy _loop_accuracy>
void page_evictioner_selector_loop<_loop_accuracy>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}

template class page_evictioner_selector_loop<page_evictioner_selector_loop::loop_accuracy::ABSOLUTELY_ACCURATE>;
template class page_evictioner_selector_loop<page_evictioner_selector_loop::loop_accuracy::PRACTICALLY_ACCURATE>;

////////////////////////////////////////////////////////////////////////////////
/////////////////////// page_evictioner_selector_random ////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <random_numbers_generator _random_numbers_generator>
page_evictioner_selector_random<_random_numbers_generator>::page_evictioner_selector_random(bf_tree_m &bufferpool, const sm_options &options) :
        page_evictioner_selector(bufferpool, options),
        _random_distribution(1, bufferpool.get_block_cnt() - 1) {}

template <>
bf_idx page_evictioner_selector_random<random_numbers_generator::FAST_RAND>::select() {
    if (!_random_state_initialized) {
        _random_state = std::random_device{}();
        _random_state_initialized = true;
    }
    _random_state = 214013 * _random_state + 2531011;
    return ((_random_state >> 16 & 0x7FFF) % (_bufferpool_size - 1)) + 1;
}

template <>
bf_idx page_evictioner_selector_random<random_numbers_generator::DEFAULT>::select() {
    return _random_distribution(_standard_random_engine);
}

template <random_numbers_generator  _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageHit(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageUnfix(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageMiss(bf_idx b_idx, PageID pid) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageFixed(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageDirty(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageBlocked(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageSwizzled(bf_idx idx) {}

template <random_numbers_generator _random_numbers_generator>
void page_evictioner_selector_random<_random_numbers_generator>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {}
