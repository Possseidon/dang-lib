#pragma once

#include "dang-math/enums.h"
#include "dang-math/global.h"
#include "dang-math/utils.h"

namespace dang::math {

// TODO: C++20 Replace SFINAE/static_assert with requires

/// @brief A vector of the templated type and dimension, using std::array as base.
template <typename T, std::size_t v_dim>
struct Vector : std::array<T, v_dim> {
    using Base = std::array<T, v_dim>;

    /// @brief Initializes all values with default values.
    constexpr Vector()
        : Base()
    {}

    /// @brief Implicit conversion between scalars and a vector containing said scalar for each component.
    /// @remark Being implicit greatly simplifies operator overloading, however GLSL does not provide this implicit
    /// conversion.
    /// @remark GLSL only allows this conversion when used in conjunction with an actual mathematical operations.
    constexpr Vector(T value)
        : Base()
    {
        for (std::size_t i = 0; i < v_dim; i++)
            (*this)[i] = value;
    }

    /// @brief Initializes x and y with the given values.
    constexpr Vector(T x, T y)
        : Base{x, y}
    {
        static_assert(v_dim == 2);
    }

    /// @brief Initializes x, y and z with the given values.
    constexpr Vector(T x, T y, T z)
        : Base{x, y, z}
    {
        static_assert(v_dim == 3);
    }

    /// @brief Initializes x, y, z and w with the given values.
    constexpr Vector(T x, T y, T z, T w)
        : Base{x, y, z, w}
    {
        static_assert(v_dim == 4);
    }

    /// @brief Converts a three-dimensional into a four-dimensional vector with the given value for w.
    /// @remark GLSL allows this kind of concatenation for any number and size of vectors, however, this is tedious to
    /// implement in C++.
    /// @remark Therefore, only this probably most common overload for turning vec3 into vec4 is provided.
    constexpr Vector(Vector<T, 3> vector, T w)
        : Base{vector[0], vector[1], vector[2], w}
    {
        static_assert(v_dim == 4);
    }

    /// @brief Allows for explicit conversion between vectors of same size but different types.
    template <typename TFrom>
    explicit constexpr Vector(const Vector<TFrom, v_dim>& other)
        : Vector()
    {
        for (std::size_t i = 0; i < v_dim; i++)
            (*this)[i] = static_cast<T>(other[i]);
    }

    /// @brief Allows for explicit conversion from single-value vectors to their respective value type.
    explicit constexpr operator T() const
    {
        static_assert(v_dim == 1);
        return (*this)[0];
    }

    using Base::operator[];

    constexpr T& operator[](Axis<v_dim> axis) { return (*this)[static_cast<size_t>(axis)]; }
    constexpr T operator[](Axis<v_dim> axis) const { return (*this)[static_cast<size_t>(axis)]; }

    /// @brief Returns the sum of all components.
    constexpr auto sum() const
    {
        static_assert(!std::is_same_v<T, bool>);
        T result{};
        for (std::size_t i = 0; i < v_dim; i++)
            result += (*this)[i];
        return result;
    }

    /// @brief Returns the product of all components.
    constexpr auto product() const
    {
        static_assert(!std::is_same_v<T, bool>);
        T result{1};
        for (std::size_t i = 0; i < v_dim; i++)
            result *= (*this)[i];
        return result;
    }

    /// @brief Returns the dot-product with the given vector.
    constexpr auto dot(const Vector& other) const
    {
        static_assert(!std::is_same_v<T, bool>);
        T result{};
        for (std::size_t i = 0; i < v_dim; i++)
            result += (*this)[i] * other[i];
        return result;
    }

    /// @brief Returns the dot-product with the vector itself.
    constexpr auto sqrdot() const
    {
        static_assert(!std::is_same_v<T, bool>);
        T result{};
        for (std::size_t i = 0; i < v_dim; i++)
            result += (*this)[i] * (*this)[i];
        return result;
    }

    /// @brief Returns the length of the vector.
    /// @remark In GLSL vec3(0).length() returns the component count.
    constexpr auto length() const
    {
        static_assert(std::is_floating_point_v<T>);
        return std::sqrt(sqrdot());
    }

