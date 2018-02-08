## = Usage: ============================================================================================================
## FIND_PACKAGE(TCMALLOC)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Find the headers and library of TCMALLOC.
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - SPEC_TCMALLOC_INCLUDE_DIR: The include directory of TCMALLOC if the module has problems finding the proper include
##                              path.
## - SPEC_TCMALLOC_LIBRARY_DIR: The directory containing TCMALLOC's library if the module has problems finding the
##                              proper library.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - TCMALLOC_FOUND:       System has library and headers of TCMALLOC.
## - TCMALLOC_INCLUDE_DIR: TCMALLOC's include directory.
## - TCMALLOC_LIBRARY:     The TCMALLOC library.
## - TCMALLOC_VERSION:     The version of the installed TCMALLOC.
## - TCMALLOC::TCMALLOC:   Target that can be used to link the TCMALLOC library using:
##                         TARGET_LINK_LIBRARY(<target> TCMALLOC::TCMALLOC).
## =====================================================================================================================

INCLUDE(FindPackageHandleStandardArgs)

# Find the include directory of TCMALLOC:
FIND_PATH(TCMALLOC_INCLUDE_DIR
          NAMES tcmalloc.h
          HINTS ${SPEC_TCMALLOC_INCLUDE_DIR}
          PATH_SUFFIXES include/gperftools include/google
          DOC "The TCMALLOC include directory."
         )

# Find TCMALLOC's library:
FIND_LIBRARY(TCMALLOC_LIBRARY
             NAMES tcmalloc
             HINTS ${SPEC_TCMALLOC_LIBRARY_DIR}
             DOC "The TCMALLOC library."
            )

# Find the version of TCMALLOC:
IF(TCMALLOC_INCLUDE_DIR)
    FILE(READ "${TCMALLOC_INCLUDE_DIR}/tcmalloc.h" TCMALLOC_H_FILE)
    STRING(REGEX REPLACE ".*#define[ \t]+TC_VERSION_STRING[ \t]+\"gperftools ([0-9\\.]+)\".*" "\\1" TCMALLOC_VERSION "${TCMALLOC_H_FILE}")
    UNSET(TCMALLOC_H_FILE)
ENDIF(TCMALLOC_INCLUDE_DIR)

# Handle the REQUIRED/QUIET/version options of FIND_PACKAGE and set TCMALLOC_FOUND:
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCMALLOC
                                  REQUIRED_VARS TCMALLOC_INCLUDE_DIR TCMALLOC_LIBRARY
                                  VERSION_VAR TCMALLOC_VERSION
                                 )

# Create the target TCMALLOC::TCMALLOC:
IF(TCMALLOC_FOUND AND NOT TARGET TCMALLOC::TCMALLOC)
    ADD_LIBRARY(TCMALLOC::TCMALLOC UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(TCMALLOC::TCMALLOC PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TCMALLOC_INCLUDE_DIR}")
    SET_PROPERTY(TARGET TCMALLOC::TCMALLOC APPEND PROPERTY IMPORTED_LOCATION "${TCMALLOC_LIBRARY}")
ENDIF(TCMALLOC_FOUND AND NOT TARGET TCMALLOC::TCMALLOC)

MARK_AS_ADVANCED(TCMALLOC_INCLUDE_DIR
                 TCMALLOC_LIBRARY
                 TCMALLOC_VERSION
                )
