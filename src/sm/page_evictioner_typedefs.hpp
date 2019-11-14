#ifndef __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP
#define __ZERO_PAGE_EVICTIONER_TYPEDEFS_HPP

#include <cstdint>

#include <random>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/additive_combine.hpp>
#include <boost/random/shuffle_order.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/inversive_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/ranlux.hpp>
#include "pcg-cpp/include/pcg_random.hpp"
#include "xorshift/xorshift.hpp"
#include "xoshiro/xoshiro.hpp"
#include "xoroshiro/xoroshiro.hpp"
#include "arc4/arc4.hpp"
#include "gjrand/gjrand.hpp"
#include "jsf/jsf.hpp"
#include "splitmix/splitmix.hpp"
#if __SIZEOF_INT128__
#include "lehmer/lehmer.hpp"
#endif // __SIZEOF_INT128__
#include "sfc/sfc.hpp"
#include "well-random/well.hpp"
#include "Random123/include/Random123/conventional/Engine.hpp"
#include "Random123/include/Random123/threefry.h"
#include "Random123/include/Random123/philox.h"
#include "Random123/include/Random123/aes.h"
#include "Random123/include/Random123/ars.h"
#include "chacha/chacha.hpp"
#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MixMaxRng.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/RanshiEngine.h"
#include "CLHEP/Random/TripleRand.h"

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
    template<class random_number_generator, class random_distribution, class ... seed_generators> class PageEvictionerSelectorRANDOMExternal;
    template<class random_number_generator, class random_distribution, bool seed_explicitly, class ... seed_generators> class PageEvictionerSelectorRANDOMExternalThreadLocal;
    template<class random_number_generator, class ... seed_generators> class PageEvictionerSelectorRANDOMCLHEP;
    template<class random_number_generator, bool seed_explicitly, class ... seed_generators> class PageEvictionerSelectorRANDOMCLHEPThreadLocal;

    // RANDOM Page Selectors from the C++ Standard Library:
    class PageEvictionerSelectorRANDOMCRand;
    using PageEvictionerSelectorRANDOMMinstdRand0 =
            PageEvictionerSelectorRANDOMExternal<std::minstd_rand0, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMinstdRand =
            PageEvictionerSelectorRANDOMExternal<std::minstd_rand, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937 =
            PageEvictionerSelectorRANDOMExternal<std::mt19937, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937_64 =
            PageEvictionerSelectorRANDOMExternal<std::mt19937_64, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24Base =
            PageEvictionerSelectorRANDOMExternal<std::ranlux24_base, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48Base =
            PageEvictionerSelectorRANDOMExternal<std::ranlux48_base, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24 =
            PageEvictionerSelectorRANDOMExternal<std::ranlux24, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48 =
            PageEvictionerSelectorRANDOMExternal<std::ranlux48, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMKnuthB =
            PageEvictionerSelectorRANDOMExternal<std::knuth_b, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMinstdRand0ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::minstd_rand0, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMinstdRandThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::minstd_rand, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::mt19937, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT19937_64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::mt19937_64, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24BaseThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::ranlux24_base, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48BaseThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::ranlux48_base, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux24ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::ranlux24, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux48ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::ranlux48, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMKnuthBThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<std::knuth_b, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from the Boost RANDOM library:
    namespace brand = boost::random;
    using PageEvictionerSelectorRANDOMRand48 =
            PageEvictionerSelectorRANDOMExternal<brand::rand48, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMEcuyer1988 =
            PageEvictionerSelectorRANDOMExternal<brand::ecuyer1988, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMKreutzer1986 =
            PageEvictionerSelectorRANDOMExternal<brand::kreutzer1986, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMTaus88 =
            PageEvictionerSelectorRANDOMExternal<brand::taus88, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMHellekalek1995 =
            PageEvictionerSelectorRANDOMExternal<brand::hellekalek1995, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT11213b =
            PageEvictionerSelectorRANDOMExternal<brand::mt11213b, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci607 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci607, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci1279 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci1279, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci2281 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci2281, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci3217 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci3217, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci4423 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci4423, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci9689 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci9689, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci19937 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci19937, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci23209 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci23209, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci44497 =
            PageEvictionerSelectorRANDOMExternal<brand::lagged_fibonacci44497, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux3 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux3, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux4 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux4, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_3 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux64_3, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_4 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux64_4, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux3_01 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux3_01, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux4_01 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux4_01, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_3_01 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux64_3_01, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_4_01 =
            PageEvictionerSelectorRANDOMExternal<brand::ranlux64_4_01, brand::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRand48ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::rand48, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMEcuyer1988ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ecuyer1988, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMKreutzer1986ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::kreutzer1986, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMTaus88ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::taus88, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMHellekalek1995ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::hellekalek1995, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMMT11213bThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::mt11213b, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci607ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci607, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci1279ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci1279, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci2281ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci2281, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci3217ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci3217, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci4423ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci4423, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci9689ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci9689, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci19937ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci19937, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci23209ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci23209, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMLaggedFibonacci44497ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::lagged_fibonacci44497, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux3ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux3, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux4ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux4, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_3ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux64_3, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_4ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux64_4, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMRanlux3_01ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux3_01, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux4_01ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux4_01, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_3_01ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux64_3_01, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMRanlux64_4_01ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<brand::ranlux64_4_01, brand::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's PCG library:
    using PageEvictionerSelectorRANDOMPCG32 =
            PageEvictionerSelectorRANDOMExternal<pcg32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32Unique =
            PageEvictionerSelectorRANDOMExternal<pcg32_unique, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32Fast =
            PageEvictionerSelectorRANDOMExternal<pcg32_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2 =
            PageEvictionerSelectorRANDOMExternal<pcg32_k2, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2Fast =
            PageEvictionerSelectorRANDOMExternal<pcg32_k2_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64 =
            PageEvictionerSelectorRANDOMExternal<pcg32_k64, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64Fast =
            PageEvictionerSelectorRANDOMExternal<pcg32_k64_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024 =
            PageEvictionerSelectorRANDOMExternal<pcg32_k1024, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024Fast =
            PageEvictionerSelectorRANDOMExternal<pcg32_k1024_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384 =
            PageEvictionerSelectorRANDOMExternal<pcg32_k16384, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384Fast =
            PageEvictionerSelectorRANDOMExternal<pcg32_k16384_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32UniqueThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_unique, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k2, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K2FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k2_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k64, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K64FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k64_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k1024, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K1024FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k1024_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k16384, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPCG32K16384FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<pcg32_k16384_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's XorShift library:
    using PageEvictionerSelectorRANDOMXORShift32 =
            PageEvictionerSelectorRANDOMExternal<xorshift32plain32a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXORShift64Star =
            PageEvictionerSelectorRANDOMExternal<xorshift64star32a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXORShift32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xorshift32plain32a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXORShift64StarThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xorshift64star32a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors based on XorShift:
    class PageEvictionerSelectorRANDOMXORWow;
    class PageEvictionerSelectorRANDOMXORShift128Plus;

    // RANDOM Page Selectors from Melissa O'Neill's Xoshiro library:
    using PageEvictionerSelectorRANDOMXoshiro128Plus32 =
            PageEvictionerSelectorRANDOMExternal<xoshiro128plus32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoshiro128StarStar32 =
            PageEvictionerSelectorRANDOMExternal<xoshiro128starstar32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoshiro128Plus32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoshiro128plus32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoshiro128StarStar32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoshiro128starstar32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Xoroshiro library:
    using PageEvictionerSelectorRANDOMXoroshiro128Plus32 =
            PageEvictionerSelectorRANDOMExternal<xoroshiro128plus32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Plus32 =
            PageEvictionerSelectorRANDOMExternal<xoroshiro64plus32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Star32 =
            PageEvictionerSelectorRANDOMExternal<xoroshiro64star32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64StarStar32 =
            PageEvictionerSelectorRANDOMExternal<xoroshiro64starstar32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro128Plus32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoroshiro128plus32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Plus32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoroshiro64plus32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64Star32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoroshiro64star32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMXoroshiro64StarStar32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<xoroshiro64starstar32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Arc4 library:
    using PageEvictionerSelectorRANDOMARC4 =
            PageEvictionerSelectorRANDOMExternal<arc4_rand32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMARC4ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<arc4_rand32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's GJrand library:
    using PageEvictionerSelectorRANDOMGjrand =
            PageEvictionerSelectorRANDOMExternal<gjrand32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMGjrandThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<gjrand32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's JSF library:
    using PageEvictionerSelectorRANDOMJSF32n =
            PageEvictionerSelectorRANDOMExternal<jsf32n, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMJSF32r =
            PageEvictionerSelectorRANDOMExternal<jsf32r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMJSF32nThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<jsf32n, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMJSF32rThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<jsf32r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's SplitMix library:
    using PageEvictionerSelectorRANDOMSplitMix32 =
            PageEvictionerSelectorRANDOMExternal<splitmix32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint64_t>,
                                                 SeedGeneratorRandomDevice<uint64_t>>;
    using PageEvictionerSelectorRANDOMSplitMix32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<splitmix32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint64_t>,
                                                            SeedGeneratorRandomDevice<uint64_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    using PageEvictionerSelectorRANDOMMCG128 =
            PageEvictionerSelectorRANDOMExternal<mcg128, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<__uint128_t>>;
    using PageEvictionerSelectorRANDOMMCG128Fast =
            PageEvictionerSelectorRANDOMExternal<mcg128_fast, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<__uint128_t>>;
    using PageEvictionerSelectorRANDOMMCG128ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<mcg128, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<__uint128_t>>;
    using PageEvictionerSelectorRANDOMMCG128FastThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<mcg128_fast, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<__uint128_t>>;
