## = Usage: ============================================================================================================
## INCLUDE(SetUpPageEvictioner)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Supported Page Evictioners: =======================================================================================
## - PageEvictionerLOOPAbsolutelyAccurate
## - PageEvictionerLOOPPracticallyAccurate
## - PageEvictionerLOOPLockFree
## - PageEvictionerLOOPThreadLocallyAccurate
## - PageEvictionerRANDOMCRand
## - PageEvictionerRANDOMMinstdRand0
## - PageEvictionerRANDOMMinstdRand
## - PageEvictionerRANDOMMT19937
## - PageEvictionerRANDOMMT19937_64
## - PageEvictionerRANDOMRanlux24Base
## - PageEvictionerRANDOMRanlux48Base
## - PageEvictionerRANDOMRanlux24
## - PageEvictionerRANDOMRanlux48
## - PageEvictionerRANDOMKnuthB
## - PageEvictionerRANDOMMinstdRand0ThreadLocal
## - PageEvictionerRANDOMMinstdRandThreadLocal
## - PageEvictionerRANDOMMT19937ThreadLocal
## - PageEvictionerRANDOMMT19937_64ThreadLocal
## - PageEvictionerRANDOMRanlux24BaseThreadLocal
## - PageEvictionerRANDOMRanlux48BaseThreadLocal
## - PageEvictionerRANDOMRanlux24ThreadLocal
## - PageEvictionerRANDOMRanlux48ThreadLocal
## - PageEvictionerRANDOMKnuthBThreadLocal
## - PageEvictionerRANDOMBoostMinstdRand0
## - PageEvictionerRANDOMBoostMinstdRand
## - PageEvictionerRANDOMBoostRand48
## - PageEvictionerRANDOMBoostEcuyer1988
## - PageEvictionerRANDOMBoostKnuthB
## - PageEvictionerRANDOMBoostKreutzer1986
## - PageEvictionerRANDOMBoostTaus88
## - PageEvictionerRANDOMBoostHellekalek1995
## - PageEvictionerRANDOMBoostMT11213b
## - PageEvictionerRANDOMBoostMT19937
## - PageEvictionerRANDOMBoostMT19937_64
## - PageEvictionerRANDOMBoostLaggedFibonacci607
## - PageEvictionerRANDOMBoostLaggedFibonacci1279
## - PageEvictionerRANDOMBoostLaggedFibonacci2281
## - PageEvictionerRANDOMBoostLaggedFibonacci3217
## - PageEvictionerRANDOMBoostLaggedFibonacci4423
## - PageEvictionerRANDOMBoostLaggedFibonacci9689
## - PageEvictionerRANDOMBoostLaggedFibonacci19937
## - PageEvictionerRANDOMBoostLaggedFibonacci23209
## - PageEvictionerRANDOMBoostLaggedFibonacci44497
## - PageEvictionerRANDOMBoostRanlux3
## - PageEvictionerRANDOMBoostRanlux4
## - PageEvictionerRANDOMBoostRanlux64_3
## - PageEvictionerRANDOMBoostRanlux64_4
## - PageEvictionerRANDOMBoostRanlux3_01
## - PageEvictionerRANDOMBoostRanlux4_01
## - PageEvictionerRANDOMBoostRanlux64_3_01
## - PageEvictionerRANDOMBoostRanlux64_4_01
## - PageEvictionerRANDOMBoostRanlux24
## - PageEvictionerRANDOMBoostRanlux48
## - PageEvictionerRANDOMBoostMinstdRand0ThreadLocal
## - PageEvictionerRANDOMBoostMinstdRandThreadLocal
## - PageEvictionerRANDOMBoostRand48ThreadLocal
## - PageEvictionerRANDOMBoostEcuyer1988ThreadLocal
## - PageEvictionerRANDOMBoostKnuthBThreadLocal
## - PageEvictionerRANDOMBoostKreutzer1986ThreadLocal
## - PageEvictionerRANDOMBoostTaus88ThreadLocal
## - PageEvictionerRANDOMBoostHellekalek1995ThreadLocal
## - PageEvictionerRANDOMBoostMT11213bThreadLocal
## - PageEvictionerRANDOMBoostMT19937ThreadLocal
## - PageEvictionerRANDOMBoostMT19937_64ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci607ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci1279ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci2281ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci3217ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci4423ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci9689ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci19937ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci23209ThreadLocal
## - PageEvictionerRANDOMBoostLaggedFibonacci44497ThreadLocal
## - PageEvictionerRANDOMBoostRanlux3ThreadLocal
## - PageEvictionerRANDOMBoostRanlux4ThreadLocal
## - PageEvictionerRANDOMBoostRanlux64_3ThreadLocal
## - PageEvictionerRANDOMBoostRanlux64_4ThreadLocal
## - PageEvictionerRANDOMBoostRanlux3_01ThreadLocal
## - PageEvictionerRANDOMBoostRanlux4_01ThreadLocal
## - PageEvictionerRANDOMBoostRanlux64_3_01ThreadLocal
## - PageEvictionerRANDOMBoostRanlux64_4_01ThreadLocal
## - PageEvictionerRANDOMBoostRanlux24ThreadLocal
## - PageEvictionerRANDOMBoostRanlux48ThreadLocal
## - PageEvictionerRANDOMPCG32
## - PageEvictionerRANDOMPCG32Unique
## - PageEvictionerRANDOMPCG32Fast
## - PageEvictionerRANDOMPCG32K2
## - PageEvictionerRANDOMPCG32K2Fast
## - PageEvictionerRANDOMPCG32K64
## - PageEvictionerRANDOMPCG32K64Fast
## - PageEvictionerRANDOMPCG32K1024
## - PageEvictionerRANDOMPCG32K1024Fast
## - PageEvictionerRANDOMPCG32K16384
## - PageEvictionerRANDOMPCG32K16384Fast
## - PageEvictionerRANDOMPCG32ThreadLocal
## - PageEvictionerRANDOMPCG32UniqueThreadLocal
## - PageEvictionerRANDOMPCG32FastThreadLocal
## - PageEvictionerRANDOMPCG32K2ThreadLocal
## - PageEvictionerRANDOMPCG32K2FastThreadLocal
## - PageEvictionerRANDOMPCG32K64ThreadLocal
## - PageEvictionerRANDOMPCG32K64FastThreadLocal
## - PageEvictionerRANDOMPCG32K1024ThreadLocal
## - PageEvictionerRANDOMPCG32K1024FastThreadLocal
## - PageEvictionerRANDOMPCG32K16384ThreadLocal
## - PageEvictionerRANDOMPCG32K16384FastThreadLocal
## - PageEvictionerRANDOMXORShift32
## - PageEvictionerRANDOMXORShift64Star
## - PageEvictionerRANDOMXORShift32ThreadLocal
## - PageEvictionerRANDOMXORShift64StarThreadLocal
## - PageEvictionerRANDOMXORWow
## - PageEvictionerRANDOMXORShift128Plus
## - PageEvictionerRANDOMXoshiro128Plus32
## - PageEvictionerRANDOMXoshiro128StarStar32
## - PageEvictionerRANDOMXoshiro128Plus32ThreadLocal
## - PageEvictionerRANDOMXoshiro128StarStar32ThreadLocal
## - PageEvictionerRANDOMXoroshiro128Plus32
## - PageEvictionerRANDOMXoroshiro64Plus32
## - PageEvictionerRANDOMXoroshiro64Star32
## - PageEvictionerRANDOMXoroshiro64StarStar32
## - PageEvictionerRANDOMXoroshiro128Plus32ThreadLocal
## - PageEvictionerRANDOMXoroshiro64Plus32ThreadLocal
## - PageEvictionerRANDOMXoroshiro64Star32ThreadLocal
## - PageEvictionerRANDOMXoroshiro64StarStar32ThreadLocal
## - PageEvictionerRANDOMARC4
## - PageEvictionerRANDOMARC4ThreadLocal
## - PageEvictionerRANDOMGjrand
## - PageEvictionerRANDOMGjrandThreadLocal
## - PageEvictionerRANDOMJSF32n
## - PageEvictionerRANDOMJSF32r
## - PageEvictionerRANDOMJSF32nThreadLocal
## - PageEvictionerRANDOMJSF32rThreadLocal
## - PageEvictionerRANDOMSplitMix32
## - PageEvictionerRANDOMSplitMix32ThreadLocal
## - PageEvictionerRANDOMMCG128
## - PageEvictionerRANDOMMCG128Fast
## - PageEvictionerRANDOMMCG128ThreadLocal
## - PageEvictionerRANDOMMCG128FastThreadLocal
## - PageEvictionerRANDOMChaCha4
## - PageEvictionerRANDOMChaCha5
## - PageEvictionerRANDOMChaCha6
## - PageEvictionerRANDOMChaCha8
## - PageEvictionerRANDOMChaCha20
## - PageEvictionerRANDOMChaCha4ThreadLocal
## - PageEvictionerRANDOMChaCha5ThreadLocal
## - PageEvictionerRANDOMChaCha6ThreadLocal
## - PageEvictionerRANDOMChaCha8ThreadLocal
## - PageEvictionerRANDOMChaCha20ThreadLocal
## - PageEvictionerRANDOMSFC32
## - PageEvictionerRANDOMSFC32ThreadLocal
## - PageEvictionerRANDOMWELL512
## - PageEvictionerRANDOMWELL521
## - PageEvictionerRANDOMWELL607
## - PageEvictionerRANDOMWELL800
## - PageEvictionerRANDOMWELL1024
## - PageEvictionerRANDOMWELL19937
## - PageEvictionerRANDOMWELL21701
## - PageEvictionerRANDOMWELL23209
## - PageEvictionerRANDOMWELL44497
## - PageEvictionerRANDOMWELL800_ME
## - PageEvictionerRANDOMWELL19937_ME
## - PageEvictionerRANDOMWELL21701_ME
## - PageEvictionerRANDOMWELL23209_ME
## - PageEvictionerRANDOMWELL44497_ME
## - PageEvictionerRANDOMWELL512ThreadLocal
## - PageEvictionerRANDOMWELL521ThreadLocal
## - PageEvictionerRANDOMWELL607ThreadLocal
## - PageEvictionerRANDOMWELL800ThreadLocal
## - PageEvictionerRANDOMWELL1024ThreadLocal
## - PageEvictionerRANDOMWELL19937ThreadLocal
## - PageEvictionerRANDOMWELL21701ThreadLocal
## - PageEvictionerRANDOMWELL23209ThreadLocal
## - PageEvictionerRANDOMWELL44497ThreadLocal
## - PageEvictionerRANDOMWELL800_METhreadLocal
## - PageEvictionerRANDOMWELL19937_METhreadLocal
## - PageEvictionerRANDOMWELL21701_METhreadLocal
## - PageEvictionerRANDOMWELL23209_METhreadLocal
## - PageEvictionerRANDOMWELL44497_METhreadLocal
## - PageEvictionerRANDOMThreefry2x32
## - PageEvictionerRANDOMThreefry4x32
## - PageEvictionerRANDOMThreefry2x64
## - PageEvictionerRANDOMThreefry4x64
## - PageEvictionerRANDOMPhilox2x32
## - PageEvictionerRANDOMPhilox4x32
## - PageEvictionerRANDOMPhilox2x64
## - PageEvictionerRANDOMPhilox4x64
## - PageEvictionerRANDOMAESNI4x32
## - (PageEvictionerRANDOMAESNI1xm128i)
## - PageEvictionerRANDOMARS4x32
## - (PageEvictionerRANDOMARS1xm128i)
## - PageEvictionerRANDOMThreefry2x32ThreadLocal
## - PageEvictionerRANDOMThreefry4x32ThreadLocal
## - PageEvictionerRANDOMThreefry2x64ThreadLocal
## - PageEvictionerRANDOMThreefry4x64ThreadLocal
## - PageEvictionerRANDOMPhilox2x32ThreadLocal
## - PageEvictionerRANDOMPhilox4x32ThreadLocal
## - PageEvictionerRANDOMPhilox2x64ThreadLocal
## - PageEvictionerRANDOMPhilox4x64ThreadLocal
## - PageEvictionerRANDOMAESNI4x32ThreadLocal
## - (PageEvictionerRANDOMAESNI1xm128iThreadLocal)
## - PageEvictionerRANDOMARS4x32ThreadLocal
## - (PageEvictionerRANDOMARS1xm128iThreadLocal)
## - PageEvictionerFIFOLowContention
## - PageEvictionerFIFOHighContention
## - PageEvictionerFILOLowContention
## - PageEvictionerLRU
## - PageEvictionerLRU2
## - PageEvictionerLRU3
## - PageEvictionerLRU4
## - PageEvictionerSLRU
## - PageEvictionerMRU
## - PageEvictionerTimestampLRU
## - PageEvictionerTimestampLRU2
## - PageEvictionerTimestampLRU3
## - PageEvictionerTimestampLRU4
## - PageEvictionerLFU
## - PageEvictionerLFUDA
## - PageEvictionerLRDV1
## - PageEvictionerLRDV2Subtraction
## - PageEvictionerLRDV2Multiplication
## - PageEvictionerCLOCKFix
## - PageEvictionerCLOCKUnfix
## - PageEvictionerCLOCKFixUnfix
## - PageEvictionerGCLOCKV1Fix
## - PageEvictionerGCLOCKV2Fix
## - PageEvictionerDGCLOCKV1Fix
## - PageEvictionerDGCLOCKV2Fix
## - PageEvictionerCARFix
## - PageEvictionerUnfix
## =====================================================================================================================

