#pragma once

#include "global.h"

namespace dang::math
{

template <typename T, std::size_t Dim>
struct Vector;

template <typename T, std::size_t Dim>
struct VectorBase : protected std::array<T, Dim> {
    inline constexpr VectorBase() : std::array<T, Dim> {} {}
    inline constexpr VectorBase(std::array<T, Dim> values) : std::array<T, Dim>(values) {}

    template<size_t Index>
    inline constexpr T& get() noexcept
    {
        return std::get<Index>(*this);
    }

    template<size_t Index>
    inline constexpr const T get() const noexcept
    {
        return std::get<Index>(*this);
    }

    inline constexpr T& operator[](std::size_t index)
    {
        return std::array<T, Dim>::operator[](index);
    }

    inline constexpr const T operator[](std::size_t index) const
    {
        return std::array<T, Dim>::operator[](index);
    }

    inline constexpr Vector<T, Dim> operator+() const
    {
        return *this;
    }

    inline constexpr Vector<T, Dim> operator-() const
    {
        return unary([](T a) { return -a; });
    }

    inline constexpr Vector<T, Dim> operator+(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a + b; });
    }

    static friend inline constexpr Vector<T, Dim> operator+(T scalar, const Vector<T, Dim>& other)
    {
        return other.unary([scalar](T a) { return scalar + a; });
    }

    inline constexpr Vector<T, Dim> operator-(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a - b; });
    }

    static friend inline constexpr Vector<T, Dim> operator-(T scalar, const Vector<T, Dim>& other)
    {
        return other.unary([scalar](T a) { return scalar - a; });
    }

    inline constexpr Vector<T, Dim> operator*(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a * b; });
    }

    static friend inline constexpr Vector<T, Dim> operator*(T scalar, const Vector<T, Dim>& other)
    {
        return other.unary([scalar](T a) { return scalar * a; });
    }

    inline constexpr Vector<T, Dim> operator/(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a / b; });
    }

    static friend inline constexpr Vector<T, Dim> operator/(T scalar, const Vector<T, Dim>& other)
    {
        return other.unary([scalar](T a) { return scalar / a; });
    }

    inline constexpr bool operator==(const Vector<T, Dim>& other) const
    {
        return all(other, [](T a, T b) { return a == b; });
    }

    static friend inline constexpr bool operator==(T scalar, const Vector<T, Dim>& other)
    {
        return all([scalar](T a) { return scalar == a; });
    }

    inline constexpr bool operator!=(const Vector<T, Dim>& other) const
    {
        return any(other, [](T a, T b) { return a != b; });
    }

    static friend inline constexpr bool operator!=(T scalar, const Vector<T, Dim>& other)
    {
        return any([scalar](T a) { return scalar != a; });
    }

    inline constexpr bool operator<(const Vector<T, Dim>& other) const
    {
        return all(other, [](T a, T b) { return a < b; });
    }

    static friend inline constexpr bool operator<(T scalar, const Vector<T, Dim>& other)
    {
        return all([scalar](T a) { return scalar < a; });
    }

    inline constexpr bool operator<=(const Vector<T, Dim>& other) const
    {
        return all(other, [](T a, T b) { return a <= b; });
    }

    static friend inline constexpr bool operator<=(T scalar, const Vector<T, Dim>& other)
    {
        return all([scalar](T a) { return scalar <= a; });
    }

    inline constexpr bool operator>(const Vector<T, Dim>& other) const
    {
        return all(other, [](T a, T b) { return a > b; });
    }

    static friend inline constexpr bool operator>(T scalar, const Vector<T, Dim>& other)
    {
        return all([scalar](T a) { return scalar > a; });
    }

    inline constexpr bool operator>=(const Vector<T, Dim>& other) const
    {
        return all(other, [](T a, T b) { return a >= b; });
    }

    static friend inline constexpr bool operator>=(T scalar, const Vector<T, Dim>& other)
    {
        return all([scalar](T a) { return scalar >= a; });
    }

    inline constexpr T sum() const
    {
        T result = T();
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i];
        return result;
    }

    inline constexpr T dot(const Vector<T, Dim>& other) const
    {
        return ((*this) * other).sum();
    }

    inline constexpr T sqrdot() const
    {
        return ((*this) * (*this)).sum();
    }

    inline constexpr T length() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::length requires a floating point type");
        return std::sqrt(sqrdot());
    }

    inline constexpr Vector<T, Dim> normalized() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::normalized requires a floating point type");
        return (*this) / length();
    }

    inline constexpr Vector<T, Dim> vectorTo(const Vector<T, Dim>& other) const
    {
        return other - *this;
    }

    inline constexpr Vector<T, Dim> distanceTo(const Vector<T, Dim>& other) const
    {
        return other - *this;
    }

    inline constexpr T cosAngleTo(const Vector<T, Dim>& other) const
    {
        return std::clamp(dot(other) / (length() * other.length()), T(-1), T(1));
    }

    inline constexpr T angleRadTo(const Vector<T, Dim>& other) const
    {
        return std::acos(cosAngleTo(other));
    }

    inline constexpr T angleTo(const Vector<T, Dim>& other) const
    {
        return radToDeg(angleRadTo(other));
    }

    inline constexpr Vector<T, Dim> abs() const
    {
        return unary([](T a) { return a < T(0) ? -a : a; });
    }

    inline constexpr Vector<T, Dim> floor() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::floor requires a floating point type");
        return unary([](T a) { return std::floor(a); });
    }

    inline constexpr Vector<T, Dim> ceil() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::ceil requires a floating point type");
        return unary([](T a) { return std::ceil(a); });
    }

    inline constexpr Vector<T, Dim> min(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a < b ? a : b; });
    }

    inline constexpr Vector<T, Dim> max(const Vector<T, Dim>& other) const
    {
        return binary(other, [](T a, T b) { return a > b ? a : b; });
    }

    inline constexpr Vector<T, Dim> reflect(const Vector<T, Dim>& normal) const
    {
        return *this - 2 * dot(normal) * normal;
    }

    template <typename TTarget>
    explicit inline constexpr operator Vector<TTarget, Dim>()
    {
        Vector<TTarget, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = static_cast<TTarget>((*this)[i]);
        return result;
    }

