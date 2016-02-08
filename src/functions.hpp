#ifndef AUDI_FUNCTIONS_HPP
#define AUDI_FUNCTIONS_HPP

#include <boost/math/special_functions/bernoulli.hpp>
#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <piranha/binomial.hpp>
#include <stdexcept>

#include "gdual.hpp"



namespace audi
{

/// Overload for the exponential
/**
 * Implements the exponential of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\exp f)} = \exp f_0 \sum_{i=0}^m \frac{\hat f^i}{i!} = \exp f_0 \left( 1 + \hat f + \frac {\hat f^2}{2!} + ... \right)
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the exponential of \p d
*/
inline gdual exp(const gdual &d)
{
    gdual retval(1., d.get_order());
    double fact=1;
    auto p0 = d.constant_cf();
    auto phat = d - p0;
    gdual tmp(phat);

    retval+=phat;
    for (auto i = 2u; i <= d.get_order(); ++i) {
        phat*=tmp;
        fact*=i;
        retval+=phat / fact;
    }
    return retval * std::exp(p0);
}

/// Overload for the logarithm
/**
 * Implements the logarithm of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\log f)} = \log f_0 + \sum_{i=1}^m (-1)^{i+1} \frac 1i \left(\frac{\hat f}{f_0}\right)^i = \log f_0 + \frac{\hat f}{f_0} - \frac 12 \left(\frac{\hat f}{f_0}\right)^2 + ...
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the logarithm of \p d
 *
 */
inline gdual log(const gdual &d)
{
    gdual retval(0., d.get_order());
    double fatt = 1;
    auto p0 = d.constant_cf();
    auto log_p0 = std::log(p0);

    auto phat = (d - p0);
    phat = phat/p0;
    gdual tmp(phat);

    retval = log_p0 + phat;
    for (auto i = 2u; i <= d.get_order(); ++i) {
        fatt *= -1;
        phat*=tmp;
        retval =  retval + fatt * phat / i;
    }
    return retval;
}

/// Overload for the exponentiation to a gdual power
/**
 * Computes the exponentiation to the power of an audi::gdual. 
 * If the exponent is an integer constant, it calls the std::pow overload. Otherwise
 * it converts \f$a^{T_f}\f$ to \f$\exp(T_g*\log(a))\f$ and computes this
 * last expression instead.
 *
 * @param[in] base the base for the exponent
 * @param[in] d audi::gdual argument
 *
 */
inline gdual pow(double base, const gdual &d)
{
    double int_part;
    // checks wether the exponent is an integer, in which case
    // it calls a different overload
    if (d.degree() == 0) {
        auto p0 = d.constant_cf();
        double float_part = std::modf(p0, &int_part);
        if (float_part == 0.) {
            return gdual(std::pow(base, p0), d.get_order()); //nan is possible here
        }
    }
    return exp(std::log(base) * d);
}

/// Overload for the exponentiation
/**
 * Implements the exponentiation of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(f^\alpha)} = f_0^\alpha \sum_{k=0}^m {\alpha \choose k} \left(\hat f / f_0\right)^k
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 * @param[in] alpha exponent
 *
 * @return an audi:gdual containing the Taylor expansion of \p d elevated to the power \p alpha
 *
 */
inline gdual pow(const gdual &d, double alpha)
{
    // We check if the exponent is representable as a positive integer, 
    // in which case we just do d*d*d*d*... etc.
    // This is also a workaround to the issue (https://github.com/darioizzo/audi/issues/6) 
    // TODO: is there a better way to do this? Calling the pow (gdual, int) overload is not possible as it
    // cannot be moved upfront.
    double n;
    if (modf(alpha, &n) == 0.0 && n > 0) {
        gdual retval(d);
        for (auto i = 1; i < (int)n; ++i) {
            retval*=d;
        }
        return retval;
    }
    auto p0 = d.constant_cf();
    auto phat = d - p0;
    gdual retval(std::pow(p0, alpha), d.get_order());
    phat = phat;
    gdual tmp(phat);

    retval+=alpha * phat * std::pow(p0, alpha-1);
    for (auto i = 2u; i <= d.get_order(); ++i) {
        phat*=tmp;
        retval+=piranha::math::binomial(alpha,i) * phat * std::pow(p0, alpha-i);
    }
    return retval;
}

// Its important this comes after the pow(gdual, double) overload
/// Overload for the integer exponentiation
/**
 * Implements the integer exponentiation of a audi::gdual. Essentially,
 * it uses the \f$\mathcal P_{n,m}\f$ multiplication on \p d \p n times
 *
 * @param[in] d audi::gdual argument
 * @param[in] n integer exponent
*/
inline gdual pow(const gdual& d, int n)
{
    if (n <= 0) { 
        return pow(d, (double)n);
    }
    gdual retval(d);
    for (auto i = 1; i < n; ++i) {
        retval*=d;
    }
    return retval;
}

/// Overload for the exponentiation of a gdual to a gdual power
/**
 * Computes the exponentiation of an audi::gdual to the power of an audi::gdual. 
 * If the exponent is an integer constant, it calls a different overload. Otherwise
 * it converts \f$T_f^{T_g}\f$ to \f$\exp(T_g*\log(T_f))\f$ and computes this
 * last expression instead.
 *
 * @param[in] d1 audi::gdual argument
 * @param[in] d2 audi::gdual argument
 *
 * @throw std::domain_error if std::log(\f$f_0\f$) is not finite (uses std::isfinite)
*/
inline gdual pow(const gdual &d1, const gdual &d2)
{
    double int_part;
    // checks wether the exponent is an integer, in which case it calls 
    // a different overload
    if (d2.degree() == 0) {
        auto p0 = d2.constant_cf();
        double float_part = std::modf(p0, &int_part);
        if (float_part == 0.) {
            return pow(d1, p0);
        }
    }
    return exp(d2 * log(d1));
}

/// Overload for the square root
/**
 * Implements the square root of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{\sqrt{f}} = \sqrt{f_0} \sum_{k=0}^m {\frac 12 \choose k} \left(\hat f / f_0\right)^k
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the square root of \p d
 *
 */
inline gdual sqrt(const gdual &d)
{
    return pow(d, 0.5); // TODO: subsitute this by similar code to cbrt?
}

/// Overload for the cubic root
/**
 * Implements the cubic root of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{\sqrt[3]{f}} = \sqrt[3]{f_0} \sum_{k=0}^m {\frac 13 \choose k} \left(\hat f / f_0\right)^k
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the square root of \p d
 *
 */
inline gdual cbrt(const gdual& d)
{
    double alpha = 1/3.;
    gdual retval(1., d.get_order());
    auto p0 = d.constant_cf();
    double cbrt_p0 = std::cbrt(p0);

    auto phat = d - p0;
    phat = phat/p0;
    gdual tmp(phat);

    retval+=alpha * phat;
    for (auto i = 2u; i <= d.get_order(); ++i) {
        phat*=tmp;
        retval+=piranha::math::binomial(alpha,i) * phat;
    }
    retval*=cbrt_p0;
    return retval;
}

/// Overload for the sine
/**
 * Implements the sine of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\sin f)} = \sin f_0 \left(\sum_{i=0}^{2i\le m} (-1)^{i} \frac{\hat f^{2i}}{(2i)!}\right) + \cos f_0 \left(\sum_{i=0}^{(2i+1)\le m} (-1)^{i} \frac{\hat f^{2i+1}}{(2i+1)!}\right) \\
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the sine of \p d
*/
inline gdual sin(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sin_p0 = std::sin(p0);
    double cos_p0 = std::cos(p0);

    double factorial=1.;
    double coeff=1.;
    gdual cos_taylor(1., d.get_order());
    gdual tmp(cos_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                             // -1, 1, -1, 1, ...
        tmp*=phat2;                             // phat^2, phat^4, phat^6 ...
        factorial*=i * (i-1.);                   // 2!, 4!, 6!, ...
        cos_taylor += (coeff/factorial) * tmp;
    }

    factorial=1.;
    coeff=1.;
    gdual sin_taylor(phat);
    tmp = sin_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                             // -1, 1, -1, 1, ...
        tmp*=phat2;                             // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                   // 3!, 5!, 7!, ...
        sin_taylor += (coeff/factorial) * tmp;
    }
    return (sin_p0 * cos_taylor + cos_p0 * sin_taylor);
}

