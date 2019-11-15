#ifndef __UNIFORM_INT_DISTRIBUTION_HPP
#define __UNIFORM_INT_DISTRIBUTION_HPP

#include <type_traits>
#include <limits>
#include <cassert>
#include <cstdint>
#include <iostream>

namespace zero::uniform_int_distribution {

    template <class int_type = int>
    class biased_uniform_int_distribution {
        static_assert(std::is_integral<int_type>::value
                   && std::is_unsigned<int_type>::value
                   && !std::is_same<int_type, bool>::value,
                      "'int_type' is not an unsigned integral type or it is bool");

    public:
        using result_type = int_type;

        struct param_type {
        public:
            using distribution_type = biased_uniform_int_distribution<int_type>;

            param_type() :
                    param_type(0) {};

            explicit param_type(int_type lowerLimit, int_type upperLimit = std::numeric_limits<int_type>::max()) :
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
                _range(upperLimit - lowerLimit + 1) {};

        explicit biased_uniform_int_distribution(const param_type& parameters) :
                _parameters(parameters) {};

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
            _range = parameters.a() - parameters.b() + 1;
        };

        result_type min() const noexcept {
            return this->a();
        };

        result_type max() const noexcept {
            return this->b();
        }

        template<typename uniform_random_number_generator>
        result_type operator()(uniform_random_number_generator& uniformRandomNumberGenerator) const {
            static_assert(std::is_integral<typename uniform_random_number_generator::result_type>::value
                       && std::is_unsigned<typename uniform_random_number_generator::result_type>::value,
                          "'uniform_random_number_generator' does not return random numbers of unsigned integral type");
            static_assert((std::is_same<result_type, uint16_t>::value
                        && (std::is_same<typename uniform_random_number_generator::result_type, uint16_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint32_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value))
                       || (std::is_same<result_type, uint32_t>::value
                        && (std::is_same<typename uniform_random_number_generator::result_type, uint32_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value))
                       || (std::is_same<result_type, uint64_t>::value
                        && std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value),
                          "'uniform_random_number_generator' does not return a random numbers of high enough width");

            if constexpr (std::is_same<result_type, uint8_t>::value) {
                uint8_t x = uint8_t(uniformRandomNumberGenerator());
                uint16_t m = uint16_t(x) * uint16_t(_range);
                return uint8_t(_offset + (m >> 8));
            } else if (std::is_same<result_type, uint16_t>::value) {
                uint16_t x = uint16_t(uniformRandomNumberGenerator());
                uint32_t m = uint32_t(x) * uint32_t(_range);
                return uint16_t(_offset + (m >> 16));
            } else if (std::is_same<result_type, uint32_t>::value) {
                uint32_t x = uint32_t(uniformRandomNumberGenerator());
                uint64_t m = uint64_t(x) * uint64_t(_range);
                return uint32_t(_offset + (m >> 32));
#if __SIZEOF_INT128__
            } else if (std::is_same<result_type, uint64_t>::value) {
                uint64_t x = uint64_t(uniformRandomNumberGenerator());
                __uint128_t m = __uint128_t(x) * __uint128_t(_range);
                return uint64_t(_offset + (m >> 64));
#endif // __SIZEOF_INT128__
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

    };

    template <class int_type = int>
    class unbiased_uniform_int_distribution {
        static_assert(std::is_integral<int_type>::value
                   && std::is_unsigned<int_type>::value
                   && !std::is_same<int_type, bool>::value,
                      "'int_type' is not an unsigned integral type or it is bool");

    public:
        using result_type = int_type;

        struct param_type {
        public:
            using distribution_type = unbiased_uniform_int_distribution<int_type>;

            param_type() :
                    param_type(0) {};

            explicit param_type(int_type lowerLimit, int_type upperLimit = std::numeric_limits<int_type>::max()) :
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

        unbiased_uniform_int_distribution() :
                unbiased_uniform_int_distribution(0) {};

        explicit unbiased_uniform_int_distribution(int_type lowerLimit,
                                                   int_type upperLimit = std::numeric_limits<int_type>::max()) :
                _parameters(lowerLimit, upperLimit),
                _offset(lowerLimit),
                _range(upperLimit - lowerLimit + 1) {};

        explicit unbiased_uniform_int_distribution(const param_type& parameters) :
                _parameters(parameters) {};

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
            _range = parameters.a() - parameters.b() + 1;
        };

        result_type min() const noexcept {
            return this->a();
        };

        result_type max() const noexcept {
            return this->b();
        }

        template<typename uniform_random_number_generator>
        result_type operator()(uniform_random_number_generator& uniformRandomNumberGenerator) const {
            static_assert(std::is_integral<typename uniform_random_number_generator::result_type>::value
                       && std::is_unsigned<typename uniform_random_number_generator::result_type>::value,
                          "'uniform_random_number_generator' does not return random numbers of unsigned integral type");
            static_assert((std::is_same<result_type, uint16_t>::value
                        && (std::is_same<typename uniform_random_number_generator::result_type, uint16_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint32_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value))
                       || (std::is_same<result_type, uint32_t>::value
                        && (std::is_same<typename uniform_random_number_generator::result_type, uint32_t>::value
                         || std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value))
                       || (std::is_same<result_type, uint64_t>::value
                        && std::is_same<typename uniform_random_number_generator::result_type, uint64_t>::value),
                          "'uniform_random_number_generator' does not return a random numbers of high enough width");

            if constexpr (std::is_same<result_type, uint8_t>::value) {
                uint8_t x = uniformRandomNumberGenerator();
                uint16_t m = uint16_t(x) * uint16_t(_range);
                uint8_t l = uint8_t(m);
                if (l < _range) {
                    uint8_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = uint16_t(x) * uint16_t(_range);
                        l = uint8_t(m);
                    }
                }
                return uint8_t(_offset + (m >> 8));
            } else if (std::is_same<result_type, uint16_t>::value) {
                uint16_t x = uniformRandomNumberGenerator();
                uint32_t m = uint32_t(x) * uint32_t(_range);
                uint16_t l = uint16_t(m);
                if (l < _range) {
                    uint16_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = uint32_t(x) * uint32_t(_range);
                        l = uint16_t(m);
                    }
                }
                return uint16_t(_offset + (m >> 16));
            } else if (std::is_same<result_type, uint32_t>::value) {
                uint32_t x = uniformRandomNumberGenerator();
                uint64_t m = uint64_t(x) * uint64_t(_range);
                uint32_t l = uint32_t(m);
                if (l < _range) {
                    uint32_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = uint64_t(x) * uint64_t(_range);
                        l = uint32_t(m);
                    }
                }
                return uint32_t(_offset + (m >> 32));
#if __SIZEOF_INT128__
            } else if (std::is_same<result_type, uint64_t>::value) {
                uint64_t x = uniformRandomNumberGenerator();
                __uint128_t m = __uint128_t(x) * __uint128_t(_range);
                uint64_t l = uint64_t(m);
                if (l < _range) {
                    uint64_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = __uint128_t(x) * __uint128_t(_range);
                        l = uint64_t(m);
                    }
                }
                return uint64_t(_offset + (m >> 64));
#endif // __SIZEOF_INT128__
            } else if (std::is_same<result_type, int8_t>::value) {
                int8_t x = uniformRandomNumberGenerator();
                int16_t m = int16_t(x) * int16_t(_range);
                int8_t l = int8_t(m);
                if (l < _range) {
                    int8_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = int16_t(x) * int16_t(_range);
                        l = int8_t(m);
                    }
                }
                return int8_t(_offset + (m >> 8));
            } else if (std::is_same<result_type, int16_t>::value) {
                int16_t x = uniformRandomNumberGenerator();
                int32_t m = int32_t(x) * int32_t(_range);
                int16_t l = int16_t(m);
                if (l < _range) {
                    int16_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = int32_t(x) * int32_t(_range);
                        l = int16_t(m);
                    }
                }
                return int16_t(_offset + (m >> 16));
            } else if (std::is_same<result_type, int32_t>::value) {
                int32_t x = uniformRandomNumberGenerator();
                int64_t m = int64_t(x) * int64_t(_range);
                int32_t l = int32_t(m);
                if (l < _range) {
                    int32_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = int64_t(x) * int64_t(_range);
                        l = int32_t(m);
                    }
                }
                return int32_t(_offset + (m >> 32));
