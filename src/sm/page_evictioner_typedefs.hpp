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
    using PageEvictionerSelectorRANDOMMinstdRand0 =
            PageEvictionerSelectorRANDOMExternal<std::minstd_rand0, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMinstdRand =
            PageEvictionerSelectorRANDOMExternal<std::minstd_rand, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937 =
            PageEvictionerSelectorRANDOMExternal<std::mt19937, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937_64 =
            PageEvictionerSelectorRANDOMExternal<std::mt19937_64, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24Base =
            PageEvictionerSelectorRANDOMExternal<std::ranlux24_base, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48Base =
            PageEvictionerSelectorRANDOMExternal<std::ranlux48_base, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24 = PageEvictionerSelectorRANDOMExternal<std::ranlux24, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48 = PageEvictionerSelectorRANDOMExternal<std::ranlux48, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMKnuthB = PageEvictionerSelectorRANDOMExternal<std::knuth_b, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from the Boost RANDOM library:
    using PageEvictionerSelectorRANDOMBoostMinstdRand0 = PageEvictionerSelectorRANDOMExternal<boost::random::minstd_rand0, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostMinstdRand = PageEvictionerSelectorRANDOMExternal<boost::random::minstd_rand, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRand48 = PageEvictionerSelectorRANDOMExternal<boost::random::rand48, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostEcuyer1988 = PageEvictionerSelectorRANDOMExternal<boost::random::ecuyer1988, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostKnuthB = PageEvictionerSelectorRANDOMExternal<boost::random::knuth_b, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostKreutzer1986 = PageEvictionerSelectorRANDOMExternal<boost::random::kreutzer1986, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostTaus88 = PageEvictionerSelectorRANDOMExternal<boost::random::taus88, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostHellekalek1995 = PageEvictionerSelectorRANDOMExternal<boost::random::hellekalek1995, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostMT11213b = PageEvictionerSelectorRANDOMExternal<boost::random::mt11213b, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostMT19937 = PageEvictionerSelectorRANDOMExternal<boost::random::mt19937, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostMT19937_64 = PageEvictionerSelectorRANDOMExternal<boost::random::mt19937_64, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci607 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci607, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci1279, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci2281, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci3217, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci4423, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci9689, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci19937, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci23209, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497 = PageEvictionerSelectorRANDOMExternal<boost::random::lagged_fibonacci44497, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux3 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux3, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux4 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux4, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux64_3 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_3, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux64_4 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_4, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux3_01 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux3_01, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux4_01 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux4_01, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux64_3_01 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_3_01, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux64_4_01 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux64_4_01, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux24 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux24, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMBoostRanlux48 = PageEvictionerSelectorRANDOMExternal<boost::random::ranlux48, SeedGeneratorRandomDevice<uint64_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's PCG library:
    using PageEvictionerSelectorRANDOMPCG32 = PageEvictionerSelectorRANDOMExternal<pcg32, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32Unique = PageEvictionerSelectorRANDOMExternal<pcg32_unique, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32Fast = PageEvictionerSelectorRANDOMExternal<pcg32_fast, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2 = PageEvictionerSelectorRANDOMExternal<pcg32_k2, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2Fast = PageEvictionerSelectorRANDOMExternal<pcg32_k2_fast, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64 = PageEvictionerSelectorRANDOMExternal<pcg32_k64, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64Fast = PageEvictionerSelectorRANDOMExternal<pcg32_k64_fast, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024 = PageEvictionerSelectorRANDOMExternal<pcg32_k1024, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024Fast = PageEvictionerSelectorRANDOMExternal<pcg32_k1024_fast, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384 = PageEvictionerSelectorRANDOMExternal<pcg32_k16384, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384Fast = PageEvictionerSelectorRANDOMExternal<pcg32_k16384_fast, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's XorShift library:
    using PageEvictionerSelectorRANDOMXORShift32 = PageEvictionerSelectorRANDOMExternal<xorshift32plain32a, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXORShift64Star = PageEvictionerSelectorRANDOMExternal<xorshift64star32a, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors based on XorShift:
    class PageEvictionerSelectorRANDOMXORWow;
    class PageEvictionerSelectorRANDOMXORShift128Plus;

    // RANDOM Page Selectors from Melissa O'Neill's Xoshiro library:
    using PageEvictionerSelectorRANDOMXoshiro128Plus32 = PageEvictionerSelectorRANDOMExternal<xoshiro128plus32, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoshiro128StarStar32 = PageEvictionerSelectorRANDOMExternal<xoshiro128starstar32 , SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Xoroshiro library:
    using PageEvictionerSelectorRANDOMXoroshiro128Plus32 = PageEvictionerSelectorRANDOMExternal<xoroshiro128plus32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Plus32 = PageEvictionerSelectorRANDOMExternal<xoroshiro64plus32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Star32 = PageEvictionerSelectorRANDOMExternal<xoroshiro64star32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64StarStar32 = PageEvictionerSelectorRANDOMExternal<xoroshiro64starstar32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Arc4 library:
    using PageEvictionerSelectorRANDOMARC4 = PageEvictionerSelectorRANDOMExternal<arc4_rand32, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's GJrand library:
    using PageEvictionerSelectorRANDOMGjrand = PageEvictionerSelectorRANDOMExternal<gjrand32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's JSF library:
    using PageEvictionerSelectorRANDOMJSF32n = PageEvictionerSelectorRANDOMExternal<jsf32n, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMJSF32r = PageEvictionerSelectorRANDOMExternal<jsf32r, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's SplitMix library:
    using PageEvictionerSelectorRANDOMSplitMix32 = PageEvictionerSelectorRANDOMExternal<splitmix32, SeedGeneratorRandomDevice<uint64_t>, SeedGeneratorRandomDevice<uint64_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    using PageEvictionerSelectorRANDOMMCG128 = PageEvictionerSelectorRANDOMExternal<mcg128, SeedGeneratorRandomDevice<__uint128_t>>;
    using PageEvictionerSelectorRANDOMMCG128Fast = PageEvictionerSelectorRANDOMExternal<mcg128_fast, SeedGeneratorRandomDevice<__uint128_t>>;
