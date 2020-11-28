#pragma once

#include "utils.h"

namespace dang::math {

// TODO: C++20 Replace SFINAE with requires

/// <summary>A vector of the templated type and dimension, using std::array as base.</summary>
template <typename T, std::size_t Dim>
struct Vector : std::array<T, Dim> {
    using Base = std::array<T, Dim>;

    /// <summary>Initializes all values with default values.</summary>
    constexpr Vector()
        : Base()
    {}

    /// <summary>Implicit conversion between scalars and a vector containing said scalar for each component.</summary>
    /// <remarks>
    /// Being implicit greatly simplifies operator overloading, however GLSL does not provide this implicit conversion.
    /// GLSL only allows this conversion when used in conjunction with an actual mathematical operations.
    /// </remarks>
    constexpr Vector(T value)
        : Base()
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = value;
    }

    /// <summary>Initializes x and y with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 2>>
    constexpr Vector(T x, T y)
        : Base{x, y}
    {}

    /// <summary>Initializes x, y and z with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 3>>
    constexpr Vector(T x, T y, T z)
        : Base{x, y, z}
    {}

    /// <summary>Initializes x, y, z and w with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr Vector(T x, T y, T z, T w)
        : Base{x, y, z, w}
    {}

    /// <summary>Converts a three-dimensional into a four-dimensional vector with the given value for w.</summary>
    /// <remarks>
    /// GLSL allows this kind of concatentation for any number and size of vectors, however, this is tedious to implement in C++.
    /// Therefore, only this probably most common overload for turning vec3 into vec4 is provided.
    /// </remarks>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr Vector(Vector<T, 3> vector, T w)
        : Base{vector[0], vector[1], vector[2], w}
    {}

