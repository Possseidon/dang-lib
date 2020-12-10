#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <type_traits>

#include "dang-utils/utils.h"

namespace dang::math {

namespace dutils = dang::utils;

// TODO: C++20 use math constants
/// <summary>The value of Pi.</summary>
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T pi_v = static_cast<T>(3.141592653589793);

/// <summary>Converts from radians to degrees by multiplying by 180 and dividing by pi.</summary>
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T degrees(T value)
{
    return value * T{180} / pi_v<T>;
}

/// <summary>Converts from degrees to radians by multiplying by pi and dividing by 180.</summary>
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T radians(T value)
{
    return value * pi_v<T> / T{180};
}

} // namespace dang::math

namespace dmath = dang::math;