#endif // __SIZEOF_INT128__

    // RANDOM Page Selectors from Melissa O'Neill's ChaCha library:
    using PageEvictionerSelectorRANDOMChaCha4 = PageEvictionerSelectorRANDOMExternal<chacha4r, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha5 = PageEvictionerSelectorRANDOMExternal<chacha5r, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha6 = PageEvictionerSelectorRANDOMExternal<chacha6r, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha8 = PageEvictionerSelectorRANDOMExternal<chacha8r, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha20 = PageEvictionerSelectorRANDOMExternal<chacha20r, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's SFC library:
    using PageEvictionerSelectorRANDOMSFC32 = PageEvictionerSelectorRANDOMExternal<sfc32, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>, SeedGeneratorRandomDevice<uint32_t>>;

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
    using PageEvictionerLOOPAbsolutelyAccurate = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPAbsolutelyAccurate, PageEvictionerFilterNone, false>;
    using PageEvictionerLOOPPracticallyAccurate = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterNone, false>;
    using PageEvictionerLOOPLockFree = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPLockFree, PageEvictionerFilterNone, false>;
    using PageEvictionerLOOPThreadLocallyAccurate = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPThreadLocallyAccurate, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from the C++ Standard Library:
    using PageEvictionerRANDOMCRand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMCRand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMinstdRand0 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand0, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMinstdRand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT19937 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT19937_64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937_64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux24Base = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24Base, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux48Base = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48Base, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux24 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux48 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from the Boost RANDOM library:
    using PageEvictionerRANDOMKnuthB = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKnuthB, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostMinstdRand0 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMinstdRand0, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostMinstdRand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMinstdRand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRand48 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRand48, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostEcuyer1988 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostEcuyer1988, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostKnuthB = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostKnuthB, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostKreutzer1986 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostKreutzer1986, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostTaus88 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostTaus88, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostHellekalek1995 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostHellekalek1995, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostMT11213b = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT11213b, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostMT19937 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT19937, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostMT19937_64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostMT19937_64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci607 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci607, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci1279 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci1279, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci2281 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci2281, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci3217 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci3217, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci4423 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci4423, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci9689 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci9689, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci19937 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci19937, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci23209 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci23209, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostLaggedFibonacci44497 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostLaggedFibonacci44497, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux3, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux64_3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_3, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux64_4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux3_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux3_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux4_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux4_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux64_3_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_3_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux64_4_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux64_4_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux24 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux24, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMBoostRanlux48 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMBoostRanlux48, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's PCG library:
    using PageEvictionerRANDOMPCG32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32Unique = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32Unique, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32Fast, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K2 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K2Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2Fast, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K64Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64Fast, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K1024 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K1024Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024Fast, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K16384 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K16384Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384Fast, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's XorShift library:
    using PageEvictionerRANDOMXORShift32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift64Star = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64Star, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners based on XorShift:
    using PageEvictionerRANDOMXORWow = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORWow, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift128Plus = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift128Plus, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoshiro library:
    using PageEvictionerRANDOMXoshiro128Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoshiro128StarStar32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128StarStar32, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoroshiro library:
    using PageEvictionerRANDOMXoroshiro128Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro128Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Star32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Star32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64StarStar32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64StarStar32, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Arc4 library:
    using PageEvictionerRANDOMARC4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARC4, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's GJrand library:
    using PageEvictionerRANDOMGjrand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMGjrand, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's JSF library:
    using PageEvictionerRANDOMJSF32n = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32n, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMJSF32r = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32r, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's SplitMix library:
    using PageEvictionerRANDOMSplitMix32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSplitMix32, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    using PageEvictionerRANDOMMCG128 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMCG128Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128Fast, PageEvictionerFilterNone, false>;