    /// @brief Returns a normalized version of the vector.
    constexpr auto normalize() const
    {
        static_assert(std::is_floating_point_v<T>);
        return (*this) / length();
    }

    /// @brief Returns a new vector, which points from the vector to the given vector.
    constexpr auto vectorTo(const Vector& other) const
    {
        static_assert(!std::is_same_v<T, bool>);
        return other - *this;
    }

    /// @brief Returns the distance to the given vector.
    constexpr auto distanceTo(const Vector& other) const
    {
        static_assert(std::is_floating_point_v<T>);
        return (other - *this).length();
    }

    /// @brief Returns the cosine of the angle to the given vector.
    constexpr auto cosAngleTo(const Vector& other) const
    {
        static_assert(std::is_floating_point_v<T>);
        return std::clamp(dot(other) / (length() * other.length()), T{-1}, T{1});
    }

    /// @brief Returns the angle to the given vector in radians.
    constexpr auto radiansTo(const Vector& other) const
    {
        static_assert(std::is_floating_point_v<T>);
        return std::acos(cosAngleTo(other));
    }

    /// @brief Returns the angle to the given vector in degrees.
    constexpr auto degreesTo(const Vector& other) const
    {
        static_assert(std::is_floating_point_v<T>);
        return dang::math::degrees(radiansTo(other));
    }

    /// @brief Converts every component from degrees into radians.
    constexpr auto radians() const
    {
        static_assert(std::is_floating_point_v<T>);
        return variadicOp(dang::math::radians<T>);
    }

    /// @brief Converts every component from radians into degrees.
    constexpr auto degrees() const
    {
        static_assert(std::is_floating_point_v<T>);
        return variadicOp(dang::math::degrees<T>);
    }

    /// @brief Returns the vector with each component being positive.
    constexpr auto abs() const
    {
        static_assert(std::is_signed_v<T>);
        return variadicOp([](T a) { return a < T{0} ? -a : a; });
    }

    /// @brief Returns the vector with each component rounded down.
    constexpr auto floor() const
    {
        static_assert(std::is_floating_point_v<T>);
        return variadicOp([](T a) { return std::floor(a); });
    }

    /// @brief Returns the vector with each component rounded up.
    constexpr auto ceil() const
    {
        static_assert(std::is_floating_point_v<T>);
        return variadicOp([](T a) { return std::ceil(a); });
    }

    /// @brief Returns the axis with the lowest value.
    constexpr auto minAxis() const
    {
        auto min_iter = std::min_element(this->begin(), this->end());
        return static_cast<Axis<v_dim>>(std::distance(this->begin(), min_iter));
    }

    /// @brief Returns the axis with the highest value.
    constexpr auto maxAxis() const
    {
        auto max_iter = std::max_element(this->begin(), this->end());
        return static_cast<Axis<v_dim>>(std::distance(this->begin(), max_iter));
    }

    /// @brief Returns a pair of the two axes with the lowest and highest values.
    constexpr auto minMaxAxis() const
    {
        auto [min_iter, max_iter] = std::minmax_element(this->begin(), this->end());
        return std::pair{static_cast<Axis<v_dim>>(std::distance(this->begin(), min_iter)),
                         static_cast<Axis<v_dim>>(std::distance(this->begin(), max_iter))};
    }

    /// @brief Returns the value of the lowest component.
    constexpr auto minValue() const { return (*this)[minAxis()]; }

    /// @brief Returns the value of the highest component.
    constexpr auto maxValue() const { return (*this)[maxAxis()]; }

    /// @brief Returns a pair of the two values for the lowest and highest components.
    constexpr auto minMaxValue() const
    {
        auto [min_axis, max_axis] = minMaxAxis();
        return std::pair{(*this)[min_axis], (*this)[max_axis]};
    }

    /// @brief Returns a vector, only taking the smaller components of both vectors.
    constexpr auto min(const Vector& other) const
    {
        static_assert(!std::is_same_v<T, bool>);
        return variadicOp([](T a, T b) { return std::min(a, b); }, other);
    }

    /// @brief Returns a vector, only taking the larger components of both vectors.
    constexpr auto max(const Vector& other) const
    {
        static_assert(!std::is_same_v<T, bool>);
        return variadicOp([](T a, T b) { return std::max(a, b); }, other);
    }