/// Overload for the cosine
/**
 * Implements the cosine of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\cos f)} = \cos f_0 \left(\sum_{i=0}^{2i\le m} (-1)^{i} \frac{\hat f^{2i}}{(2i)!}\right) - \sin f_0 \left(\sum_{i=0}^{(2i+1)\le m} (-1)^{i} \frac{\hat f^{2i+1}}{(2i+1)!}\right)
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the cosine of \p d
*/
inline gdual cos(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sin_p0 = std::sin(p0);
    double cos_p0 = std::cos(p0);

    double factorial=1.;
    double coeff=1.;
    gdual cos_taylor(1., d.get_order());
    gdual tmp(cos_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                              // -1, 1, -1, 1, ...
        tmp*=phat2;                              // phat^2, phat^4, phat^6 ...
        factorial*=i * (i-1.);                    // 2!, 4!, 6!, ...
        cos_taylor += (coeff/factorial) * tmp;
    }

    factorial=1.;
    coeff=1.;
    gdual sin_taylor(phat);
    tmp = sin_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                              // -1, 1, -1, 1, ...
        tmp*=phat2;                              // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                    // 3!, 5!, 7!, ...
        sin_taylor += (coeff/factorial) * tmp;
    }
    return (cos_p0 * cos_taylor - sin_p0 * sin_taylor);
}

