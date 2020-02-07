#pragma once

#include "utils.h"

namespace dang::math
{

template <typename T, std::size_t Dim>
struct Vector;

namespace detail
{

template <typename T, std::size_t Dim>
struct VectorBase : protected std::array<T, Dim> {
    using Base = std::array<T, Dim>;

    inline constexpr VectorBase() : Base{} {}
    inline constexpr VectorBase(Base values) : Base(values) {}

    using Base::operator[];

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

    inline constexpr Vector<T, Dim> normalize() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::normalize requires a floating point type");
        return (*this) / length();
    }

    inline constexpr Vector<T, Dim> vectorTo(const Vector<T, Dim>& other) const
    {
        return other - *this;
    }

    inline constexpr T distanceTo(const Vector<T, Dim>& other) const
    {
        return (other - *this).length();
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

    inline constexpr Vector<T, Dim> operator+() const
    {
        return *this;
    }

    inline constexpr Vector<T, Dim> operator-() const
    {
        return unary([](T a) { return -a; });
    }

#define DMATH_VECTOR_OPERATION(op) \
    friend inline constexpr Vector<T, Dim> operator op(Vector<T, Dim> lhs, const Vector<T, Dim>& rhs) \
    { return lhs op ## = rhs; } \
    friend inline constexpr Vector<T, Dim>& operator op ## =(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs) \
    { return assignment(lhs, rhs, [](T& a, T b) { a op ## = b; }); }

    DMATH_VECTOR_OPERATION(+);
    DMATH_VECTOR_OPERATION(-);
    DMATH_VECTOR_OPERATION(*);
    DMATH_VECTOR_OPERATION(/ );

#undef DMATH_VECTOR_OPERATION

#define DMATH_VECTOR_COMPARE(merge, op) \
    friend inline constexpr bool operator op(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs) \
    { return lhs.merge(rhs, [](T a, T b) { return a op b; }); }

    DMATH_VECTOR_COMPARE(all, == );
    DMATH_VECTOR_COMPARE(any, != );
    DMATH_VECTOR_COMPARE(all, < );
    DMATH_VECTOR_COMPARE(all, <= );
    DMATH_VECTOR_COMPARE(all, > );
    DMATH_VECTOR_COMPARE(all, >= );

#undef DMATH_VECTOR_COMPARE

    template <typename TTarget>
    explicit inline constexpr operator Vector<TTarget, Dim>() const
    {
        Vector<TTarget, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = static_cast<TTarget>((*this)[i]);
        return result;
    }

    template <size_t Index>
    inline constexpr T& get() noexcept
    {
        return std::get<Index>(*this);
    }

    template <size_t Index>
    inline constexpr const T& get() const noexcept
    {
        return std::get<Index>(*this);
    }

    template <size_t... Indices>
    inline constexpr Vector<T, sizeof...(Indices)> swizzle() const
    {
        Vector<T, sizeof...(Indices)> result;
        size_t resultIndex = 0;
        for (size_t index : std::array{ Indices... })
            result[resultIndex++] = (*this)[index];
        return result;
    }

    template <size_t... Indices>
    inline constexpr void setSwizzle(Vector<T, sizeof...(Indices)> vector)
    {
        size_t otherIndex = 0;
        for (size_t index : std::array<std::size_t, sizeof...(Indices)>{ Indices... })
            (*this)[index] = vector[otherIndex++];
    }

    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;

private:
    template <typename Op>
    static inline constexpr Vector<T, Dim>& assignment(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs, const Op& op)
    {
        for (std::size_t i = 0; i < Dim; i++)
            op(lhs[i], rhs[i]);
        return lhs;
    }

    template <typename Op>
    inline constexpr Vector<T, Dim> binary(const Vector<T, Dim>& other, const Op& op) const
    {
        Vector<T, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = op((*this)[i], other[i]);
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
    inline constexpr bool all(const Vector<T, Dim>& other, const Op& op) const
    {
        bool result = true;
        for (std::size_t i = 0; i < Dim; i++)
            result = result && op((*this)[i], other[i]);
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
};

}

template <typename T, std::size_t Dim>
struct Vector : public detail::VectorBase<T, Dim> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, Dim> values) : detail::VectorBase<T, Dim>(values) {}
    inline constexpr Vector(T value)
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = value;
    }
};

#define DMATH_DEFINE_SWIZZLE(name, ...) \
inline constexpr Vector<T, sizeof(#name) - 1> name() const { return this->swizzle<__VA_ARGS__>(); } \
inline void set_ ## name(const Vector<T, sizeof(#name) - 1>& vector) { this->setSwizzle<__VA_ARGS__>(vector); }

template <typename T>
struct Vector<T, 1> : public detail::VectorBase<T, 1> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 1> values) : detail::VectorBase(values) {}
    inline constexpr Vector(T x) : detail::VectorBase<T, 1>({ x }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }

    inline constexpr operator T& () { return std::get<0>(*this); }
    inline constexpr operator T() const { return std::get<0>(*this); }
};

template <typename T>
struct Vector<T, 2> : public detail::VectorBase<T, 2> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 2> values) : detail::VectorBase<T, 2>(values) {}
    inline constexpr Vector(T value) : detail::VectorBase<T, 2>({ value, value }) {}
    inline constexpr Vector(T x, T y) : detail::VectorBase<T, 2>({ x, y }) {}