#endif // __SIZEOF_INT128__

    // RANDOM Page Selectors from Melissa O'Neill's ChaCha library:
    using PageEvictionerSelectorRANDOMChaCha4 =
            PageEvictionerSelectorRANDOMExternal<chacha4r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha5 =
            PageEvictionerSelectorRANDOMExternal<chacha5r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha6 =
            PageEvictionerSelectorRANDOMExternal<chacha6r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha8 =
            PageEvictionerSelectorRANDOMExternal<chacha8r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha20 =
            PageEvictionerSelectorRANDOMExternal<chacha20r, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha4ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<chacha4r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha5ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<chacha5r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha6ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<chacha6r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha8ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<chacha8r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMChaCha20ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<chacha20r, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Melissa O'Neill's SFC library:
    using PageEvictionerSelectorRANDOMSFC32 =
            PageEvictionerSelectorRANDOMExternal<sfc32, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMSFC32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<sfc32, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>,
                                                            SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from Sergiu Deitsch's random library:
    using PageEvictionerSelectorRANDOMWELL512 =
            PageEvictionerSelectorRANDOMExternal<Well512a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL521 =
            PageEvictionerSelectorRANDOMExternal<Well521a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL607 =
            PageEvictionerSelectorRANDOMExternal<Well607a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL800 =
            PageEvictionerSelectorRANDOMExternal<Well800a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL1024 =
            PageEvictionerSelectorRANDOMExternal<Well1024a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL19937 =
            PageEvictionerSelectorRANDOMExternal<Well19937a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL21701 =
            PageEvictionerSelectorRANDOMExternal<Well21701a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL23209 =
            PageEvictionerSelectorRANDOMExternal<Well23209a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL44497 =
            PageEvictionerSelectorRANDOMExternal<Well44497a, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL800_ME =
            PageEvictionerSelectorRANDOMExternal<Well800a_ME, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL19937_ME =
            PageEvictionerSelectorRANDOMExternal<Well19937a_ME, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL21701_ME =
            PageEvictionerSelectorRANDOMExternal<Well21701a_ME, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL23209_ME =
            PageEvictionerSelectorRANDOMExternal<Well23209a_ME, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL44497_ME =
            PageEvictionerSelectorRANDOMExternal<Well44497a_ME, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL512ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well512a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL521ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well521a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL607ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well607a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL800ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well800a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL1024ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well1024a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL19937ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well19937a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL21701ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well21701a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL23209ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well23209a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL44497ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well44497a, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL800_METhreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well800a_ME, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL19937_METhreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well19937a_ME, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL21701_METhreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well21701a_ME, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL23209_METhreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well23209a_ME, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMWELL44497_METhreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<Well44497a_ME, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;

    // RANDOM Page Selectors from QuinoaComputing's Random123 library:
    using PageEvictionerSelectorRANDOMThreefry2x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Threefry2x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry4x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Threefry4x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry2x64 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Threefry2x64>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry4x64 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Threefry4x64>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox2x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Philox2x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox4x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Philox4x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox2x64 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Philox2x64>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox4x64 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::Philox4x64>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