#endif // __SIZEOF_INT128__

    // RANDOM Page Evictioners from Melissa O'Neill's ChaCha library:
    using PageEvictionerRANDOMChaCha4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha5 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha5, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha6 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha6, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha8 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha8, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha20 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha20, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's SFC library:
    using PageEvictionerRANDOMSFC32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSFC32, PageEvictionerFilterNone, false>;

    // FIFO/FILO (First in, first out/First in, last out) Page Evictioners:
    using PageEvictionerFIFOLowContention = PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOLowContention<>, PageEvictionerFilterNone, false>;
    using PageEvictionerFIFOHighContention = PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFIFOHighContention<>, PageEvictionerFilterNone, false>;
    using PageEvictionerFILOLowContention = PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiFILOLowContention<>, PageEvictionerFilterNone, false>;

    // LRU/MRU (Least/Most Recently Used) Page Evictioners:
    using PageEvictionerLRU = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRU, PageEvictionerFilterNone, false>;
    using PageEvictionerLRU2 = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<2>, PageEvictionerFilterNone, false>;
    using PageEvictionerLRU3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<3>, PageEvictionerFilterNone, false>;
    using PageEvictionerLRU4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRUK<4>, PageEvictionerFilterNone, false>;
    using PageEvictionerSLRU = PageEvictionerSelectAndFilter<PageEvictionerSelectorSLRU<>, PageEvictionerFilterNone, false>;
    using PageEvictionerMRU = PageEvictionerSelectAndFilter<PageEvictionerSelectorQuasiMRU<>, PageEvictionerFilterNone, false>;
    using PageEvictionerTimestampLRU = PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRU<>, PageEvictionerFilterNone, false>;
    using PageEvictionerTimestampLRU2 = PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<2>, PageEvictionerFilterNone, false>;
    using PageEvictionerTimestampLRU3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<3>, PageEvictionerFilterNone, false>;
    using PageEvictionerTimestampLRU4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorTimestampLRUK<4>, PageEvictionerFilterNone, false>;

    // LFU (Least Frequently Used) Page Evictioners:
    using PageEvictionerLFU = PageEvictionerSelectAndFilter<PageEvictionerSelectorLFU<>, PageEvictionerFilterNone, false>;
    using PageEvictionerLFUDA = PageEvictionerSelectAndFilter<PageEvictionerSelectorLFUDA<>, PageEvictionerFilterNone, false>;

    // LRD (Least Reference Density) Page Evictioners:
    using PageEvictionerLRDV1 = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV1, PageEvictionerFilterNone, false>;
    using PageEvictionerLRDV2Subtraction = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionSubtraction<10>>, PageEvictionerFilterNone, false>;
    using PageEvictionerLRDV2Multiplication = PageEvictionerSelectAndFilter<PageEvictionerSelectorLRDV2<10, AgingFunctionMultiplication<>>, PageEvictionerFilterNone, false>;

    // CLOCK Page Evictioners:
    using PageEvictionerCLOCKFix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<>, true>;
    using PageEvictionerCLOCKUnfix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<false, true>, true>;
    using PageEvictionerCLOCKFixUnfix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterCLOCK<true, true>, true>;
    using PageEvictionerGCLOCKV1Fix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<>, true>;
    using PageEvictionerGCLOCKV2Fix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, false, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true>;
    using PageEvictionerDGCLOCKV1Fix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true>, true>;
    using PageEvictionerDGCLOCKV2Fix = PageEvictionerSelectAndFilter<PageEvictionerSelectorLOOPPracticallyAccurate, PageEvictionerFilterGCLOCK<1, true, true, true, 5, 2, 1, false, true, 5, 2, 1, true, true, 25, 10, 5, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1, false, true, 5, 2, 1>, true>;
    using PageEvictionerCARFix = PageEvictionerCAR<>;
    using PageEvictionerUnfix = PageEvictionerCAR<true>;
}

#endif // __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
