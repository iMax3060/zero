#ifndef __UNIFORM_INT_DISTRIBUTION_HPP
#define __UNIFORM_INT_DISTRIBUTION_HPP

#include <type_traits>
#include <limits>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <cmath>
#include "cgs/meta.hpp"
#include "gcem.hpp"
#include <boost/integer.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/detail/uniform_int_float.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/ranlux.hpp>

namespace zero::uniform_int_distribution {

    namespace details {

        template <typename int_type>
        constexpr uint8_t log2(int_type n) {
            return ((n < 2) ? 1 : 1 + log2(n / 2));
        };

    } // details

    template <class int_type = int>
    class biased_uniform_int_distribution {
        static_assert(std::is_integral<int_type>::value
                      && !std::is_same<int_type, bool>::value,
                      "'int_type' is not an integral type or it is bool");

    public:
        using result_type = int_type;

        struct param_type {
        public:
            using distribution_type = biased_uniform_int_distribution<int_type>;

            param_type() :
                    param_type(0) {};

            explicit param_type(int_type lowerLimit = 0, int_type upperLimit = std::numeric_limits<int_type>::max()) :
                    _lowerLimit(lowerLimit),
                    _upperLimit(upperLimit) {
                assert(_lowerLimit < _upperLimit);
            };

            int_type a() const noexcept {
                return _lowerLimit;
            }

            int_type b() const noexcept {
                return _upperLimit;
            }

            friend bool operator==(const param_type& parameters0, const param_type& parameters1) noexcept {
                return parameters0._lowerLimit == parameters1._lowerLimit
                    && parameters0._upperLimit == parameters1._upperLimit;
            }

            friend bool operator!=(const param_type& parameters0, const param_type& parameters1) noexcept {
                return !(parameters0 == parameters1);
            }

        private:
            int_type _lowerLimit;

            int_type _upperLimit;
        };

        biased_uniform_int_distribution() :
                biased_uniform_int_distribution(0) {};

        explicit biased_uniform_int_distribution(int_type lowerLimit,
                                                 int_type upperLimit = std::numeric_limits<int_type>::max()) :
                _parameters(lowerLimit, upperLimit),
                _offset(lowerLimit),
                _range(upperLimit - lowerLimit),
                _rangeBits(static_cast<uint8_t>(std::ceil(std::log2(_range + 2)))),
                _fallbackDistribution(lowerLimit, upperLimit) {};

        explicit biased_uniform_int_distribution(const param_type& parameters) :
                biased_uniform_int_distribution(parameters.a(), parameters.b()) {};

        void reset() const noexcept {};

        result_type a() const noexcept {
            return _parameters.a();
        };

        result_type b() const noexcept {
            return _parameters.b();
        };

        param_type param() const noexcept {
            return _parameters;
        };

        void param(const param_type& parameters) noexcept {
            _parameters = parameters;
            _offset = parameters.a();
            _range = parameters.a() - parameters.b();
            _rangeBits = static_cast<uint8_t>(std::ceil(std::log2(_range + 2)));
            _fallbackDistribution.param(parameters);
        };

        result_type min() const noexcept {
            return this->a();
        };

        result_type max() const noexcept {
            return this->b();
        }