#if R123_USE_AES_NI
    using PageEvictionerSelectorRANDOMAESNI4x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::AESNI4x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
//    using PageEvictionerSelectorRANDOMAESNI1xm128i =
//            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::AESNI1xm128i>, std::uniform_int_distribution<bf_idx>,
//                                                 SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMARS4x32 =
            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::ARS4x32>, std::uniform_int_distribution<bf_idx>,
                                                 SeedGeneratorRandomDevice<uint32_t>>;
//    using PageEvictionerSelectorRANDOMARS1xm128i =
//            PageEvictionerSelectorRANDOMExternal<r123::Engine<r123::ARS1xm128i>, std::uniform_int_distribution<bf_idx>,
//                                                 SeedGeneratorRandomDevice<uint32_t>>;
#endif // R123_USE_AES_NI
    using PageEvictionerSelectorRANDOMThreefry2x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Threefry2x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry4x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Threefry4x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry2x64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Threefry2x64>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMThreefry4x64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Threefry4x64>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox2x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Philox2x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox4x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Philox4x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox2x64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Philox2x64>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMPhilox4x64ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::Philox4x64>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
#if R123_USE_AES_NI
    using PageEvictionerSelectorRANDOMAESNI4x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::AESNI4x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
//    using PageEvictionerSelectorRANDOMAESNI1xm128iThreadLocal =
//            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::AESNI1xm128i>, std::uniform_int_distribution<bf_idx>,
//                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
    using PageEvictionerSelectorRANDOMARS4x32ThreadLocal =
            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::ARS4x32>, std::uniform_int_distribution<bf_idx>,
                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
