#ifndef __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
#define __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP

#include <cstdint>

#include <random>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/additive_combine.hpp>
#include <boost/random/shuffle_order.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/inversive_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/ranlux.hpp>
#include "pcg-cpp/include/pcg_random.hpp"
#include "9b769cefccac1f2bd728596da3a856dd/xorshift.hpp"
#include "3eb1bcc5418c4ae83c4c6a86d9cbb1cd/xoshiro.hpp"
#include "f76f4bb7b7f67ff0850199ab7c077bf7/xoroshiro.hpp"
#include "4f2bf4b4f3a221ef051cf108d6b64d5a/arc4.hpp"
#include "7a783e20f71259cc13e219829bcea4ac/gjrand.hpp"
#include "85cff47d4bad8de6bdeb671f9c76c814/jsf.hpp"
#include "6179748664e88ef3c34860f44309fc71/splitmix.hpp"
#if __SIZEOF_INT128__
#include "aeae7628565f15fb3fef54be8533e39c/lehmer.hpp"
#endif // __SIZEOF_INT128__
#include "f0fe8877e4deb3f6b9200a17c18bf155/chacha.hpp"
#include "f1f7821f07cf76504a97f6537c818083/sfc.hpp"

namespace zero::buffer_pool {
    /* BEGIN --- Forward Declarations ---BEGIN */
    template <class selector_class, class filter_class, bool filter_early> class PageEvictionerSelectAndFilter;

    // LOOP Page Selectors:
    class PageEvictionerSelectorLOOPAbsolutelyAccurate;
    class PageEvictionerSelectorLOOPPracticallyAccurate;
    class PageEvictionerSelectorLOOPLockFree;
    class PageEvictionerSelectorLOOPThreadLocallyAccurate;

    // RANDOM Page Selector Auxiliary Classes:
    template<typename seed_type> struct SeedGeneratorChrono;
    template<typename seed_type> struct SeedGeneratorRandomDevice;
    template<class random_number_generator, class ... seed_generators> class PageEvictionerSelectorRANDOMExternal;
    template<class random_number_generator, bool seed_explicitly, class ... seed_generators> class PageEvictionerSelectorRANDOMExternalThreadLocal;

    // RANDOM Page Selectors from the C++ Standard Library:
    class PageEvictionerSelectorRANDOMCRand;
    typedef PageEvictionerSelectorRANDOMExternal<std::minstd_rand0, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMMinstdRand0;
    typedef PageEvictionerSelectorRANDOMExternal<std::minstd_rand, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMMinstdRand;
    typedef PageEvictionerSelectorRANDOMExternal<std::mt19937, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMMT19937;
    typedef PageEvictionerSelectorRANDOMExternal<std::mt19937_64, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMMT19937_64;
    typedef PageEvictionerSelectorRANDOMExternal<std::ranlux24_base, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMRanlux24Base;
    typedef PageEvictionerSelectorRANDOMExternal<std::ranlux48_base, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMRanlux48Base;
    typedef PageEvictionerSelectorRANDOMExternal<std::ranlux24, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMRanlux24;
    typedef PageEvictionerSelectorRANDOMExternal<std::ranlux48, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMRanlux48;
    typedef PageEvictionerSelectorRANDOMExternal<std::knuth_b, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMKnuthB;

    // RANDOM Page Selectors from the Boost RANDOM library:
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::minstd_rand0, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostMinstdRand0;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::minstd_rand, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostMinstdRand;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::rand48, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRand48;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ecuyer1988, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostEcuyer1988;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::knuth_b, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostKnuthB;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::kreutzer1986, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostKreutzer1986;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::taus88, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostTaus88;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::hellekalek1995, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostHellekalek1995;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::mt11213b, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostMT11213b;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::mt19937, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostMT19937;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::mt19937_64, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMBoostMT19937_64;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci607, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci607;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci1279, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci2281, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci3217, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci4423, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci9689, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci19937, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci23209, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci44497, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux3, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux3;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux4, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux4;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_3, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMBoostRanlux64_3;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_4, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMBoostRanlux64_4;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux3_01, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux3_01;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux4_01, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux4_01;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_3_01, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux64_3_01;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_4_01, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux64_4_01;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux24, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMBoostRanlux24;
    typedef PageEvictionerSelectorRANDOMExternal<boost::random::ranlux48, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMBoostRanlux48;

