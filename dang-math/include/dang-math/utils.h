#pragma once

#include <array>
#include <cstddef>
#include <optional>

namespace dang::math
{

/// <summary>The value of Pi.</summary>
constexpr double pi = 3.14159265358979323846;

/// <summary>Converts from radians to degrees using pi.</summary>
template <typename T>
constexpr T radToDeg(T value)
{
    return value * T(180) / T(pi);
}

/// <summary>Converts from degrees to radians using pi.</summary>
template <typename T>
constexpr T degToRad(T value)
{
    return value * T(pi) / T(180);
}

}

namespace dmath = dang::math;
