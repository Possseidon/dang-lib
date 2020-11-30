#pragma once

#include "utils.h"

#include "dang-utils/enum.h"

#include "consts.h"
#include "enums.h"
#include "vector.h"

namespace dang::math {

namespace detail {

/// <summary>Performs a floor division on the given arguments.</summary>
template <typename T>
constexpr T floordiv(T numerator, T denominator)
{
    if constexpr (std::is_floating_point_v<T>) {
        T value = numerator / denominator;
        T trunc = static_cast<T>(static_cast<long long>(value));
        T frac = value - trunc;
        return frac < 0 ? trunc - 1 : trunc;
    }
    else if constexpr (std::is_integral_v<T>) {
        if ((numerator > 0) == (denominator > 0))
            return numerator / denominator;
        return (numerator - denominator + 1) / denominator;
    }
}

/// <summary>Uses floordiv to implement a floor modulus.</summary>
template <typename T>
static constexpr T floormod(T numerator, T denominator)
{
    return numerator - floordiv(numerator, denominator) * denominator;
}

} // namespace detail

template <typename T, std::size_t Dim>
struct Bounds;

/// <summary>An iterator, allowing iteration of any-dimensional integral bounds.</summary>
/// <remarks>
/// The last vector component iterates first, followed by the second to last, etc...
/// This results in better caching for the common use-case of iterating an array[x][y][z]
/// </remarks>
template <typename T, std::size_t Dim>
struct BoundsIterator {
    static_assert(std::is_integral_v<T>, "BoundsIterator can only be used with integral types");

    using iterator_category = std::forward_iterator_tag;
    using value_type = Vector<T, Dim>;
    using difference_type = std::ptrdiff_t;
    using pointer = Vector<T, Dim>*;
    using reference = Vector<T, Dim>&;

    constexpr BoundsIterator() = default;

    explicit constexpr BoundsIterator(Bounds<T, Dim> bounds, Vector<T, Dim> current)
        : bounds_(bounds)
        , current_(current)
    {}

    constexpr BoundsIterator& operator++()
    {
        current_[Dim - 1]++;
        for (std::size_t d = Dim - 1; d != 0; d--) {
            if (current_[d] < bounds_.high[d])
                break;
            current_[d] = bounds_.low[d];
            current_[d - 1]++;
        }
        return *this;
    }

    constexpr BoundsIterator& operator++(int)
    {
        BoundsIterator old(*this);
        ++(*this);
        return old;
    }

    constexpr bool operator==(const BoundsIterator& other) const { return current_ == other.current_; }

    constexpr bool operator!=(const BoundsIterator& other) const { return current_ != other.current_; }

    constexpr const Vector<T, Dim>& operator*() const { return current_; }

    constexpr const Vector<T, Dim>* operator->() const { return &current_; }

private:
    Bounds<T, Dim> bounds_;
    Vector<T, Dim> current_;
};

/// <summary>Generic bounds with low and high values for any dimensional vectors.</summary>
/// <remarks>Integral bounds are high exclusive: [low, high)</remarks>
template <typename T, std::size_t Dim>
struct Bounds {
    Vector<T, Dim> low;
    Vector<T, Dim> high;

    /// <summary>Initializes the bounds with zero-vectors for low and high.</summary>
    constexpr Bounds() = default;

    /// <summary>Initializes high with the given value and low with zero.</summary>
    explicit constexpr Bounds(const Vector<T, Dim>& high)
        : high(high)
    {}

    /// <summary>Initializes low and high with the given values.</summary>
    constexpr Bounds(const Vector<T, Dim>& low, const Vector<T, Dim>& high)
        : low(low)
        , high(high)
    {}

    /// <summary>Provides simplified access for one-dimensional bounds.</summary>
    T& lowValue()
    {
        static_assert(Dim == 1);
        return low.x();
    }

    /// <summary>Provides simplified access for one-dimensional bounds.</summary>
    constexpr T lowValue() const
    {
        static_assert(Dim == 1);
        return low.x();
    }

    /// <summary>Provides simplified access for one-dimensional bounds.</summary>
    T& highValue()
    {
        static_assert(Dim == 1);
        return high.x();
    }

    /// <summary>Provides simplified access for one-dimensional bounds.</summary>
    constexpr T highValue() const
    {
        static_assert(Dim == 1);
        return high.x();
    }

    /// <summary>Returns true, when high is bigger than or equal to low.</summary>
    constexpr bool isNormalized() const { return low.allLessEqual(high); }

    /// <summary>Returns bounds with any non-normalized components swapped.</summary>
    constexpr Bounds normalize() const
    {
        Bounds result;
        for (std::size_t i = 0; i < Dim; i++) {
            if (low[i] <= high[i]) {
                result.low[i] = low[i];
                result.high[i] = high[i];
            }
            else {
                result.low[i] = high[i];
                result.high[i] = low[i];
            }
        }
        return result;
    }

    /// <summary>Returns the size of the bounds, which is equal to <c>high - low</c>.</summary>
    constexpr Vector<T, Dim> size() const { return high - low; }

    /// <summary>Returns the center of the bounds, rounded down for integral types.</summary>
    constexpr Vector<T, Dim> center() const
    {
        if constexpr (std::is_integral_v<T>)
            return (low + high - 1) / T(2);
        else
            return (low + high) + size() / T(2);
    }

    /// <summary>Returns true, if other is enclosed by the calling bounds.</summary>
    constexpr bool contains(const Bounds& other) const
    {
        return other.low.allGreaterEqual(low) && other.high.allLessEqual(high);
    }