/// Computes both sine and cosine
/**
 * As most of the computations for the sine and cosine is the same, it is twice as fast
 * to get both sine and cosine at once rather than computing them in sequence.
 * Use this function when both sine and cosine are needed.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an std::array containing the Taylor expansions of sine and the cosine (first element, second element)
 *
*/
std::array<gdual,2> sin_and_cos(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sin_p0 = std::sin(p0);
    double cos_p0 = std::cos(p0);

    double factorial=1.;
    double coeff=1.;
    gdual cos_taylor(1., d.get_order());
    gdual tmp(cos_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                             // -1, 1, -1, 1, ...
        tmp*=phat2;                             // phat^2, phat^4, phat^6 ...
        factorial *= i * (i-1.);                   // 2!, 4!, 6!, ...
        cos_taylor += (coeff/factorial) * tmp;
    }

    factorial=1.;
    coeff=1.;
    gdual sin_taylor(phat);
    tmp = sin_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        coeff*=-1.;                             // -1, 1, -1, 1, ...
        tmp*=phat2;                             // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                   // 3!, 5!, 7!, ...
        sin_taylor += (coeff/factorial) * tmp;
    }
    auto sine = sin_p0 * cos_taylor + cos_p0 * sin_taylor;
    auto cosine = cos_p0 * cos_taylor - sin_p0 * sin_taylor;
    return std::array<gdual,2>{{std::move(sine), std::move(cosine)}};
}

/// Overload for the tangent
/**
 * Implements the tangent of a audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\tan f)} = \frac{\tan f_0 + \sum_{k=1}^{k \le 2k+1} B_{2k} \frac{(-4)^k(1-4^k)}{2k!}x^{2k - 1}}{1 - \tan f_0 \sum_{k=1}^{k \le 2k+1} \frac{B_{2k}(-4)^k(1-4^k)}{2k!}x^{2k - 1} }
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$ and \f$ B_{2k}\f$ are the Bernoulli numbers.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the tangent of \p d
 *
*/
inline gdual tan(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;
    double tan_p0 = std::tan(p0);

    // Pre-compute Bernoulli numbers.
    std::vector<double> bn;
    boost::math::bernoulli_b2n<double>(0, (d.get_order() + 1) / 2 + 1, std::back_inserter(bn)); // Fill vector with even Bernoulli numbers.

    gdual tan_taylor = phat;
    // Factors
    double factorial=24.;
    double four_k = 16.;
    for (auto k=2u; 2 * k - 1 <= d.get_order(); ++k)
    {
        phat*=phat2;
        tan_taylor += bn[k] * four_k * (1 - std::abs(four_k)) / factorial * phat;
        four_k*=-4.;
        factorial*=(2. * k + 1.) * (2. * k + 2.);
    }
    return (tan_p0 + tan_taylor) / (1. - tan_p0 * tan_taylor);
}

