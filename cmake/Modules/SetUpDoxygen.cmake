## = Usage: ============================================================================================================
## INCLUDE(SetUpDoxygen)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Adds the target doxygen used to generate API documentation in the build directory's subdirectory documentation.
## =====================================================================================================================

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/zero.dox DOXY_FILE_LOCAL)     # Configuration of doxygen is done manually
    STRING(REGEX REPLACE "#[^\n]*" "" DOXY_FILE "${DOXY_FILE_LOCAL}")   # (instead of CONFIGURE_FILE) to allow
    STRING(REGEX REPLACE "\n([ \t]*\n)+" "\n" DOXY_FILE "${DOXY_FILE}") # CodeDocs[xyz] the usage of the same zero.dox.
    STRING(REGEX REPLACE "\n([ \t]*PROJECT_NAME[ \t]*=[ \t]*)[^\n]*"
                         "\n\\1\"${PROJECT_NAME}\"" DOXY_FILE "${DOXY_FILE}")  # Set PROJECT_NAME
    STRING(REGEX REPLACE "\n([ \t]*PROJECT_NUMBER[ \t]*=[ \t]*)[^\n]*"
                         "\n\\1\"${ZERO_VERSION}\"" DOXY_FILE "${DOXY_FILE}")  # Set PROJECT_NUMBER
    STRING(REGEX REPLACE "\n([ \t]*OUTPUT_DIRECTORY[ \t]*=[ \t]*)[^\n]*"
                         "\n\\1${CMAKE_CURRENT_BINARY_DIR}/documentation" DOXY_FILE "${DOXY_FILE}")  # Set OUTPUT_DIRECTORY
    STRING(REGEX REPLACE "\n([ \t]*INPUT[ \t]*=[ \t]*)[^\n]*"
                         "\n\\1${CMAKE_SOURCE_DIR}/src" DOXY_FILE "${DOXY_FILE}")  # Set INPUT
    STRING(REGEX REPLACE "\n([ \t]*EXCLUDE[ \t]*=[ \t]*)[^\n]*"
                         "\n\\1${CMAKE_SOURCE_DIR}/src/third_party" DOXY_FILE "${DOXY_FILE}")  # Set EXCLUDE
    STRING(REGEX REPLACE "^[ \t]*\n" "" DOXY_FILE "${DOXY_FILE}")
    FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/doxyguration "${DOXY_FILE}")
    ADD_CUSTOM_TARGET(doxygen
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyguration
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
            )
ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS "INFO: How to install doxygen: http://www.stack.nl/~dimitri/doxygen/manual/install.html")
ENDIF(DOXYGEN_FOUND)
