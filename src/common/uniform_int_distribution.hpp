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
#include "w_debug.h"
#include <boost/integer.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/detail/uniform_int_float.hpp>
#include <boost/random/taus88.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/ranlux.hpp>

namespace zero::uniform_int_distribution {

    namespace details {

        template<typename int_type>
        constexpr uint16_t log2(int_type n) {
            return ((n < 2) ? 1 : 1 + log2(n / 2));
        };
    } // details

    /*!\class   biased_uniform_int_distribution
     * \brief   Distributes random numbers from a PRNG uniformly (but biased) distributed in a range
     * \details This is a drop-in replacement for the \c std::uniform_int_distribution and the
     *          \c boost::random::uniform_int_distribution with the following differences:
     *          - It offers a significantly higher performance for fast PRNGs.
     *          - The output of this distribution facility is not perfectly uniform---instead it is biased based on the
     *            given range and PRNG. If this would be used with a given integer range \f$[0,5]\f$ and with a PRNG
     *            returning uniformly distributed 3-bit integers, the following distribution would be achieved:
     *            - \f$0 \longrightarrow \frac{2}{8}\f$ from the uniformly distributed random integers 0 and 6
     *            - \f$1 \longrightarrow \frac{2}{8}\f$ from the uniformly distributed random integers 1 and 7
     *            - \f$2 \longrightarrow \frac{1}{8}\f$ from the uniformly distributed random integer 2
     *            - \f$3 \longrightarrow \frac{1}{8}\f$ from the uniformly distributed random integer 3
     *            - \f$4 \longrightarrow \frac{1}{8}\f$ from the uniformly distributed random integer 4
     *            - \f$5 \longrightarrow \frac{1}{8}\f$ from the uniformly distributed random integer 5
     *
     *            The output distribution is unbiased when the output range is a divisor of the PRNG's range and it is
     *            almost unbiased when the output range is much smaller than the PRNG's range.
     *          - In contrast to \c std::uniform_int_distribution , it supports PRNGs generating random floating point
     *            numbers.
     *          - The bitwidth and range (only for PRNGs generating random integers) of the given PRNG's random numbers
     *            need to be higher than the one of the given output range
     *            \link biased_uniform_int_distribution::_range \endlink. Therefore, this distribution facility does not
     *            need to support multiple calls to the given PRNG for the generation of one random number in the given
     *            range.
     *          - It uses the \c boost::random::detail::uniform_int_float to transform random floating point numbers
     *            from the respective PRNG to uniformly distributed random integers.
     *          - It requires compiler support for 128-bit arithmetic when \c int_type has 64-bit width.
     *
     * \author  Max Gilbert
     * \note    This implementation using metaprogramming is a generalization of the algorithm "Integer Multiplication
     *          (Biased)" as presented by Melissa E. O’Neill in her shootout
     *          <a href="http://www.pcg-random.org/posts/bounded-rands.html">Efficiently Generating a Number in a Range</a>
     *
     * @tparam int_type The integer data type of the output range.
     */
    template<class int_type = int>
    class biased_uniform_int_distribution {
        static_assert(std::is_integral<int_type>::value
                      && !std::is_same<int_type, bool>::value,
                      "'int_type' is not an integral type or it is bool");

    public:
        /*!\typedef result_type
         * \brief   The output data type of this distribution facility
         */
        using result_type = int_type;

        /*!\struct param_type
         * \brief  Configuration of a random distribution facility
         */
        struct param_type {
        public:
            /*!\typedef distribution_type
             * \brief   The type of distribution facility this configuration is for
             */
            using distribution_type = biased_uniform_int_distribution<int_type>;

            /*!\fn    param_type()
             * \brief Constructs an unspecified configuration for a random distribution facility
             */
            param_type() :
                    param_type(0) {};

            /*!\fn    param_type(int_type lowerLimit, int_type upperLimit)
             * \brief Constructs a configuration for a random distribution facility with given lower and upper limits
             *
             * @param lowerLimit Lower limit of the random distribution's output range
             * @param upperLimit Upper limit of the random distribution's output range
             */
            explicit param_type(int_type lowerLimit = 0, int_type upperLimit = std::numeric_limits<int_type>::max()) :
                    _lowerLimit(lowerLimit),
                    _upperLimit(upperLimit) {
                assert(_lowerLimit < _upperLimit);
            };

            /*!\fn    a() noexcept
             * \brief Returns the lower limit of the random distribution's output range
             *
             * @return Lower limit of the random distribution's output range
             */
            int_type a() const noexcept {
                return _lowerLimit;
            }