//    using PageEvictionerSelectorRANDOMARS1xm128iThreadLocal =
//            PageEvictionerSelectorRANDOMExternalThreadLocal<r123::Engine<r123::ARS1xm128i>, std::uniform_int_distribution<bf_idx>,
//                                                            true, SeedGeneratorRandomDevice<uint32_t>>;
#endif // R123_USE_AES_NI

    // RANDOM Page Selectors from CLHEP's Random library:
    using PageEvictionerSelectorRANDOMDualRand =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::DualRand, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMHurd160 =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::Hurd160Engine, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMHurd288 =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::Hurd288Engine, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRANMAR =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::HepJamesRandom, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMMixMax =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::MixMaxRng, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRANECU =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::RanecuEngine, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRanshi =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::RanshiEngine, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMTripleRand =
            PageEvictionerSelectorRANDOMCLHEP<CLHEP::TripleRand, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMDualRandThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::DualRand, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMHurd160ThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::Hurd160Engine, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMHurd288ThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::Hurd288Engine, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRANMARThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::HepJamesRandom, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMMixMaxThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::MixMaxRng, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRANECUThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::RanecuEngine, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMRanshiThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::RanshiEngine, true, SeedGeneratorRandomDevice<bf_idx>>;
    using PageEvictionerSelectorRANDOMTripleRandThreadLocal =
            PageEvictionerSelectorRANDOMCLHEPThreadLocal<CLHEP::TripleRand, true, SeedGeneratorRandomDevice<bf_idx>>;

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
    using PageEvictionerRANDOMKnuthB = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKnuthB, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMinstdRand0ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRand0ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMinstdRandThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMinstdRandThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT19937ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT19937_64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT19937_64ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux24BaseThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24BaseThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux48BaseThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48BaseThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux24ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux24ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux48ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux48ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMKnuthBThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKnuthBThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from the Boost RANDOM library:
    using PageEvictionerRANDOMRand48 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRand48, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMEcuyer1988 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMEcuyer1988, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMKreutzer1986 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKreutzer1986, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMTaus88 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMTaus88, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHellekalek1995 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHellekalek1995, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT11213b = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT11213b, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci607 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci607, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci1279 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci1279, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci2281 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci2281, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci3217 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci3217, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci4423 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci4423, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci9689 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci9689, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci19937 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci19937, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci23209 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci23209, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci44497 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci44497, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux3, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_3 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_3, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux3_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux3_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux4_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux4_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_3_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_3_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_4_01 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_4_01, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRand48ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRand48ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMEcuyer1988ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMEcuyer1988ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMKreutzer1986ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMKreutzer1986ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMTaus88ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMTaus88ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHellekalek1995ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHellekalek1995ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMT11213bThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMT11213bThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci607ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci607ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci1279ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci1279ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci2281ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci2281ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci3217ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci3217ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci4423ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci4423ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci9689ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci9689ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci19937ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci19937ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci23209ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci23209ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMLaggedFibonacci44497ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMLaggedFibonacci44497ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux3ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux3ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux4ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux4ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_3ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_3ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_4ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_4ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux3_01ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux3_01ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux4_01ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux4_01ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_3_01ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_3_01ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanlux64_4_01ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanlux64_4_01ThreadLocal, PageEvictionerFilterNone, false>;

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
    using PageEvictionerRANDOMPCG32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32UniqueThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32UniqueThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32FastThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K2ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K2FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K2FastThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K64FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K64FastThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K1024ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K1024FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K1024FastThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K16384ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPCG32K16384FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPCG32K16384FastThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's XorShift library:
    using PageEvictionerRANDOMXORShift32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift64Star = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64Star, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift64StarThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift64StarThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners based on XorShift:
    using PageEvictionerRANDOMXORWow = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORWow, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXORShift128Plus = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXORShift128Plus, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoshiro library:
    using PageEvictionerRANDOMXoshiro128Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoshiro128StarStar32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128StarStar32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoshiro128Plus32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128Plus32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoshiro128StarStar32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoshiro128StarStar32ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Xoroshiro library:
    using PageEvictionerRANDOMXoroshiro128Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro128Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Plus32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Plus32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Star32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Star32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64StarStar32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64StarStar32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro128Plus32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro128Plus32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Plus32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Plus32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64Star32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64Star32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMXoroshiro64StarStar32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMXoroshiro64StarStar32ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Arc4 library:
    using PageEvictionerRANDOMARC4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARC4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMARC4ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARC4ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's GJrand library:
    using PageEvictionerRANDOMGjrand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMGjrand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMGjrandThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMGjrandThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's JSF library:
    using PageEvictionerRANDOMJSF32n = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32n, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMJSF32r = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32r, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMJSF32nThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32nThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMJSF32rThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMJSF32rThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's SplitMix library:
    using PageEvictionerRANDOMSplitMix32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSplitMix32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMSplitMix32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSplitMix32ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's Lehmer/MCG library:
