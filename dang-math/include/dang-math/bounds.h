#pragma once

#include "dang-math/consts.h"
#include "dang-math/enums.h"
#include "dang-math/global.h"
#include "dang-math/vector.h"

#include "dang-utils/enum.h"

namespace dang::math {

namespace detail {

/// @brief Performs a floor division on the given arguments.
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

/// @brief Uses floordiv to implement a floor modulus.
template <typename T>
static constexpr T floormod(T numerator, T denominator)
{
    return numerator - floordiv(numerator, denominator) * denominator;
}

} // namespace detail

template <typename T, std::size_t VDim>
struct Bounds;

/// @brief An iterator, allowing iteration of any-dimensional integral bounds.
/// @remark The last vector component iterates first, followed by the second to last, etc...
/// @remark This results in better caching for the common use-case of iterating an array[x][y][z]
template <typename T, std::size_t VDim>
struct BoundsIterator {
    static_assert(std::is_integral_v<T>, "BoundsIterator can only be used with integral types");

    using iterator_category = std::forward_iterator_tag;
    using value_type = Vector<T, VDim>;
    using difference_type = std::ptrdiff_t;
    using pointer = Vector<T, VDim>*;
    using reference = Vector<T, VDim>&;

    constexpr BoundsIterator() = default;

    explicit constexpr BoundsIterator(Bounds<T, VDim> bounds, Vector<T, VDim> current)
        : bounds_(bounds)
        , current_(current)
    {}

