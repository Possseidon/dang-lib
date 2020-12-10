#pragma once

#include "dang-math/global.h"

namespace dang::math {

// TODO: C++20 use math constants
/// @brief The value of Pi.
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T pi_v = static_cast<T>(3.141592653589793);

/// @brief Converts from radians to degrees by multiplying by 180 and dividing by pi.
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T degrees(T value)
{
    return value * T{180} / pi_v<T>;
}

/// @brief Converts from degrees to radians by multiplying by pi and dividing by 180.
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T radians(T value)
{
    return value * pi_v<T> / T{180};
}

} // namespace dang::math
