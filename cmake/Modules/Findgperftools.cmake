## = Usage: ============================================================================================================
## FIND_PACKAGE(gperftools)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Find the headers and library of gperftools.
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - SPEC_gperftools_INCLUDE_DIR: The include directory of the gperftools if the module has problems finding the proper
##                                include path.
## - SPEC_gperftools_LIBRARY_DIR: The directory containing the gperftools's library if the module has problems finding
##                                the proper library.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - gperftools_FOUND:       System has library and headers of gperftools.
## - gperftools_INCLUDE_DIR: The gperftools' include directory.
## - gperftools_LIBRARY:     The gperftools library.
## - gperftools_VERSION:     The version of the installed gperftools.
## - gperftools::gperftools: Target that can be used to link the gperftools library using:
##                           TARGET_LINK_LIBRARY(<target> gperftools::gperftools).
## =====================================================================================================================

INCLUDE(FindPackageHandleStandardArgs)

# Find the include directory of the gperftools:
FIND_PATH(gperftools_INCLUDE_DIR
          NAMES heap-checker.h
          HINTS ${SPEC_gperftools_INCLUDE_DIR}
          PATH_SUFFIXES include/gperftools include/google
          DOC "The gperftools include directory."
         )

# Find the gperftools' library:
FIND_LIBRARY(gperftools_LIBRARY
             NAMES profiler
             HINTS ${SPEC_gperftools_LIBRARY_DIR}
             DOC "The gperftools library."
            )

# Find the version of gperftools:
FIND_PATH(gperftools_TCMALLOC_INCLUDE_DIR
          NAMES tcmalloc.h
          PATHS "${gperftools_INCLUDE_DIR}"
          NO_DEFAULT_PATH
         )
IF(gperftools_TCMALLOC_INCLUDE_DIR)
    FILE(READ "${gperftools_TCMALLOC_INCLUDE_DIR}/tcmalloc.h" TCMALLOC_H_FILE)
    STRING(REGEX REPLACE ".*#define[ \t]+TC_VERSION_STRING[ \t]+\"gperftools ([0-9\\.]+)\".*" "\\1" gperftools_VERSION "${TCMALLOC_H_FILE}")
    UNSET(TCMALLOC_H_FILE)
ENDIF(gperftools_TCMALLOC_INCLUDE_DIR)
UNSET(gperftools_TCMALLOC_INCLUDE_DIR)

# Handle the REQUIRED/QUIET/version options of FIND_PACKAGE and set gperftools_FOUND:
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gperftools
                                  REQUIRED_VARS gperftools_INCLUDE_DIR gperftools_LIBRARY
                                  VERSION_VAR gperftools_VERSION
                                 )

# Create the target gperftools::gperftools:
IF(gperftools_FOUND AND NOT TARGET gperftools::gperftools)
    ADD_LIBRARY(gperftools::gperftools UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(gperftools::gperftools PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${gperftools_INCLUDE_DIR}")
    SET_PROPERTY(TARGET gperftools::gperftools APPEND PROPERTY IMPORTED_LOCATION "${gperftools_LIBRARY}")
ENDIF(gperftools_FOUND AND NOT TARGET gperftools::gperftools)

MARK_AS_ADVANCED(gperftools_INCLUDE_DIR
                 gperftools_LIBRARY
                 gperftools_VERSION
                )