    // RANDOM Page Selectors from Melissa O'Neill's PCG library:
    typedef PageEvictionerSelectorRANDOMExternal<pcg32, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_unique, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32Unique;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_fast, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32Fast;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k2, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K2;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k2_fast, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K2Fast;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k64, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K64;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k64_fast, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K64Fast;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k1024, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K1024;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k1024_fast, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K1024Fast;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k16384, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K16384;
    typedef PageEvictionerSelectorRANDOMExternal<pcg32_k16384_fast, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMPCG32K16384Fast;

    // RANDOM Page Selectors from Melissa O'Neill's XorShift library:
    typedef PageEvictionerSelectorRANDOMExternal<xorshift32plain32a, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXORShift32;
    typedef PageEvictionerSelectorRANDOMExternal<xorshift64star32a, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXORShift64Star;

    // RANDOM Page Selectors based on XorShift:
    class PageEvictionerSelectorRANDOMXORWow;
    class PageEvictionerSelectorRANDOMXORShift128Plus;

    // RANDOM Page Selectors from Melissa O'Neill's Xoshiro library:
    typedef PageEvictionerSelectorRANDOMExternal<xoshiro128plus32, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoshiro128Plus32;
    typedef PageEvictionerSelectorRANDOMExternal<xoshiro128starstar32 , SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoshiro128StarStar32;

    // RANDOM Page Selectors from Melissa O'Neill's Xoroshiro library:
    typedef PageEvictionerSelectorRANDOMExternal<xoroshiro128plus32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoroshiro128Plus32;
    typedef PageEvictionerSelectorRANDOMExternal<xoroshiro64plus32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoroshiro64Plus32;
    typedef PageEvictionerSelectorRANDOMExternal<xoroshiro64star32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoroshiro64Star32;
    typedef PageEvictionerSelectorRANDOMExternal<xoroshiro64starstar32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMXoroshiro64StarStar32;

    // RANDOM Page Selectors from Melissa O'Neill's Arc4 library:
    typedef PageEvictionerSelectorRANDOMExternal<arc4_rand32, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMARC4;

    // RANDOM Page Selectors from Melissa O'Neill's GJrand library:
    typedef PageEvictionerSelectorRANDOMExternal<gjrand32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMGjrand;

    // RANDOM Page Selectors from Melissa O'Neill's JSF library:
    typedef PageEvictionerSelectorRANDOMExternal<jsf32n, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMJSF32n;
    typedef PageEvictionerSelectorRANDOMExternal<jsf32r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMJSF32r;

    // RANDOM Page Selectors from Melissa O'Neill's SplitMix library:
    typedef PageEvictionerSelectorRANDOMExternal<splitmix32, SeedGeneratorRandomDevice<uint64_t>, SeedGeneratorRandomDevice<uint64_t>> PageEvictionerSelectorRANDOMSplitMix32;

    // RANDOM Page Selectors from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    typedef PageEvictionerSelectorRANDOMExternal<mcg128, SeedGeneratorRandomDevice<__uint128_t>> PageEvictionerSelectorRANDOMMCG128;
    typedef PageEvictionerSelectorRANDOMExternal<mcg128_fast, SeedGeneratorRandomDevice<__uint128_t>> PageEvictionerSelectorRANDOMMCG128Fast;
#endif // __SIZEOF_INT128__