#if __SIZEOF_INT128__
    using PageEvictionerRANDOMMCG128 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMCG128Fast = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128Fast, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMCG128ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMCG128FastThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMCG128FastThreadLocal, PageEvictionerFilterNone, false>;
#endif // __SIZEOF_INT128__

    // RANDOM Page Evictioners from Melissa O'Neill's ChaCha library:
    using PageEvictionerRANDOMChaCha4 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha4, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha5 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha5, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha6 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha6, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha8 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha8, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha20 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha20, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha4ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha4ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha5ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha5ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha6ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha6ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha8ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha8ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMChaCha20ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMChaCha20ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Melissa O'Neill's SFC library:
    using PageEvictionerRANDOMSFC32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSFC32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMSFC32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMSFC32ThreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from Sergiu Deitsch's random library:
    using PageEvictionerRANDOMWELL512 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL512, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL521 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL521, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL607 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL607, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL800 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL800, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL1024 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL1024, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL19937 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL19937, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL21701 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL21701, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL23209 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL23209, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL44497 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL44497, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL800_ME = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL800_ME, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL19937_ME = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL19937_ME, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL21701_ME = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL21701_ME, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL23209_ME = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL23209_ME, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL44497_ME = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL44497_ME, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL512ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL512ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL521ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL521ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL607ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL607ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL800ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL800ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL1024ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL1024ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL19937ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL19937ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL21701ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL21701ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL23209ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL23209ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL44497ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL44497ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL800_METhreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL800_METhreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL19937_METhreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL19937_METhreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL21701_METhreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL21701_METhreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL23209_METhreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL23209_METhreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMWELL44497_METhreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMWELL44497_METhreadLocal, PageEvictionerFilterNone, false>;

    // RANDOM Page Evictioners from QuinoaComputing's Random123 library:
    using PageEvictionerRANDOMThreefry2x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry2x32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry4x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry4x32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry2x64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry2x64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry4x64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry4x64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox2x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox2x32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox4x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox4x32, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox2x64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox2x64, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox4x64 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox4x64, PageEvictionerFilterNone, false>;
