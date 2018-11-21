MACRO(INSTALL_GENERATED_H)
    SET(ig_tmp)
    FOREACH(tmp ${ARGV})
        LIST(APPEND ig_tmp ${CMAKE_CURRENT_BINARY_DIR}/${tmp})
    ENDFOREACH(tmp)
    INSTALL(FILES
            ${ig_tmp}
            DESTINATION include
           )
ENDMACRO(INSTALL_GENERATED_H)

# Add config and src-subdirectories to the include directories:
INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/sm
        ${CMAKE_SOURCE_DIR}/src/cmd
        ${CMAKE_SOURCE_DIR}/src/third_party
        ${CMAKE_SOURCE_DIR}/src/third_party/junction
        ${CMAKE_SOURCE_DIR}/src/third_party/turf
)

# Add generated headers from config and src-subdirectories to the include directories:
INCLUDE_DIRECTORIES (
        ${CMAKE_BINARY_DIR}/config
        ${CMAKE_BINARY_DIR}/src/common
        ${CMAKE_BINARY_DIR}/src/sm
        ${CMAKE_BINARY_DIR}/src/cmd
        ${CMAKE_BINARY_DIR}/src/third_party
        ${CMAKE_BINARY_DIR}/src/third_party/junction/include
        ${CMAKE_BINARY_DIR}/src/third_party/junction/turf/include
)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(sm)
ADD_SUBDIRECTORY(cmd)
ADD_SUBDIRECTORY(third_party)
