#pragma once

#include "utils.h"
#include "vector.h"
#include "matrix.h"

namespace dang::math
{

template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystem;

template <typename T, std::size_t Dim>
struct Line;

template <typename T, std::size_t Dim>
struct Plane;

template <typename T, std::size_t Dim>
struct Spat;

namespace detail
{

template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystemBase {
    Vector<T, Dim> s;
    Matrix<T, AxisCount, Dim> d;

    inline constexpr AxisSystemBase() = default;

    inline constexpr AxisSystemBase(Vector<T, Dim> s, Matrix<T, AxisCount, Dim> d)
        : s(s)
        , d(d)
    {
    }

    inline constexpr Line<T, Dim> line(std::size_t index) const {
        return Line<T, Dim>(s, d[index]);
    }

    inline constexpr Plane<T, Dim> plane(std::size_t index1, std::size_t index2) const {
        return Plane<T, Dim>(s, { d[index1], d[index2] });
    }

    inline constexpr Spat<T, Dim> spat(std::size_t index1, std::size_t index2, std::size_t index3) const {
        return Spat<T, Dim>(s, { d[index1], d[index2], d[index3] });
    }

    inline constexpr Vector<T, Dim> operator[](const Vector<T, Dim>& factor) const
    {
        return s + d * factor;
    }

    inline constexpr std::optional<Vector<T, Dim>> factorAt(const Vector<T, Dim>& point) const
    {
        static_assert(Dim == AxisCount, "factorAt requires dimension and axis-count to be equal");
        if (auto inv = d.inverse())
            return *inv * (point - s);
        return std::nullopt;
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
struct LineBase : public AxisSystemBase<T, Dim, 1> {
    inline constexpr LineBase() : AxisSystemBase<T, Dim, 1>() {}
    inline constexpr LineBase(Vector<T, Dim> s, Vector<T, Dim> d) : AxisSystemBase<T, Dim, 1>(s, d) {}

    inline constexpr const Vector<T, Dim>& direction() const
    {
        return this->d[0];
    }

    inline constexpr Vector<T, Dim>& direction()
    {
        return this->d[0];
    }

    inline constexpr Vector<T, Dim> head() const
    {
        return this->s + direction();
    }

    inline void setHead(const Vector<T, Dim>& position)
    {
        direction() = position - this->s;
    }

    inline constexpr Vector<T, Dim> tail() const
    {
        return this->s;
    }

    inline void setTail(const Vector<T, Dim>& position)
    {
        direction() += position - this->s;
        this->s = position;
    }

    inline constexpr T length() const
    {
        return direction().length();
    }

    inline constexpr T orthoProj(const Vector<T, Dim>& point) const
    {
        return direction().dot(point - this->s) / direction().sqrdot();
    }
};

template <typename T, std::size_t Dim>
struct PlaneBase : public AxisSystemBase<T, Dim, 2> {
    inline constexpr PlaneBase() : AxisSystemBase<T, Dim, 2>() {}
    inline constexpr PlaneBase(Vector<T, Dim> s, Matrix<T, 2, Dim> d) : AxisSystemBase<T, Dim, 2>(s, d) {}

    inline constexpr T area()
    {
        return this->d[0].length() * this->d[1].length();
    }
};

template <typename T, std::size_t Dim>
struct SpatBase : public AxisSystemBase<T, Dim, 3> {
    inline constexpr SpatBase() : AxisSystemBase<T, Dim, 3>() {}
    inline constexpr SpatBase(Vector<T, Dim> s, Matrix<T, 3, Dim> d) : AxisSystemBase<T, Dim, 3>(s, d) {}
};

}

template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystem : public detail::AxisSystemBase<T, Dim, AxisCount> {
    inline constexpr AxisSystem() : detail::AxisSystemBase<T, Dim, AxisCount>() {}
    inline constexpr AxisSystem(Vector<T, Dim> s, Matrix<T, AxisCount, Dim> d) : detail::AxisSystemBase<T, Dim, AxisCount>(s, d) {}
};

template <typename T, std::size_t Dim>
struct Line : public detail::LineBase<T, Dim> {
    inline constexpr Line() : detail::LineBase<T, Dim>() {}
    inline constexpr Line(Vector<T, Dim> s, Vector<T, Dim> d) : detail::LineBase<T, Dim>(s, d) {}
};

enum class LineSide {
    Left,
    Hit,
    Right,
    COUNT
};

template <typename T>
struct Line<T, 2> : public detail::LineBase<T, 2> {
    inline constexpr Line() : detail::LineBase<T, 2>() {}
    inline constexpr Line(Vector<T, 2> s, Vector<T, 2> d) : detail::LineBase<T, 2>(s, d) {}

    inline constexpr T distanceTo(const Vector<T, 2>& point) const
    {
        if (this->direction() == T(0))
            return this->s.distanceTo(point);
        Line<T, 2> rotated{ this->s, this->direction().cross().normalized() };
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
              { this->direction().x(), this->direction().y() },
              { -other.direction().x(), -other.direction().y() },
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
    inline constexpr Plane(Vector<T, Dim> s, Matrix<T, 2, Dim> d) : detail::PlaneBase<T, Dim>(s, d) {}
};

template <typename T>
struct Plane<T, 2> : public detail::PlaneBase<T, 2> {
    inline constexpr Plane() : detail::PlaneBase<T, 2>() {}
    inline constexpr Plane(Vector<T, 2> s, Matrix<T, 2, 2> d) : detail::PlaneBase<T, 2>(s, d) {}
};

template <typename T>
struct Plane<T, 3> : public detail::PlaneBase<T, 3>  {
    inline constexpr Plane() : detail::PlaneBase<T, 3>() {}
    inline constexpr Plane(Vector<T, 3> s, Matrix<T, 2, 3> d) : detail::PlaneBase<T, 3>(s, d) {}
};

using Plane1 = Plane<float, 1>;
using Plane2 = Plane<float, 2>;
using Plane3 = Plane<float, 3>;

template <typename T, std::size_t Dim>
struct Spat : public detail::SpatBase<T, Dim> {
    inline constexpr Spat() : detail::SpatBase<T, Dim>() {}
    inline constexpr Spat(Vector<T, Dim> s, Matrix<T, 3, Dim> d) : detail::SpatBase<T, Dim>(s, d) {}
};

template <typename T>
struct Spat<T, 2> : public detail::SpatBase<T, 2> {
    inline constexpr Spat() : detail::SpatBase<T, 2>() {}
    inline constexpr Spat(Vector<T, 2> s, Matrix<T, 3, 2> d) : detail::SpatBase<T, 2>(s, d) {}
};

template <typename T>
struct Spat<T, 3> : public detail::SpatBase<T, 3>  {
    inline constexpr Spat() : detail::SpatBase<T, 3>() {}
    inline constexpr Spat(Vector<T, 3> s, Matrix<T, 3, 3> d) : detail::SpatBase<T, 3>(s, d) {}
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

}