#if R123_USE_AES_NI
    using PageEvictionerRANDOMAESNI4x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMAESNI4x32, PageEvictionerFilterNone, false>;
//    using PageEvictionerRANDOMAESNI1xm128i = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMAESNI1xm128i, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMARS4x32 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARS4x32, PageEvictionerFilterNone, false>;
//    using PageEvictionerRANDOMARS1xm128i = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARS1xm128i, PageEvictionerFilterNone, false>;
#endif // R123_USE_AES_NI
    using PageEvictionerRANDOMThreefry2x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry2x32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry4x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry4x32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry2x64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry2x64ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMThreefry4x64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMThreefry4x64ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox2x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox2x32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox4x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox4x32ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox2x64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox2x64ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMPhilox4x64ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMPhilox4x64ThreadLocal, PageEvictionerFilterNone, false>;
#if R123_USE_AES_NI
    using PageEvictionerRANDOMAESNI4x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMAESNI4x32ThreadLocal, PageEvictionerFilterNone, false>;
//    using PageEvictionerRANDOMAESNI1xm128iThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMAESNI1xm128iThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMARS4x32ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARS4x32ThreadLocal, PageEvictionerFilterNone, false>;
//    using PageEvictionerRANDOMARS1xm128iThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMARS1xm128iThreadLocal, PageEvictionerFilterNone, false>;
#endif // R123_USE_AES_NI

    // RANDOM Page Evictioners from CLHEP's Random library:
    using PageEvictionerRANDOMDualRand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMDualRand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHurd160 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHurd160, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHurd288 = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHurd288, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRANMAR = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRANMAR, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMixMax = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMixMax, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRANECU = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRANECU, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanshi = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanshi, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMTripleRand = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMTripleRand, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMDualRandThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMDualRandThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHurd160ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHurd160ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMHurd288ThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMHurd288ThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRANMARThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRANMARThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMMixMaxThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMMixMaxThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRANECUThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRANECUThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMRanshiThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMRanshiThreadLocal, PageEvictionerFilterNone, false>;
    using PageEvictionerRANDOMTripleRandThreadLocal = PageEvictionerSelectAndFilter<PageEvictionerSelectorRANDOMTripleRandThreadLocal, PageEvictionerFilterNone, false>;

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
