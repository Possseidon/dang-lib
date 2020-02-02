#pragma once

#include "utils.h"
#include "vector.h"
#include "matrix.h"

namespace dang::math
{

namespace detail
{
          
template <typename T, std::size_t Dim>
struct Line;

template <typename T, std::size_t Dim>
struct LineBase {
    Vector<T, Dim> s;
    Vector<T, Dim> d;

    inline constexpr LineBase() = default;

    inline constexpr LineBase(Vector<T, Dim> support, Vector<T, Dim> direction)
        : s(support)
        , d(direction)
    {
    }

    inline constexpr const Vector<T, Dim> head() const
    {
        return s + d;
    }

    void setHead(const Vector<T, Dim>& position)
    {
        d = position - s;
    }

    inline constexpr const Vector<T, Dim> tail() const
    {
        return s;
    }

    void setTail(const Vector<T, Dim>& position)
    {
        d += position - s;
        s = position;
    }

    inline constexpr const Vector<T, Dim> operator[](T factor) const
    {
        return s + factor * d;
    }

    inline constexpr T orthoProj(const Vector<T, Dim>& point) const
    {
        return d.dot(point - s) / d.sqrdot();
    }
                                                
    friend inline constexpr bool operator==(const Line<T, Dim>& lhs, const Line<T, Dim>& rhs) 
    {
        return lhs.s == rhs.s && lhs.d == rhs.d;
    }
    
    friend inline constexpr bool operator!=(const Line<T, Dim>& lhs, const Line<T, Dim>& rhs) 
    {
        return lhs.s != rhs.s || lhs.d != rhs.d;
    }
};

}

enum class LineSide {
    Left,
    Hit,
    Right,
    COUNT
};

template <typename T, std::size_t Dim>
struct Line : public detail::LineBase<T, Dim> {
    inline constexpr Line() : detail::LineBase<T, Dim>() {}
    inline constexpr Line(Vector<T, Dim> support, Vector<T, Dim> direction) : detail::LineBase<T, Dim>(support, direction) {}
};

template <typename T>
struct Line<T, 2> : public detail::LineBase<T, 2> {
    inline constexpr Line() : detail::LineBase<T, 2>() {}
    inline constexpr Line(Vector<T, 2> support, Vector<T, 2> direction) : detail::LineBase<T, 2>(support, direction) {}

    inline constexpr T distanceTo(const Vector<T, 2>& point) const
    {
        if (this->d == T(0))
            return this->s.distanceTo(point);
        Line<T, 2> rotated{ this->s, this->d.cross().normalized() };
        return rotated.orthoProj(point);
    }

    inline constexpr LineSide sideOf(const Vector<T, 2>& point) const
    {
        T sideFactor = this->distanceTo(point);
        if (sideFactor > 0)
            return LineSide::Left;
        else if (sideFactor != 0)
            return LineSide::Right;
        return LineSide::Hit;
    }
};

template <typename T>
struct Line<T, 3> : public detail::LineBase<T, 3> {
    inline constexpr Line() : detail::LineBase<T, 3>() {}
    inline constexpr Line(Vector<T, 3> support, Vector<T, 3> direction) : detail::LineBase<T, 3>(support, direction) {}
};

using Line1 = Line<float, 1>;
using Line2 = Line<float, 2>;
using Line3 = Line<float, 3>;

}