#if __SIZEOF_INT128__
            } else if (std::is_same<result_type, int64_t>::value) {
                int64_t x = uniformRandomNumberGenerator();
                __int128_t m = __int128_t(x) * __int128_t(_range);
                int64_t l = int64_t(m);
                if (l < _range) {
                    int64_t t = -_range;
                    if (t >= _range) {
                        t -= _range;
                        if (t >= _range)
                            t %= _range;
                    }
                    while (l < t) {
                        x = uniformRandomNumberGenerator();
                        m = __int128_t(x) * __int128_t(_range);
                        l = int64_t(m);
                    }
                }
                return int64_t(_offset + (m >> 64));
#endif // __SIZEOF_INT128__
            }
        };

        friend bool operator==(const unbiased_uniform_int_distribution<result_type>& distribution0,
                               const unbiased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return distribution0._parameters == distribution1._parameters;
        };

        friend bool operator!=(const unbiased_uniform_int_distribution<result_type>& distribution0,
                               const unbiased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return !(distribution0 == distribution1);
        };

        template<class CharT, class Traits, class result_type>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& outputStream,
                                                             const unbiased_uniform_int_distribution<result_type>& distribution) {
            outputStream << distribution._parameters._a << "<" << distribution._parameters._b << std::endl;

            return outputStream;
        };

        template<class CharT, class Traits, class result_type>
        friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& inputStream,
                                                             const unbiased_uniform_int_distribution<result_type>& distribution) {
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

    };

} // zero::uniform_int_distribution

#endif // __UNIFORM_INT_DISTRIBUTION_HPP
