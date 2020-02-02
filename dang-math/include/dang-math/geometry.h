#pragma once

#include "utils.h"
#include "vector.h"
#include "matrix.h"

namespace dang::math
{

template <typename T, std::size_t Dim>
struct Line;

template <typename T, std::size_t Dim>
struct Plane;

template <typename T, std::size_t Dim>
struct AxisSystem;

namespace detail
{

template <typename T, std::size_t Dim>
struct LineBase {
    Vector<T, Dim> s;
    Vector<T, Dim> d;

    inline constexpr LineBase() = default;

    inline constexpr LineBase(Vector<T, Dim> s, Vector<T, Dim> d)
        : s(s)
        , d(d)
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


template <typename T, std::size_t Dim>
struct PlaneBase {
    Vector<T, Dim> s;
    Vector<T, Dim> dx;
    Vector<T, Dim> dy;

    inline constexpr PlaneBase() = default;

    inline constexpr PlaneBase(Vector<T, Dim> s, Vector<T, Dim> dx, Vector<T, Dim> dy)
        : s(s)
        , dx(dx)
        , dy(dy)
    {
    }

    inline constexpr Vector<T, Dim> operator[](const Vector<T, 2>& factor) const
    {
        return s + factor.x() * dx + factor.y() * dy;
    }

};

template <typename T, std::size_t Dim>
struct AxisSystemBase {
    Vector<T, Dim> s;
    dutils::EnumArray<Axis<Dim>, Vector<T, Dim>> d;
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
    inline constexpr Line(Vector<T, Dim> s, Vector<T, Dim> d) : detail::LineBase<T, Dim>(s, d) {}
};

template <typename T>
struct Line<T, 2> : public detail::LineBase<T, 2> {
    inline constexpr Line() : detail::LineBase<T, 2>() {}
    inline constexpr Line(Vector<T, 2> s, Vector<T, 2> d) : detail::LineBase<T, 2>(s, d) {}

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

    inline constexpr Matrix<T, 3, 2> intersectionMatrix(const Line<T, 2>& other) const
    {
        return Matrix<T, 3, 2>({
              { this->d.x(), this->d.y() },
              { -other.d.x(), -other.d.y() },
              { other.s.x() - this->s.x(), other.s.y() - this->s.y() }
            });
    }

    inline constexpr std::optional<T> intersectionFactor(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solveCol(0);
    }

    inline constexpr std::optional<Vector<T, 2>> intersectionFactors(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solve();
    }

    inline constexpr std::optional<Vector<T, 2>> intersectionPoint(const Line<T, 2>& other) const
    {
        if (auto factor = intersectionFactor(other))
            return (*this)[*factor];
        return std::nullopt;
    }
};

template <typename T>
struct Line<T, 3> : public detail::LineBase<T, 3> {
    inline constexpr Line() : detail::LineBase<T, 3>() {}
    inline constexpr Line(Vector<T, 3> s, Vector<T, 3> d) : detail::LineBase<T, 3>(s, d) {}
};

using Line1 = Line<float, 1>;
using Line2 = Line<float, 2>;
using Line3 = Line<float, 3>;

template <typename T, std::size_t Dim>
struct Plane : public detail::PlaneBase<T, Dim> {
    inline constexpr Plane() : detail::PlaneBase<T, Dim>() {}
    inline constexpr Plane(Vector<T, 1> s, Vector<T, 1> dx, Vector<T, 1> dy) : detail::PlaneBase<T, 1>(s, dx, dy) {}
};

template <typename T>
struct Plane<T, 2> : public detail::PlaneBase<T, 2> {
    inline constexpr Plane() : detail::PlaneBase<T, 2>() {}
    inline constexpr Plane(Vector<T, 2> s, Vector<T, 2> dx, Vector<T, 2> dy) : detail::PlaneBase<T, 2>(s, dx, dy) {}
};

template <typename T>
struct Plane<T, 3> : public detail::PlaneBase<T, 3> {
    inline constexpr Plane() : detail::PlaneBase<T, 3>() {}
    inline constexpr Plane(Vector<T, 3> s, Vector<T, 3> dx, Vector<T, 3> dy) : detail::PlaneBase<T, 3>(s, dx, dy) {}
};

using Plane1 = Plane<float, 1>;
using Plane2 = Plane<float, 2>;
using Plane3 = Plane<float, 3>;

}
