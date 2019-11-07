## = Usage: ============================================================================================================
## INCLUDE(SetUpPageEvictioner)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Supported Page Evictioners: =======================================================================================
## - PageEvictionerLOOPAbsolutelyAccurate
## - PageEvictionerLOOPPracticallyAccurate
## - PageEvictionerLOOPThreadLocallyAccurate
## - PageEvictionerCLOCKFix
## - PageEvictionerCLOCKUnfix
## - PageEvictionerCLOCKFixUnfix
## - PageEvictionerGCLOCKV1Fix
## - PageEvictionerGCLOCKV2Fix
## - PageEvictionerDGCLOCKV1Fix
## - PageEvictionerDGCLOCKV2Fix
## - PageEvictionerFIFOLowContention
## - PageEvictionerFIFOHighContention
## - PageEvictionerFILOLowContention
## - PageEvictionerLRU
## - PageEvictionerSLRU
## - PageEvictionerLRU2
## - PageEvictionerLRU3
## - PageEvictionerLRU4
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
## - PageEvictionerRANDOMFastRand
## - PageEvictionerRANDOMMinstdRand0
## - PageEvictionerRANDOMMinstdRand
## - PageEvictionerRANDOMMT19937
## - PageEvictionerRANDOMMT19937_64
## - PageEvictionerRANDOMRanlux24Base
## - PageEvictionerRANDOMRanlux48Base
## - PageEvictionerRANDOMRanlux24
## - PageEvictionerRANDOMRanlux48
## - PageEvictionerRANDOMKnuthB
## - PageEvictionerRANDOMCRand
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
## - PageEvictionerRANDOMXORShift32
## - PageEvictionerRANDOMXORShift64
## - PageEvictionerRANDOMXORShift96
## - PageEvictionerRANDOMXORShift128
## - PageEvictionerRANDOMXORWow
## - PageEvictionerRANDOMXORShift64Star
## - PageEvictionerRANDOMXORShift128Plus
## - PageEvictionerCARFix
## - PageEvictionerCARUnfix
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
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFIFOLowContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFIFOLowContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFIFOHighContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFIFOHighContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerFILOLowContention")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerFILOLowContention!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerSLRU")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerSLRU!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU2")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU2!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU3")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU3!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLRU4")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLRU4!")
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
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMFastRand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMFastRand!")
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
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMCRand")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMCRand!")
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
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift32")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift32!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift64")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift64!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift96")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift96!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift128")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift128!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORWow")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORWow!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift64Star")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift64Star!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerRANDOMXORShift128Plus")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerRANDOMXORShift128Plus!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCARFix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCARFix!")
ELSEIF(PAGE_EVICTIONER STREQUAL "PageEvictionerCARUnfix")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerCARUnfix!")
ELSE(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPAbsolutelyAccurate")
    SET(PAGE_EVICTIONER "PageEvictionerLOOPPracticallyAccurate")
    MESSAGE(WARNING "The set page evictioner is unknown. Changed to the default page evictioner!")
    MESSAGE(STATUS "INFO: The selected page evictioner is PageEvictionerLOOPPracticallyAccurate!")
ENDIF(PAGE_EVICTIONER STREQUAL "PageEvictionerLOOPAbsolutelyAccurate")
