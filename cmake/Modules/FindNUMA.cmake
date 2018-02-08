## = Usage: ============================================================================================================
## FIND_PACKAGE(NUMA)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Find the headers and library of the NUMA policy library.
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - SPEC_NUMA_INCLUDE_DIR: The include directory of the NUMA policy library if the module has problems finding the
##                          proper include path.
## - SPEC_NUMA_LIBRARY_DIR: The directory containing the NUMA policy library if the module has problems finding the
##                          proper library.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - NUMA_FOUND:       System has library and headers of the NUMA policy library.
## - NUMA_INCLUDE_DIR: The NUMA policy library's include directory.
## - NUMA_LIBRARY:     The NUMA policy library.
## - NUMA_VERSION_API: The API version of the installed NUMA policy library.
## - NUMA::NUMA:       Target that can be used to link the NUMA policy library using:
##                     TARGET_LINK_LIBRARY(<target> NUMA::NUMA).
## =====================================================================================================================

INCLUDE(FindPackageHandleStandardArgs)

# Find the include directory of the NUMA policy library:
FIND_PATH(NUMA_INCLUDE_DIR
          NAMES numa.h
          HINTS ${SPEC_NUMA_INCLUDE_DIR}
          PATH_SUFFIXES include
          DOC "The NUMA include directory.")

# Find the API version of the NUMA policy library:
IF(NUMA_INCLUDE_DIR)
    FILE(READ "${NUMA_INCLUDE_DIR}/numa.h" NUMA_H_FILE)
    STRING(REGEX REPLACE ".*#define[ \t]+LIBNUMA_API_VERSION[ \t]([0-9\\.]+)\n.*" "\\1" NUMA_VERSION_API "${NUMA_H_FILE}")
    UNSET(NUMA_H_FILE)
ENDIF(NUMA_INCLUDE_DIR)

# Find the NUMA policy library:
FIND_LIBRARY(NUMA_LIBRARY
             NAMES numa
             HINTS ${SPEC_NUMA_LIBRARY_DIR}
             DOC "The NUMA policy library.")

# Handle the REQUIRED/QUIET/version options of FIND_PACKAGE and set NUMA_FOUND:
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NUMA
                                  REQUIRED_VARS NUMA_INCLUDE_DIR NUMA_LIBRARY
                                  VERSION_VAR NUMA_VERSION_API
                                 )

# Create the target NUMA::NUMA:
IF(NUMA_FOUND AND NOT TARGET NUMA::NUMA)
    ADD_LIBRARY(NUMA::NUMA UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(NUMA::NUMA PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NUMA_INCLUDE_DIR}")
    SET_PROPERTY(TARGET NUMA::NUMA APPEND PROPERTY IMPORTED_LOCATION "${NUMA_LIBRARY}")
ENDIF(NUMA_FOUND AND NOT TARGET NUMA::NUMA)

MARK_AS_ADVANCED(NUMA_INCLUDE_DIR
                 NUMA_LIBRARY
                 NUMA_VERSION_API
                )