/// Overload for the hyperbolic sine
/**
 * Implements the hyperbolic sine of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\sin f)} = \sinh f_0 \left(\sum_{i=0}^{2i\le m} \frac{\hat f^{2i}}{(2i)!}\right) + \cosh f_0 \left(\sum_{i=0}^{(2i+1)\le m} \frac{\hat f^{2i+1}}{(2i+1)!}\right) \\
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the hyperbolic sine of \p d
*/
inline gdual sinh(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sinh_p0 = std::sinh(p0);
    double cosh_p0 = std::cosh(p0);

    double factorial=1.;
    gdual cosh_taylor(1., d.get_order());
    gdual tmp(cosh_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^2, phat^4, phat^6 ...
        factorial*= i * (i-1.);                  // 2!, 4!, 6!, ...
        cosh_taylor +=  tmp / factorial;
    }

    factorial=1.;
    gdual sinh_taylor(phat);
    tmp = sinh_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                   // 3!, 5!, 7!, ...
        sinh_taylor += tmp / factorial;
    }
    return (sinh_p0 * cosh_taylor + cosh_p0 * sinh_taylor);
}

/// Overload for the hyperbolic cosine
/**
 * Implements the hyperbolic cosine of a audi::gdual. 
 * Essentially it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\sin f)} = \cosh f_0 \left(\sum_{i=0}^{2i\le m} \frac{\hat f^{2i}}{(2i)!}\right) + \sinh f_0 \left(\sum_{i=0}^{(2i+1)\le m} \frac{\hat f^{2i+1}}{(2i+1)!}\right) \\
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the hyperbolic cosine of \p d
*/
inline gdual cosh(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sinh_p0 = std::sinh(p0);
    double cosh_p0 = std::cosh(p0);

    double factorial=1.;
    gdual cosh_taylor(1., d.get_order());
    gdual tmp(cosh_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^2, phat^4, phat^6 ...
        factorial*= i * (i-1.);                  // 2!, 4!, 6!, ...
        cosh_taylor +=  tmp / factorial;
    }

    factorial=1.;
    gdual sinh_taylor(phat);
    tmp = sinh_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                   // 3!, 5!, 7!, ...
        sinh_taylor += tmp / factorial;
    }
    return (cosh_p0 * cosh_taylor + sinh_p0 * sinh_taylor);
}

/// Computes both the hyperbolic sine and the hyperbolic cosine
/**
 * As most of the computations for the hyperbolic sine and hyperbolic cosine is the same, it is twice as fast
 * to get them both at once rather than computing them in sequence.
 * Use this function when both the hyperbolic sine and the hyperbolic cosine are needed.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an std::array containing the Taylor expansions of hyperbolic sine and cosine (first element, second element)
 *
*/
std::array<gdual,2> sinh_and_cosh(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;

    double sinh_p0 = std::sinh(p0);
    double cosh_p0 = std::cosh(p0);

    double factorial=1.;
    gdual cosh_taylor(1.);
    gdual tmp(cosh_taylor);
    for (auto i=2u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^2, phat^4, phat^6 ...
        factorial*=i * (i-1.);                   // 2!, 4!, 6!, ...
        cosh_taylor += tmp / factorial;
    }

    factorial=1.;
    gdual sinh_taylor(phat);
    tmp = sinh_taylor;
    for (auto i=3u; i<=d.get_order(); i+=2) {
        tmp*=phat2;                             // phat^3, phat^5, phat^7 ...
        factorial*=i * (i-1.);                   // 3!, 5!, 7!, ...
        sinh_taylor += tmp / factorial;
    }
    auto sineh = sinh_p0 * cosh_taylor + cosh_p0 * sinh_taylor;
    auto cosineh = cosh_p0 * cosh_taylor + sinh_p0 * sinh_taylor;
    return std::array<gdual,2>{{std::move(sineh), std::move(cosineh)}};
}

