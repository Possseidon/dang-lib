#pragma once

#include "dang-math/utils.h"

namespace dang::math {

/// <summary>Linearly interpolates between two values.</summary>
template <typename T, typename TFactor>
constexpr T interpolate(const T& from, const T& to, TFactor factor)
{
    return from + factor * (to - from);
}

namespace interp {

/// <summary>Simple identity function.</summary>
auto linear = [](auto x) { return x; };

/// <summary>Starts slow, ends fast.</summary>
auto quadratic = [](auto x) { return x * x; };
/// <summary>Starts fast, ends slow.</summary>
auto inv_quadratic = [](auto x) { return x * (2 - x); };
/// <summary>Starts slow, ends slow.</summary>
auto cubic = [](auto x) { return x * x * (3 - 2 * x); };

/// <summary>True exponential interpolation, quadratic is faster and usually sufficient.</summary>
auto exp = [](auto x) { return x * std::exp(x) / std::exp(1); };
/// <summary>True inverse-exponential interpolation, inv_quadratic is faster and usually sufficient.</summary>
auto inv_exp = [](auto x) { return 1 - exp(1 - x); };
/// <summary>True cosine interpolation, cubic is faster and usually sufficient.</summary>
auto cosine = [](auto x) { return (1 - std::cos(x * pi_v<decltype(x)>)) / 2; };

/// <summary>Alias for cubic.</summary>
auto smooth = cubic;
/// <summary>Alias for quadtratic.</summary>
auto smooth_start = quadratic;
/// <summary>Alias for inv_quadtratic.</summary>
auto smooth_end = inv_quadratic;

} // namespace interp

} // namespace dang::math