    /// <summary>Returns true, if other is enclosed by the calling bounds.</summary>
    /// <remarks>Comparison is inclusive for low and exclusive for high.</remarks>
    constexpr bool contains(const Vector<T, Dim>& vector) const
    {
        return vector.allGreaterEqual(low) && vector.allLess(high);
    }

    /// <summary>Returns true, if other is enclosed by the calling bounds.</summary>
    /// <remarks>Comparison is inclusive for both low and high.</remarks>
    constexpr bool containsInclusive(const Vector<T, Dim>& vector) const
    {
        return vector.allGreaterEqual(low) && vector.allLessEqual(high);
    }

    /// <summary>Returns true, if other is enclosed by the calling bounds.</summary>
    /// <remarks>Comparison is exclusive for both low and high.</remarks>
    constexpr bool containsExclusive(const Vector<T, Dim>& vector) const
    {
        return vector.allGreater(low) && vector.allLess(high);
    }

    /// <summary>Clamps the given bounds, resulting in an intersection of both bounds.</summary>
    constexpr Bounds clamp(const Bounds& other) const { return {low.max(other.low), high.min(other.high)}; }

    /// <summary>Clamps the given point into the calling bounds.</summary>
    /// <remarks>For integral types, high is clamped exclusive.</remarks>
    constexpr Vector<T, Dim> clamp(const Vector<T, Dim>& vector) const
    {
        if constexpr (std::is_integral_v<T>)
            return vector.max(low).min(high - 1);
        else
            return vector.max(low).min(high);
    }

    /// <summary>Returns the result of a symmetrical modulus on the given vector.</summary>
    constexpr Vector<T, Dim> mod(Vector<T, Dim> vector) const
    {
        for (std::size_t i = 0; i < Dim; i++)
            vector[i] = low[i] + detail::floormod(vector[i] - low[i], high[i] - low[i]);
        return vector;
    }

    /// <summary>Returns the bounds offset by the given amount.</summary>
    constexpr Bounds offset(Vector<T, Dim> amount) const { return {low + amount, high + amount}; }

    /// <summary>Returns the bounds outset by the given amount.</summary>
    constexpr Bounds outset(Vector<T, Dim> amount) const { return {low - amount, high + amount}; }

    /// <summary>Returns the bounds inset by the given amount.</summary>
    constexpr Bounds inset(Vector<T, Dim> amount) const { return {low + amount, high - amount}; }

    /// <summary>Returns an enum-array, mapping corners to the actual positions of the corners.</summary>
    constexpr dutils::EnumArray<Corner<Dim>, Vector<T, Dim>> corners() const
    {
        static_assert(Dim >= 1 && Dim <= 3);
        dutils::EnumArray<Corner<Dim>, Vector<T, Dim>> result;
        for (auto corner : Corner<Dim>())
            result[corner] = low + Vector<T, Dim>(CornerVector<Dim>[corner]) * (high - low);
        return result;
    }

    /// <summary>Returns true, if both bounds are identical.</summary>
    friend constexpr bool operator==(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low == rhs.low && lhs.high == rhs.high;
    }

    /// <summary>Returns true, if either low or high differs between the given bounds.</summary>
    friend constexpr bool operator!=(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low != rhs.low || lhs.high != rhs.high;
    }

    /// <summary>Returns true, if high of the first bounds is less than low of the second bounds.</summary>
    friend constexpr bool operator<(const Bounds& lhs, const Bounds& rhs) { return lhs.high.allLess(rhs.low); }

    /// <summary>Returns true, if high of the first bounds is less than or equal to low of the second bounds.</summary>
    friend constexpr bool operator<=(const Bounds& lhs, const Bounds& rhs) { return lhs.high.allLessEqual(rhs.low); }

    /// <summary>Returns true, if low of the first bounds is greater than high of the second bounds.</summary>
    friend constexpr bool operator>(const Bounds& lhs, const Bounds& rhs) { return lhs.low.allGreater(rhs.high); }

    /// <summary>Returns true, if low of the first bounds is greater than or equal to high of the second bounds.</summary>
    friend constexpr bool operator>=(const Bounds& lhs, const Bounds& rhs) { return lhs.low.allGreaterEqual(rhs.high); }

    /// <summary>Returns a bounds-iterator, allowing for range-based iteration.</summary>
    constexpr BoundsIterator<T, Dim> begin() const { return BoundsIterator<T, Dim>(*this, low); }

    /// <summary>Returns a bounds-iterator, allowing for range-based iteration.</summary>
    constexpr BoundsIterator<T, Dim> end() const { return ++BoundsIterator<T, Dim>(*this, high - 1); }
};

template <std::size_t Dim>
using bounds = Bounds<float, Dim>;

template <std::size_t Dim>
using dbounds = Bounds<double, Dim>;

template <std::size_t Dim>
using ibounds = Bounds<int, Dim>;

template <std::size_t Dim>
using ubounds = Bounds<unsigned, Dim>;

template <std::size_t Dim>
using sbounds = Bounds<std::size_t, Dim>;

using bounds1 = bounds<1>;
using bounds2 = bounds<2>;
using bounds3 = bounds<3>;

using dbounds1 = dbounds<1>;
using dbounds2 = dbounds<2>;
using dbounds3 = dbounds<3>;

using ibounds1 = ibounds<1>;
using ibounds2 = ibounds<2>;
using ibounds3 = ibounds<3>;

using ubounds1 = ubounds<1>;
using ubounds2 = ubounds<2>;
using ubounds3 = ubounds<3>;

using sbounds1 = sbounds<1>;
using sbounds2 = sbounds<2>;
using sbounds3 = sbounds<3>;

} // namespace dang::math