    /// @brief Returns a vector, for which each component is clamped between low and high.
    constexpr auto clamp(const Vector& low, const Vector& high) const
    {
        static_assert(!std::is_same_v<T, bool>);
        return variadicOp([](T a, T b, T c) { return std::clamp(a, b, c); }, low, high);
    }

    /// @brief Reflects the vector on the given plane normal.
    /// @remark The normal is assumed to be normalized.
    constexpr auto reflect(const Vector& normal) const
    {
        static_assert(!std::is_same_v<T, bool>);
        return *this - 2 * dot(normal) * normal;
    }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto lessThan(const Vector& other) const { return variadicOp(std::less{}, other); }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto lessThanEqual(const Vector& other) const { return variadicOp(std::less_equal{}, other); }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto greaterThan(const Vector& other) const { return variadicOp(std::greater{}, other); }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto greaterThanEqual(const Vector& other) const { return variadicOp(std::greater_equal{}, other); }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto equal(const Vector& other) const { return variadicOp(std::equal_to{}, other); }

    /// @brief Component-wise comparison, returning a bvec.
    constexpr auto notEqual(const Vector& other) const { return variadicOp(std::not_equal_to{}, other); }

    /// @brief Provided as constexpr, as std::array does not.
    constexpr auto operator==(const Vector& other) const { return equal(other).all(); }

    /// @brief Provided as constexpr, as std::array does not.
    constexpr auto operator!=(const Vector& other) const { return notEqual(other).any(); }

    /// @brief Whether all components are true.
    constexpr auto all() const
    {
        static_assert(std::is_same_v<T, bool>);
        for (std::size_t i = 0; i < v_dim; i++)
            if (!(*this)[i])
                return false;
        return true;
    }

    /// @brief Whether any component is true.
    constexpr auto any() const
    {
        static_assert(std::is_same_v<T, bool>);
        for (std::size_t i = 0; i < v_dim; i++)
            if ((*this)[i])
                return true;
        return false;
    }

    /// @brief Whether no component is true.
    constexpr auto none() const
    {
        static_assert(std::is_same_v<T, bool>);
        for (std::size_t i = 0; i < v_dim; i++)
            if ((*this)[i])
                return false;
        return true;
    }

    /// @brief Inverts each component.
    /// @remark Known as "not" in GLSL, which cannot be used in C++.
    constexpr auto invert() const
    {
        static_assert(std::is_same_v<T, bool>);
        return variadicOp(std::logical_not{});
    }

    /// @brief Simply returns the vector.
    constexpr auto operator+() const
    {
        static_assert(!std::is_same_v<T, bool>);
        return *this;
    }

    /// @brief Returns the vector with each component negated.
    constexpr auto operator-() const
    {
        static_assert(std::is_signed_v<T>);
        return variadicOp(std::negate{});
    }

    /// @brief Component-wise addition of two vectors.
    friend constexpr auto operator+(const Vector& lhs, const Vector& rhs)
    {
        static_assert(!std::is_same_v<T, bool>);
        return lhs.variadicOp(std::plus{}, rhs);
    }

    /// @brief Component-wise addition of two vectors.
    constexpr auto& operator+=(const Vector& other)
    {
        static_assert(!std::is_same_v<T, bool>);
        return assignmentOp(std::plus{}, other);
    }

    /// @brief Component-wise subtraction of two vectors.
    friend constexpr auto operator-(const Vector& lhs, const Vector& rhs)
    {
        static_assert(!std::is_same_v<T, bool>);
        return lhs.variadicOp(std::minus{}, rhs);
    }

    /// @brief Component-wise subtraction of two vectors.
    constexpr auto& operator-=(const Vector& other)
    {
        static_assert(!std::is_same_v<T, bool>);
        return assignmentOp(std::minus{}, other);
    }

    /// @brief Component-wise multiplication of two vectors.
    friend constexpr auto operator*(const Vector& lhs, const Vector& rhs)
    {
        static_assert(!std::is_same_v<T, bool>);
        return lhs.variadicOp(std::multiplies{}, rhs);
    }

    /// @brief Component-wise multiplication of two vectors.
    constexpr auto& operator*=(const Vector& other)
    {
        static_assert(!std::is_same_v<T, bool>);
        return assignmentOp(std::multiplies{}, other);
    }

