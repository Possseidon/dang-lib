#pragma once

#include "dang-math/global.h"
#include "dang-math/utils.h"

namespace dang::math {

/// @brief Linearly interpolates between two values.
template <typename T, typename TFactor>
constexpr T interpolate(const T& from, const T& to, TFactor factor)
{
    return from + factor * (to - from);
}

namespace interp {

/// @brief Simple identity function.
auto linear = [](auto x) { return x; };

/// @brief Starts slow, ends fast.
auto quadratic = [](auto x) { return x * x; };
/// @brief Starts fast, ends slow.
auto inv_quadratic = [](auto x) { return x * (2 - x); };
/// @brief Starts slow, ends slow.
auto cubic = [](auto x) { return x * x * (3 - 2 * x); };

/// @brief True exponential interpolation, quadratic is faster and usually sufficient.
auto exp = [](auto x) { return x * std::exp(x) / std::exp(1); };
/// @brief True inverse-exponential interpolation, inv_quadratic is faster and usually sufficient.
auto inv_exp = [](auto x) { return 1 - exp(1 - x); };
/// @brief True cosine interpolation, cubic is faster and usually sufficient.
auto cosine = [](auto x) { return (1 - std::cos(x * pi_v<decltype(x)>)) / 2; };

/// @brief Alias for cubic.
auto smooth = cubic;
/// @brief Alias for quadtratic.
auto smooth_start = quadratic;
/// @brief Alias for inv_quadtratic.
auto smooth_end = inv_quadratic;

} // namespace interp

} // namespace dang::math
