#pragma once

#include "dang-math/vector.h"
#include "dang-utils/utils.h"

namespace dang::math::noise {

// Converted from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

/// @brief A permutation seed, which is used by simplex noise generation.
/// @remark Values wrap around in the range [0..289) and should be integers
struct PermuteFactors {
    float f1 = 1.0f;
    float f2 = 34.0f;

    /// @brief Selects one of the 82944 distinct factors from the given integral seed.
    template <typename T>
    static PermuteFactors fromSeed(T seed)
    {
        static_assert(std::is_integral_v<T>);
        auto seed_unsigned = static_cast<std::make_unsigned_t<T>>(seed);
        auto x = dutils::removeOddBits(seed_unsigned);
        auto y = dutils::removeOddBits(seed_unsigned >> 1);
        auto a = 1 + x % 288;
        auto b = 1 + y % 288;
        return {static_cast<float>(a), static_cast<float>(b)};
    }
};

namespace detail {

template <std::size_t v_dim>
[[nodiscard]] constexpr vec<v_dim> permute(vec<v_dim> x, const PermuteFactors& factors)
{
    return ((x * factors.f2 + factors.f1) * x).mod(289.0f);
}

[[nodiscard]] constexpr float permute(float x, const PermuteFactors& factors) { return permute(vec1(x), factors).x(); }

template <std::size_t v_dim>
[[nodiscard]] constexpr vec<v_dim> taylorInvSqrt(vec<v_dim> r)
{
    return 1.79284291400159f - 0.85373472095314f * r;
}

[[nodiscard]] constexpr float taylorInvSqrt(float x) { return taylorInvSqrt(vec1(x)).x(); }

[[nodiscard]] constexpr vec4 grad4(float j, vec4 ip)
{
    constexpr vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
    vec4 p, s;

    p.set_xyz(((vec3(j) * ip.xyz()).fract() * 7.0).floor() * ip.z() - 1.0);
    p.w() = 1.5f - p.xyz().abs().dot(ones.xyz());
    s = vec4(p.lessThan(vec4(0.0)));
    p.set_xyz(p.xyz() + (s.xyz() * 2.0 - 1.0) * s.w());

    return p;
}

} // namespace detail

[[nodiscard]] constexpr float simplex(vec2 v, const PermuteFactors& factors = {})
{
    auto permute = [&](auto x) { return detail::permute(x, factors); };

    constexpr vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);
    vec2 i = (v + v.dot(C.y())).floor();
    vec2 x0 = v - i + i.dot(C.x());
    vec2 i1 = x0.x() > x0.y() ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);
    vec4 x12 = x0.swizzle<0, 1, 0, 1>() + C.swizzle<0, 0, 2, 2>();
    x12.set_xy(x12.xy() - i1);
    i = i.mod(289.0f);
    vec3 p = permute(permute(i.y() + vec3(0.0f, i1.y(), 1.0f)) + i.x() + vec3(0.0f, i1.x(), 1.0f));
    vec3 m = (0.5f - vec3(x0.sqrdot(), x12.xy().sqrdot(), x12.zw().sqrdot())).max(0.0f);
    m *= m * m * m;
    vec3 x = 2.0f * (p * C.w()).fract() - 1.0f;
    vec3 h = x.abs() - 0.5f;
    vec3 ox = (x + 0.5f).floor();
    vec3 a0 = x - ox;
    m *= detail::taylorInvSqrt(a0 * a0 + h * h);
    vec3 g;
    g.x() = a0.x() * x0.x() + h.x() * x0.y();
    g.set_yz(a0.yz() * x12.xz() + h.yz() * x12.yw());
    return 130.0f * m.dot(g);
}

