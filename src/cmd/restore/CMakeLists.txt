# Define the source files of the restore library:
SET(restore_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/genarchive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mergeruns.cpp
    )

# Add the library of this directory restore:
ADD_LIBRARY(restore ${restore_SRCS})

SET_PROPERTY(TARGET restore PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