    /// @brief Component-wise division of two vectors.
    friend constexpr auto operator/(const Vector& lhs, const Vector& rhs)
    {
        static_assert(!std::is_same_v<T, bool>);
        return lhs.variadicOp(std::divides{}, rhs);
    }

    /// @brief Component-wise division of two vectors.
    constexpr auto& operator/=(const Vector& other)
    {
        static_assert(!std::is_same_v<T, bool>);
        return assignmentOp(std::divides{}, other);
    }

    /// @brief Returns a swizzle of the given components.
    template <std::size_t... v_indices>
    constexpr auto swizzle() const
    {
        return Vector<T, sizeof...(v_indices)>{std::get<v_indices>(*this)...};
    }

    /// @brief Sets a swizzle for the given components.
    template <std::size_t... v_indices, std::size_t... v_other_indices>
    constexpr void setSwizzleHelper(Vector<T, sizeof...(v_indices)> vector, std::index_sequence<v_other_indices...>)
    {
        ((std::get<v_indices>(*this) = std::get<v_other_indices>(vector)), ...);
    }

    /// @brief Sets a swizzle for the given components.
    template <std::size_t... v_indices>
    constexpr void setSwizzle(Vector<T, sizeof...(v_indices)> vector)
    {
        setSwizzleHelper<v_indices...>(vector, std::make_index_sequence<sizeof...(v_indices)>());
    }

    /// @brief Performs an operation on each component using an arbitrary number of other vectors.
    template <typename TOperation, typename... TVectors>
    constexpr auto variadicOp(TOperation operation, const TVectors&... vectors) const
    {
        Vector<decltype(operation((*this)[0], vectors[0]...)), v_dim> result;
        for (std::size_t i = 0; i < v_dim; i++)
            result[i] = operation((*this)[i], vectors[i]...);
        return result;
    }

    /// @brief Performs an operation with another vector and assigns the result to itself.
    template <typename TOperation>
    constexpr auto& assignmentOp(TOperation operation, const Vector& other)
    {
        for (std::size_t i = 0; i < v_dim; i++)
            (*this)[i] = operation((*this)[i], other[i]);
        return *this;
    }

    /// @brief Returns a string representing the vector in the form [x, y, z].
    auto format() const { return (std::stringstream() << *this).str(); }

    /// @brief Appends a string representation of the vector in the form [x, y, z] to the stream.
    friend auto& operator<<(std::ostream& stream, const Vector& vector)
    {
        auto old_flags = stream.flags();
        if constexpr (std::is_same_v<T, bool>)
            stream << std::boolalpha;

        stream << '[';
        if constexpr (v_dim > 0) {
            stream << vector[0];
            for (std::size_t i = 1; i < v_dim; i++)
                stream << ", " << vector[i];
        }
        if constexpr (std::is_same_v<T, bool>)
            stream.flags(old_flags);
        return stream << ']';
    }

    // --- Vector<T, 1> ---

    /// @brief The x-component of the vector.
    constexpr auto& x()
    {
        static_assert(v_dim >= 1 && v_dim <= 4);
        return (*this)[0];
    }

    /// @brief The x-component of the vector.
    constexpr auto x() const
    {
        static_assert(v_dim >= 1 && v_dim <= 4);
        return (*this)[0];
    }

    // --- Vector<T, 2> ---

    /// @brief The y-component of the vector.
    constexpr auto& y()
    {
        static_assert(v_dim >= 2 && v_dim <= 4);
        return (*this)[1];
    }

    /// @brief The y-component of the vector.
    constexpr auto y() const
    {
        static_assert(v_dim >= 1 && v_dim <= 4);
        return (*this)[1];
    }

    /// @brief Creates a vector from the given slope, which is NOT normalized.
    /// @remark The x-component is always one except if std::nullopt is given, which returns a vertical vector of length
    /// one.
    static constexpr auto fromSlope(std::optional<T> slope)
    {
        static_assert(v_dim == 2);
        static_assert(!std::is_same_v<T, bool>);
        return slope ? Vector<T, 2>(1, *slope) : Vector<T, 2>(0, 1);
    }

