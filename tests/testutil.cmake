# CMake function to help build testcases.
# Include this file from the CMakeLists.txt

# To specify alternate configuration definitions 
# (such as defined in sm/btree_test_env.h)
# add to the ALTERNATE_TEST_CONFIGS variable below.

# set(ALTERNATE_TEST_CONFIGS DEFAULT_SWIZZLING_OFF)

# To pick up these xml files from Jenkins,
# Set "**/test-reports/*.xml" as the report file filter.
FUNCTION(X_ADD_GTEST targetname)
    ADD_TEST(${targetname} ${targetname} --gtest_output=xml:test-reports/result_${targetname}.xml)
ENDFUNCTION(X_ADD_GTEST)

# CMake function to add compile target, linking, ADD_TEST directives.
# If ALTERNATE_TEST_CONFIGS configurations are defined, then add each alternate
# configuration (${f}) to the testname and to COMPILE_DEFINITIONS
FUNCTION(X_ADD_TESTCASE targetname the_libraries)
   ADD_EXECUTABLE(${targetname} ${CMAKE_CURRENT_SOURCE_DIR}/${targetname}.cpp)
   TARGET_LINK_LIBRARIES(${targetname} ${the_libraries})
   X_ADD_GTEST(${targetname})
   FOREACH(f ${ALTERNATE_TEST_CONFIGS})
      ADD_EXECUTABLE(${targetname}_${f} ${CMAKE_CURRENT_SOURCE_DIR}/${targetname}.cpp)
      TARGET_LINK_LIBRARIES(${targetname}_${f} ${the_libraries})
      X_ADD_GTEST(${targetname}_${f})
      SET_PROPERTY(TARGET ${targetname}_${f} PROPERTY COMPILE_DEFINITIONS ${f})
   ENDFOREACH(f)
ENDFUNCTION(X_ADD_TESTCASE)
