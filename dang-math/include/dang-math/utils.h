#pragma once

#include <array>
#include <functional>
#include <optional>
#include <type_traits>

namespace dang::math
{

/// <summary>The value of Pi.</summary>
constexpr double pi = 3.14159265358979323846;

/// <summary>Converts from radians to degrees using pi.</summary>
template <typename T>
constexpr T radToDeg(T value)
{
    return value * static_cast<T>(180) / static_cast<T>(pi);
}

/// <summary>Converts from degrees to radians using pi.</summary>
template <typename T>
constexpr T degToRad(T value)
{
    return value * static_cast<T>(pi) / static_cast<T>(180);
}

}

namespace dmath = dang::math;
