## = Usage: ============================================================================================================
## INCLUDE(ExistenceChecks)    -- (after adding the location to the CMAKE_MODULE_PATH)
## =====================================================================================================================
## = Features: =========================================================================================================
## - Checks the existence of all header files from the include directory included to any of the project's files.
## - Checks the existence of some built-in types.
## - Checks the existence of some built-in symbols.
## - Checks the existence of some standard functions.
## - Checks the existence of some OS-dependent functions.
## =====================================================================================================================
## = WARNING: ==========================================================================================================
## - The set of checked headers, types, symbols, std- and non-std-function will very likely be outdated!
## =====================================================================================================================
## = Postconditions: ===================================================================================================
## - Each check sets a corresponding variable (see code for variable names) which will be available as preprocessor
##   macros.
## =====================================================================================================================
## = How to update: ====================================================================================================
## - Change also config/shore-config-env.h.cmake
## - Change also config/shore-config-h.in
## =====================================================================================================================

# Load CMake-files required to check existence of objects of the C/C++-standard, the OS or installed libraries:
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckPrototypeExists)

# Define a bunch of environment specific macros. This replaces the old autoconf/autoheader stuffs:
##################### Header Files: #####################
CHECK_INCLUDE_FILE_CXX(algorithm HAVE_ALGORITHM)
CHECK_INCLUDE_FILE_CXX(array HAVE_ARRAY)
CHECK_INCLUDE_FILE_CXX(atomic HAVE_ATOMIC)
CHECK_INCLUDE_FILE_CXX(bitset HAVE_BITSET)
CHECK_INCLUDE_FILE_CXX(boost/algorithm/string.hpp HAVE_BOOST_ALGORITHM_STRING_HPP)
CHECK_INCLUDE_FILE_CXX(boost/asio.hpp HAVE_BOOST_ASIO_HPP)
CHECK_INCLUDE_FILE_CXX(boost/concept_check.hpp HAVE_BOOST_CONCEPT_CHECK_HPP)
CHECK_INCLUDE_FILE_CXX(boost/date_time/gregorian/gregorian.hpp HAVE_BOOST_DATE_TIME_GREGORIAN_GREGORIAN_HPP)
CHECK_INCLUDE_FILE_CXX(boost/date_time/posix_time/posix_time.hpp HAVE_BOOST_DATE_TIME_POSIX_TIME_POSIX_TIME_HPP)
CHECK_INCLUDE_FILE_CXX(boost/filesystem.hpp HAVE_BOOST_FILESYSTEM_HPP)
CHECK_INCLUDE_FILE_CXX(boost/foreach.hpp HAVE_BOOST_FOREACH_HPP)
CHECK_INCLUDE_FILE_CXX(boost/lexical_cast.hpp HAVE_BOOST_LEXICAL_CAST_HPP)
CHECK_INCLUDE_FILE_CXX(boost/program_options/errors.hpp HAVE_BOOST_PROGRAM_OPTIONS_ERRORS_HPP)
CHECK_INCLUDE_FILE_CXX(boost/program_options.hpp HAVE_BOOST_PROGRAM_OPTIONS_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/additive_combine.hpp HAVE_BOOST_RANDOM_ADDITIVE_COMBINE_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/inversive_congruential.hpp HAVE_BOOST_RANDOM_INVERSIVE_CONGRUENTIAL_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/lagged_fibonacci.hpp HAVE_BOOST_RANDOM_LAGGED_FIBONACCI_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/linear_congruential.hpp HAVE_BOOST_RANDOM_LINEAR_CONGRUENTIAL_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/mersenne_twister.hpp HAVE_BOOST_RANDOM_MERSENNE_TWISTER_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/ranlux.hpp HAVE_BOOST_RANDOM_RANLUX_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/shuffle_order.hpp HAVE_BOOST_RANDOM_SHUFFLE_ORDER_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/taus88.hpp HAVE_BOOST_RANDOM_TAUS88_HPP)
CHECK_INCLUDE_FILE_CXX(boost/random/uniform_int_distribution.hpp HAVE_BOOST_RANDOM_UNIFORM_INT_DISTRIBUTION_HPP)
CHECK_INCLUDE_FILE_CXX(boost/scoped_array.hpp HAVE_BOOST_SCOPED_ARRAY_HPP)
CHECK_INCLUDE_FILE_CXX(boost/static_assert.hpp HAVE_BOOST_STATIC_ASSERT_HPP)
CHECK_INCLUDE_FILE_CXX(cassert HAVE_CASSERT)
CHECK_INCLUDE_FILE_CXX(cctype HAVE_CCTYPE)
CHECK_INCLUDE_FILE_CXX(cerrno HAVE_CERRNO)
CHECK_INCLUDE_FILE_CXX(chrono HAVE_CHRONO)
CHECK_INCLUDE_FILE_CXX(climits HAVE_CLIMITS)
CHECK_INCLUDE_FILE_CXX(cmath HAVE_CMATH)
CHECK_INCLUDE_FILE_CXX(condition_variable HAVE_CONDITION_VARIABLE)
CHECK_INCLUDE_FILE_CXX(cstdarg HAVE_CSTDARG)
CHECK_INCLUDE_FILE_CXX(cstddef HAVE_CSTDDEF)
CHECK_INCLUDE_FILE_CXX(cstdint HAVE_CSTDINT)
CHECK_INCLUDE_FILE_CXX(cstdio HAVE_CSTDIO)
CHECK_INCLUDE_FILE_CXX(cstdlib HAVE_CSTDLIB)
CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
CHECK_INCLUDE_FILE_CXX(ctime HAVE_CTIME)
CHECK_INCLUDE_FILE_CXX(deque HAVE_DEQUE)
CHECK_INCLUDE_FILE_CXX(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE_CXX(error.h HAVE_ERROR_H)
CHECK_INCLUDE_FILE_CXX(exception HAVE_EXCEPTION)
CHECK_INCLUDE_FILE_CXX(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE_CXX(fstream HAVE_FSTREAM)
CHECK_INCLUDE_FILE_CXX(functional HAVE_FUNCTIONAL)
CHECK_INCLUDE_FILE_CXX(iomanip HAVE_IOMANIP)
CHECK_INCLUDE_FILE_CXX(iosfwd HAVE_IOSFWD)
CHECK_INCLUDE_FILE_CXX(iostream HAVE_IOSTREAM)
CHECK_INCLUDE_FILE_CXX(limits HAVE_LIMITS)
CHECK_INCLUDE_FILE_CXX(list HAVE_LIST)
CHECK_INCLUDE_FILE_CXX(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE_CXX(map HAVE_MAP)
CHECK_INCLUDE_FILE_CXX(memory HAVE_MEMORY)
CHECK_INCLUDE_FILE_CXX(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE_CXX(mutex HAVE_MUTEX)
CHECK_INCLUDE_FILE_CXX(MUTrace/mutrace.h HAVE_MUTRACE_MUTRACE_H)
CHECK_INCLUDE_FILE_CXX(netinet/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE_CXX(new HAVE_NEW)
CHECK_INCLUDE_FILE_CXX(numeric HAVE_NUMERIC)
CHECK_INCLUDE_FILE_CXX(ostream HAVE_OSTREAM)
CHECK_INCLUDE_FILE_CXX(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILE_CXX(queue HAVE_QUEUE)
CHECK_INCLUDE_FILE_CXX(random HAVE_RANDOM)
CHECK_INCLUDE_FILE_CXX(regex HAVE_REGEX)
CHECK_INCLUDE_FILE_CXX(set HAVE_SET)
CHECK_INCLUDE_FILE_CXX(sstream HAVE_SSTREAM)
CHECK_INCLUDE_FILE_CXX(stdexcept HAVE_STDEXCEPT)
CHECK_INCLUDE_FILE_CXX(string HAVE_STRING)
CHECK_INCLUDE_FILE_CXX(sys/mman.h HAVE_SYS_MMAN_H)
CHECK_INCLUDE_FILE_CXX(sys/param.h HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE_CXX(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE_CXX(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE_CXX(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE_CXX(sys/uio.h HAVE_SYS_UIO_H)
CHECK_INCLUDE_FILE_CXX(thread HAVE_THREAD)
CHECK_INCLUDE_FILE_CXX(type_traits HAVE_TYPE_TRAITS)
CHECK_INCLUDE_FILE_CXX(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE_CXX(unordered_map HAVE_UNORDERED_MAP)
CHECK_INCLUDE_FILE_CXX(unordered_set HAVE_UNORDERED_SET)
CHECK_INCLUDE_FILE_CXX(utility HAVE_UTILITY)
CHECK_INCLUDE_FILE_CXX(vector HAVE_VECTOR)
CHECK_INCLUDE_FILE_CXX(Wincrypt.h HAVE_WINCRYPT_H)
CHECK_INCLUDE_FILE_CXX(windows.h HAVE_WINDOWS_H)

CHECK_INCLUDE_FILES(valgrind.h HAVE_VALGRIND_H)
CHECK_INCLUDE_FILES(valgrind/valgrind.h HAVE_VALGRIND_VALGRIND_H)

####################### Types and Symbols: ########################
CHECK_SYMBOL_EXISTS(char_t "" HAVE_CHAR_T)
CHECK_SYMBOL_EXISTS(long_t "" HAVE_LONG_T)
CHECK_SYMBOL_EXISTS(short_t "" HAVE_SHORT_T)
CHECK_SYMBOL_EXISTS(uchar_t "" HAVE_UCHAR_T)
CHECK_SYMBOL_EXISTS(ulong_t "" HAVE_ULONG_T)
CHECK_SYMBOL_EXISTS(ushort_t "" HAVE_USHORT_T)

CHECK_SYMBOL_EXISTS(MAP_ALIGN "sys/mman.h" HAVE_DECL_MAP_ALIGN)
CHECK_SYMBOL_EXISTS(MAP_ANON "sys/mman.h" HAVE_DECL_MAP_ANON)
CHECK_SYMBOL_EXISTS(MAP_ANONYMOUS "sys/mman.h" HAVE_DECL_MAP_ANONYMOUS)
CHECK_SYMBOL_EXISTS(MAP_FIXED "sys/mman.h" HAVE_DECL_MAP_FIXED)
CHECK_SYMBOL_EXISTS(MAP_NORESERVE "sys/mman.h" HAVE_DECL_MAP_NORESERVE)

#################### std-Functions: #####################
CHECK_FUNCTION_EXISTS(vprintf HAVE_VPRINTF)
CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
CHECK_FUNCTION_EXISTS(memalign HAVE_MEMALIGN)
CHECK_FUNCTION_EXISTS(membar_enter HAVE_MEMBAR_ENTER)
CHECK_FUNCTION_EXISTS(memcntl HAVE_MEMCNTL)
CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)
CHECK_FUNCTION_EXISTS(posix_spawn HAVE_POSIX_SPAWN)
CHECK_FUNCTION_EXISTS(valloc HAVE_VALLOC)
CHECK_FUNCTION_EXISTS(strerror HAVE_STRERROR)

################## Non-std-Functions: ###################
# these need CHECK_SYMBOL_EXISTS or CHECK_PROTOTYPE_EXISTS, not CHECK_FUNCTION_EXISTS
CHECK_PROTOTYPE_EXISTS(clock_gettime "sys/time.h;time.h" HAVE_CLOCK_GETTIME)
CHECK_PROTOTYPE_EXISTS(gethrtime "sys/time.h;time.h" HAVE_GETHRTIME)
CHECK_PROTOTYPE_EXISTS(gettimeofday "sys/time.h;time.h" HAVE_GETTIMEOFDAY)

CHECK_PROTOTYPE_EXISTS(pthread_attr_getstack "pthread.h" HAVE_PTHREAD_ATTR_GETSTACK)
CHECK_PROTOTYPE_EXISTS(pthread_attr_getstacksize "pthread.h" HAVE_PTHREAD_ATTR_GETSTACKSIZE)

CHECK_PROTOTYPE_EXISTS(getpagesizes "sys/mman.h" HAVE_GETPAGESIZES)