            /*!\fn    b() noexcept
             * \brief Returns the upper limit of the random distribution's output range
             *
             * @return Upper limit of the random distribution's output range
             */
            int_type b() const noexcept {
                return _upperLimit;
            }

            /*!\fn    operator==(const param_type& parameters0, const param_type& parameters1) noexcept
             * \brief Compares two configuration for random distribution facilities for equality
             *
             * @param parameters0 Configuration of a
             *                    \link zero::uniform_int_distribution::biased_uniform_int_distribution \endlink
             * @param parameters1 Configuration of another
             *                    \link zero::uniform_int_distribution::biased_uniform_int_distribution \endlink
             * @return            This is \c true iff lower and upper limits defined in both configurations are equal,
             *                    otherwise this is \c false .
             */
            friend bool operator==(const param_type& parameters0, const param_type& parameters1) noexcept {
                return parameters0._lowerLimit == parameters1._lowerLimit
                       && parameters0._upperLimit == parameters1._upperLimit;
            }

            /*!\fn    operator!=(const param_type& parameters0, const param_type& parameters1) noexcept
             * \brief Compares two configuration for random distribution facilities for inequality
             *
             * @param parameters0 Configuration of a
             *                    \link zero::uniform_int_distribution::biased_uniform_int_distribution \endlink
             * @param parameters1 Configuration of another
             *                    \link zero::uniform_int_distribution::biased_uniform_int_distribution \endlink
             * @return            This is \c true iff the two configurations are not equal according to
             *                    \link operator==() \endlink , otherwise this is \c false .
             */
            friend bool operator!=(const param_type& parameters0, const param_type& parameters1) noexcept {
                return !(parameters0 == parameters1);
            }

        private:
            /*!\var   _lowerLimit
             * \brief The lower limit of the random distribution's output range
             */
            int_type _lowerLimit;

            /*!\var   _upperLimit
             * \brief The upper limit of the random distribution's output range
             */
            int_type _upperLimit;
        };

        /*!\fn    biased_uniform_int_distribution()
         * \brief Constructs an unspecified biased uniform random integer distribution facility
         */
        biased_uniform_int_distribution() :
                biased_uniform_int_distribution(0) {};

        /*!\fn    biased_uniform_int_distribution(int_type lowerLimit, int_type upperLimit)
         * \brief Constructs a biased uniform random integer distribution facility with given lower and upper limits
         *
         * @param lowerLimit Lower limit of the random distribution's output range
         * @param upperLimit Upper limit of the random distribution's output range
         */
        explicit biased_uniform_int_distribution(int_type lowerLimit,
                                                 int_type upperLimit = std::numeric_limits<int_type>::max()) :
                _parameters(lowerLimit, upperLimit),
                _offset(lowerLimit),
                _range(upperLimit - lowerLimit),
                _rangeBits(static_cast<uint16_t>(std::ceil(std::log2(_range + 2)))),
                _fallbackDistribution(lowerLimit, upperLimit) {};

        /*!\fn    biased_uniform_int_distribution(const param_type& parameters)
         * \brief Constructs a biased uniform random integer distribution facility based on a given configuration
         *
         * @param parameters The configuration providing the required parameters
         */
        explicit biased_uniform_int_distribution(const param_type& parameters) :
                biased_uniform_int_distribution(parameters.a(), parameters.b()) {};

        /*!\fn      reset() const noexcept
         * \brief   Resets the internal state of the random distribution facility
         * \details This biased uniform random integer distribution facility does not have an internal state and
         *          therefore, this does nothing.
         */
        void reset() const noexcept {};

        /*!\fn    a() const noexcept
         * \brief Returns the lower limit of the output range
         *
         * @return Lower limit of the output range
         */
        result_type a() const noexcept {
            return _parameters.a();
        };

        /*!\fn    b() const noexcept
         * \brief Returns the upper limit of the output range
         *
         * @return Upper limit of the output range
         */
        result_type b() const noexcept {
            return _parameters.b();
        };

        /*!\fn    param() const noexcept
         * \brief Returns the configuration of this random distribution facility
         *
         * @return The configuration of this random distribution facility
         */
        param_type param() const noexcept {
            return _parameters;
        };

        /*!\fn    param(const param_type& parameters) noexcept
         * \brief Changes this biased uniform random integer distribution facility according the given configuration
         *
         * @param parameters The configuration providing the required parameters
         */
        void param(const param_type& parameters) noexcept {
            _parameters = parameters;
            _offset = parameters.a();
            _range = parameters.a() - parameters.b();
            _rangeBits = static_cast<uint16_t>(std::ceil(std::log2(_range + 2)));
            _fallbackDistribution.param(parameters);
        };

