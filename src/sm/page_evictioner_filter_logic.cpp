#include "page_evictioner_filter_logic.hpp"

using namespace zero::buffer_pool;

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicNOT /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class>
PageEvictionerFilterLogicNOT<filter_class>::PageEvictionerFilterLogicNOT(BufferPool& bufferPool, const sm_options& options) :
        _filter(bufferPool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value, "'filter_class' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class>
bool PageEvictionerFilterLogicNOT<filter_class>::preFilter(bf_idx idx) {
    return !_filter.preFilter(idx);
}

template <class filter_class>
bool PageEvictionerFilterLogicNOT<filter_class>::filter(bf_idx idx) {
    return !_filter.filter(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageHit(bf_idx idx) {
    _filter.updateOnPageHit(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageUnfix(bf_idx idx) {
    _filter.updateOnPageUnfix(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter.updateOnPageMiss(b_idx, pid);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageFixed(bf_idx idx) {
    _filter.updateOnPageFixed(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageDirty(bf_idx idx) {
    _filter.updateOnPageDirty(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageBlocked(bf_idx idx) {
    _filter.updateOnPageBlocked(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageSwizzled(bf_idx idx) {
    _filter.updateOnPageSwizzled(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT<filter_class>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter.updateOnPageExplicitlyUnbuffered(idx);
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicAND /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class0, class filter_class1>
PageEvictionerFilterLogicAND<filter_class0, filter_class1>::PageEvictionerFilterLogicAND(BufferPool &bufferPool, const sm_options &options) :
        _filter0(bufferPool, options),
        _filter1(bufferPool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class0>::value, "'filter_class0' is not of type 'PageEvictionerFilter'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class1>::value, "'filter_class1' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class0, class filter_class1>
bool PageEvictionerFilterLogicAND<filter_class0, filter_class1>::preFilter(bf_idx idx) {
    return _filter0.preFilter(idx) && _filter1.preFilter(idx);
}

template <class filter_class0, class filter_class1>
bool PageEvictionerFilterLogicAND<filter_class0, filter_class1>::filter(bf_idx idx) {
    return _filter0.filter(idx) && _filter1.filter(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageHit(bf_idx idx) {
    _filter0.updateOnPageHit(idx);
    _filter1.updateOnPageHit(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageUnfix(bf_idx idx) {
    _filter0.updateOnPageUnfix(idx);
    _filter1.updateOnPageUnfix(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter0.updateOnPageMiss(b_idx, pid);
    _filter1.updateOnPageMiss(b_idx, pid);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageFixed(bf_idx idx) {
    _filter0.updateOnPageFixed(idx);
    _filter1.updateOnPageFixed(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageDirty(bf_idx idx) {
    _filter0.updateOnPageDirty(idx);
    _filter1.updateOnPageDirty(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageBlocked(bf_idx idx) {
    _filter0.updateOnPageBlocked(idx);
    _filter1.updateOnPageBlocked(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageSwizzled(bf_idx idx) {
    _filter0.updateOnPageSwizzled(idx);
    _filter1.updateOnPageSwizzled(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND<filter_class0, filter_class1>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter0.updateOnPageExplicitlyUnbuffered(idx);
    _filter1.updateOnPageExplicitlyUnbuffered(idx);
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicOR //////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class0, class filter_class1>
PageEvictionerFilterLogicOR<filter_class0, filter_class1>::PageEvictionerFilterLogicOR(BufferPool& bufferPool, const sm_options& options) :
        _filter0(bufferPool, options),
        _filter1(bufferPool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class0>::value, "'filter_class0' is not of type 'PageEvictionerFilter'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class1>::value, "'filter_class1' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class0, class filter_class1>
bool PageEvictionerFilterLogicOR<filter_class0, filter_class1>::preFilter(bf_idx idx) {
    return _filter0.preFilter(idx) || _filter1.preFilter(idx);
}

template <class filter_class0, class filter_class1>
bool PageEvictionerFilterLogicOR<filter_class0, filter_class1>::filter(bf_idx idx) {
    return _filter0.filter(idx) || _filter1.filter(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageHit(bf_idx idx) {
    _filter0.updateOnPageHit(idx);
    _filter1.updateOnPageHit(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageUnfix(bf_idx idx) {
    _filter0.updateOnPageUnfix(idx);
    _filter1.updateOnPageUnfix(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter0.updateOnPageMiss(b_idx, pid);
    _filter1.updateOnPageMiss(b_idx, pid);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageFixed(bf_idx idx) {
    _filter0.updateOnPageFixed(idx);
    _filter1.updateOnPageFixed(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageDirty(bf_idx idx) {
    _filter0.updateOnPageDirty(idx);
    _filter1.updateOnPageDirty(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageBlocked(bf_idx idx) {
    _filter0.updateOnPageBlocked(idx);
    _filter1.updateOnPageBlocked(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageSwizzled(bf_idx idx) {
    _filter0.updateOnPageSwizzled(idx);
    _filter1.updateOnPageSwizzled(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR<filter_class0, filter_class1>::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter0.updateOnPageExplicitlyUnbuffered(idx);
    _filter1.updateOnPageExplicitlyUnbuffered(idx);
};
