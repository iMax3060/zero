#ifndef __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
#define __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP

#include <cstdint>

namespace zero::buffer_pool {
    /* BEGIN --- Forward Declarations ---BEGIN */
    template <class selector_class, class filter_class, bool filter_early> class PageEvictionerSelectAndFilter;

    class PageEvictionerSelectorLOOPAbsolutelyAccurate;
    class PageEvictionerSelectorLOOPPracticallyAccurate;
    class PageEvictionerSelectorLOOPThreadLocallyAccurate;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOLowContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOHighContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFILOLowContention;
    class PageEvictionerSelectorLRU;
    template <bf_idx protected_block_ppm = 10000> class PageEvictionerSelectorSLRU;
    template <size_t k = 2, bool on_page_unfix = false> class PageEvictionerSelectorLRUK;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t mru_list_check_ppm = 10000> class PageEvictionerSelectorQuasiMRU;
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorTimestampLRU;
    template <size_t k = 2, bf_idx resort_threshold_ppm = 750000, bool on_page_unfix = false> class PageEvictionerSelectorTimestampLRUK;
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorLFU;
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorLFUDA;
    class PageEvictionerSelectorLRDV1;

    class PageEvictionerSelectorRANDOMDefault;
    class PageEvictionerSelectorRANDOMFastRand;
    class PageEvictionerSelectorRANDOMMinstdRand0;
    class PageEvictionerSelectorRANDOMMinstdRand;
    class PageEvictionerSelectorRANDOMMT19937;
    class PageEvictionerSelectorRANDOMMT19937_64;
    class PageEvictionerSelectorRANDOMRanlux24Base;
    class PageEvictionerSelectorRANDOMRanlux48Base;
    class PageEvictionerSelectorRANDOMRanlux24;
    class PageEvictionerSelectorRANDOMRanlux48;
    class PageEvictionerSelectorRANDOMKnuthB;
    class PageEvictionerSelectorRANDOMCRand;
    class PageEvictionerSelectorRANDOMBoostMinstdRand0;
    class PageEvictionerSelectorRANDOMBoostMinstdRand;
    class PageEvictionerSelectorRANDOMBoostRand48;
    class PageEvictionerSelectorRANDOMBoostEcuyer1988;
    class PageEvictionerSelectorRANDOMBoostKnuthB;
    class PageEvictionerSelectorRANDOMBoostKreutzer1986;
    class PageEvictionerSelectorRANDOMBoostTaus88;
    class PageEvictionerSelectorRANDOMBoostHellekalek1995;
    class PageEvictionerSelectorRANDOMBoostMT11213b;
    class PageEvictionerSelectorRANDOMBoostMT19937;
    class PageEvictionerSelectorRANDOMBoostMT19937_64;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci607;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209;
    class PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497;
    class PageEvictionerSelectorRANDOMBoostRanlux3;
    class PageEvictionerSelectorRANDOMBoostRanlux4;
    class PageEvictionerSelectorRANDOMBoostRanlux64_3;
    class PageEvictionerSelectorRANDOMBoostRanlux64_4;
    class PageEvictionerSelectorRANDOMBoostRanlux3_01;
    class PageEvictionerSelectorRANDOMBoostRanlux4_01;
    class PageEvictionerSelectorRANDOMBoostRanlux64_4_01;
    class PageEvictionerSelectorRANDOMBoostRanlux24;
    class PageEvictionerSelectorRANDOMBoostRanlux48;
    class PageEvictionerSelectorRANDOMXORShift32;
    class PageEvictionerSelectorRANDOMXORShift64;
    class PageEvictionerSelectorRANDOMXORShift96;
    class PageEvictionerSelectorRANDOMXORShift128;
    class PageEvictionerSelectorRANDOMXORWow;
    class PageEvictionerSelectorRANDOMXORShift64Star;
    class PageEvictionerSelectorRANDOMXORShift128Plus;

