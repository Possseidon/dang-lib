#pragma once

#include "dang-math/consts.h"
#include "dang-math/enums.h"
#include "dang-math/global.h"
#include "dang-math/vector.h"
#include "dang-utils/enum.h"

namespace dang::math {

namespace detail {

/// @brief Performs a floor division on the given arguments.
template <std::floating_point T>
constexpr T floordiv(T numerator, T denominator)
{
    T value = numerator / denominator;
    T trunc = static_cast<T>(static_cast<long long>(value));
    T frac = value - trunc;
    return frac < 0 ? trunc - 1 : trunc;
}

template <std::integral T>
constexpr T floordiv(T numerator, T denominator)
{
    if ((numerator > 0) == (denominator > 0))
        return numerator / denominator;
    return (numerator - denominator + 1) / denominator;
}

/// @brief Uses floordiv to implement a floor modulus.
template <typename T>
static constexpr T floormod(T numerator, T denominator)
{
    return numerator - floordiv(numerator, denominator) * denominator;
}

} // namespace detail

template <typename T, std::size_t v_dim>
struct Bounds;

/// @brief An iterator, allowing iteration of any-dimensional integral bounds.
/// @remark By default the last vector component iterates first, followed by the second to last, etc...
/// @remark This results in better caching for the common use-case of iterating an array[x][y][z]
template <std::integral T, std::size_t v_dim, bool v_x_first = false>
struct BoundsIterator {
    using Type = T;
    static constexpr auto dim = v_dim;
    static constexpr auto x_first = v_x_first;

    using Bounds = dang::math::Bounds<T, dim>;

    using iterator_category = std::forward_iterator_tag;
    using value_type = Vector<T, dim>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    constexpr BoundsIterator() = default;

    explicit constexpr BoundsIterator(Bounds bounds, value_type current)
        : bounds_(bounds)
        , current_(current)
    {}

