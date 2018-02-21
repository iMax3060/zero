# Add config, other src-subdirectories cmd and other cmd-subdirectories to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/src/cmd/base
        ${CMAKE_SOURCE_DIR}/src/cmd/restore
        ${CMAKE_SOURCE_DIR}/src/cmd/kits
        ${CMAKE_SOURCE_DIR}/src/cmd/webserver
        ${CMAKE_SOURCE_DIR}/src/cmd/loginspect
        ${CMAKE_SOURCE_DIR}/src/cmd
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/src/sm
        ${CMAKE_SOURCE_DIR}/src/common
)

# Add generated headers from config and other src-subdirectories to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_BINARY_DIR}/config
        ${CMAKE_BINARY_DIR}/src/sm
        ${CMAKE_BINARY_DIR}/src/common
)

# Define the source files of the webserverLib library:
SET(webserver_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/http.cpp
   )

# Add the library of this directory webserverLib:
ADD_LIBRARY(webserverLib ${webserver_SRCS})

SET_PROPERTY(TARGET webserverLib PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)

# Add the webserver executable:
ADD_EXECUTABLE(webserver main.cpp)

SET_PROPERTY(TARGET webserver PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)

# Set the shared libraries used by the executable webserver:
SET(webserver_LIBS
    # zapps components:
    webserverLib
    kits
    zapps_base
    restore
    loginspect
    # Zero/Shore libraries:
    sm
    common
    # Third-party dependencies:
    Threads::Threads
    backward
)

TARGET_LINK_LIBRARIES(webserver ${webserver_LIBS})