[[nodiscard]] constexpr float simplex(vec3 v, const PermuteFactors& factors = {})
{
    auto permute = [&](auto x) { return detail::permute(x, factors); };

    constexpr vec2 C = vec2(1.0f / 6.0f, 1.0f / 3.0f);
    constexpr vec4 D = vec4(0.0f, 0.5f, 1.0f, 2.0f);

    vec3 i = (v + v.dot(C.y())).floor();
    vec3 x0 = v - i + i.dot(C.x());

    // vec3 g = x0.yzx().step(x0);
    vec3 g = x0.swizzle<1, 2, 2>().step(x0.swizzle<0, 1, 0>());
    g.z() = 1.0f - g.z(); // Ugly fix

    vec3 l = 1.0f - g;
    vec3 i1 = g.min(l.zxy());
    vec3 i2 = g.max(l.zxy());

    vec3 x1 = x0 - i1 + 1.0f * C.x();
    vec3 x2 = x0 - i2 + 2.0f * C.x();
    vec3 x3 = x0 - 1.0f + 3.0f * C.x();

    i = i.mod(289.0f);
    vec4 p =
        permute(permute(permute(i.z() + vec4(0.0f, i1.z(), i2.z(), 1.0f)) + i.y() + vec4(0.0f, i1.y(), i2.y(), 1.0f)) +
                i.x() + vec4(0.0f, i1.x(), i2.x(), 1.0f));

    float n_ = 1.0f / 7.0f;
    vec3 ns = n_ * D.wyz() - D.swizzle<0, 2, 0>();

    vec4 j = p - 49.0f * (p * ns.z() * ns.z()).floor();

    vec4 x_ = (j * ns.z()).floor();
    vec4 y_ = (j - 7.0f * x_).floor();

    vec4 x = x_ * ns.x() + ns.y();
    vec4 y = y_ * ns.x() + ns.y();
    vec4 h = 1.0f - x.abs() - y.abs();

    vec4 b0 = vec4(x.x(), x.y(), y.x(), y.y());
    vec4 b1 = vec4(x.z(), x.w(), y.z(), y.w());

    vec4 s0 = b0.floor() * 2.0f + 1.0f;
    vec4 s1 = b1.floor() * 2.0f + 1.0f;
    vec4 sh = -h.step(vec4(0.0f));

    vec4 a0 = b0.xzyw() + s0.xzyw() * sh.swizzle<0, 0, 1, 1>();
    vec4 a1 = b1.xzyw() + s1.xzyw() * sh.swizzle<2, 2, 3, 3>();

    vec3 p0 = vec3(a0.x(), a0.y(), h.x());
    vec3 p1 = vec3(a0.z(), a0.w(), h.y());
    vec3 p2 = vec3(a1.x(), a1.y(), h.z());
    vec3 p3 = vec3(a1.z(), a1.w(), h.w());

    vec4 norm = detail::taylorInvSqrt(vec4(p0.sqrdot(), p1.sqrdot(), p2.sqrdot(), p3.sqrdot()));
    p0 *= norm.x();
    p1 *= norm.y();
    p2 *= norm.z();
    p3 *= norm.w();

    vec4 m = (0.6f - vec4(x0.sqrdot(), x1.sqrdot(), x2.sqrdot(), x3.sqrdot())).max(0.0f);
    m *= m * m * m;
    return 42.0f * m.dot(vec4(p0.dot(x0), p1.dot(x1), p2.dot(x2), p3.dot(x3)));
}

float simplex(vec4 v, const PermuteFactors& factors = {})
{
    auto permute = [&](auto x) { return detail::permute(x, factors); };

    constexpr vec2 C = vec2(0.138196601125010504f, 0.309016994374947451f);

    vec4 i = (v + v.dot(C.y())).floor();
    vec4 x0 = v - i + i.dot(C.x());

    vec4 i0;

    vec3 isX = x0.yzw().step(x0.x());
    vec3 isYZ = x0.swizzle<2, 3, 3>().step(x0.swizzle<1, 1, 2>());

    i0.x() = isX.x() + isX.y() + isX.z();
    i0.set_yzw(1.0f - isX);

    i0.y() += isYZ.x() + isYZ.y();
    i0.set_zw(i0.zw() + 1.0f - isYZ.xy());

    i0.z() += isYZ.z();
    i0.w() += 1.0f - isYZ.z();

    vec4 i3 = i0.clamp(0.0f, 1.0f);
    vec4 i2 = (i0 - 1.0f).clamp(0.0f, 1.0f);
    vec4 i1 = (i0 - 2.0f).clamp(0.0f, 1.0f);

    vec4 x1 = x0 - i1 + 1.0f * C.x();
    vec4 x2 = x0 - i2 + 2.0f * C.x();
    vec4 x3 = x0 - i3 + 3.0f * C.x();
    vec4 x4 = x0 - 1.0f + 4.0f * C.x();

    i = i.mod(289.0f);
    float j0 = permute(permute(permute(permute(i.w()) + i.z()) + i.y()) + i.x());
    vec4 j1 = permute(permute(permute(permute(i.w() + vec4(i1.w(), i2.w(), i3.w(), 1.0f)) + i.z() +
                                      vec4(i1.z(), i2.z(), i3.z(), 1.0f)) +
                              i.y() + vec4(i1.y(), i2.y(), i3.y(), 1.0f)) +
                      i.x() + vec4(i1.x(), i2.x(), i3.x(), 1.0f));

    vec4 ip = vec4(1.0f / 294.0f, 1.0f / 49.0f, 1.0f / 7.0f, 0.0f);

    vec4 p0 = detail::grad4(j0, ip);
    vec4 p1 = detail::grad4(j1.x(), ip);
    vec4 p2 = detail::grad4(j1.y(), ip);
    vec4 p3 = detail::grad4(j1.z(), ip);
    vec4 p4 = detail::grad4(j1.w(), ip);

    vec4 norm = detail::taylorInvSqrt(vec4(p0.sqrdot(), p1.sqrdot(), p2.sqrdot(), p3.sqrdot()));
    p0 *= norm.x();
    p1 *= norm.y();
    p2 *= norm.z();
    p3 *= norm.w();
    p4 *= detail::taylorInvSqrt(p4.sqrdot());

    vec3 m0 = (0.6f - vec3(x0.sqrdot(), x1.sqrdot(), x2.sqrdot())).max(0.0f);
    vec2 m1 = (0.6f - vec2(x3.sqrdot(), x4.sqrdot())).max(0.0f);
    m0 *= m0 * m0 * m0;
    m1 *= m1 * m1 * m1;
    return 49.0f * (m0.dot(vec3(p0.dot(x0), p1.dot(x1), p2.dot(x2))) + m1.dot(vec2(p3.dot(x3), p4.dot(x4))));
}

} // namespace dang::math::noise