/// Overload for the hyperbolic tangent
/**
 * Implements the hyperbolic tangent of a audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\tan f)} = \frac{\tanh f_0 + \sum_{k=1}^{k \le 2k+1} B_{2k} \frac{4^k(4^k-1)}{2k!}x^{2k - 1}}{1 + \tanh f_0 \sum_{k=1}^{k \le 2k+1} \frac{B_{2k}4^k(4^k-1)}{2k!}x^{2k - 1} }
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$ and \f$ B_{2k}\f$ are the Bernoulli numbers.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the hyperbolic tangent of \p d
 *
*/
inline gdual tanh(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto phat2 = phat * phat;
    double tanh_p0 = std::tanh(p0);

    // Pre-compute Bernoulli numbers.
    std::vector<double> bn;
    boost::math::bernoulli_b2n<double>(0, (d.get_order() + 1) / 2 + 1, std::back_inserter(bn)); // Fill vector with even Bernoulli numbers.

    gdual tanh_taylor = phat;
    // Factors
    double factorial=24.;
    double four_k = 16.;
    for (auto k=2u; 2 * k - 1 <= d.get_order(); ++k) {
        phat*=phat2;
        tanh_taylor += bn[k] * four_k * (four_k - 1.) / factorial * phat;
        four_k*=4.;
        factorial*=(2 * k + 1.) * (2 * k + 2.);
    }
    return (tanh_p0 + tanh_taylor) / (1. + tanh_p0 * tanh_taylor);
}


/// Overload for the inverse hyperbolic tangent
/**
 * Implements the inverse hyperbolic tangent of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{atanh} f)} =  \mbox{atanh} f_0 +\frac 12 \sum_{k=1}^m \left(\frac{1}{(1-f_0)^k} + \frac{(-1)^{k+1}}{(1+f_0)^k}\right) \frac {\hat f^k}{k}
 * \f]
 *
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse hyperbolic tangent of \p d
 *
*/
inline gdual atanh(const gdual& d)
{
    auto p0 = d.constant_cf();
    auto phat = (d - p0);
    auto powphat(phat);
    double atanh_p0 = std::atanh(p0);

    gdual retval(0.);
    double coeff = 1;

    for (auto k=1u; k <= d.get_order(); ++k) {
        double add = (1. / std::pow(1 - p0, k) + coeff / std::pow(1 + p0, k)) / k;
        retval += add * powphat;
        coeff*=-1;
        powphat*=phat;
    }
    return atanh_p0 + 0.5 * retval;
}