        /*!\fn    min() const noexcept
         * \brief Returns the minimum integer value returned by this random distribution facility
         *
         * @return Lower limit of the output range
         */
        result_type min() const noexcept {
            return this->a();
        };

        /*!\fn    max() const noexcept
         * \brief Returns the maximum integer value returned by this random distribution facility
         *
         * @return Upper limit of the output range
         */
        result_type max() const noexcept {
            return this->b();
        }

        /*!\fn      operator()(uniform_random_number_generator& uniformRandomNumberGenerator) const
         * \brief   Generates the next biased uniformly distributed random integer in the given range from the given
         *          PRNG
         * \details
         *
         * @tparam uniform_random_number_generator The type of PRNG that should be used to obtain the used random number
         *                                         which is the entropy used for the generation of the biased uniformly
         *                                         distributed random integer.
         * @param uniformRandomNumberGenerator     The instance of \c uniform_random_number_generator that should be
         *                                         used to generate the next random integer with this distribution.
         * @return                                 A biased uniformly distributed random integer generated with entropy
         *                                         i.e. a uniformly distributed random number generated using the given
         *                                         \c uniformRandomNumberGenerator .
         */
        template<typename uniform_random_number_generator>
        result_type operator()(uniform_random_number_generator& uniformRandomNumberGenerator) const {
            // The output data type of the underlying PRNG:
            using generator_type = typename uniform_random_number_generator::result_type;
            // The unsigned equivalent of the output data type of the underlying PRNG:
            // In case the output data type of the underlying PRNG is already unsigned, this should be the same or one
            // with a higher bitwidth and in case the output data type of the underlying PRNG is float, this should be
            // an unsigned integer data type with the same or a higher bitwidth.
            using unsigned_generator_type = typename boost::uint_t<sizeof(generator_type) * CHAR_BIT>::fast;
            // The unsigned equivalent of this random distribution facility's output type:
            // In case the output type of this random distribution facility is already unsigned, this is the same type.
            using unsigned_result_type = typename std::make_unsigned<result_type>::type;

            // Whether the actual output range of the underlying PRNG is known at compile-time:
            constexpr bool minMaxAvailable = cgs::is_constexpr<uniform_random_number_generator::min>()
                                             && cgs::is_constexpr<uniform_random_number_generator::max>();
            // The length of the actual output range of the underlying PRNG or 0 if it outputs floats or if the range is
            // not known at compile-time:
            constexpr unsigned_generator_type generatorRange =
                    !minMaxAvailable || !std::is_integral<generator_type>::value ? 0
                                                                                 : gcem::abs(
                            static_cast<unsigned_generator_type>(uniform_random_number_generator::max())
                            - static_cast<unsigned_generator_type>(uniform_random_number_generator::min()));
            // The effective bitwidth of the actual output range of the underlying integer PRNG if known at compile-time
            // or the bitwidth of the output data type of the underlying PRNG:
            constexpr uint16_t generatorBits = std::is_integral<generator_type>::value
                                               && generatorRange < std::numeric_limits<unsigned_generator_type>::max()
                                               ? !minMaxAvailable ? static_cast<uint16_t>(0)
                                                                  : static_cast<uint16_t>(
                                                         details::log2<unsigned_generator_type>(generatorRange + 1) - 1)
                                               : static_cast<uint16_t>(sizeof(generator_type) * CHAR_BIT);
            // The bitwidth of the output data type of the underlying PRNG:
            constexpr uint16_t generatorTypeBits = static_cast<uint16_t>(sizeof(generator_type) * CHAR_BIT);

            // The highest possible length of the output range of this random distribution facility (based on the range
            // of the output data type):
            constexpr unsigned_result_type resultRange = std::numeric_limits<unsigned_result_type>::max();
            // The bitwidth of the output data type of this random distribution facility:
            constexpr uint16_t resultBits = static_cast<uint16_t>(sizeof(result_type) * CHAR_BIT);

            // Unsigned and signed integer types with a bitwidth which is at least double the bitwidth of the one of the
            // output data type of this random distribution facility:
            using unsigned_double_width_result_type = typename boost::uint_t<resultBits * 2>::fast;
            using signed_double_width_result_type = typename boost::int_t<resultBits * 2>::fast;

            // The output range of the underlying PRNG is known at compile-time and it is a PRNG generating integer
            // numbers:
            if constexpr (minMaxAvailable && std::is_integral<generator_type>::value) {
                // The effective bitwidth of the output range of the PRNG is at least as high as the bitwidth of the
                // output data type of this random distribution facility:
                if constexpr (resultBits <= generatorBits) {
                    DBG1(<< "Best Case: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    unsigned_generator_type x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator()
                                                                                     - uniform_random_number_generator::min());
                    if constexpr (generatorBits != details::log2(generatorRange)) {
                        constexpr unsigned_generator_type bitMask =
                                gcem::pow<unsigned_generator_type, unsigned_generator_type>(2, generatorBits) - 1;
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
                    // The effective bitwidth of the output range of the PRNG is lower than the one of the output data
                    // type of this random distribution facility. But the effective bitwidth of the output range of this
                    // random distribution facility (which is never known at compile-time) might still be lower than the
                    // effective bitwidth of the output range of the PRNG:
                } else {
                    // The effective bitwidth of the output range of the PRNG is at least as high as the effective
                    // bitwidth of the output range of this random distribution facility:
                    if (_rangeBits <= generatorBits) {
                        DBG1(<< "176: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                        unsigned_generator_type x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator()
                                                                                         - uniform_random_number_generator::min());
                        if constexpr (generatorBits != details::log2(generatorRange)) {
                            constexpr unsigned_generator_type bitMask =
                                    gcem::pow<unsigned_generator_type, unsigned_generator_type>(2, generatorBits) - 1;
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
                        // The effective bitwidth of the output range of the PRNG is lower than the effective bitwidth of
                        // the output range of this random distribution facility and therefore multiple random numbers
                        // generated by the underlying PRNG are required to get enough entropy for a biased uniformly
                        // distributed random integer in the output range:
                    } else {
                        DBG1(<< "205: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " > log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                        return _fallbackDistribution(uniformRandomNumberGenerator);
                    }
                }
                // The output range of the underlying PRNG is not known at compile-time or it is a PRNG generating floating
                // point numbers:
            } else {
                unsigned_generator_type generatorRange;
                if constexpr (std::is_integral<generator_type>::value) {
                    generatorRange = gcem::abs(
                            static_cast<unsigned_generator_type>(uniform_random_number_generator::max())
                            - static_cast<unsigned_generator_type>(uniform_random_number_generator::min()));
                } else {

                    generatorRange = gcem::abs(
                            static_cast<unsigned_generator_type>(boost::random::detail::uniform_int_float<
                                    uniform_random_number_generator>::max())
                            - static_cast<unsigned_generator_type>(boost::random::detail::uniform_int_float<
                                    uniform_random_number_generator>::min()));
                }
                uint16_t generatorBits = generatorRange == std::numeric_limits<unsigned_generator_type>::max()
                                         ? static_cast<uint16_t>(sizeof(generator_type) * CHAR_BIT)
                                         : static_cast<uint16_t>(std::floor(std::log2(generatorRange + 1)));

                // The effective bitwidth of the output range of the PRNG is at least as high as the effective bitwidth
                // of the output range of this random distribution facility:
                if (_rangeBits <= generatorBits) {
                    DBG1(<< "223: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " <= log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    unsigned_generator_type x;
                    if constexpr (std::is_integral<generator_type>::value) {
                        x = static_cast<unsigned_generator_type>(uniformRandomNumberGenerator()
                                                                 - uniform_random_number_generator::min());
                        if (generatorBits < generatorTypeBits) {
                            unsigned_generator_type bitMask =
                                    gcem::pow<unsigned_generator_type, unsigned_generator_type>(2, generatorBits) - 1;
                            x = x & bitMask;
                        }
                    } else {
                        boost::random::detail::uniform_int_float<uniform_random_number_generator> floatToInt(
                                uniformRandomNumberGenerator);

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
                    // The effective bitwidth of the output range of the PRNG is lower than the effective bitwidth of the
                    // output range of this random distribution facility and therefore multiple random numbers generated by
                    // the underlying PRNG are required to get enough entropy for a biased uniformly distributed random
                    // integer in the output range:
                } else {
                    DBG1(<< "258: " << typeid(uniformRandomNumberGenerator).name() << " (" << static_cast<uint32_t>(_rangeBits) << " > log2(" << static_cast<uint64_t>(generatorRange) << ") = " << static_cast<uint32_t>(generatorBits) << ")");
                    return _fallbackDistribution(uniformRandomNumberGenerator);
                }
            }
        };

        /*!\fn    operator==(const biased_uniform_int_distribution<result_type>& distribution0,
         *                   const biased_uniform_int_distribution<result_type>& distribution1) noexcept
         * \brief Compares two biased uniform random integer distribution facilities for equality
         *
         * @param distribution0 A random distribution facility
         * @param distribution1 Another random distribution facility
         * @return              This is \c true iff lower and upper limits of both random distribution facilities are
         *                      equal, otherwise this is \c false .
         */
        friend bool operator==(const biased_uniform_int_distribution<result_type>& distribution0,
                               const biased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return distribution0._parameters == distribution1._parameters;
        };

        /*!\fn    operator!=(const biased_uniform_int_distribution<result_type>& distribution0,
         *                   const biased_uniform_int_distribution<result_type>& distribution1) noexcept
         * \brief Compares two biased uniform random integer distribution facilities for inequality
         *
         * @param distribution0 A random distribution facility
         * @param distribution1 Another random distribution facility
         * @return              This is \c true iff the two random distribution facilities are not equal according to
         *                      \link operator==() \endlink , otherwise this is \c false .
         */
        friend bool operator!=(const biased_uniform_int_distribution<result_type>& distribution0,
                               const biased_uniform_int_distribution<result_type>& distribution1) noexcept {
            return !(distribution0 == distribution1);
        };

        /*!\fn      operator<<(std::basic_ostream<CharT, Traits>& outputStream,
         *                     const biased_uniform_int_distribution<result_type>& distribution)
         * \brief   Serializes the given random distribution facility to the given stream
         * \details Serializes the given biased uniform random integer distribution facility to the given output stream
         *          by printing the required parameters (lower and upper limit of the output range) to the given stream.
         *
         * @tparam CharT
         * @tparam Traits
         * @tparam result_type
         * @param outputStream  The output stream to serialize the given random distribution facility to.
         * @param distribution  The random distribution facility to serialize to the given output stream.
         * @return              The used output stream
         */
        template<class CharT, class Traits, class result_type>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& outputStream,
                                                             const biased_uniform_int_distribution<
                                                                     result_type>& distribution) {
            outputStream << distribution._parameters._a << "<" << distribution._parameters._b << std::endl;

            return outputStream;
        };

        /*!\fn      operator>>(std::basic_istream<CharT, Traits>& inputStream,
         *                     const biased_uniform_int_distribution<result_type>& distribution)
         * \brief   Deserializes a random distribution facility from the given stream and apply its parameters to the
         *          given random distribution facility
         * \details Deserializes a biased uniform random integer distribution facility from the given output stream by
         *          reading the required parameters (lower and upper limit of the output range) from the given stream
         *          and applying those to the given random distribution facility.
         *
         * @tparam CharT
         * @tparam Traits
         * @tparam result_type
         * @param inputStream   The input stream from which to deserialize the given random distribution facility.
         * @param distribution  The random distribution facility to deserialize the serialized random distribution
         *                      facility into.
         * @return              The used input stream
         */
        template<class CharT, class Traits, class result_type>
        friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& inputStream,
                                                             const biased_uniform_int_distribution<
                                                                     result_type>& distribution) {
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
        /*!\var   _parameters
         * \brief The configuration of this random distribution facility
         */
        param_type _parameters;

        /*!\var     _offset
         * \brief   The lower limit of output range of this random distribution facility
         * \details In a first step, a random number in the range from 0 to \link _range \endlink is calculated and
         *          afterwards this is shifted by this offset.
         */
        int_type _offset;

        /*!\var     _range
         * \brief   The length of the output range of this random distribution facility
         * \details In a first step, a random number in the range from 0 to this value is calculated and afterwards this
         *          is shifted by the \link _offset \endlink .
         */
        int_type _range;

        /*!\var     _rangeBits
         * \brief   The number of bits required to differentiate all the values in the output range of this random
         *          distribution facility
         * \details This is equivalent to the bitwidth of the entropy required to generate one biased uniformly
         *          distributed random integer.
         */
        uint16_t _rangeBits;

        /*!\var     _fallbackDistribution
         * \brief   The fallback random distribution facility
         * \details This is used when the entropy of one random number generated using the underlying PRNG is too low
         *          for the output range of this random distribution resulting in the need to collect entropy generating
         *          multiple random numbers using the underlying PRNG.
         */
        boost::random::uniform_int_distribution<int_type> _fallbackDistribution;
    };
} // zero::uniform_int_distribution

#endif // __UNIFORM_INT_DISTRIBUTION_HPP
