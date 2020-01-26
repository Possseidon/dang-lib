#pragma once

#include "global.h"

#include "vector.h"

namespace dang::math
{

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

    inline constexpr explicit Bounds() = default;

    inline constexpr explicit Bounds(Vector<T, Dim> high)
        : high(high)
    {
    }

    inline constexpr explicit Bounds(Vector<T, Dim> low, Vector<T, Dim> high)
        : low(low)
        , high(high)
    {
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
