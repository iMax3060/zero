# Define the source files of the zapps_base libarary:
SET(zapps_base_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/basethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/command.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scanner.cpp
   )

# Add the library of this directory zapps_base:
ADD_LIBRARY(zapps_base ${zapps_base_SRCS})

IF(LINK_TIME_OPTIMIZATION)
    SET_PROPERTY(TARGET zapps_base PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
ENDIF(LINK_TIME_OPTIMIZATION)