    constexpr BoundsIterator& operator++()
    {
        if constexpr (v_x_first) {
            current_[0]++;
            for (std::size_t d = 0; d != dim - 1; d++) {
                if (current_[d] < bounds_.high[d])
                    break;
                current_[d] = bounds_.low[d];
                current_[d + 1]++;
            }
        }
        else {
            current_[dim - 1]++;
            for (std::size_t d = dim - 1; d != 0; d--) {
                if (current_[d] < bounds_.high[d])
                    break;
                current_[d] = bounds_.low[d];
                current_[d - 1]++;
            }
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

    constexpr const value_type& operator*() const { return current_; }

    constexpr const value_type* operator->() const { return &current_; }

private:
    Bounds bounds_;
    value_type current_;
};

/// @brief Additional information for Bounds::facing to avoid overlaps when combining multiple bounds.
struct BoundsClipInfo {
    /// @brief Whether the x-side should be the one without any clipping.
    bool x_main = false;
    /// @brief Whether clipping should occur on both positive and negative sides.
    bool both = false;
};

/// @brief Generic bounds with low and high values for any dimensional vectors.
/// @remark Integral bounds are high exclusive: [low, high)
template <typename T, std::size_t v_dim>
struct Bounds {
    using Type = T;
    static constexpr auto dim = v_dim;

    using Point = Vector<T, dim>;
    using Size = Vector<T, dim>;
    using Offset = Vector<T, dim>;
    using Corner = dang::math::Corner<dim>;
    using Facing = dang::math::Facing<dim>;
    using ClipInfo = BoundsClipInfo;

    Point low;
    Point high;

    /// @brief Initializes the bounds with zero-vectors for low and high.
    constexpr Bounds() = default;

    /// @brief Initializes high with the given value and low with zero.
    explicit constexpr Bounds(const Point& high)
        : high(high)
    {}

    /// @brief Initializes low and high with the given values.
    constexpr Bounds(const Point& low, const Point& high)
        : low(low)
        , high(high)
    {}

    /// @brief Provides simplified access for one-dimensional bounds.
    T& lowValue() requires(dim == 1) { return low.x(); }

    /// @brief Provides simplified access for one-dimensional bounds.
    constexpr T lowValue() const requires(dim == 1) { return low.x(); }

    /// @brief Provides simplified access for one-dimensional bounds.
    T& highValue() requires(dim == 1) { return high.x(); }

    /// @brief Provides simplified access for one-dimensional bounds.
    constexpr T highValue() const requires(dim == 1) { return high.x(); }

    /// @brief Returns true, when high is bigger than or equal to low.
    constexpr bool isNormalized() const { return low.allLessEqual(high); }

    /// @brief Returns bounds with any non-normalized components swapped.
    constexpr Bounds normalize() const
    {
        Bounds result;
        for (std::size_t i = 0; i < dim; i++) {
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
    constexpr Size size() const { return high - low; }

    /// @brief Returns the center of the bounds, rounded down for integral types.
    constexpr Point center() const
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
    constexpr bool contains(const Point& point) const { return point.allGreaterEqual(low) && point.allLess(high); }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    /// @remark Comparison is inclusive for both low and high.
    constexpr bool containsInclusive(const Point& point) const
    {
        return point.allGreaterEqual(low) && point.allLessEqual(high);
    }

    /// @brief Returns true, if other is enclosed by the calling bounds.
    /// @remark Comparison is exclusive for both low and high.
    constexpr bool containsExclusive(const Point& point) const { return point.allGreater(low) && point.allLess(high); }

    /// @brief Clamps the given bounds, resulting in an intersection of both bounds.
    constexpr Bounds clamp(const Bounds& other) const { return {low.max(other.low), high.min(other.high)}; }

    /// @brief Clamps the given point into the calling bounds.
    /// @remark For integral types, high is clamped exclusive.
    constexpr Point clamp(const Point& point) const
    {
        if constexpr (std::is_integral_v<T>)
            return point.max(low).min(high - 1);
        else
            return point.max(low).min(high);
    }

    /// @brief Returns the result of a symmetrical modulus on the given vector.
    constexpr Point mod(const Point& point) const
    {
        Point result;
        for (std::size_t i = 0; i < dim; i++)
            result[i] = low[i] + detail::floormod(point[i] - low[i], high[i] - low[i]);
        return result;
    }

    /// @brief Returns the bounds offset by the given amount.
    constexpr Bounds offset(const Offset& amount) const { return {low + amount, high + amount}; }

    /// @brief Returns the bounds outset by the given amount.
    constexpr Bounds outset(const Offset& amount) const { return {low - amount, high + amount}; }

    /// @brief Returns the bounds inset by the given amount.
    constexpr Bounds inset(const Offset& amount) const { return {low + amount, high - amount}; }

    /// @brief Returns an enum-array, mapping corners to the actual positions of the corners.
    constexpr dutils::EnumArray<Corner, Point> corners() const requires(dim >= 1 && dim <= 3)
    {
        dutils::EnumArray<Corner, Point> result;
        for (auto corner : dutils::enumerate<Corner>)
            result[corner] = low + Offset(corner_vector<dim>[corner]) * (high - low);
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
    constexpr auto begin() const requires std::integral<T> { return BoundsIterator<T, dim>(*this, low); }

    /// @brief Returns a bounds-iterator, allowing for range-based iteration.
    constexpr auto end() const requires std::integral<T> { return ++BoundsIterator<T, dim>(*this, high - 1); }

    struct XFirst {
        using iterator = BoundsIterator<T, dim, true>;

        Bounds bounds;

        constexpr iterator begin() const { return iterator(bounds, bounds.low); }
        constexpr iterator end() const { return ++iterator(bounds, bounds.high - 1); }
    };

    constexpr XFirst xFirst() const { return {*this}; }

    /// @brief Returns bounds representing a single side of the square/cube/...
    /// @param facing The side of the square/cube/... starting at 0 and going -x, +x, -y, +y, ...
    /// @param clip Allows for clipping to avoid overlaps when combining multiple bounds.
    /// @param width Optional width of the bounds, defaults to 1.
    constexpr Bounds facing(std::size_t facing, std::optional<ClipInfo> clip = {}, T width = 1) const
    {
        return facingHelper(facing, width, clip, std::make_index_sequence<dim>());
    }

    /// @brief Allows using an enum value for facing.
    constexpr Bounds facing(Facing facing, std::optional<ClipInfo> clip = {}, T width = 1) const
    {
        return this->facing(static_cast<std::size_t>(facing), clip, width);
    }

    /// @brief Remaps a point from the bounds to the target bounds.
    /// @remarks For the inverse operation, simply swap the bounds.
    constexpr Point map(const Point& point, const Bounds& target) const
    {
        return target.low + (point - low) * target.size() / size();
    }

private:
    template <std::size_t... v_axes>
    constexpr Bounds facingHelper(std::size_t facing,
                                  T width,
                                  std::optional<ClipInfo> clip,
                                  std::index_sequence<v_axes...>) const
    {
        auto positive = (facing & 1) != 0;
        auto axis = facing >> 1;
        assert(axis < dim);
        return {{(v_axes == axis
                      ? (positive ? high[v_axes] - width : low[v_axes])
                      : low[v_axes] +
                            (clip && ((clip->both || !positive) && clip->x_main == (axis < v_axes)) ? width : 0))...},
                {(v_axes == axis
                      ? (positive ? high[v_axes] : low[v_axes] + width)
                      : high[v_axes] -
                            (clip && ((clip->both || positive) && clip->x_main == (axis < v_axes)) ? width : 0))...}};
    }
};

template <typename T, std::size_t v_dim>
Bounds(Vector<T, v_dim>) -> Bounds<T, v_dim>;

template <typename T, std::size_t v_dim>
Bounds(Vector<T, v_dim>, Vector<T, v_dim>) -> Bounds<T, v_dim>;

template <std::size_t v_dim>
using bounds = Bounds<float, v_dim>;

template <std::size_t v_dim>
using dbounds = Bounds<double, v_dim>;

template <std::size_t v_dim>
using ibounds = Bounds<int, v_dim>;

template <std::size_t v_dim>
using ubounds = Bounds<unsigned, v_dim>;

template <std::size_t v_dim>
using sbounds = Bounds<std::size_t, v_dim>;

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
