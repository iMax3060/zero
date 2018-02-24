# Add config, other src-subdirectories and cmd-subdirectories to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/src/cmd/base
        ${CMAKE_SOURCE_DIR}/src/cmd/restore
        ${CMAKE_SOURCE_DIR}/src/cmd/kits
        ${CMAKE_SOURCE_DIR}/src/cmd/webserver
        ${CMAKE_SOURCE_DIR}/src/cmd/loginspect
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

ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(restore)
ADD_SUBDIRECTORY(kits)
ADD_SUBDIRECTORY(webserver)
ADD_SUBDIRECTORY(loginspect)

# Add the zapps executable:
ADD_EXECUTABLE(zapps main.cpp)

# Set the shared libraries used by the executable zapps:
SET(zapps_LIBS
    # zapps components:
    zapps_base
    kits
    restore
    loginspect
    # Zero/Shore libraries:
    sm
    common
    # Third-party dependencies:
    Threads::Threads
    backward
   )

IF(LINK_TIME_OPTIMIZATION)
    SET_PROPERTY(TARGET zapps PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
ENDIF(LINK_TIME_OPTIMIZATION)

TARGET_LINK_LIBRARIES(zapps ${zapps_LIBS})
