## = Usage: ============================================================================================================
## FIND_PACKAGE(LibCDS)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Find the headers and library of LibCDS (Concurrent Data Structures)
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - SPEC_LibCDS_INCLUDE_DIR: The include directory of LibCDS if the module has problems finding the proper include path.
## - SPEC_LibCDS_LIBRARY_DIR: The directory containing LibCDS if the module has problems finding the proper library.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - LibCDS_FOUND:       System has library and headers of LibCDS.
## - LibCDS_INCLUDE_DIR: The LibCDS's include directory.
## - LibCDS_LIBRARY:     The LibCDS library.
## - LibCDS_VERSION:     The version of the installed LibCDS library.
## - LibCDS::LibCDS:     Target that can be used to link the LibCDS library using:
##                       TARGET_LINK_LIBRARY(<target> LibCDS::LibCDS).
## =====================================================================================================================

INCLUDE(FindPackageHandleStandardArgs)

# Find the include directory of LibCDS:
FIND_PATH(LibCDS_INCLUDE_DIR
          NAMES container/basket_queue.h
                container/fcqueue.h
                container/msqueue.h
                container/optimistic_queue.h
                container/rwqueue.h
                container/segmented_queue.h
                container/vyukov_mpmc_cycle_queue.h
          HINTS ${SPEC_LibCDS_INCLUDE_DIR}
          PATH_SUFFIXES include/cds
          DOC "The LibCDS include directory."
         )

# Find the LibCDS library:
FIND_LIBRARY(LibCDS_LIBRARY
             NAMES cds_d
             HINTS ${SPEC_LibCDS_LIBRARY_DIR}
             DOC "The LibCDS library."
            )

# Find the version of LibCDS:
IF(LibCDS_INCLUDE_DIR)
    FILE(READ "${LibCDS_INCLUDE_DIR}/version.h" LibCDS_H_FILE)
    STRING(REGEX REPLACE ".*#define[ \t]+CDS_VERSION_STRING[ \t]+\"([0-9\\.]+)\".*" "\\1" LibCDS_VERSION "${LibCDS_H_FILE}")
    UNSET(LibCDS_H_FILE)
ENDIF(LibCDS_INCLUDE_DIR)

# Handle the REQUIRED/QUIET/version options of FIND_PACKAGE and set LibCDS_FOUND:
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibCDS
                                  REQUIRED_VARS LibCDS_INCLUDE_DIR LibCDS_LIBRARY
                                  VERSION_VAR LibCDS_VERSION
        )

# Create the target LibCDS::LibCDS:
IF(LibCDS_FOUND AND NOT TARGET LibCDS::LibCDS)
    ADD_LIBRARY(LibCDS::LibCDS UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(LibCDS::LibCDS PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibCDS_INCLUDE_DIR}")
    SET_PROPERTY(TARGET LibCDS::LibCDS APPEND PROPERTY IMPORTED_LOCATION "${LibCDS_LIBRARY}")
ENDIF(LibCDS_FOUND AND NOT TARGET LibCDS::LibCDS)

MARK_AS_ADVANCED(LibCDS_INCLUDE_DIR
                 LibCDS_LIBRARY
                 LibCDS_VERSION
                )