    // RANDOM Page Selectors from Melissa O'Neill's ChaCha library:
    typedef PageEvictionerSelectorRANDOMExternal<chacha4r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMChaCha4;
    typedef PageEvictionerSelectorRANDOMExternal<chacha5r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMChaCha5;
    typedef PageEvictionerSelectorRANDOMExternal<chacha6r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMChaCha6;
    typedef PageEvictionerSelectorRANDOMExternal<chacha8r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMChaCha8;
    typedef PageEvictionerSelectorRANDOMExternal<chacha20r, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMChaCha20;

    // RANDOM Page Selectors from Melissa O'Neill's SFC library:
    typedef PageEvictionerSelectorRANDOMExternal<sfc32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>> PageEvictionerSelectorRANDOMSFC32;

    // FIFO/FILO (First in, first out/First in, last out) Page Selectors:
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOLowContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFIFOHighContention;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t initial_list_check_ppm = 10000> class PageEvictionerSelectorQuasiFILOLowContention;

    // LRU/MRU (Least/Most Recently Used) Page Selectors:
    class PageEvictionerSelectorLRU;
    template <bf_idx protected_block_ppm = 10000> class PageEvictionerSelectorSLRU;
    template <size_t k = 2, bool on_page_unfix = false> class PageEvictionerSelectorLRUK;
    template <uint32_t retry_list_check_ppm = 1000000, uint32_t mru_list_check_ppm = 10000> class PageEvictionerSelectorQuasiMRU;
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorTimestampLRU;
    template <size_t k = 2, bf_idx resort_threshold_ppm = 750000, bool on_page_unfix = false> class PageEvictionerSelectorTimestampLRUK;

    // LFU (Least Frequently Used) Page Selectors:
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorLFU;
    template <bf_idx resort_threshold_ppm = 750000> class PageEvictionerSelectorLFUDA;

    // LRD (Least Reference Density) Page Selectors:
    class PageEvictionerSelectorLRDV1;
    template <uint64_t subtrahend = 10> struct AgingFunctionSubtraction;
    template <uint64_t factor_ppm = 750000> struct AgingFunctionMultiplication;
    template <uint64_t aging_frequency = 10, class aging_function = AgingFunctionSubtraction<>> class PageEvictionerSelectorLRDV2;

    // No Page Filter:
    class PageEvictionerFilterNone;

    // CLOCK Page Filters:
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

    // LOOP Page Evictioners:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPAbsolutelyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPPracticallyAccurate;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPLockFree, PageEvictionerFilterNone, false> PageEvictionerLOOPLockFree;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPThreadLocallyAccurate, PageEvictionerFilterNone, false> PageEvictionerLOOPThreadLocallyAccurate;

    // RANDOM Page Evictioners from the C++ Standard Library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMCRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMCRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand0, PageEvictionerFilterNone, false> PageEvictionerRANDOMMinstdRand0;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand, PageEvictionerFilterNone, false> PageEvictionerRANDOMMinstdRand;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937, PageEvictionerFilterNone, false> PageEvictionerRANDOMMT19937;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937_64, PageEvictionerFilterNone, false> PageEvictionerRANDOMMT19937_64;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24Base, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux24Base;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48Base, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux48Base;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux24;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48, PageEvictionerFilterNone, false> PageEvictionerRANDOMRanlux48;

    // RANDOM Page Evictioners from the Boost RANDOM library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKnuthB, PageEvictionerFilterNone, false> PageEvictionerRANDOMKnuthB;
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
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_3_01, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux64_3_01;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_4_01, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux64_4_01;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux24, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux24;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux48, PageEvictionerFilterNone, false> PageEvictionerRANDOMBoostRanlux48;

    // RANDOM Page Evictioners from Melissa O'Neill's PCG library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32Unique, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32Unique;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32Fast;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K2;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K2Fast;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K64;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K64Fast;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K1024;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K1024Fast;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K16384;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMPCG32K16384Fast;

    // RANDOM Page Evictioners from Melissa O'Neill's XorShift library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64Star, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift64Star;