    static inline constexpr Vector<T, 2> fromSlope(std::optional<T> slope)
    {
        if (slope)
            return { 1, *slope };
        return { 0, 1 };
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

    DMATH_DEFINE_SWIZZLE(xy, 0, 1);
    DMATH_DEFINE_SWIZZLE(yx, 1, 0);

    inline constexpr Vector<T, 2> cross() const
    {
        return { -y(), x() };
    }

    inline constexpr T cross(const Vector<T, 2>& other) const
    {
        return x() * other.y() - y() * other.x();
    }

    inline constexpr std::optional<T> slope() const
    {
        static_assert(std::is_floating_point_v<T>, "vec2::slope requires a floating point type");
        if (x() != T())
            return y() / x();
        return std::nullopt;
    }
};

template <typename T>
struct Vector<T, 3> : public detail::VectorBase<T, 3> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 3> values) : detail::VectorBase<T, 3>(values) {}
    inline constexpr Vector(T value) : detail::VectorBase<T, 3>({ value, value, value }) {}
    inline constexpr Vector(T x, T y, T z) : detail::VectorBase<T, 3>({ x, y, z }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
    inline constexpr T& y() { return std::get<1>(*this); }
    inline constexpr T y() const { return std::get<1>(*this); }
    inline constexpr T& z() { return std::get<2>(*this); }
    inline constexpr T z() const { return std::get<2>(*this); }

    DMATH_DEFINE_SWIZZLE(xy, 0, 1);
    DMATH_DEFINE_SWIZZLE(xz, 0, 2);
    DMATH_DEFINE_SWIZZLE(yx, 1, 0);
    DMATH_DEFINE_SWIZZLE(yz, 1, 2);
    DMATH_DEFINE_SWIZZLE(zx, 2, 0);
    DMATH_DEFINE_SWIZZLE(zy, 2, 1);

    DMATH_DEFINE_SWIZZLE(xyz, 0, 1, 2);
    DMATH_DEFINE_SWIZZLE(xzy, 0, 2, 1);
    DMATH_DEFINE_SWIZZLE(yxz, 1, 0, 2);
    DMATH_DEFINE_SWIZZLE(yzx, 1, 2, 0);
    DMATH_DEFINE_SWIZZLE(zxy, 2, 0, 1);
    DMATH_DEFINE_SWIZZLE(zyx, 2, 1, 0);

    inline constexpr Vector<T, 3> cross(const Vector<T, 3>& other) const
    {
        return { (*this)[1] * other[2] - (*this)[2] * other[1],
                 (*this)[2] * other[0] - (*this)[0] * other[2],
                 (*this)[0] * other[1] - (*this)[1] * other[0] };
    }
};

template <typename T>
struct Vector<T, 4> : public detail::VectorBase<T, 4> {
    inline constexpr Vector() = default;
    inline constexpr Vector(std::array<T, 4> values) : detail::VectorBase<T, 4>(values) {}
    inline constexpr Vector(T value) : detail::VectorBase<T, 4>({ value, value, value, value }) {}
    inline constexpr Vector(T value, T w) : detail::VectorBase<T, 4>({ value, value, value, w }) {}
    inline constexpr Vector(Vector<T, 3> vector, T w) : detail::VectorBase<T, 4>({ vector[0], vector[1], vector[2], w }) {}
    inline constexpr Vector(T x, T y, T z, T w) : detail::VectorBase<T, 4>({ x, y, z, w }) {}

    inline constexpr T& x() { return std::get<0>(*this); }
    inline constexpr T x() const { return std::get<0>(*this); }
    inline constexpr T& y() { return std::get<1>(*this); }
    inline constexpr T y() const { return std::get<1>(*this); }
    inline constexpr T& z() { return std::get<2>(*this); }
    inline constexpr T z() const { return std::get<2>(*this); }
    inline constexpr T& w() { return std::get<3>(*this); }
    inline constexpr T w() const { return std::get<3>(*this); }

    DMATH_DEFINE_SWIZZLE(xy, 0, 1);
    DMATH_DEFINE_SWIZZLE(xz, 0, 2);
    DMATH_DEFINE_SWIZZLE(xw, 0, 3);
    DMATH_DEFINE_SWIZZLE(yx, 1, 0);
    DMATH_DEFINE_SWIZZLE(yz, 1, 2);
    DMATH_DEFINE_SWIZZLE(yw, 1, 3);
    DMATH_DEFINE_SWIZZLE(zx, 2, 0);
    DMATH_DEFINE_SWIZZLE(zy, 2, 1);
    DMATH_DEFINE_SWIZZLE(zw, 2, 3);
    DMATH_DEFINE_SWIZZLE(wx, 3, 0);
    DMATH_DEFINE_SWIZZLE(wy, 3, 1);
    DMATH_DEFINE_SWIZZLE(wz, 3, 2);