        template<typename uniform_random_number_generator>
        result_type operator()(uniform_random_number_generator& uniformRandomNumberGenerator) const {
            using generator_type = typename uniform_random_number_generator::result_type;
            using unsigned_generator_type = typename boost::uint_t<sizeof(generator_type) * CHAR_BIT>::fast;
            using unsigned_result_type = typename std::make_unsigned<result_type>::type;

            constexpr bool minMaxAvailable = cgs::is_constexpr<uniform_random_number_generator::min>()
                                          && cgs::is_constexpr<uniform_random_number_generator::max>();
            constexpr unsigned_generator_type generatorRange =
                    !minMaxAvailable || !std::is_integral<generator_type>::value ? 0
                                                                                 : gcem::abs(static_cast<unsigned_generator_type>(uniform_random_number_generator::max())
                                                                                           - static_cast<unsigned_generator_type>(uniform_random_number_generator::min()));
            constexpr uint8_t generatorBits = std::is_integral<generator_type>::value
                                           && generatorRange < std::numeric_limits<unsigned_generator_type>::max()
                                                   ? !minMaxAvailable ? static_cast<uint8_t>(0)
                                                                      : static_cast<uint8_t>(details::log2<unsigned_generator_type>(generatorRange + 1) - 1)
                                                   : static_cast<uint8_t>(sizeof(generator_type) * CHAR_BIT);
            constexpr uint8_t generatorTypeBits = static_cast<uint8_t>(sizeof(generator_type) * CHAR_BIT);

            constexpr unsigned_result_type resultRange = std::numeric_limits<unsigned_result_type>::max();
            constexpr uint8_t resultBits = static_cast<uint8_t>(sizeof(result_type) * CHAR_BIT);

            using unsigned_double_width_result_type = typename boost::uint_t<resultBits * 2>::fast;
            using signed_double_width_result_type = typename boost::int_t<resultBits * 2>::fast;

            if constexpr (minMaxAvailable && std::is_integral<generator_type>::value) {
                if constexpr (resultBits <= generatorBits) {
                    DBG1( << "146: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    unsigned_generator_type x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator()
                                                                                   - uniform_random_number_generator::min());
                    if constexpr (generatorBits != details::log2(generatorRange)) {
                        constexpr unsigned_generator_type bitMask = gcem::pow(2, generatorBits) - 1;
                        x = x & bitMask;
                    }
                    if constexpr (std::is_unsigned<result_type>::value && resultBits <= 32 && generatorBits <= 32) {
                        unsigned_double_width_result_type m = static_cast<unsigned_double_width_result_type>(x)
                                                            * (static_cast<unsigned_double_width_result_type>(_range)
                                                             + static_cast<unsigned_double_width_result_type>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else if (std::is_unsigned<result_type>::value) {
                        __uint128_t m = static_cast<__uint128_t>(x)
                                      * (static_cast<__uint128_t>(_range)
                                       + static_cast<__uint128_t>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else if (std::is_signed<result_type>::value && resultBits <= 32 && generatorBits <= 32) {
                        signed_double_width_result_type m = static_cast<signed_double_width_result_type>(x)
                                                          * (static_cast<signed_double_width_result_type>(_range)
                                                           + static_cast<signed_double_width_result_type>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else {
                        __int128_t m = static_cast<__int128_t>(x)
                                     * (static_cast<__int128_t>(_range)
                                      + static_cast<__int128_t>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    }
                } else {
                    if (_rangeBits <= generatorBits) {
                        DBG1( << "176: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                        unsigned_generator_type x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator()
                                                                                       - uniform_random_number_generator::min());
                        if constexpr (generatorBits != details::log2(generatorRange)) {
                            constexpr unsigned_generator_type bitMask = gcem::pow(2, generatorBits) - 1;
                            x = x & bitMask;
                        }
                        if constexpr (std::is_unsigned<result_type>::value && resultBits <= 32 && generatorBits <= 32) {
                            unsigned_double_width_result_type m = static_cast<unsigned_double_width_result_type>(x)
                                                                * (static_cast<unsigned_double_width_result_type>(_range)
                                                                 + static_cast<unsigned_double_width_result_type>(1));
                            return static_cast<result_type>(_offset + (m >> generatorBits));
                        } else if (std::is_unsigned<result_type>::value) {
                            __uint128_t m = static_cast<__uint128_t>(x)
                                          * (static_cast<__uint128_t>(_range)
                                           + static_cast<__uint128_t>(1));
                            return static_cast<result_type>(_offset + (m >> generatorBits));
                        } else if (std::is_signed<result_type>::value && resultBits <= 32 && generatorBits <= 32) {
                            signed_double_width_result_type m = static_cast<signed_double_width_result_type>(x)
                                                              * (static_cast<signed_double_width_result_type>(_range)
                                                               + static_cast<signed_double_width_result_type>(1));
                            return static_cast<result_type>(_offset + (m >> generatorBits));
                        } else {
                            __int128_t m = static_cast<__int128_t>(x)
                                         * (static_cast<__int128_t>(_range)
                                          + static_cast<__int128_t>(1));
                            return static_cast<result_type>(_offset + (m >> generatorBits));
                        }
                    } else {
                        DBG1( << "205: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " > log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                        return _fallbackDistribution(uniformRandomNumberGenerator);
                    }
                }
            } else {
                unsigned_generator_type generatorRange;
                if constexpr (std::is_integral<generator_type>::value) {
                    generatorRange = gcem::abs(static_cast<unsigned_generator_type>(uniform_random_number_generator::max())
                                             - static_cast<unsigned_generator_type>(uniform_random_number_generator::min()));
                } else {

                    generatorRange = gcem::abs(static_cast<unsigned_generator_type>(boost::random::detail::uniform_int_float<uniform_random_number_generator>::max())
                                             - static_cast<unsigned_generator_type>(boost::random::detail::uniform_int_float<uniform_random_number_generator>::min()));
                }
                uint8_t generatorBits = generatorRange == std::numeric_limits<unsigned_generator_type>::max() ? static_cast<uint8_t>(sizeof(generator_type) * CHAR_BIT)
                                                                                                              : static_cast<uint8_t>(std::floor(std::log2(generatorRange + 1)));

                if (_rangeBits <= generatorBits) {
                    DBG1( << "223: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    unsigned_generator_type x;
                    if constexpr (std::is_integral<generator_type>::value) {
                        x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator() - uniform_random_number_generator::min());
                        if (generatorBits < generatorTypeBits) {
                            unsigned_generator_type bitMask = gcem::pow(2, generatorBits) - 1;
                            x = x & bitMask;
                        }
                    } else {
                        boost::random::detail::uniform_int_float<uniform_random_number_generator> floatToInt(uniformRandomNumberGenerator);

                        x = static_cast<unsigned_generator_type>(floatToInt());
                    }
                    if constexpr (std::is_unsigned<result_type>::value && resultBits <= 32 && generatorTypeBits <= 32) {
                        unsigned_double_width_result_type m = static_cast<unsigned_double_width_result_type>(x)
                                                            * (static_cast<unsigned_double_width_result_type>(_range)
                                                             + static_cast<unsigned_double_width_result_type>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else if (std::is_unsigned<result_type>::value) {
                        __uint128_t m = static_cast<__uint128_t>(x)
                                      * (static_cast<__uint128_t>(_range)
                                       + static_cast<__uint128_t>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else if (std::is_signed<result_type>::value && resultBits <= 32 && generatorTypeBits <= 32) {
                        signed_double_width_result_type m = static_cast<signed_double_width_result_type>(x)
                                                          * (static_cast<signed_double_width_result_type>(_range)
                                                           + static_cast<signed_double_width_result_type>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    } else {
                        __int128_t m = static_cast<__int128_t>(x)
                                     * (static_cast<__int128_t>(_range)
                                      + static_cast<__int128_t>(1));
                        return static_cast<result_type>(_offset + (m >> generatorBits));
                    }
                } else {
                    DBG1( << "258: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " > log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    return _fallbackDistribution(uniformRandomNumberGenerator);
                }
            }
        };

        friend bool operator==(const biased_uniform_int_distribution<result_type>& distribution0,
                               const biased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return distribution0._parameters == distribution1._parameters;
        };

        friend bool operator!=(const biased_uniform_int_distribution<result_type>& distribution0,
                               const biased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return !(distribution0 == distribution1);
        };

        template<class CharT, class Traits, class result_type>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& outputStream,
                                                             const biased_uniform_int_distribution<result_type>& distribution) {
            outputStream << distribution._parameters._a << "<" << distribution._parameters._b << std::endl;

            return outputStream;
        };

        template<class CharT, class Traits, class result_type>
        friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& inputStream,
                                                             const biased_uniform_int_distribution<result_type>& distribution) {
            result_type lowerBound;
            char separator;
            result_type upperBound;

            inputStream >> lowerBound >> separator >> upperBound;
            if (separator == '<') {
                distribution.param(param_type(lowerBound, upperBound));
            }

            return inputStream;
        };

    private:
        param_type _parameters;

        int_type _offset;

        int_type _range;

        uint8_t _rangeBits;

        boost::random::uniform_int_distribution<int_type> _fallbackDistribution;

    };

} // zero::uniform_int_distribution

#endif // __UNIFORM_INT_DISTRIBUTION_HPP