    // RANDOM Page Evictioners based on XorShift:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORWow, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORWow;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift128Plus, PageEvictionerFilterNone, false> PageEvictionerRANDOMXORShift128Plus;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoshiro library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128Plus32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoshiro128Plus32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128StarStar32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoshiro128StarStar32;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoroshiro library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro128Plus32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoroshiro128Plus32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Plus32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoroshiro64Plus32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Star32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoroshiro64Star32;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64StarStar32, PageEvictionerFilterNone, false> PageEvictionerRANDOMXoroshiro64StarStar32;

    // RANDOM Page Evictioners from Melissa O'Neill's Arc4 library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARC4, PageEvictionerFilterNone, false> PageEvictionerRANDOMARC4;

    // RANDOM Page Evictioners from Melissa O'Neill's GJrand library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMGjrand, PageEvictionerFilterNone, false> PageEvictionerRANDOMGjrand;

    // RANDOM Page Evictioners from Melissa O'Neill's JSF library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32n, PageEvictionerFilterNone, false> PageEvictionerRANDOMJSF32n;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32r, PageEvictionerFilterNone, false> PageEvictionerRANDOMJSF32r;

    // RANDOM Page Evictioners from Melissa O'Neill's SplitMix library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSplitMix32, PageEvictionerFilterNone, false> PageEvictionerRANDOMSplitMix32;

    // RANDOM Page Evictioners from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128, PageEvictionerFilterNone, false> PageEvictionerRANDOMMCG128;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128Fast, PageEvictionerFilterNone, false> PageEvictionerRANDOMMCG128Fast;
#endif // __SIZEOF_INT128__

    // RANDOM Page Evictioners from Melissa O'Neill's ChaCha library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha4, PageEvictionerFilterNone, false> PageEvictionerRANDOMChaCha4;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha5, PageEvictionerFilterNone, false> PageEvictionerRANDOMChaCha5;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha6, PageEvictionerFilterNone, false> PageEvictionerRANDOMChaCha6;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha8, PageEvictionerFilterNone, false> PageEvictionerRANDOMChaCha8;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha20, PageEvictionerFilterNone, false> PageEvictionerRANDOMChaCha20;

    // RANDOM Page Evictioners from Melissa O'Neill's SFC library:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSFC32, PageEvictionerFilterNone, false> PageEvictionerRANDOMSFC32;

    // FIFO/FILO (First in, first out/First in, last out) Page Evictioners:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOLowContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOHighContention<>, PageEvictionerFilterNone, false> PageEvictionerFIFOHighContention;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFILOLowContention<>, PageEvictionerFilterNone, false> PageEvictionerFILOLowContention;

    // LRU/MRU (Least/Most Recently Used) Page Evictioners:
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

    // LFU (Least Frequently Used) Page Evictioners:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLFU<>, PageEvictionerFilterNone, false> PageEvictionerLFU;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLFUDA<>, PageEvictionerFilterNone, false> PageEvictionerLFUDA;

    // LRD (Least Reference Density) Page Evictioners:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV1, PageEvictionerFilterNone, false> PageEvictionerLRDV1;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionSubtraction<10>>, PageEvictionerFilterNone, false> PageEvictionerLRDV2Subtraction;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionMultiplication<>>, PageEvictionerFilterNone, false> PageEvictionerLRDV2Multiplication;

    // CLOCK Page Evictioners:
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<>, true> PageEvictionerCLOCKFix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<false, true>, true> PageEvictionerCLOCKUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<true, true>, true> PageEvictionerCLOCKFixUnfix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<>, true> PageEvictionerGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerGCLOCKV2Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true>, true> PageEvictionerDGCLOCKV1Fix;
    typedef PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true> PageEvictionerDGCLOCKV2Fix;
    typedef PageEvictionerCAR<> PageEvictionerCARFix;
    typedef PageEvictionerCAR<true> PageEvictionerUnfix;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
