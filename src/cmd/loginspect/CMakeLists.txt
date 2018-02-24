# Define the source files of the loginspect library:
SET(loginspect_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/logcat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/agglog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/verifylog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/truncatelog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logpagestats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logrecinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loganalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/propstats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dbscan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/addbackup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xctlatency.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tracerestore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/archstats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nodbgen.cpp
   )

# Add the library of this directory loginspect:
ADD_LIBRARY(loginspect ${loginspect_SRCS})

IF(LINK_TIME_OPTIMIZATION)
    SET_PROPERTY(TARGET loginspect PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
ENDIF(LINK_TIME_OPTIMIZATION)

