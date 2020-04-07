#pragma once

#include "utils.h"

namespace dang::math
{

/// <summary>A vector of the templated type and dimension, using std::array as base.</summary>
template <typename T, std::size_t Dim>
struct Vector : std::array<T, Dim> {
    /// <summary>Initializes all values with default values.</summary>
    constexpr Vector()
        : std::array<T, Dim>{}
    {
    }

    /// <summary>Initializes all values with the given value.</summary>
    constexpr Vector(T value)
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = value;
    }

    /// <summary>Initializes x and y with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 2>>
    constexpr Vector(T x, T y)
        : std::array<T, Dim>{ x, y }
    {
    }

    /// <summary>Initializes x, y and z with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 3>>
    constexpr Vector(T x, T y, T z)
        : std::array<T, Dim>{ x, y, z }
    {
    }

    /// <summary>Initializes x, y, z and optionally w with the given values.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr Vector(T x, T y, T z, T w = {})
        : std::array<T, Dim>{ x, y, z, w }
    {
    }

    /// <summary>Converts a three-dimensional into a four-dimensional vector with the given value for w.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr Vector(Vector<T, 3> vector, T w)
        : std::array<T, Dim>{ vector[0], vector[1], vector[2], w }
    {
    }

    /// <summary>Returns the sum of all components.</summary>
    constexpr T sum() const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i];
        return result;
    }

    /// <summary>Returns the dot-product with the given vector.</summary>
    constexpr T dot(const Vector<T, Dim>& other) const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i] * other[i];
        return result;
    }

    /// <summary>Returns the dot-product with the vector itself.</summary>
    constexpr T sqrdot() const
    {
        T result{};
        for (std::size_t i = 0; i < Dim; i++)
            result += (*this)[i] * (*this)[i];
        return result;
    }

    /// <summary>Returns the length of the vector.</summary>
    constexpr T length() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::length requires a floating point type");
        return std::sqrt(sqrdot());
    }

    /// <summary>Returns a normalized version of the vector.</summary>
    constexpr Vector<T, Dim> normalize() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::normalize requires a floating point type");
        return (*this) / length();
    }

    /// <summary>Returns a new vector, which points from the vector to the given vector.</summary>
    constexpr Vector<T, Dim> vectorTo(const Vector<T, Dim>& other) const
    {
        return other - *this;
    }

    /// <summary>Returns the distance to the given vector.</summary>
    constexpr T distanceTo(const Vector<T, Dim>& other) const
    {
        return (other - *this).length();
    }

    /// <summary>Returns the cosine of the angle to the given vector.</summary>
    constexpr T cosAngleTo(const Vector<T, Dim>& other) const
    {
        return std::clamp(dot(other) / (length() * other.length()), T(-1), T(1));
    }

    /// <summary>Returns the angle to the given vector in radians.</summary>
    constexpr T angleRadTo(const Vector<T, Dim>& other) const
    {
        return std::acos(cosAngleTo(other));
    }

    /// <summary>Returns the angle to the given vector in degrees.</summary>
    constexpr T angleTo(const Vector<T, Dim>& other) const
    {
        return radToDeg(angleRadTo(other));
    }

    /// <summary>Converts every component from radians into degrees.</summary>
    constexpr Vector<T, Dim> radToDeg() const
    {
        return unaryOp(&dmath::radToDeg<T>);
    }

    /// <summary>Converts every component from degrees into radians.</summary>
    constexpr Vector<T, Dim> degToRad() const
    {
        return unaryOp(&dmath::degToRad<T>);
    }

    /// <summary>Returns the vector with each component being positive.</summary>
    constexpr Vector<T, Dim> abs() const
    {
        return unaryOp([](T a) { return a < T(0) ? -a : a; });
    }

    /// <summary>Returns the vector with each component rounded down.</summary>
    constexpr Vector<T, Dim> floor() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::floor requires a floating point type");
        return unaryOp([](T a) { return std::floor(a); });
    }

    /// <summary>Returns the vector with each component rounded up.</summary>
    constexpr Vector<T, Dim> ceil() const
    {
        static_assert(std::is_floating_point_v<T>, "vec::ceil requires a floating point type");
        return unaryOp([](T a) { return std::ceil(a); });
    }

    /// <summary>Returns a vector, only taking the smaller components of both vectors.</summary>
    constexpr Vector<T, Dim> min(const Vector<T, Dim>& other) const
    {
        return binaryOp(other, [](T a, T b) { return a < b ? a : b; });
    }

    /// <summary>Returns a vector, only taking the larger components of both vectors.</summary>
    constexpr Vector<T, Dim> max(const Vector<T, Dim>& other) const
    {
        return binaryOp(other, [](T a, T b) { return a > b ? a : b; });
    }

    /// <summary>Reflects the vector on the given plane normal.</summary>
    constexpr Vector<T, Dim> reflect(const Vector<T, Dim>& normal) const
    {
        return *this - 2 * dot(normal) * normal;
    }

    /// <summary>Simply returns the vector.</summary>
    constexpr Vector<T, Dim> operator+() const
    {
        return *this;
    }

    /// <summary>Returns the vector with each component negated.</summary>
    constexpr Vector<T, Dim> operator-() const
    {
        return unaryOp(std::negate<T>{});
    }

    /// <summary>Component-wise addition of two vectors.</summary>
    friend constexpr Vector<T, Dim> operator+(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.binaryOp(rhs, std::plus<T>{});
    }

    /// <summary>Component-wise addition of two vectors.</summary>
    friend constexpr Vector<T, Dim>& operator+=(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.assignmentOp(rhs, std::plus<T>{});
    }

    /// <summary>Component-wise subtraction of two vectors.</summary>
    friend constexpr Vector<T, Dim> operator-(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.binaryOp(rhs, std::minus<T>{});
    }

    /// <summary>Component-wise subtraction of two vectors.</summary>
    friend constexpr Vector<T, Dim>& operator-=(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.assignmentOp(rhs, std::minus<T>{});
    }

    /// <summary>Component-wise multiplication of two vectors.</summary>
    friend constexpr Vector<T, Dim> operator*(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.binaryOp(rhs, std::multiplies<T>{});
    }

    /// <summary>Component-wise multiplication of two vectors.</summary>
    friend constexpr Vector<T, Dim>& operator*=(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.assignmentOp(rhs, std::multiplies<T>{});
    }

    /// <summary>Component-wise division of two vectors.</summary>
    friend constexpr Vector<T, Dim> operator/(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.binaryOp(rhs, std::divides<T>{});
    }

    /// <summary>Component-wise division of two vectors.</summary>
    friend constexpr Vector<T, Dim>& operator/=(Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.assignmentOp(rhs, std::divides<T>{});
    }

    /// <summary>Returns true, if, between both vectors, all components are equal.</summary>
    friend constexpr bool operator==(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.all(std::equal_to<T>{}, rhs);
    }

    /// <summary>Returns true, if, between both vectors, any components are not equal.</summary>
    friend constexpr bool operator!=(const Vector<T, Dim>& lhs, const Vector<T, Dim>& rhs)
    {
        return lhs.any(std::not_equal_to<T>{}, rhs);
    }

    /// <summary>Returns true, if, between both vectors, all components are equal.</summary>
    constexpr bool allEqualTo(const Vector<T, Dim>& other) const
    {
        return all(std::equal_to<T>{}, other);
    }

    /// <summary>Returns true, if, between both vectors, any components are equal.</summary>
    constexpr bool anyEqualTo(const Vector<T, Dim>& other) const
    {
        return any(std::equal_to<T>{}, other);
    }

    /// <summary>Returns true, if, between both vectors, no components are equal.</summary>
    constexpr bool noneEqualTo(const Vector<T, Dim>& other) const
    {
        return none(std::equal_to<T>{}, other);
    }

    /// <summary>Returns true, if, between both vectors, all components differ.</summary>
    constexpr bool allNotEqualTo(const Vector<T, Dim>& other) const
    {
        return all(std::not_equal_to<T>{}, other);
    }

    /// <summary>Returns true, if, between both vectors, any components differ.</summary>
    constexpr bool anyNotEqualTo(const Vector<T, Dim>& other) const
    {
        return any(std::not_equal_to<T>{}, other);
    }

    /// <summary>Returns true, if, between both vectors, no components differ.</summary>
    constexpr bool noneNotEqualTo(const Vector<T, Dim>& other) const
    {
        return none(std::not_equal_to<T>{}, other);
    }

    /// <summary>Returns true, if all components are less than the components of the given vector.</summary>
    constexpr bool allLess(const Vector<T, Dim>& other) const
    {
        return all(std::less<T>{}, other);
    }

    /// <summary>Returns true, if any component is less than the components of the given vector.</summary>
    constexpr bool anyLess(const Vector<T, Dim>& other) const
    {
        return any(std::less<T>{}, other);
    }

    /// <summary>Returns true, if no component is less than the components of the given vector.</summary>
    constexpr bool noneLess(const Vector<T, Dim>& other) const
    {
        return none(std::less<T>{}, other);
    }

    /// <summary>Returns true, if all components are less than or equal to the components of the given vector.</summary>
    constexpr bool allLessEqual(const Vector<T, Dim>& other) const
    {
        return all(std::less_equal<T>{}, other);
    }

    /// <summary>Returns true, if any component is less than or equal to the components of the given vector.</summary>
    constexpr bool anyLessEqual(const Vector<T, Dim>& other) const
    {
        return any(std::less_equal<T>{}, other);
    }

    /// <summary>Returns true, if no component is less than or equal to the components of the given vector.</summary>
    constexpr bool noneLessEqual(const Vector<T, Dim>& other) const
    {
        return none(std::less_equal<T>{}, other);
    }

    /// <summary>Returns true, if all components are greater than the components of the given vector.</summary>
    constexpr bool allGreater(const Vector<T, Dim>& other) const
    {
        return all(std::greater<T>{}, other);
    }

    /// <summary>Returns true, if any component is greater than the components of the given vector.</summary>
    constexpr bool anyGreater(const Vector<T, Dim>& other) const
    {
        return any(std::greater<T>{}, other);
    }

    /// <summary>Returns true, if no component is greater than the components of the given vector.</summary>
    constexpr bool noneGreater(const Vector<T, Dim>& other) const
    {
        return none(std::greater<T>{}, other);
    }

    /// <summary>Returns true, if all components are greater than or equal to the components of the given vector.</summary>
    constexpr bool allGreaterEqual(const Vector<T, Dim>& other) const
    {
        return all(std::greater_equal<T>{}, other);
    }

    /// <summary>Returns true, if any component is greater than or equal to the components of the given vector.</summary>
    constexpr bool anyGreaterEqual(const Vector<T, Dim>& other) const
    {
        return any(std::greater_equal<T>{}, other);
    }

    /// <summary>Returns true, if no component is greater than or equal to the components of the given vector.</summary>
    constexpr bool noneGreaterEqual(const Vector<T, Dim>& other) const
    {
        return none(std::greater_equal<T>{}, other);
    }

    /// <summary>Allows explicit casting between different types.</summary>
    template <typename TTarget>
    explicit constexpr operator Vector<TTarget, Dim>() const
    {
        Vector<TTarget, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = static_cast<TTarget>((*this)[i]);
        return result;
    }

    /// <summary>Used for tuple-unpacking.</summary>
    template <std::size_t Index>
    constexpr T& get() noexcept
    {
        return std::get<Index>(*this);
    }

    /// <summary>Used for tuple-unpacking.</summary>
    template <std::size_t Index>
    constexpr const T& get() const noexcept
    {
        return std::get<Index>(*this);
    }

    /// <summary>Returns a swizzle of the given components.</summary>
    template <std::size_t... Indices>
    constexpr Vector<T, sizeof...(Indices)> swizzle() const
    {
        Vector<T, sizeof...(Indices)> result;
        std::size_t resultIndex = 0;
        for (std::size_t index : std::array{ Indices... })
            result[resultIndex++] = (*this)[index];
        return result;
    }

    /// <summary>Sets a swizzle for the given components.</summary>
    template <std::size_t... Indices>
    constexpr void setSwizzle(Vector<T, sizeof...(Indices)> vector)
    {
        std::size_t otherIndex = 0;
        for (std::size_t index : std::array{ Indices... })
            (*this)[index] = vector[otherIndex++];
    }

    /// <summary>Performs a unary operation on each component and returns the result.</summary>
    template <typename TOperation>
    constexpr Vector<T, Dim> unaryOp(TOperation operation) const
    {
        Vector<T, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = operation((*this)[i]);
        return result;
    }

    /// <summary>Performs a binary operation with another vector and returns the result.</summary>
    template <typename TOperation>
    constexpr Vector<T, Dim> binaryOp(const Vector<T, Dim>& other, TOperation operation) const
    {
        Vector<T, Dim> result;
        for (std::size_t i = 0; i < Dim; i++)
            result[i] = operation((*this)[i], other[i]);
        return result;
    }

    /// <summary>Performs a binary operation with another vector and assigns the result to itself and returns it.</summary>
    template <typename TOperation>
    constexpr Vector<T, Dim>& assignmentOp(const Vector<T, Dim>& other, TOperation operation)
    {
        for (std::size_t i = 0; i < Dim; i++)
            (*this)[i] = operation((*this)[i], other[i]);
        return *this;
    }

    /// <summary>Returns, wether all components satisfy a given predicate.</summary>
    template <typename TPredicate, typename... TOthers>
    constexpr bool all(TPredicate predicate, const TOthers&... others) const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if (!predicate((*this)[i], others[i]...))
                return false;
        return true;
    }

    /// <summary>Returns, wether any component satisfies a given predicate.</summary>
    template <typename TPredicate, typename... TOthers>
    constexpr bool any(TPredicate predicate, const TOthers&... others) const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if (predicate((*this)[i], others[i]...))
                return true;
        return false;
    }

    /// <summary>Returns, wether none of the components satisfy a given predicate.</summary>
    template <typename TPredicate, typename... TOthers>
    constexpr bool none(TPredicate predicate, const TOthers&... others) const
    {
        for (std::size_t i = 0; i < Dim; i++)
            if (predicate((*this)[i], others[i]...))
                return false;
        return true;
    }

    // --- Vector<T, 1> ---  

    /// <summary>The x-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 1 && Dim <= 4>>
    constexpr T& x()
    {
        return std::get<0>(*this);
    }

    /// <summary>The x-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 1 && Dim <= 4>>
    constexpr T x() const
    {
        return std::get<0>(*this);
    }

    /// <summary>Allows for implicit conversion between a one-dimensional vector and its base type.</summary>
    template <typename = std::enable_if_t<Dim == 1>>
    constexpr operator T& ()
    {
        return std::get<0>(*this);
    }

    /// <summary>Allows for implicit conversion between a one-dimensional vector and its base type.</summary>
    template <typename = std::enable_if_t<Dim == 1>>
    constexpr operator T() const
    {
        return std::get<0>(*this);
    }

    // --- Vector<T, 2> ---

    /// <summary>The y-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 2 && Dim <= 4>>
    constexpr T& y()
    {
        return std::get<1>(*this);
    }

    /// <summary>The y-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 2 && Dim <= 4>>
    constexpr T y() const
    {
        return std::get<1>(*this);
    }

    /// <summary>Creates a vector from the given slope, which is NOT normalized.</summary>
    /// <remarks>The x-component is always one except if std::nullopt is given, which returns a vertical vector of length one.</remarks>
    template <typename = std::enable_if_t<Dim == 2>>
    static constexpr Vector<T, 2> fromSlope(std::optional<T> slope)
    {
        return slope ? Vector<T, 2>(1, *slope) : Vector<T, 2>(0, 1);
    }

    /// <summary>Creates a normalized vector of the given angle in radians.</summary>
    /// <remarks>Zero points to positive x, while an increase rotates counter-clockwise.</remarks>
    template <typename = std::enable_if_t<Dim == 2>>
    static constexpr Vector<T, 2> fromAngleRad(T radians)
    {
        return { std::cos(radians), std::sin(radians) };
    }

    /// <summary>Creates a normalized vector of the given angle in degrees.</summary>
    /// <remarks>Zero points to positive x, while an increase rotates counter-clockwise.</remarks>
    template <typename = std::enable_if_t<Dim == 2>>
    static constexpr Vector<T, 2> fromAngle(T degrees)
    {
        return fromAngleRad(degToRad(degrees));
    }

    /// <summary>Rotates the vector counter-clockwise by 90 degrees by simply swapping its components and negating the new x.</summary>
    template <typename = std::enable_if_t<Dim == 2>>
    constexpr Vector<T, 2> cross() const
    {
        return { -y(), x() };
    }

    /// <summary>Returns the two-dimensional cross-product with the given vector.</summary>
    /// <remarks>Equivalent to <c>x1 * y2 - y1 * x2</c></remarks>
    template <typename = std::enable_if_t<Dim == 2>>
    constexpr T cross(const Vector<T, 2>& other) const
    {
        return x() * other.y() - y() * other.x();
    }

    /// <summary>Returns the slope of the vector or std::nullopt if infinite.</summary>
    template <typename = std::enable_if_t<Dim == 2>>
    constexpr std::optional<T> slope() const
    {
        static_assert(std::is_floating_point_v<T>, "vec2::slope requires a floating point type");
        if (x() != T())
            return y() / x();
        return std::nullopt;
    }

    // --- Vector<T, 3> ---

    /// <summary>The z-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 3 && Dim <= 4>>
    constexpr T& z()
    {
        return std::get<2>(*this);
    }

    /// <summary>The z-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim >= 3 && Dim <= 4>>
    constexpr T z() const
    {
        return std::get<2>(*this);
    }

    /// <summary>Returns the cross-product with the given vector.</summary>
    template <typename = std::enable_if_t<Dim == 3>>
    constexpr Vector<T, 3> cross(const Vector<T, 3>& other) const
    {
        return { (*this)[1] * other[2] - (*this)[2] * other[1],
                 (*this)[2] * other[0] - (*this)[0] * other[2],
                 (*this)[0] * other[1] - (*this)[1] * other[0] };
    }

    // --- Vector<T, 4> ---

    /// <summary>The w-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr T& w()
    {
        return std::get<3>(*this);
    }

    /// <summary>The w-component of the vector.</summary>
    template <typename = std::enable_if_t<Dim == 4>>
    constexpr T w() const
    {
        return std::get<3>(*this);
    }

    // --- Swizzles ---

#define DMATH_DEFINE_SWIZZLE(name, ...) \
constexpr Vector<T, sizeof(#name) - 1> name() const { return this->swizzle<__VA_ARGS__>(); } \
inline void set_ ## name(const Vector<T, sizeof(#name) - 1>& vector) { this->setSwizzle<__VA_ARGS__>(vector); }

    DMATH_DEFINE_SWIZZLE(xy, 0, 1); ;
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

template <typename T, std::size_t Dim>
struct std::tuple_size<Vector<T, Dim>> {
    static constexpr int value = Dim;
};

template <typename T, std::size_t Dim, std::size_t Index>
struct std::tuple_element<Index, Vector<T, Dim>> {
    using type = T;
};

}