SET(PAGE_EVICTIONER ON CACHE STRING "Page Evictioner used by the Buffer Pool")
IF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPAbsolutelyAccurate")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPAbsolutelyAccurate!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPPracticallyAccurate")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPPracticallyAccurate!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPLockFree")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPLockFree!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPThreadLocallyAccurate")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPThreadLocallyAccurate!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMCRand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMCRand!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMinstdRand0")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMinstdRand0!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMinstdRand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMinstdRand!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMT19937")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMT19937!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMT19937_64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMT19937_64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux24Base")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux24Base!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux48Base")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux48Base!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux24")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux24!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux48")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux48!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMKnuthB")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMKnuthB!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMinstdRand0ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMinstdRand0ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMinstdRandThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMinstdRandThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMT19937ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMT19937ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMT19937_64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMT19937_64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux24BaseThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux24BaseThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux48BaseThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux48BaseThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux24ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux24ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMRanlux48ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMRanlux48ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMKnuthBThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMKnuthBThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMinstdRand0")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMinstdRand0!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMinstdRand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMinstdRand!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRand48")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRand48!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostEcuyer1988")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostEcuyer1988!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostKnuthB")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostKnuthB!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostKreutzer1986")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostKreutzer1986!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostTaus88")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostTaus88!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostHellekalek1995")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostHellekalek1995!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT11213b")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT11213b!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT19937")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT19937!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT19937_64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT19937_64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci607")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci607!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci1279")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci1279!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci2281")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci2281!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci3217")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci3217!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci4423")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci4423!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci9689")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci9689!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci19937")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci19937!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci23209")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci23209!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci44497")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci44497!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux3")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux3!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_3")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_3!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux3_01")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux3_01!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux4_01")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux4_01!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_3_01")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_3_01!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_4_01")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_4_01!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux24")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux24!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux48")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux48!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMinstdRand0ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMinstdRand0ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMinstdRandThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMinstdRandThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRand48ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRand48ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostEcuyer1988ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostEcuyer1988ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostKnuthBThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostKnuthBThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostKreutzer1986ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostKreutzer1986ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostTaus88ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostTaus88ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostHellekalek1995ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostHellekalek1995ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT11213bThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT11213bThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT19937ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT19937ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostMT19937_64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostMT19937_64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci607ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci607ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci1279ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci1279ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci2281ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci2281ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci3217ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci3217ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci4423ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci4423ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci9689ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci9689ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci19937ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci19937ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci23209ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci23209ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostLaggedFibonacci44497ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostLaggedFibonacci44497ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux3ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux3ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux4ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux4ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_3ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_3ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_4ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_4ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux3_01ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux3_01ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux4_01ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux4_01ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_3_01ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_3_01ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux64_4_01ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux64_4_01ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux24ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux24ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMBoostRanlux48ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMBoostRanlux48ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32Unique")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32Unique!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K2")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K2!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K2Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K2Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K64Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K64Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K1024")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K1024!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K1024Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K1024Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K16384")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K16384!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K16384Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K16384Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32UniqueThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32UniqueThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K2ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K2ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K2FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K2FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K64FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K64FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K1024ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K1024ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K1024FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K1024FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K16384ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K16384ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPCG32K16384FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPCG32K16384FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift64Star")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift64Star!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift64StarThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift64StarThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORWow")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORWow!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift128Plus")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift128Plus!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoshiro128Plus32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoshiro128Plus32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoshiro128StarStar32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoshiro128StarStar32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoshiro128Plus32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoshiro128Plus32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoshiro128StarStar32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoshiro128StarStar32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro128Plus32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro128Plus32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64Plus32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64Plus32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64Star32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64Star32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64StarStar32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64StarStar32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro128Plus32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro128Plus32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64Plus32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64Plus32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64Star32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64Star32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXoroshiro64StarStar32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXoroshiro64StarStar32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARC4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARC4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARC4ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARC4ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMGjrand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMGjrand!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMGjrandThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMGjrandThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMJSF32n")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMJSF32n!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMJSF32r")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMJSF32r!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMJSF32nThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMJSF32nThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMJSF32rThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMJSF32rThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMSplitMix32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMSplitMix32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMSplitMix32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMSplitMix32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMCG128")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMCG128!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMCG128Fast")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMCG128Fast!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMCG128ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMCG128ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMMCG128FastThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMMCG128FastThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha5")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha5!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha6")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha6!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha8")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha8!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha20")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha20!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha4ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha4ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha5ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha5ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha6ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha6ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha8ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha8ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMChaCha20ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMChaCha20ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMSFC32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMSFC32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMSFC32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMSFC32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL512")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL512!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL521")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL521!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL607")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL607!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL800")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL800!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL1024")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL1024!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL19937")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL19937!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL21701")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL21701!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL23209")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL23209!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL44497")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL44497!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL800_ME")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL800_ME!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL19937_ME")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL19937_ME!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL21701_ME")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL21701_ME!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL23209_ME")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL23209_ME!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL44497_ME")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL44497_ME!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL512ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL512ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL521ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL521ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL607ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL607ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL800ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL800ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL1024ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL1024ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL19937ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL19937ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL21701ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL21701ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL23209ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL23209ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL44497ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL44497ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL800_METhreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL800_METhreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL19937_METhreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL19937_METhreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL21701_METhreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL21701_METhreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL23209_METhreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL23209_METhreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMWELL44497_METhreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMWELL44497_METhreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry2x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry2x32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry4x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry4x32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry2x64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry2x64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry4x64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry4x64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox2x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox2x32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox4x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox4x32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox2x64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox2x64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox4x64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox4x64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMAESNI4x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMAESNI4x32!")
#ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMAESNI1xm128i")
#    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMAESNI1xm128i!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARS4x32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARS4x32!")
#ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARS1xm128i")
#    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARS1xm128i!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry2x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry2x32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry4x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry4x32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry2x64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry2x64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMThreefry4x64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMThreefry4x64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox2x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox2x32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox4x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox4x32ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox2x64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox2x64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMPhilox4x64ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMPhilox4x64ThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMAESNI4x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMAESNI4x32ThreadLocal!")
#ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMAESNI1xm128iThreadLocal")
#    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMAESNI1xm128iThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARS4x32ThreadLocal")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARS4x32ThreadLocal!")
#ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMARS1xm128iThreadLocal")
#    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMARS1xm128iThreadLocal!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFIFOLowContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFIFOLowContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFIFOHighContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFIFOHighContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFILOLowContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFILOLowContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU2")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU2!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU3")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU3!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerSLRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerSLRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerMRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerMRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerTimestampLRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerTimestampLRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerTimestampLRU2")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerTimestampLRU2!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerTimestampLRU3")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerTimestampLRU3!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerTimestampLRU4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerTimestampLRU4!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLFU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLFU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLFUDA")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLFUDA!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRDV1")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRDV1!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRDV2Subtraction")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRDV2Subtraction!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRDV2Multiplication")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRDV2Multiplication!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCLOCKFix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCLOCKFix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCLOCKUnfix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCLOCKUnfix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCLOCKFixUnfix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCLOCKFixUnfix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerGCLOCKV1Fix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerGCLOCKV1Fix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerGCLOCKV2Fix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerGCLOCKV2Fix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerDGCLOCKV1Fix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerDGCLOCKV1Fix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerDGCLOCKV2Fix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerDGCLOCKV2Fix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCARFix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCARFix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerUnfix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerUnfix!")
ELSE(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPAbsolutelyAccurate")
    SET(PAGE_EVICTIONER "PageEvictionerLOOPPracticallyAccurate")
    MESSAGE(WARNING "The set page evictioner is unknown. Changed to the default page evictioner!")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPPracticallyAccurate!")
ENDIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPAbsolutelyAccurate")
