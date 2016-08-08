#pragma once

namespace dm {

// constexpr pow function
// http://stackoverflow.com/a/16443849
template <class T>
constexpr T pow(const T base, unsigned const exponent) {
    return (exponent == 0) ? 1 : (base * pow(base, exponent - 1));
}
template <typename T, T base, unsigned exponent>
using pow_ = std::integral_constant<T, pow(base, exponent)>;
}
