INCLUDE(testutil)

SET(COMMON_TEST_LIBS gtest_main common)

ENABLE_TESTING()

X_ADD_TESTCASE(test_errcodes gtest_main)
X_ADD_TESTCASE(test_lsns "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_vectors "${COMMON_TEST_LIBS}")

X_ADD_TESTCASE(test_fancylatches "${COMMON_TEST_LIBS}")

X_ADD_TESTCASE(test_bitvector "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_darray "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_endian "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_gc_pool_forest "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_heap "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_key_t gtest_main)
X_ADD_TESTCASE(test_list "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_markable_pointer "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_memblock "${COMMON_TEST_LIBS}") #FIXME fails on ubuntu 12 due to limitations of gtest with expected crashes in MT environment
X_ADD_TESTCASE(test_rc "${COMMON_TEST_LIBS}")
X_ADD_TESTCASE(test_w_okvl "${COMMON_TEST_LIBS}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
SET(TEST_STAT_GENFILES_FILES
    test_stat_collect_enum_gen.h
    test_stat_dec_gen.cpp
    test_stat_msg_gen.h
    test_stat_struct_gen.h
    test_stat_collect_gen.cpp
    test_stat_inc_gen.cpp
    test_stat_out_gen.cpp)
ADD_CUSTOM_COMMAND(OUTPUT ${TEST_STAT_GENFILES_FILES}
  COMMAND perl ${CMAKE_SOURCE_DIR}/tools/stats.pl ${CMAKE_CURRENT_SOURCE_DIR}/test_stat.dat ${COPY_GENERATED_FILES}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_stat.dat
)
ADD_CUSTOM_TARGET(TEST_STAT_GENFILES DEPENDS ${TEST_STAT_GENFILES_FILES})

X_ADD_TESTCASE(test_stat "${the_libraries}")
ADD_DEPENDENCIES(test_stat TEST_STAT_GENFILES)
