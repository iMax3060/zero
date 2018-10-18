## = Usage: ============================================================================================================
## INCLUDE(SetUpConfigurations)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Supported Build Types: ============================================================================================
## - Debug: See Debug1
## - Debug1: Enables debug symbols, low optimization and sets W_DEBUG_LEVEL=1
## - Debug3: Enables debug symbols, disables optimization and sets W_DEBUG_LEVEL=3
## - Debug5: Enables debug symbols, disables optimization and sets W_DEBUG_LEVEL=5
## - Release: See RelWithDebInfo
## - RelWithDebInfo: Enables debug symbols, high optimization and sets W_DEBUG_LEVEL=0
## - Profile: Enables debug symbols, high optimization, sets W_DEBUG_LEVEL=0 and uses position-independent code
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - NOWARN: Use default warning level instead of a custom extended one.
## - PEDANTIC: Warn about non-C++-Standard-conform code.
## - COMPILE_GENERIC: Create generic instead of CPU-specific code.
## - USE_MMAP: Use mmap to read from log and log archive.
## - LINK_TIME_OPTIMIZATION: Use link-time optimization between compilation units.
## =====================================================================================================================
## = Preconditions: ====================================================================================================
## - CMAKE_CXX_COMPILER_ID either matches Clang (->Clang), GNU (->GCC), Intel (->IPCP) or MSVC (->Microsoft Visual
##   Studio). Different compilers compile with wrong flags.
## - CMAKE_BUILD_TYPE probably set to either Debug, Debug1, Debug3, Debug5, RelWithDebInfo or Profile.
## - CMAKE_CONFIGURATION_TYPES probably set to something.
## - NOWARN (boolean) probably set.
## - PEDANTIC (boolean) probably set.
## - COMPILE_GENERIC (boolean) probably set.
## - USE_MMAP (boolean) probably set.
## - LINK_TIME_OPTIMIZATION (boolean) probably set.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - If CMAKE_BUILD_TYPE wasn't set, it is set to RelWithDebInfo.
## - If CMAKE_BUILD_TYPE was set to Release, it is set to RelWithDebInfo.
## - If CMAKE_BUILD_TYPE was set to Debug, it is set to Debug1 as those are equivalent.
## - If CMAKE_BUILD_TYPE was set to something else, the processing aborts.
## - If CMAKE_CONFIGURATION_TYPES was set to something, it is set to Debug1;Debug3;Debug5;RelWithDebInfo;Profile.
## - CMAKE_CXX_FLAGS_RELWITHDEBINFO are set according to the compiler.
## - CMAKE_CXX_FLAGS_DEBUG1 are set according to the compiler.
## - CMAKE_CXX_FLAGS_DEBUG3 are set according to the compiler.
## - CMAKE_CXX_FLAGS_DEBUG5 are set according to the compiler.
## - CMAKE_CXX_FLAGS_PROFILE are set.
## - COMPILE_DEFINITIONS are are set according to the build type and USE_MMAP.
## - W_WARNINGS is set according to the compiler if NOWARN isn't set.
## - W_PEDANTIC is set according to the compiler if PEDANTIC is set.
## - ALWAYS_FLAGS are set according to the compiler.
## - MARGE_NATIVE_FLAGS are set according to the compiler if COMPILE_GENERIC isn't set.
## - INTERPROCEDURAL_OPTIMIZATION is used for all targets if LINK_TIME_OPTIMIZATION isn't set to OFF.
## =====================================================================================================================
IF(NOT SET_UP_CONFIGURATIONS_DONE)
    SET(SET_UP_CONFIGURATIONS_DONE 1)

    # No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
    # Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
    IF(CMAKE_CONFIGURATION_TYPES) # multiconfig generator?
        SET(CMAKE_CONFIGURATION_TYPES "Debug1;Debug3;Debug5;RelWithDebInfo;Profile" CACHE STRING "" FORCE)
    ELSE(CMAKE_CONFIGURATION_TYPES)
        IF(NOT CMAKE_BUILD_TYPE)
            MESSAGE(STATUS "Defaulting to release build with debug symbols.")
            SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
            MESSAGE(STATUS "Using debug1 build as debug is a synonym for that.")
            SET(CMAKE_BUILD_TYPE Debug1 CACHE STRING "" FORCE)
        ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
            MESSAGE(STATUS "Using release build with debug symbols instead of ${CMAKE_BUILD_TYPE}.")
            SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
        ENDIF(NOT CMAKE_BUILD_TYPE)
        IF(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug1" OR CMAKE_BUILD_TYPE STREQUAL "Debug3" OR CMAKE_BUILD_TYPE STREQUAL "Debug5" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "Profile"))
            MESSAGE(FATAL_ERROR "The selected build type ${CMAKE_BUILD_TYPE} is unknown! The following build types are valid: Debug1, Debug3, Debug5, RelWithDebInfo, Profile")
        ENDIF(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug1" OR CMAKE_BUILD_TYPE STREQUAL "Debug3" OR CMAKE_BUILD_TYPE STREQUAL "Debug5" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "Profile"))
        SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
        # set the valid options for cmake-gui drop-down list
        SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug1;Debug3;Debug5;RelWithDebInfo;Profile")
    ENDIF(CMAKE_CONFIGURATION_TYPES)

    MESSAGE(STATUS "Selected Build Type: ${CMAKE_BUILD_TYPE}")
    # now set up the Profile configuration

    # The debug builds are all based on the default debug build:
    SET(CMAKE_CXX_FLAGS_DEBUG1  "${CMAKE_CXX_FLAGS_DEBUG1} ${CMAKE_CXX_FLAGS_DEBUG}")
    SET(CMAKE_CXX_FLAGS_DEBUG3  "${CMAKE_CXX_FLAGS_DEBUG3} ${CMAKE_CXX_FLAGS_DEBUG}")
    SET(CMAKE_CXX_FLAGS_DEBUG5  "${CMAKE_CXX_FLAGS_DEBUG5} ${CMAKE_CXX_FLAGS_DEBUG}")

    # The release build with debug info is defined with higher optimization level (if it's one of the known compilers), else the default release build with debug info is used.
    # Also increses the optimization of the debug1 build for some compilers:
    IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -finline-functions")
        IF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-Og" CMAKE_CXX_FLAGS_DEBUG1 "${CMAKE_CXX_FLAGS_DEBUG1}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG1 "${CMAKE_CXX_FLAGS_DEBUG1} -Og")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
        SET(W_WARNINGS ${W_WARNINGS} -Wall -Wextra -Wpointer-arith)
        SET(W_PEDANTIC ${W_PEDANTIC} -Wpedantic -Wno-c++11-long-long)
        SET(ALWAYS_FLAGS ${ALWAYS_FLAGS} -stdlib=libc++ -fno-strict-aliasing)
        SET(MARGE_NATIVE_FLAGS ${MARGE_NATIVE_FLAGS} -march=native)
    ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -fexpensive-optimizations -finline-functions")
        IF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-Og" CMAKE_CXX_FLAGS_DEBUG1 "${CMAKE_CXX_FLAGS_DEBUG1}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG1 "${CMAKE_CXX_FLAGS_DEBUG1} -Og")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
        SET(W_WARNINGS ${W_WARNINGS} -Wall -Wextra -Woverloaded-virtual -Wpointer-arith -Wwrite-strings)
        SET(W_PEDANTIC ${W_PEDANTIC} -Wpedantic -Wno-long-long)
        SET(ALWAYS_FLAGS ${ALWAYS_FLAGS} -fno-strict-aliasing)
        SET(MARGE_NATIVE_FLAGS ${MARGE_NATIVE_FLAGS} -march=native)
    ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -O3 -inline-level=2")
        IF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O1" CMAKE_CXX_FLAGS_DEBUG1 "${CMAKE_CXX_FLAGS_DEBUG1}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG1  "${CMAKE_CXX_FLAGS_DEBUG1} -O1")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG1 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG3 "${CMAKE_CXX_FLAGS_DEBUG3} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG3 MATCHES "-O[a-z0-9]")
        IF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            STRING(REGEX REPLACE "-O[a-z0-9]" "-O0" CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5}")
        ELSE(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
            SET(CMAKE_CXX_FLAGS_DEBUG5 "${CMAKE_CXX_FLAGS_DEBUG5} -O0")
        ENDIF(CMAKE_CXX_FLAGS_DEBUG5 MATCHES "-O[a-z0-9]")
        SET(W_WARNINGS ${W_WARNINGS} -w3)
        SET(W_PEDANTIC ${W_PEDANTIC} -pedantic -Wno-long-long)
        SET(ALWAYS_FLAGS ${ALWAYS_FLAGS} -no-ansi-alias)
        SET(MARGE_NATIVE_FLAGS ${MARGE_NATIVE_FLAGS} -xHost)
    ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/DEBUG:FULL /O2")
        SET(W_WARNINGS ${W_WARNINGS} /W4)
        SET(W_PEDANTIC ${W_PEDANTIC} /permissive-)
    ELSE("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        MESSAGE(AUTHOR_WARNING "The C++-Compiler is neither Clang, GCC, ICPC nor MSVC so some compiler-specific flags cannot be set.")
    ENDIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")


    OPTION(NOWARN "Use the default warning level of the compiler." OFF)
    IF(NOWARN)
        UNSET(W_WARNINGS)
        MESSAGE(STATUS "The default warning level is used as -DNOWARN=ON was specified.")
    ELSE(NOWARN)
        MESSAGE(STATUS "The increased warning level is used as -DNOWARN=ON wasn't specified.")
    ENDIF(NOWARN)

    OPTION(PEDANTIC "Non C++-Standard-conform code doesn't produces warnings." OFF)
    IF(NOT PEDANTIC)
        UNSET(W_PEDANTIC)
        MESSAGE(STATUS "Non C++-Standard-conform code doesn't produces warnings as -DPEDANTIC=ON wasn't specified.")
    ELSE(NOT PEDANTIC)
        MESSAGE(STATUS "Non C++-Standard-conform code produces warnings as -DPEDANTIC=ON was specified.")
    ENDIF(NOT PEDANTIC)

    OPTION(COMPILE_GENERIC "Generic instead of CPU-specific binaries are produced." OFF)
    IF(COMPILE_GENERIC)
        UNSET(MARGE_NATIVE_FLAGS)
        MESSAGE(STATUS "Generic binaries are produced as -DCOMPILE_GENERIC=ON was specified.")
    ELSE(COMPILE_GENERIC)
        MESSAGE(STATUS "CPU-specific binaries are produced as -DCOMPILE_GENERIC=ON wasn't specified.")
    ENDIF(COMPILE_GENERIC)

    OPTION(LINK_TIME_OPTIMIZATION "Use link-time optimization between compilation units." ON)
    IF(LINK_TIME_OPTIMIZATION)
        MESSAGE(STATUS "Link-time optimization is used as -DLINK_TIME_OPTIMIZATION=OFF wasn't specified.")
    ELSE(LINK_TIME_OPTIMIZATION)
        MESSAGE(STATUS "Link-time optimization is disabled as -DLINK_TIME_OPTIMIZATION=OFF wasn't specified. This may degrade the performance of the program.")
    ENDIF(LINK_TIME_OPTIMIZATION)

    # The profile build is intended to be used for performance analysis:
    ## Based on the release build with debug info:
    SET(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    ## Activate position independent code (-fPIC in GCC):
    IF(CMAKE_BUILD_TYPE STREQUAL "Profile")
        SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Profile")

    ADD_COMPILE_OPTIONS(${W_WARNINGS} ${W_PEDANTIC} ${ALWAYS_FLAGS} ${MARGE_NATIVE_FLAGS})

    # Define the Macros NDEBUG (deactivate C/C++-asserts), DEBUG and W_DEBUG_LEVEL (higher level -> more check logics and asserts):
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:RelWithDebInfo>:NDEBUG W_DEBUG_LEVEL=0>)
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug1>:         DEBUG W_DEBUG_LEVEL=1>)
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug3>:         DEBUG W_DEBUG_LEVEL=3>)
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug5>:         DEBUG W_DEBUG_LEVEL=5>)
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Profile>:       NDEBUG W_DEBUG_LEVEL=0>)

    OPTION(USE_MMAP "Use mmap to read from log and log archive" ON)
    IF(USE_MMAP)
        SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS USE_MMAP)
        MESSAGE(STATUS "mmap is used to read from log and log archive as -DUSE_MMAP=OFF wasn't specified.")
    ELSE(USE_MMAP)
        MESSAGE(STATUS "read() instead of mmap used to read from the log and log archive as -DUSE_MMAP=OFF was specified.")
    ENDIF(USE_MMAP)
ENDIF(NOT SET_UP_CONFIGURATIONS_DONE)