    template <uint64_t subtrahend = 10> struct AgingFunctionSubtraction;
    template <uint64_t factor_ppm = 750000> struct AgingFunctionMultiplication;
    template <uint64_t aging_frequency = 10, class aging_function = AgingFunctionSubtraction<>> class PageEvictionerSelectorLRDV2;
    class PageEvictionerFilterNone;
    template <bool on_hit = true, bool on_unfix = false, bool on_miss = true, bool on_fixed = false, bool on_dirty = false, bool on_blocked = false, bool on_swizzled = false> class PageEvictionerFilterCLOCK;
    template <uint16_t decrement = 1, bool discriminate_pages = false,
            bool on_hit = true, bool set_on_hit = false, uint16_t level0_on_hit = 5, uint16_t level1_on_hit = 2, uint16_t level2_on_hit = 1,
            bool on_unfix = false, bool set_on_unfix = false, uint16_t level0_on_unfix = 5, uint16_t level1_on_unfix = 2, uint16_t level2_on_unfix = 1,
            bool on_miss = true, bool set_on_miss = true, uint16_t level0_on_miss = 25, uint16_t level1_on_miss = 10, uint16_t level2_on_miss = 5,
            bool on_fixed = false, bool set_on_fixed = false, uint16_t level0_on_fixed = 5, uint16_t level1_on_fixed = 2, uint16_t level2_on_fixed = 1,
            bool on_dirty = false, bool set_on_dirty = false, uint16_t level0_on_dirty = 5, uint16_t level1_on_dirty = 2, uint16_t level2_on_dirty = 1,
            bool on_blocked = false, bool set_on_blocked = false, uint16_t level0_on_blocked = 5, uint16_t level1_on_blocked = 2, uint16_t level2_on_blocked = 1,
            bool on_swizzled = false, bool set_on_swizzled = false, uint16_t level0_on_swizzled = 5, uint16_t level1_on_swizzled = 2, uint16_t level2_on_swizzled = 1> class PageEvictionerFilterGCLOCK;
    template <bool on_page_unfix = false> class PageEvictionerCAR;
    /* END --- Forward Declarations --- END */

    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPPracticallyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPThreadLocallyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPThreadLocallyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<>, true> PageEvictionerCLOCKFix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<false, true>, true> PageEvictionerCLOCKUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<true, true>, true> PageEvictionerCLOCKFixUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<>, true> PageEvictionerGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true>, true> PageEvictionerDGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerDGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOLowContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOHighContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOHighContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFILOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFILOLowContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRU, PageEvictionerFilterNone, false> PageEvictionerLRU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<2>, PageEvictionerFilterNone, false> PageEvictionerLRU2;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<3>, PageEvictionerFilterNone, false> PageEvictionerLRU3;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<4>, PageEvictionerFilterNone, false> PageEvictionerLRU4;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorSLRU<>, PageEvictionerFilterNone, false> PageEvictionerSLRU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiMRU<>, PageEvictionerFilterNone, false> PageEvictionerMRU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRU<>, PageEvictionerFilterNone, false> PageEvictionerTimestampLRU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<2>, PageEvictionerFilterNone, false> PageEvictionerTimestampLRU2;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<3>, PageEvictionerFilterNone, false> PageEvictionerTimestampLRU3;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<4>, PageEvictionerFilterNone, false> PageEvictionerTimestampLRU4;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLFU<>, PageEvictionerFilterNone, false> PageEvictionerLFU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLFUDA<>, PageEvictionerFilterNone, false> PageEvictionerLFUDA;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV1, PageEvictionerFilterNone, false> PageEvictionerLRDV1;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionSubtraction<10>>, PageEvictionerFilterNone, false> PageEvictionerLRDV2Subtraction;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionMultiplication<>>, PageEvictionerFilterNone, false> PageEvictionerLRDV2Multiplication;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMDefault, PageEvictionerFilterNone, false> PageEvictionerRANDOMDefault;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMFastRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMFastRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand0, PageEvictionerFilterNone, false> PageEvictionerRANDOMMinstdRand0;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMMinstdRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937, PageEvictionerFilterNone, false> PageEvictionerRANDOMMT19937;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937_64, PageEvictionerFilterNone, false> PageEvictionerRANDOMMT19937_64;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24Base, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux24Base;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48Base, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux48Base;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux24;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux48;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKnuthB, PageEvictionerFilterNone, false> PageEvictionerRANDOMKnuthB;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMCRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMCRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMinstdRand0, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostMinstdRand0;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMinstdRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostMinstdRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRand48, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRand48;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostEcuyer1988, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostEcuyer1988;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostKnuthB, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostKnuthB;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostKreutzer1986, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostKreutzer1986;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostTaus88, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostTaus88;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostHellekalek1995, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostHellekalek1995;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT11213b, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostMT11213b;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT19937, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostMT19937;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT19937_64, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostMT19937_64;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci607, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci607;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci1279;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci2281;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci3217;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci4423;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci9689;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci19937;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci23209;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostLaggedFibonacci44497;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux3, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux3;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux4, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux4;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_3, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux64_3;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_4, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux64_4;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux3_01, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux3_01;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux4_01, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux4_01;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_4_01, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux64_4_01;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux24, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux24;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux48, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux48;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift64;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift96, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift96;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift128, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift128;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORWow, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORWow;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64Star, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift64Star;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift128Plus, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift128Plus;
    typedef PageEvictionerCAR<> PageEvictionerCARFix;
    typedef PageEvictionerCAR<true> PageEvictionerUnfix;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
