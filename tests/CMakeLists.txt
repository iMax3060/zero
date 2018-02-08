SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/tests")

# Add config and src to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/sm
        ${CMAKE_SOURCE_DIR}/src/cmd
        ${CMAKE_SOURCE_DIR}/src/third_party
) 

# Add generated headers from config and src to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_BINARY_DIR}/config
        ${CMAKE_BINARY_DIR}/src/common
        ${CMAKE_BINARY_DIR}/src/sm
        ${CMAKE_BINARY_DIR}/src/cmd
)

# Add headers of Google Test to the include directories:
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/third_party/googletest/googletest/include)

# Generate sections for each function and each data item in the object file:
IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-fdata-sections -ffunction-sections")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    # strip out unused data/function. this also removes -rdynamic
    SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-fdata-sections -ffunction-sections")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-fdata-sections -ffunction-sections")
ENDIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

IF("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-fdata-sections -ffunction-sections")
ELSEIF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    # strip out unused data/function. this also removes -rdynamic
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-fdata-sections -ffunction-sections")
ELSEIF("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-fdata-sections -ffunction-sections")
ENDIF("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(sm)
