INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/cmd/base
  ${CMAKE_SOURCE_DIR}/src/cmd/restore
  ${CMAKE_SOURCE_DIR}/src/cmd/kits
  ${CMAKE_SOURCE_DIR}/src/cmd/webserver
  ${CMAKE_SOURCE_DIR}/src/cmd/loginspect
  ${CMAKE_SOURCE_DIR}/config
  ${CMAKE_SOURCE_DIR}/src/sm
  ${CMAKE_SOURCE_DIR}/src/common
)
# to look for generated headers
INCLUDE_DIRECTORIES (
  ${CMAKE_BINARY_DIR}/config
  ${CMAKE_BINARY_DIR}/config
  ${CMAKE_BINARY_DIR}/src/sm
  ${CMAKE_BINARY_DIR}/src/common
)

add_subdirectory(base)
add_subdirectory(restore)
add_subdirectory(kits)
add_subdirectory(webserver)
add_subdirectory(loginspect)

add_executable(zapps main.cpp ${BACKWARD_ENABLE})

set(zapps_LIBS
    # zapps components
    zapps_base
    kits
    restore
    loginspect
    # Zero/Shore libraries
    common
    sm
    # third-party dependencies
    pthread
)

target_link_libraries(zapps ${zapps_LIBS})

# This will add libraries, definitions and include directories needed by backward
# by setting each property on the target.
add_backward(zapps)
