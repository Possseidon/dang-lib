#pragma once

#include <cstddef>
#include <array>

namespace dang::math
{

constexpr double pi = 3.14159265358979323846;

template <typename T>
T radToDeg(T value)
{
    return value * T(180) / T(pi);
}

template <typename T>
T degToRad(T value)
{
    return value * T(pi) / T(180);
}

}

namespace dmath = dang::math;