private:
    template <typename Op>
    inline constexpr Vector<T, Dim> binary(const Vector<T, Dim>& other, const Op& op) const
    {
        Vector<T, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool all(const Vector<T, Dim>& other, const Op& op) const
    {
        bool result = true;
        for (std::size_t i = 0; i < Dim; i++)
            result = result && op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    inline constexpr Vector<T, Dim> unary(const Op& op) const
    {
        Vector<T, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = op((*this)[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool any(const Vector<T, Dim>& other, const Op& op) const
    {
        bool result = false;
        for (std::size_t i = 0; i < Dim; i++)
            result = result || op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool all(const Op& op) const
    {
        bool result = true;
        for (std::size_t i = 0; i < Dim; i++)
            result = result && op((*this)[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool any(const Op& op) const
    {
        bool result = false;
        for (std::size_t i = 0; i < Dim; i++)
            result = result || op((*this)[i]);
        return result;
    }
};

template <typename T, std::size_t Dim>
struct Vector : public VectorBase<T, Dim> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, Dim> values) : VectorBase<T, Dim>(values) {}
    inline constexpr Vector(T value)
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = value;
    }
};

template <typename T>
struct Vector<T, 1> : public VectorBase<T, 1> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 1> values) : VectorBase(values) {}
    inline constexpr Vector(T x) : VectorBase<T, 1>({ x }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
};

template <typename T>
struct Vector<T, 2> : public VectorBase<T, 2> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 2> values) : VectorBase<T, 2>(values) {}
    inline constexpr Vector(T value) : VectorBase<T, 2>({ value, value }) {}
    inline constexpr Vector(T x, T y) : VectorBase<T, 2>({ x, y }) {}

    static inline constexpr Vector<T, 2> fromSlope(T slope)
    {
        return { 1, slope };
    }

    static inline constexpr Vector<T, 2> fromAngleRad(T radians)
    {
        return { std::cos(radians), std::sin(radians) };
    }

    static inline constexpr Vector<T, 2> fromAngle(T degrees)
    {
        return fromAngleRad(degToRad(degrees));
    }

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
    inline constexpr T& y() { return std::get<1>(*this); }
    inline constexpr T y() const { return std::get<1>(*this); }

    inline constexpr Vector<T, 2> cross()
    {
        return { -y(), x() };
    }

    inline constexpr T cross(const Vector<T, 3>& other)
    {
        return x() * other.y() - y() * x();
    }

    inline constexpr T slope()
    {
        static_assert(std::is_floating_point_v<T>, "vec2::slope requires a floating point type");
        return y() / x();
    }
};

template <typename T>
struct Vector<T, 3> : public VectorBase<T, 3> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 3> values) : VectorBase<T, 3>(values) {}
    inline constexpr Vector(T value) : VectorBase<T, 3>({ value, value, value }) {}
    inline constexpr Vector(T x, T y, T z) : VectorBase<T, 3>({ x, y, z }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
    inline constexpr T& y() { return std::get<1>(*this); }
    inline constexpr T y() const { return std::get<1>(*this); }
    inline constexpr T& z() { return std::get<2>(*this); }
    inline constexpr T z() const { return std::get<2>(*this); }

    inline constexpr Vector<T, 3> cross(const Vector<T, 3>& other)
    {
        return { (*this)[1] * other[2] - (*this)[2] * other[1],
                 (*this)[2] * other[0] - (*this)[0] * other[2],
                 (*this)[0] * other[1] - (*this)[1] * other[0] };
    }
};

template <typename T>
struct Vector<T, 4> : public VectorBase<T, 4> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 4> values) : VectorBase<T, 4>(values) {}
    inline constexpr Vector(T value) : VectorBase<T, 4>({ value, value, value, value }) {}
    inline constexpr Vector(T value, T w) : VectorBase<T, 4>({ value, value, value, w }) {}
    inline constexpr Vector(T x, T y, T z, T w) : VectorBase<T, 4>({ x, y, z, w }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
    inline constexpr T& y() { return std::get<1>(*this); }
    inline constexpr T y() const { return std::get<1>(*this); }
    inline constexpr T& z() { return std::get<2>(*this); }
    inline constexpr T z() const { return std::get<2>(*this); }
    inline constexpr T& w() { return std::get<3>(*this); }
    inline constexpr T w() const { return std::get<3>(*this); }
};

template <std::size_t Dim>
using vec = Vector<float, Dim>;

template <std::size_t Dim>
using dvec = Vector<double, Dim>;

template <std::size_t Dim>
using ivec = Vector<int, Dim>;

template <std::size_t Dim>
using uvec = Vector<unsigned int, Dim>;

template <std::size_t Dim>
using svec = Vector<std::size_t, Dim>;

using vec1 = vec<1>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

using dvec1 = dvec<1>;
using dvec2 = dvec<2>;
using dvec3 = dvec<3>;
using dvec4 = dvec<4>;

using ivec1 = ivec<1>;
using ivec2 = ivec<2>;
using ivec3 = ivec<3>;
using ivec4 = ivec<4>;

using uvec1 = uvec<1>;
using uvec2 = uvec<2>;
using uvec3 = uvec<3>;
using uvec4 = uvec<4>;

using svec1 = svec<1>;
using svec2 = svec<2>;
using svec3 = svec<3>;
using svec4 = svec<4>;

template<typename T, std::size_t Dim>
struct std::tuple_size<Vector<T, Dim>> {
    static constexpr int value = Dim;
};

template<typename T, std::size_t Dim, size_t Index>
struct std::tuple_element<Index, Vector<T, Dim>> {
    using type = T;
};

}
