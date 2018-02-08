## = Usage: ============================================================================================================
## FIND_PACKAGE(LibRT)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Find the headers and library of LibRT (as in POSIX.1b Realtime Extensions)
## =====================================================================================================================
## = Further Settings: =================================================================================================
## - SPEC_LibRT_INCLUDE_DIR: The include directory of LibRT if the module has problems finding the proper include path.
## - SPEC_LibRT_LIBRARY_DIR: The directory containing LibRT if the module has problems finding the proper library.
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - LibRT_FOUND:       System has library and headers of LibRT.
## - LibRT_INCLUDE_DIR: The LibRT's include directory.
## - LibRT_LIBRARY:     The LibRT library.
## - LibRT_VERSION:     The version of the installed LibRT library.
## - LibRT::LibRT:      Target that can be used to link the LibRT library using:
##                      TARGET_LINK_LIBRARY(<target> LibRT::LibRT).
## =====================================================================================================================

INCLUDE(FindPackageHandleStandardArgs)

# Find the include directory of LibRT:
FIND_PATH(LibRT_INCLUDE_DIR
          NAMES time.h
          HINTS ${SPEC_LibRT_INCLUDE_DIR}
          PATH_SUFFIXES include
          DOC "The LibRT include directory.")

# Find the LibRT library:
FIND_LIBRARY(LibRT_LIBRARY
             NAMES rt
             HINTS ${SPEC_LibRT_LIBRARY_DIR}
             DOC "The LibRT library.")

# Find the version of LibRT:
IF(LibRT_LIBRARY)
    GET_FILENAME_COMPONENT(LibRT_LIBRARY_REALPATH "${LibRT_LIBRARY}" REALPATH)
    STRING(REGEX REPLACE ".+-([0-9\\.]+).so" "\\1" LibRT_VERSION "${LibRT_LIBRARY_REALPATH}")
    UNSET(LibRT_LIBRARY_REALPATH)
ENDIF(LibRT_LIBRARY)

# Handle the REQUIRED/QUIET/version options of FIND_PACKAGE and set LibRT_FOUND:
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibRT
                                  REQUIRED_VARS LibRT_INCLUDE_DIR LibRT_LIBRARY
                                  VERSION_VAR LibRT_VERSION
                                 )

# Create the target LibRT::LibRT:
IF(LibRT_FOUND AND NOT TARGET LibRT::LibRT)
    ADD_LIBRARY(LibRT::LibRT UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(LibRT::LibRT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibRT_INCLUDE_DIR}")
    SET_PROPERTY(TARGET LibRT::LibRT APPEND PROPERTY IMPORTED_LOCATION "${LibRT_LIBRARY}")
ENDIF(LibRT_FOUND AND NOT TARGET LibRT::LibRT)

MARK_AS_ADVANCED(LibRT_INCLUDE_DIR
                 LibRT_LIBRARY
                 LibRT_VERSION
                )