    /// <summary>Allows explicit conversion between vector of same size but different types.</summary>
    template <typename TFrom>
    explicit constexpr Vector(const Vector<TFrom, Dim>& other)
        : Vector()
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = static_cast<T>(other[i]);
    }

    /// <summary>Returns the sum of all components.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto sum() const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i];
        return result;
    }

    /// <summary>Returns the product of all components.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto product() const
    {
        T result{1};
        for (std::size_t i = 0; i < Dim; i++)
            result *= (*this)[i];
        return result;
    }

    /// <summary>Returns the dot-product with the given vector.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto dot(const Vector& other) const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i] * other[i];
        return result;
    }

    /// <summary>Returns the dot-product with the vector itself.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto sqrdot() const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i] * (*this)[i];
        return result;
    }

    /// <summary>Returns the length of the vector.</summary>
    /// <remarks>In GLSL vec3(0).length() returns the component count.</remarks>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto length() const
    {
        return std::sqrt(sqrdot());
    }

    /// <summary>Returns a normalized version of the vector.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto normalize() const
    {
        return (*this) / length();
    }

    /// <summary>Returns a new vector, which points from the vector to the given vector.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto vectorTo(const Vector& other) const
    {
        return other - *this;
    }

    /// <summary>Returns the distance to the given vector.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto distanceTo(const Vector& other) const
    {
        return (other - *this).length();
    }

    /// <summary>Returns the cosine of the angle to the given vector.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto cosAngleTo(const Vector& other) const
    {
        return std::clamp(dot(other) / (length() * other.length()), T{-1}, T{1});
    }

    /// <summary>Returns the angle to the given vector in radians.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto radiansTo(const Vector& other) const
    {
        return std::acos(cosAngleTo(other));
    }

    /// <summary>Returns the angle to the given vector in degrees.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto degreesTo(const Vector& other) const
    {
        return dmath::degrees(radiansTo(other));
    }

    /// <summary>Converts every component from degrees into radians.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto radians() const
    {
        return variadicOp(dmath::radians<T>);
    }

    /// <summary>Converts every component from radians into degrees.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto degrees() const
    {
        return variadicOp(dmath::degrees<T>);
    }

    /// <summary>Returns the vector with each component being positive.</summary>
    template <typename = std::enable_if_t<std::is_signed_v<T>>>
    constexpr auto abs() const
    {
        return variadicOp([](T a) { return a < T{0} ? -a : a; });
    }

    /// <summary>Returns the vector with each component rounded down.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto floor() const
    {
        return variadicOp([](T a) { return std::floor(a); });
    }

    /// <summary>Returns the vector with each component rounded up.</summary>
    template <typename = std::enable_if_t<std::is_floating_point_v<T>>>
    constexpr auto ceil() const
    {
        return variadicOp([](T a) { return std::ceil(a); });
    }

    /// <summary>Returns a vector, only taking the smaller components of both vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto min(const Vector& other) const
    {
        return variadicOp([](T a, T b) { return std::min(a, b); }, other);
    }

    /// <summary>Returns a vector, only taking the larger components of both vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto max(const Vector& other) const
    {
        return variadicOp([](T a, T b) { return std::max(a, b); }, other);
    }

    /// <summary>Returns a vector, for which each component is clamped between low and high.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto clamp(const Vector& low, const Vector& high) const
    {
        return variadicOp([](T a, T b, T c) { return std::clamp(a, b, c); }, low, high);
    }

    /// <summary>Reflects the vector on the given plane normal.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto reflect(const Vector& normal) const
    {
        return *this - 2 * dot(normal) * normal;
    }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto lessThan(const Vector& other) const { return variadicOp(std::less<>{}, other); }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto lessThanEqual(const Vector& other) const { return variadicOp(std::less_equal<>{}, other); }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto greaterThan(const Vector& other) const { return variadicOp(std::greater<>{}, other); }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto greaterThanEqual(const Vector& other) const { return variadicOp(std::greater_equal<>{}, other); }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto equal(const Vector& other) const { return variadicOp(std::equal_to<>{}, other); }

    /// <summary>Component-wise comparison, returning a bvec.</summary>
    constexpr auto notEqual(const Vector& other) const { return variadicOp(std::not_equal_to<>{}, other); }

    /// <summary>Provided as constexpr, as std::array does not.</summary>
    constexpr auto operator==(const Vector& other) const { return equal(other).all(); }

    /// <summary>Provided as constexpr, as std::array does not.</summary>
    constexpr auto operator!=(const Vector& other) const { return notEqual(other).any(); }

    /// <summary>Whether all components satisfy a given predicate.</summary>
    template <typename = std::enable_if_t<std::is_same_v<T, bool>>>
    constexpr auto all() const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if (!(*this)[i])
                return false;
        return true;
    }

    /// <summary>Whether any component satisfies a given predicate.</summary>
    template <typename = std::enable_if_t<std::is_same_v<T, bool>>>
    constexpr auto any() const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if ((*this)[i])
                return true;
        return false;
    }

    /// <summary>Whether no component satisfies a given predicate.</summary>
    template <typename = std::enable_if_t<std::is_same_v<T, bool>>>
    constexpr auto none() const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if ((*this)[i])
                return false;
        return true;
    }

    /// <summary>Inverts each component.</summary>
    /// <remarks>Known as "not" in GLSL, which cannot be used in C++.</remarks>
    template <typename = std::enable_if_t<std::is_same_v<T, bool>>>
    constexpr auto invert() const
    {
        return variadicOp(std::logical_not<>{});
    }

    /// <summary>Simply returns the vector.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto operator+() const
    {
        return *this;
    }

    /// <summary>Returns the vector with each component negated.</summary>
    template <typename = std::enable_if_t<std::is_signed_v<T>>>
    constexpr auto operator-() const
    {
        return variadicOp(std::negate<>{});
    }

    /// <summary>Component-wise addition of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    friend constexpr auto operator+(const Vector& lhs, const Vector& rhs)
    {
        return lhs.variadicOp(std::plus<>{}, rhs);
    }

    /// <summary>Component-wise addition of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto& operator+=(const Vector& other)
    {
        return assignmentOp(std::plus<>{}, other);
    }

    /// <summary>Component-wise subtraction of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    friend constexpr auto operator-(const Vector& lhs, const Vector& rhs)
    {
        return lhs.variadicOp(std::minus<>{}, rhs);
    }

    /// <summary>Component-wise subtraction of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto& operator-=(const Vector& other)
    {
        return assignmentOp(std::minus<>{}, other);
    }

    /// <summary>Component-wise multiplication of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    friend constexpr auto operator*(const Vector& lhs, const Vector& rhs)
    {
        return lhs.variadicOp(std::multiplies<>{}, rhs);
    }

    /// <summary>Component-wise multiplication of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto& operator*=(const Vector& other)
    {
        return assignmentOp(std::multiplies<>{}, other);
    }

    /// <summary>Component-wise division of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    friend constexpr auto operator/(const Vector& lhs, const Vector& rhs)
    {
        return lhs.variadicOp(std::divides<>{}, rhs);
    }

    /// <summary>Component-wise division of two vectors.</summary>
    template <typename = std::enable_if_t<!std::is_same_v<T, bool>>>
    constexpr auto& operator/=(const Vector& other)
    {
        return assignmentOp(std::divides<>{}, other);
    }

    /// <summary>Returns a swizzle of the given components.</summary>
    template <std::size_t... Indices>
    constexpr auto swizzle() const
    {
        return Vector<T, sizeof...(Indices)>{std::get<Indices>(*this)...};
    }

    /// <summary>Sets a swizzle for the given components.</summary>
    template <std::size_t... Indices, std::size_t... OtherIndices>
    constexpr void setSwizzleHelper(Vector<T, sizeof...(Indices)> vector, std::index_sequence<OtherIndices...>)
    {
        ((std::get<Indices>(*this) = std::get<OtherIndices>(vector)), ...);
    }

    /// <summary>Sets a swizzle for the given components.</summary>
    template <std::size_t... Indices>
    constexpr void setSwizzle(Vector<T, sizeof...(Indices)> vector)
    {
        setSwizzleHelper<Indices...>(vector, std::make_index_sequence<sizeof...(Indices)>());
    }

    /// <summary>Performs an operation on each component using an arbitrary number of other vectors.</summary>
    template <typename TOperation, typename... TVectors>
    constexpr auto variadicOp(TOperation operation, const TVectors&... vectors) const
    {
        Vector<decltype(operation((*this)[0], vectors[0]...)), Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = operation((*this)[i], vectors[i]...);
        return result;
    }

    /// <summary>Performs an operation with another vector and assigns the result to itself.</summary>
    template <typename TOperation>
    constexpr auto& assignmentOp(TOperation operation, const Vector& other)
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = operation((*this)[i], other[i]);
        return *this;
    }

    /// <summary>Returns a string representing the vector in the form [x, y, z].</summary>
    auto format() const { return (std::stringstream() << *this).str(); }

    /// <summary>Appends a string representation of the vector in the form [x, y, z] to the stream.</summary>
    friend auto& operator<<(std::ostream& stream, const Vector& vector)
    {
        auto old_flags = stream.flags();
        if constexpr (std::is_same_v<T, bool>)
            stream << std::boolalpha;

        stream << '[';
        if constexpr (Dim > 0) {
            stream << vector[0];
            for (std::size_t i = 1; i < Dim; i++)
                stream << ", " << vector[i];
        }
        if constexpr (std::is_same_v<T, bool>)
            stream.flags(old_flags);
        return stream << ']';
    }

    // --- Vector<T, 1> ---

    /// <summary>The x-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 1 && Dim <= 4>>
    constexpr auto& x()
    {
        return (*this)[0];
    }

    /// <summary>The x-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 1 && Dim <= 4>>
    constexpr auto x() const
    {
        return (*this)[0];
    }

    // --- Vector<T, 2> ---

    /// <summary>The y-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 2 && Dim <= 4>>
    constexpr auto& y()
    {
        return (*this)[1];
    }

    /// <summary>The y-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 2 && Dim <= 4>>
    constexpr auto y() const
    {
        return (*this)[1];
    }

    /// <summary>Creates a vector from the given slope, which is NOT normalized.</summary>
    /// <remarks>The x-component is always one except if std::nullopt is given, which returns a vertical vector of length one.</remarks>
    template <typename = std::enable_if_t<Dim == 2 && !std::is_same_v<T, bool>>>
    static constexpr auto fromSlope(std::optional<T> slope)
    {
        return slope ? Vector<T, 2>(1, *slope) : Vector<T, 2>(0, 1);
    }

    /// <summary>Creates a normalized vector of the given angle in radians.</summary>
    /// <remarks>Zero points to positive x, while an increase rotates counter-clockwise.</remarks>
    template <typename = std::enable_if_t<Dim == 2 && std::is_floating_point_v<T>>>
    static constexpr auto fromRadians(T radians)
    {
        return Vector<T, 2>{std::cos(radians), std::sin(radians)};
    }

    /// <summary>Creates a normalized vector of the given angle in degrees.</summary>
    /// <remarks>Zero points to positive x, while an increase rotates counter-clockwise.</remarks>
    template <typename = std::enable_if_t<Dim == 2 && std::is_floating_point_v<T>>>
    static constexpr auto fromDegrees(T degrees)
    {
        return fromRadians(dmath::radians(degrees));
    }

    /// <summary>Rotates the vector counter-clockwise by 90 degrees by simply swapping its components and negating the new x.</summary>
    template <typename = std::enable_if_t<Dim == 2 && !std::is_same_v<T, bool>>>
    constexpr auto cross() const
    {
        return Vector<T, 2>{-y(), x()};
    }

    /// <summary>Returns the two-dimensional cross-product with the given vector.</summary>
    /// <remarks>Equivalent to <c>x1 * y2 - y1 * x2</c></remarks>
    template <typename = std::enable_if_t<Dim == 2 && !std::is_same_v<T, bool>>>
    constexpr auto cross(const Vector<T, 2>& other) const
    {
        return x() * other.y() - y() * other.x();
    }

    /// <summary>Returns the slope of the vector or std::nullopt if infinite.</summary>
    template <typename = std::enable_if_t<Dim == 2 && std::is_floating_point_v<T>>>
    constexpr std::optional<T> slope() const
    {
        if (x() != T())
            return y() / x();
        return std::nullopt;
    }

    // --- Vector<T, 3> ---

    /// <summary>The z-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 3 && Dim <= 4>>
    constexpr auto& z()
    {
        return (*this)[2];
    }

    /// <summary>The z-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 3 && Dim <= 4>>
    constexpr auto z() const
    {
        return (*this)[2];
    }

    /// <summary>Returns the cross-product with the given vector.</summary>
    template <typename = std::enable_if_t<Dim == 3 && !std::is_same_v<T, bool>>>
    constexpr auto cross(const Vector<T, 3>& other) const
    {
        return Vector<T, 3>{(*this)[1] * other[2] - (*this)[2] * other[1],
                            (*this)[2] * other[0] - (*this)[0] * other[2],
                            (*this)[0] * other[1] - (*this)[1] * other[0]};
    }

    // --- Vector<T, 4> ---

    /// <summary>The w-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr auto& w()
    {
        return (*this)[3];
    }

    /// <summary>The w-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr auto w() const
    {
        return (*this)[3];
    }

    // --- Swizzles ---