    constexpr BoundsIterator& operator++()
    {
        current_[VDim - 1]++;
        for (std::size_t d = VDim - 1; d != 0; d--) {
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

    constexpr const Vector<T, VDim>& operator*() const { return current_; }

    constexpr const Vector<T, VDim>* operator->() const { return &current_; }

private:
    Bounds<T, VDim> bounds_;
    Vector<T, VDim> current_;
};

/// @brief Generic bounds with low and high values for any dimensional vectors.
/// @remark Integral bounds are high exclusive: [low, high)
template <typename T, std::size_t VDim>
struct Bounds {
    Vector<T, VDim> low;
    Vector<T, VDim> high;

    /// @brief Initializes the bounds with zero-vectors for low and high.
    constexpr Bounds() = default;

    /// @brief Initializes high with the given value and low with zero.
    explicit constexpr Bounds(const Vector<T, VDim>& high)
        : high(high)
    {}

    /// @brief Initializes low and high with the given values.
    constexpr Bounds(const Vector<T, VDim>& low, const Vector<T, VDim>& high)
        : low(low)
        , high(high)
    {}

    /// @brief Provides simplified access for one-dimensional bounds.
    T& lowValue()
    {
        static_assert(VDim == 1);
        return low.x();
    }

    /// @brief Provides simplified access for one-dimensional bounds.
    constexpr T lowValue() const
    {
        static_assert(VDim == 1);
        return low.x();
    }

    /// @brief Provides simplified access for one-dimensional bounds.
    T& highValue()
    {
        static_assert(VDim == 1);
        return high.x();
    }

    /// @brief Provides simplified access for one-dimensional bounds.
    constexpr T highValue() const
    {
        static_assert(VDim == 1);
        return high.x();
    }

    /// @brief Returns true, when high is bigger than or equal to low.
    constexpr bool isNormalized() const { return low.allLessEqual(high); }

    /// @brief Returns bounds with any non-normalized components swapped.
    constexpr Bounds normalize() const
    {
        Bounds result;
        for (std::size_t i = 0; i < VDim; i++) {
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

    /// @brief Returns the size of the bounds, which is equal to <c>high - low</c>.
    constexpr Vector<T, VDim> size() const { return high - low; }

    /// @brief Returns the center of the bounds, rounded down for integral types.
    constexpr Vector<T, VDim> center() const
    {
        if constexpr (std::is_integral_v<T>)
            return (low + high - 1) / T(2);
        else
            return (low + high) + size() / T(2);
    }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    constexpr bool contains(const Bounds& other) const
    {
        return other.low.allGreaterEqual(low) && other.high.allLessEqual(high);
    }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    /// @remark Comparison is inclusive for low and exclusive for high.
    constexpr bool contains(const Vector<T, VDim>& vector) const
    {
        return vector.allGreaterEqual(low) && vector.allLess(high);
    }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    /// @remark Comparison is inclusive for both low and high.
    constexpr bool containsInclusive(const Vector<T, VDim>& vector) const
    {
        return vector.allGreaterEqual(low) && vector.allLessEqual(high);
    }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    /// @remark Comparison is exclusive for both low and high.
    constexpr bool containsExclusive(const Vector<T, VDim>& vector) const
    {
        return vector.allGreater(low) && vector.allLess(high);
    }

    /// @brief Clamps the given bounds, resulting in an intersection of both bounds.
    constexpr Bounds clamp(const Bounds& other) const { return {low.max(other.low), high.min(other.high)}; }

    /// @brief Clamps the given point into the calling bounds.
    /// @remark For integral types, high is clamped exclusive.
    constexpr Vector<T, VDim> clamp(const Vector<T, VDim>& vector) const
    {
        if constexpr (std::is_integral_v<T>)
            return vector.max(low).min(high - 1);
        else
            return vector.max(low).min(high);
    }

    /// @brief Returns the result of a symmetrical modulus on the given vector.
    constexpr Vector<T, VDim> mod(Vector<T, VDim> vector) const
    {
        for (std::size_t i = 0; i < VDim; i++)
            vector[i] = low[i] + detail::floormod(vector[i] - low[i], high[i] - low[i]);
        return vector;
    }

    /// @brief Returns the bounds offset by the given amount.
    constexpr Bounds offset(Vector<T, VDim> amount) const { return {low + amount, high + amount}; }

    /// @brief Returns the bounds outset by the given amount.
    constexpr Bounds outset(Vector<T, VDim> amount) const { return {low - amount, high + amount}; }

    /// @brief Returns the bounds inset by the given amount.
    constexpr Bounds inset(Vector<T, VDim> amount) const { return {low + amount, high - amount}; }

    /// @brief Returns an enum-array, mapping corners to the actual positions of the corners.
    constexpr dutils::EnumArray<Corner<VDim>, Vector<T, VDim>> corners() const
    {
        static_assert(VDim >= 1 && VDim <= 3);
        dutils::EnumArray<Corner<VDim>, Vector<T, VDim>> result;
        for (auto corner : Corner<VDim>())
            result[corner] = low + Vector<T, VDim>(CornerVector<VDim>[corner]) * (high - low);
        return result;
    }

    /// @brief Returns true, if both bounds are identical.
    friend constexpr bool operator==(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low == rhs.low && lhs.high == rhs.high;
    }

    /// @brief Returns true, if either low or high differs between the given bounds.
    friend constexpr bool operator!=(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low != rhs.low || lhs.high != rhs.high;
    }

    /// @brief Returns true, if high of the first bounds is less than low of the second bounds.
    friend constexpr bool operator<(const Bounds& lhs, const Bounds& rhs) { return lhs.high.allLess(rhs.low); }

    /// @brief Returns true, if high of the first bounds is less than or equal to low of the second bounds.
    friend constexpr bool operator<=(const Bounds& lhs, const Bounds& rhs) { return lhs.high.allLessEqual(rhs.low); }

    /// @brief Returns true, if low of the first bounds is greater than high of the second bounds.
    friend constexpr bool operator>(const Bounds& lhs, const Bounds& rhs) { return lhs.low.allGreater(rhs.high); }

    /// @brief Returns true, if low of the first bounds is greater than or equal to high of the second bounds.
    friend constexpr bool operator>=(const Bounds& lhs, const Bounds& rhs) { return lhs.low.allGreaterEqual(rhs.high); }

    /// @brief Returns a bounds-iterator, allowing for range-based iteration.
    constexpr BoundsIterator<T, VDim> begin() const { return BoundsIterator<T, VDim>(*this, low); }

    /// @brief Returns a bounds-iterator, allowing for range-based iteration.
    constexpr BoundsIterator<T, VDim> end() const { return ++BoundsIterator<T, VDim>(*this, high - 1); }
};

template <std::size_t VDim>
using bounds = Bounds<float, VDim>;

template <std::size_t VDim>
using dbounds = Bounds<double, VDim>;

template <std::size_t VDim>
using ibounds = Bounds<int, VDim>;

template <std::size_t VDim>
using ubounds = Bounds<unsigned, VDim>;

template <std::size_t VDim>
using sbounds = Bounds<std::size_t, VDim>;

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
