# Define the source files of the kits experiments:
SET(experiments_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/restore_cmd.cpp
   )

# Add the library of this directory experiments:
ADD_LIBRARY(experiments ${experiments_SRCS})

SET_PROPERTY(TARGET experiments PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