#define DMATH_DEFINE_SWIZZLE(name, ...)                                                                                \
    template <typename = std::enable_if_t<Dim >= sizeof(#name) - 1>>                                                   \
    constexpr auto name() const                                                                                        \
    {                                                                                                                  \
        return swizzle<__VA_ARGS__>();                                                                                 \
    }                                                                                                                  \
    template <typename = std::enable_if_t<Dim >= sizeof(#name) - 1>>                                                   \
    void set_##name(const Vector<T, sizeof(#name) - 1>& vector)                                                        \
    {                                                                                                                  \
        setSwizzle<__VA_ARGS__>(vector);                                                                               \
    }

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

#undef DMATH_DEFINE_SWIZZLE
};

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

template <std::size_t Dim>
using bvec = Vector<bool, Dim>;

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

using bvec1 = bvec<1>;
using bvec2 = bvec<2>;
using bvec3 = bvec<3>;
using bvec4 = bvec<4>;

} // namespace dang::math

template <typename T, std::size_t Dim>
struct std::tuple_size<dang::math::Vector<T, Dim>> : tuple_size<array<T, Dim>> {};

template <typename T, std::size_t Dim, std::size_t Index>
struct std::tuple_element<Index, dang::math::Vector<T, Dim>> : tuple_element<Index, array<T, Dim>> {};
