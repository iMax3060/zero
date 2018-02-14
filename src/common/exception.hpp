#ifndef ZERO_COMMON_EXCEPTION_HPP
#define ZERO_COMMON_EXCEPTION_HPP

#include <stdexcept>

#include "w_defines.h"

#define throw0(condition, exception)    do {    \
    if (condition)                              \
        throw exception;                        \
} while(0)

#ifndef W_DEBUG_LEVEL
#define W_DEBUG_LEVEL
#endif

#if W_DEBUG_LEVEL>=1
#define throw1(condition, exception)    throw0(condition, exception)
#else
#define throw1(condition, exception)    if (false) { (void)(condition); }
#endif

#if W_DEBUG_LEVEL>=2
#define throw2(condition, exception)    throw1(condition, exception)
#else
#define throw2(condition, exception)    if (false) { (void)(condition); }
#endif

#if W_DEBUG_LEVEL>=3
#define throw3(condition, exception)    throw2(condition, exception)
#else
#define throw3(condition, exception)    if (false) { (void)(condition); }
#endif

#if W_DEBUG_LEVEL>=4
#define throw4(condition, exception)    throw3(condition, exception)
#else
#define throw4(condition, exception)    if (false) { (void)(condition); }
#endif

#if W_DEBUG_LEVEL>=5
#define throw5(condition, exception)    throw4(condition, exception)
#else
#define throw5(condition, exception)    if (false) { (void)(condition); }
#endif

#define throw9(condition, exception)    if (false) { (void)(condition); }

namespace zero {
    class RuntimeException : public std::runtime_error {
    public:
        RuntimeException(std::string const &message) :
                std::runtime_error(message) {};
    };
}

#endif // ZERO_COMMON_EXCEPTION_HPP
