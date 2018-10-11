#include "page_evictioner_filter_logic.hpp"

using namespace zero::buffer_pool;

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicNOT /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class>
PageEvictionerFilterLogicNOT::PageEvictionerFilterLogicNOT(bf_tree_m &bufferpool, const sm_options &options) :
        _filter(bufferpool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class>::value, "'filter_class' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class, bool first_call>
bool PageEvictionerFilterLogicNOT::filter<first_call>(bf_idx idx) {
    return !_filter.filter<first_call>(idx);
};

template bool PageEvictionerFilterLogicNOT::filter<true>(bf_idx idx);
template bool PageEvictionerFilterLogicNOT::filter<false>(bf_idx idx);

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageHit(bf_idx idx) {
    _filter.updateOnPageHit(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageUnfix(bf_idx idx) {
    _filter.updateOnPageUnfix(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter.updateOnPageMiss(b_idx, pid);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageFixed(bf_idx idx) {
    _filter.updateOnPageFixed(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageDirty(bf_idx idx) {
    _filter.updateOnPageDirty(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageBlocked(bf_idx idx) {
    _filter.updateOnPageBlocked(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageSwizzled(bf_idx idx) {
    _filter.updateOnPageSwizzled(idx);
};

template <class filter_class>
void PageEvictionerFilterLogicNOT::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter.updateOnPageExplicitlyUnbuffered(idx);
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicAND /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class0, class filter_class1>
PageEvictionerFilterLogicAND::PageEvictionerFilterLogicAND(bf_tree_m &bufferpool, const sm_options &options) :
        _filter0(bufferpool, options),
        _filter1(bufferpool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class0>::value, "'filter_class0' is not of type 'PageEvictionerFilter'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class1>::value, "'filter_class1' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class0, class filter_class1, bool first_call>
bool PageEvictionerFilterLogicAND::filter<first_call>(bf_idx idx) {
    return _filter0.filter<first_call>(idx) && _filter1.filter<first_call>(idx);
};

template bool PageEvictionerFilterLogicAND::filter<true>(bf_idx idx);
template bool PageEvictionerFilterLogicAND::filter<false>(bf_idx idx);

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageHit(bf_idx idx) {
    _filter0.updateOnPageHit(idx);
    _filter1.updateOnPageHit(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageUnfix(bf_idx idx) {
    _filter0.updateOnPageUnfix(idx);
    _filter1.updateOnPageUnfix(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter0.updateOnPageMiss(b_idx, pid);
    _filter1.updateOnPageMiss(b_idx, pid);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageFixed(bf_idx idx) {
    _filter0.updateOnPageFixed(idx);
    _filter1.updateOnPageFixed(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageDirty(bf_idx idx) {
    _filter0.updateOnPageDirty(idx);
    _filter1.updateOnPageDirty(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageBlocked(bf_idx idx) {
    _filter0.updateOnPageBlocked(idx);
    _filter1.updateOnPageBlocked(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageSwizzled(bf_idx idx) {
    _filter0.updateOnPageSwizzled(idx);
    _filter1.updateOnPageSwizzled(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicAND::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter0.updateOnPageExplicitlyUnbuffered(idx);
    _filter1.updateOnPageExplicitlyUnbuffered(idx);
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PageEvictionerFilterLogicOR //////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class filter_class0, class filter_class1>
PageEvictionerFilterLogicOR::PageEvictionerFilterLogicOR(bf_tree_m &bufferpool, const sm_options &options) :
        _filter0(bufferpool, options),
        _filter1(bufferpool, options) {
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class0>::value, "'filter_class0' is not of type 'PageEvictionerFilter'!");
    static_assert(std::is_base_of<PageEvictionerFilter, filter_class1>::value, "'filter_class1' is not of type 'PageEvictionerFilter'!");
};

template <class filter_class0, class filter_class1, bool first_call>
bool PageEvictionerFilterLogicOR::filter<first_call>(bf_idx idx) {
    return _filter0.filter<first_call>(idx) || _filter1.filter<first_call>(idx);
};

template bool PageEvictionerFilterLogicOR::filter<true>(bf_idx idx);
template bool PageEvictionerFilterLogicOR::filter<false>(bf_idx idx);

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageHit(bf_idx idx) {
    _filter0.updateOnPageHit(idx);
    _filter1.updateOnPageHit(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageUnfix(bf_idx idx) {
    _filter0.updateOnPageUnfix(idx);
    _filter1.updateOnPageUnfix(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageMiss(bf_idx b_idx, PageID pid) {
    _filter0.updateOnPageMiss(b_idx, pid);
    _filter1.updateOnPageMiss(b_idx, pid);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageFixed(bf_idx idx) {
    _filter0.updateOnPageFixed(idx);
    _filter1.updateOnPageFixed(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageDirty(bf_idx idx) {
    _filter0.updateOnPageDirty(idx);
    _filter1.updateOnPageDirty(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageBlocked(bf_idx idx) {
    _filter0.updateOnPageBlocked(idx);
    _filter1.updateOnPageBlocked(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageSwizzled(bf_idx idx) {
    _filter0.updateOnPageSwizzled(idx);
    _filter1.updateOnPageSwizzled(idx);
};

template <class filter_class0, class filter_class1>
void PageEvictionerFilterLogicOR::updateOnPageExplicitlyUnbuffered(bf_idx idx) {
    _filter0.updateOnPageExplicitlyUnbuffered(idx);
    _filter1.updateOnPageExplicitlyUnbuffered(idx);
};