/// Overload for the inverse tangent
/**
 * Implements the inverse tangent of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{atan} f)} =  \mbox{atan} f_0 + \sum_{k=1}^{2k-1\le m} \left(\frac{1 + \sum_{j=1}^{2j\le 2k-1} {2k-1 \choose 2j} f_0^{2j}(-1)^j}{(1+f_0^2)^{2k-1}}\right) \frac {\hat f^{2k-1}}{2k-1}(-1)^{k+1} + \sum_{k=1}^{2k\le m} \left(\frac{\sum_{j=1}^{2j-1\le 2k} {2k \choose 2j-1} f_0^{2j-1}(-1)^{j+1}}{(1+f_0^2)^{2k}}\right) \frac {\hat f^{2k}}{2k}(-1)^k
 * \f]
 *
 * This formula derives directly from the formula for audi::atanh noting that: \f$ \mbox{atan}(z) = i \mbox{atanh}(-iz)\f$
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse tangent of \p d
 *
*/
inline gdual atan(const gdual& d)
{
    double p0 = d.constant_cf();
    auto phat = (d - p0) / (1. + p0*p0);
    auto powphat(phat);

    gdual retval(std::atan(p0));
    double coeff1 = 1.;
    double coeff2 = -1.;

    for (auto k=1u; k <= d.get_order(); ++k) {
        if (k % 2) {        // This is for odd powers 1..3..5
            double binom = 1.;
            double f0 = p0 * p0;
            double cf_i = -1.;
            for (auto j = 1u; 2*j <= k; ++j) {
                binom += piranha::math::binomial(k, 2*j) * f0 * cf_i;
                f0 *= p0 * p0;
                cf_i *= -1.;
            }
            retval += binom * powphat * coeff1 / k;
            coeff1 *= -1.;
        } else {            //This is for even powers 2..4..6
            double binom = 0.;
            double f0 = p0;
            double cf_i = 1.;
            for (auto j = 1u; 2*j - 1 <= k; ++j) {
                binom += piranha::math::binomial(k, 2*j - 1) * f0 * cf_i;
                f0 *= p0 * p0;
                cf_i *= -1;
            }
            retval += binom * powphat * coeff2 / k;
            coeff2 *= -1;
        }
        powphat*=phat;
    }
    return retval;
}

/// Overload for the inverse hyperbolic sine
/**
 * Implements the inverse inverse hyperbolic sine of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{asinh} f)} = T_{\left(\log\left(f + \sqrt{1 + f^2}\right)\right)}
 * \f]
 *
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse hyperbolic sine of \p d
 *
*/
inline gdual asinh(const gdual& d)
{
    return log(d + sqrt(1. + d*d));
}

/// Overload for the inverse hyperbolic cosine
/**
 * Implements the inverse inverse hyperbolic cosine of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{acosh} f)} = T_{\left(\log\left(f + \sqrt{f^2 - 1}\right)\right)}
 * \f]
 *
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse hyperbolic cosine of \p d
 *
*/
inline gdual acosh(const gdual& d)
{
    return log(d + sqrt(d*d - 1.));
}

/// Overload for the inverse sine
/**
 * Implements the inverse inverse sine of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{asin} f)} = T_{\left(\mbox{atan} \left(f / \sqrt{1 - f^2}\right)\right)}
 * \f]
 *
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse sine of \p d
 *
*/
inline gdual asin(const gdual& d)
{
    return atan(d / sqrt(1. - d*d));
}

/// Overload for the inverse cosine
/**
 * Implements the inverse inverse cosine of an audi::gdual. 
 * Essentially, it performs the following computations in the \f$\mathcal P_{n,m}\f$
 * algebra:
 *
 * \f[
 * T_{(\mbox{acos} f)} = T_{\left(\mbox{atan} \left(\sqrt{1 - f^2} / f\right)\right)}
 * \f]
 *
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi::gdual containing the Taylor expansion of the inverse cosine of \p d
 *
*/
inline gdual acos(const gdual& d)
{
    return 0.5 * boost::math::constants::pi<double>() - atan(d / sqrt(1. - d*d));
}

/// Overload for the absolute value
/**
 * Implements the absolute value of a audi::gdual. 
 * Essentially, it inverts the sign of \f$T_f\f$ as follows:
 *
 * \f[
 * T_{(\mbox{abs} f)} = \left\{ \begin{array}{ll} T_f & f_0 \ge 0 \\ -T_f & f_0 < 0 \end{array} \right.
 * \f]
 *
 * where \f$T_f = f_0 + \hat f\f$. 
 *
 * \note If \f$f_0\f$ is zero, the right Taylor expansion will be returned rather than nans.
 *
 * @param[in] d audi::gdual argument
 *
 * @return an audi:gdual containing the Taylor expansion of the absoute value of \p d
 *
*/
inline gdual abs(const gdual& d)
{
    auto p0 = d.constant_cf();
    if (p0 >= 0) {
        return d;
    } 
    return -d;
}


} // end of namespace audi 

#endif