    /// @brief Creates a normalized vector of the given angle in radians.
    /// @remark Zero points to positive x, while an increase rotates counter-clockwise.
    static constexpr auto fromRadians(T radians)
    {
        static_assert(v_dim == 2);
        static_assert(std::is_floating_point_v<T>);
        return Vector<T, 2>{std::cos(radians), std::sin(radians)};
    }

    /// @brief Creates a normalized vector of the given angle in degrees.
    /// @remark Zero points to positive x, while an increase rotates counter-clockwise.
    static constexpr auto fromDegrees(T degrees)
    {
        static_assert(v_dim == 2);
        static_assert(std::is_floating_point_v<T>);
        return fromRadians(dang::math::radians(degrees));
    }

    /// @brief Rotates the vector counter-clockwise by 90 degrees by simply swapping its components and negating the new
    /// x.
    constexpr auto cross() const
    {
        static_assert(v_dim == 2);
        static_assert(std::is_signed_v<T>);
        return Vector<T, 2>{-y(), x()};
    }

    /// @brief Returns the two-dimensional cross-product with the given vector.
    /// @remark Equivalent to <c>x1 * y2 - y1 * x2</c>
    constexpr auto cross(const Vector<T, 2>& other) const
    {
        static_assert(v_dim == 2);
        static_assert(!std::is_same_v<T, bool>);
        return x() * other.y() - y() * other.x();
    }

    /// @brief Returns the slope of the vector or std::nullopt if infinite.
    constexpr std::optional<T> slope() const
    {
        static_assert(v_dim == 2);
        static_assert(std::is_floating_point_v<T>);
        if (x() != T())
            return y() / x();
        return std::nullopt;
    }

    // --- Vector<T, 3> ---

    /// @brief The z-component of the vector.
    constexpr auto& z()
    {
        static_assert(v_dim >= 3 && v_dim <= 4);
        return (*this)[2];
    }

    /// @brief The z-component of the vector.
    constexpr auto z() const
    {
        static_assert(v_dim >= 3 && v_dim <= 4);
        return (*this)[2];
    }

    /// @brief Returns the cross-product with the given vector.
    constexpr auto cross(const Vector<T, 3>& other) const
    {
        static_assert(v_dim == 3);
        static_assert(!std::is_same_v<T, bool>);
        return Vector<T, 3>{(*this)[1] * other[2] - (*this)[2] * other[1],
                            (*this)[2] * other[0] - (*this)[0] * other[2],
                            (*this)[0] * other[1] - (*this)[1] * other[0]};
    }

    // --- Vector<T, 4> ---

    /// @brief The w-component of the vector.
    constexpr auto& w()
    {
        static_assert(v_dim == 4);
        return (*this)[3];
    }

    /// @brief The w-component of the vector.
    constexpr auto w() const
    {
        static_assert(v_dim == 4);
        return (*this)[3];
    }

    // --- Swizzles ---

#define DMATH_DEFINE_SWIZZLE(name, ...)                                                                                \
    constexpr auto name() const                                                                                        \
    {                                                                                                                  \
        static_assert(v_dim >= sizeof(#name) - 1);                                                                     \
        return swizzle<__VA_ARGS__>();                                                                                 \
    }                                                                                                                  \
    void set_##name(const Vector<T, sizeof(#name) - 1>& vector)                                                        \
    {                                                                                                                  \
        static_assert(v_dim >= sizeof(#name) - 1);                                                                     \
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

template <std::size_t v_dim>
using vec = Vector<float, v_dim>;

template <std::size_t v_dim>
using dvec = Vector<double, v_dim>;

template <std::size_t v_dim>
using ivec = Vector<int, v_dim>;

template <std::size_t v_dim>
using uvec = Vector<unsigned, v_dim>;

template <std::size_t v_dim>
using svec = Vector<std::size_t, v_dim>;

template <std::size_t v_dim>
using bvec = Vector<bool, v_dim>;

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

template <typename T, std::size_t v_dim>
struct std::tuple_size<dang::math::Vector<T, v_dim>> : tuple_size<array<T, v_dim>> {};

template <typename T, std::size_t v_dim, std::size_t v_index>
struct std::tuple_element<v_index, dang::math::Vector<T, v_dim>> : tuple_element<v_index, array<T, v_dim>> {};