    DMATH_DEFINE_SWIZZLE(xyz, 0, 1, 2);
    DMATH_DEFINE_SWIZZLE(xyw, 0, 1, 3);
    DMATH_DEFINE_SWIZZLE(xzy, 0, 2, 1);
    DMATH_DEFINE_SWIZZLE(xzw, 0, 2, 3);
    DMATH_DEFINE_SWIZZLE(yxz, 1, 0, 2);
    DMATH_DEFINE_SWIZZLE(yxw, 1, 0, 3);
    DMATH_DEFINE_SWIZZLE(yzx, 1, 2, 0);
    DMATH_DEFINE_SWIZZLE(yzw, 1, 2, 3);
    DMATH_DEFINE_SWIZZLE(zxy, 2, 0, 1);
    DMATH_DEFINE_SWIZZLE(zxw, 2, 0, 3);
    DMATH_DEFINE_SWIZZLE(zyx, 2, 1, 0);
    DMATH_DEFINE_SWIZZLE(zyw, 2, 1, 3);
    DMATH_DEFINE_SWIZZLE(wxy, 3, 0, 1);
    DMATH_DEFINE_SWIZZLE(wxz, 3, 0, 2);
    DMATH_DEFINE_SWIZZLE(wyx, 3, 1, 0);
    DMATH_DEFINE_SWIZZLE(wyz, 3, 1, 2);
    DMATH_DEFINE_SWIZZLE(wzx, 3, 2, 0);
    DMATH_DEFINE_SWIZZLE(wzy, 3, 2, 1);

    DMATH_DEFINE_SWIZZLE(xyzw, 0, 1, 2, 3);
    DMATH_DEFINE_SWIZZLE(xywz, 0, 1, 3, 2);
    DMATH_DEFINE_SWIZZLE(xzyw, 0, 2, 1, 3);
    DMATH_DEFINE_SWIZZLE(xzwy, 0, 2, 3, 1);
    DMATH_DEFINE_SWIZZLE(xwyz, 0, 3, 1, 2);
    DMATH_DEFINE_SWIZZLE(xwzy, 0, 3, 2, 1);
    DMATH_DEFINE_SWIZZLE(yxzw, 1, 0, 2, 3);
    DMATH_DEFINE_SWIZZLE(yxwz, 1, 0, 3, 2);
    DMATH_DEFINE_SWIZZLE(yzxw, 1, 2, 0, 3);
    DMATH_DEFINE_SWIZZLE(yzwx, 1, 2, 3, 0);
    DMATH_DEFINE_SWIZZLE(ywxz, 1, 3, 0, 2);
    DMATH_DEFINE_SWIZZLE(ywzx, 1, 3, 2, 0);
    DMATH_DEFINE_SWIZZLE(zyxw, 2, 1, 0, 3);
    DMATH_DEFINE_SWIZZLE(zywx, 2, 1, 3, 0);
    DMATH_DEFINE_SWIZZLE(zxyw, 2, 0, 1, 3);
    DMATH_DEFINE_SWIZZLE(zxwy, 2, 0, 3, 1);
    DMATH_DEFINE_SWIZZLE(zwyx, 2, 3, 1, 0);
    DMATH_DEFINE_SWIZZLE(zwxy, 2, 3, 0, 1);
    DMATH_DEFINE_SWIZZLE(wyzx, 3, 1, 2, 0);
    DMATH_DEFINE_SWIZZLE(wyxz, 3, 1, 0, 2);
    DMATH_DEFINE_SWIZZLE(wzyx, 3, 2, 1, 0);
    DMATH_DEFINE_SWIZZLE(wzxy, 3, 2, 0, 1);
    DMATH_DEFINE_SWIZZLE(wxyz, 3, 0, 1, 2);
    DMATH_DEFINE_SWIZZLE(wxzy, 3, 0, 2, 1);
};

#undef DMATH_DEFINE_SWIZZLE

template <std::size_t Dim>
using vec = Vector<float, Dim>;

template <std::size_t Dim>
using dvec = Vector<double, Dim>;

template <std::size_t Dim>
using ivec = Vector<int, Dim>;

template <std::size_t Dim>
using uvec = Vector<unsigned, Dim>;

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

template <typename T, std::size_t Dim>
struct std::tuple_size<Vector<T, Dim>> {
    static constexpr int value = Dim;
};

template <typename T, std::size_t Dim, size_t Index>
struct std::tuple_element<Index, Vector<T, Dim>> {
    using type = T;
};

}
