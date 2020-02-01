#pragma once

#include "utils.h"
#include "vector.h"
#include "enums.h"
#include "consts.h"

#include "dang-utils/enum.h"

namespace dang::math
{

namespace detail
{

template <typename T>
inline constexpr T floordiv(T numerator, T denominator)
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

template <typename T>
static inline constexpr T floormod(T numerator, T denominator)
{
    return numerator - floordiv(numerator, denominator) * denominator;
}

}

template <typename T, std::size_t Dim>
struct Bounds;

template <typename T, std::size_t Dim>
struct BoundsIterator : public std::iterator<std::forward_iterator_tag, Vector<T, Dim>> {
    static_assert(std::is_integral_v<T>, "BoundsIterator can only be used with integral types");

    inline constexpr BoundsIterator() = default;

    inline constexpr explicit BoundsIterator(Bounds<T, Dim> bounds, Vector<T, Dim> current)
        : bounds_(bounds)
        , current_(current)
    {
    }

    inline constexpr BoundsIterator& operator++()
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

    inline constexpr bool operator==(const BoundsIterator& other) const
    {
        return current_ == other.current_;
    }

    inline constexpr bool operator!=(const BoundsIterator& other) const
    {
        return current_ != other.current_;
    }

    inline constexpr const Vector<T, Dim>& operator*() const
    {
        return current_;
    }

private:
    Bounds<T, Dim> bounds_;
    Vector<T, Dim> current_;
};

template <typename T, std::size_t Dim>
struct Bounds {
    Vector<T, Dim> low;
    Vector<T, Dim> high;

    inline constexpr Bounds() = default;

    inline constexpr explicit Bounds(const Vector<T, Dim>& high)
        : high(high)
    {
    }

    inline constexpr Bounds(const Vector<T, Dim>& low, const Vector<T, Dim>& high)
        : low(low)
        , high(high)
    {
    }

    inline constexpr bool isNormalized() const
    {
        return low <= high;
    }

    inline constexpr Bounds normalize() const
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

    inline constexpr Vector<T, Dim> size() const
    {
        return high - low;
    }

    inline constexpr Vector<T, Dim> center() const
    {
        if constexpr (std::is_integral_v<T>)
            return (low + high - 1) / T(2);
        else
            return (low + high) + size() / T(2);
    }

    inline constexpr bool contains(const Bounds& other) const
    {
        return other.low >= low && other.high <= high;
    }

    inline constexpr bool contains(const Vector<T, Dim>& vector) const
    {
        return vector >= low && vector < high;
    }

    inline constexpr Bounds clamp(const Bounds& other) const
    {
        return {
            low.max(other.low),
            high.min(other.high)
        };
    }

    inline constexpr Vector<T, Dim> clamp(const Vector<T, Dim>& vector) const
    {
        if constexpr (std::is_integral_v<T>)
            return vector.max(low).min(high - 1);
        else
            return vector.max(low).min(high);
    }

    inline constexpr Vector<T, Dim> mod(Vector<T, Dim> vector) const
    {
        for (std::size_t i = 0; i < Dim; i++)
            vector[i] = low[i] + detail::floormod(vector[i] - low[i], high[i] - low[i]);
        return vector;
    }

    inline constexpr Bounds offset(Vector<T, Dim> amount) const
    {
        return {
            low + amount,
            high + amount
        };
    }

    inline constexpr Bounds outset(Vector<T, Dim> amount) const
    {
        return {
            low - amount,
            high + amount
        };
    }

    inline constexpr Bounds inset(Vector<T, Dim> amount) const
    {
        return {
            low + amount,
            high - amount
        };
    }

    template <typename = std::enable_if_t<(Dim >= 1 && Dim <= 3)>>
    inline constexpr dutils::EnumArray<Corner<Dim>, Vector<T, Dim>> corners() const
    {
        dutils::EnumArray<Corner<Dim>, Vector<T, Dim>> result{};
        constexpr auto cornerArray = dutils::EnumValues<Corner<Dim>>;
        for (auto corner : cornerArray)
            result[corner] = low + Vector<T, Dim>(CornerVector<Dim>[corner]) * (high - low);
        return result;
    }

    friend inline constexpr bool operator==(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low == rhs.low && lhs.high == rhs.high;
    }

    friend inline constexpr bool operator!=(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low != rhs.low || lhs.high != rhs.high;
    }

    friend inline constexpr bool operator<(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.high < rhs.low;
    }

    friend inline constexpr bool operator<=(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.high <= rhs.low;
    }

    friend inline constexpr bool operator>(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low > rhs.high;
    }

    friend inline constexpr bool operator>=(const Bounds& lhs, const Bounds& rhs)
    {
        return lhs.low >= rhs.high;
    }

    inline constexpr BoundsIterator<T, Dim> begin() const
    {
        return BoundsIterator<T, Dim>(*this, low);
    }

    inline constexpr BoundsIterator<T, Dim> end() const
    {
        return ++BoundsIterator<T, Dim>(*this, high - 1);
    }
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

}
