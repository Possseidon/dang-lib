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
    Vector<T, Dim> support;
    Matrix<T, AxisCount, Dim> directions;

    inline constexpr AxisSystemBase() = default;

    inline constexpr AxisSystemBase(Vector<T, Dim> support, Matrix<T, AxisCount, Dim> directions)
        : support(support)
        , directions(directions)
    {
    }

    inline constexpr Line<T, Dim> line(std::size_t index) const
    {
        return Line<T, Dim>(support, directions[index]);
    }

    inline constexpr Plane<T, Dim> plane(std::size_t index1, std::size_t index2) const
    {
        return Plane<T, Dim>(support, { directions[index1], directions[index2] });
    }

    inline constexpr Spat<T, Dim> spat(std::size_t index1, std::size_t index2, std::size_t index3) const
    {
        return Spat<T, Dim>(support, { directions[index1], directions[index2], directions[index3] });
    }

    inline constexpr Vector<T, Dim> operator[](const Vector<T, AxisCount>& factor) const
    {
        return support + directions * factor;
    }

    inline constexpr std::optional<Vector<T, Dim>> factorAt(const Vector<T, Dim>& point) const
    {
        static_assert(Dim == AxisCount, "factorAt requires dimension and axis-count to be equal");
        if (auto inv = directions.inverse())
            return *inv * (point - support);
        return std::nullopt;
    }

    friend inline constexpr bool operator==(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support == rhs.support && lhs.directions == rhs.directions;
    }

    friend inline constexpr bool operator!=(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support != rhs.support || lhs.directions != rhs.directions;
    }
};

template <typename T, std::size_t Dim>
struct LineBase : public AxisSystemBase<T, Dim, 1> {
    inline constexpr LineBase() : AxisSystemBase<T, Dim, 1>() {}
    inline constexpr LineBase(Vector<T, Dim> support, Vector<T, Dim> directions) : AxisSystemBase<T, Dim, 1>(support, directions) {}

    inline constexpr const Vector<T, Dim>& direction() const
    {
        return this->directions[0];
    }

    inline constexpr Vector<T, Dim>& direction()
    {
        return this->directions[0];
    }

    inline constexpr Vector<T, Dim> head() const
    {
        return this->support + direction();
    }

    inline void setHead(const Vector<T, Dim>& position)
    {
        direction() = position - this->support;
    }

    inline constexpr Vector<T, Dim> tail() const
    {
        return this->support;
    }

    inline void setTail(const Vector<T, Dim>& position)
    {
        direction() += position - this->support;
        this->support = position;
    }

    inline constexpr T length() const
    {
        return direction().length();
    }

    inline constexpr T orthoProj(const Vector<T, Dim>& point) const
    {
        return direction().dot(point - this->support) / direction().sqrdot();
    }
};

template <typename T, std::size_t Dim>
struct PlaneBase : public AxisSystemBase<T, Dim, 2> {
    inline constexpr PlaneBase() : AxisSystemBase<T, Dim, 2>() {}
    inline constexpr PlaneBase(Vector<T, Dim> support, Matrix<T, 2, Dim> directions) : AxisSystemBase<T, Dim, 2>(support, directions) {}

    inline constexpr T area()
    {
        return this->directions[0].length() * this->directions[1].length();
    }

    inline constexpr std::optional<Vector<T, 2>> orthoProj(Vector<T, Dim> point) const
    {
        auto dxs = this->directions[0].sqrdot();
        auto dys = this->directions[1].sqrdot();
        auto dxy = this->directions[0].dot(this->directions[1]);

        auto div = dxs * dys - dxy * dxy;
        if (div == T())
            return std::nullopt;

        point -= this->support;
        auto dxp = this->directions[0].dot(point);
        auto dyp = this->directions[1].dot(point);
        return Vector<T, 2>(dys * dxp - dxy * dyp, dxs * dyp - dxy * dxp) / div;
    }
};

template <typename T, std::size_t Dim>
struct SpatBase : public AxisSystemBase<T, Dim, 3> {
    inline constexpr SpatBase() : AxisSystemBase<T, Dim, 3>() {}
    inline constexpr SpatBase(Vector<T, Dim> support, Matrix<T, 3, Dim> directions) : AxisSystemBase<T, Dim, 3>(support, directions) {}
};

}

template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystem : public detail::AxisSystemBase<T, Dim, AxisCount> {
    inline constexpr AxisSystem() : detail::AxisSystemBase<T, Dim, AxisCount>() {}
    inline constexpr AxisSystem(Vector<T, Dim> support, Matrix<T, AxisCount, Dim> directions) : detail::AxisSystemBase<T, Dim, AxisCount>(support, directions) {}
};

template <typename T, std::size_t Dim>
struct Line : public detail::LineBase<T, Dim> {
    inline constexpr Line() : detail::LineBase<T, Dim>() {}
    inline constexpr Line(Vector<T, Dim> support, Vector<T, Dim> directions) : detail::LineBase<T, Dim>(support, directions) {}
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
    inline constexpr Line(Vector<T, 2> support, Vector<T, 2> directions) : detail::LineBase<T, 2>(support, directions) {}

    inline constexpr T distanceTo(const Vector<T, 2>& point) const
    {
        if (this->direction() == T())
            return this->support.distanceTo(point);
        Line<T, 2> rotated{ this->support, this->direction().cross().normalized() };
        return rotated.orthoProj(point);
    }

    inline constexpr LineSide sideOf(const Vector<T, 2>& point) const
    {
        T sideFactor = distanceTo(point);
        if (sideFactor > 0)
            return LineSide::Left;
        else if (sideFactor != 0)
            return LineSide::Right;
        return LineSide::Hit;
    }

    inline constexpr Matrix<T, 3, 2> intersectionMatrix(const Line<T, 2>& other) const
    {
        return Matrix<T, 3, 2>({
            this->direction(),
            -other.direction(),
            other.support - this->support });
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
    inline constexpr Line(Vector<T, 3> support, Vector<T, 3> directions) : detail::LineBase<T, 3>(support, directions) {}
};

using Line1 = Line<float, 1>;
using Line2 = Line<float, 2>;
using Line3 = Line<float, 3>;

template <typename T, std::size_t Dim>
struct Plane : public detail::PlaneBase<T, Dim> {
    inline constexpr Plane() : detail::PlaneBase<T, Dim>() {}
    inline constexpr Plane(Vector<T, Dim> support, Matrix<T, 2, Dim> directions) : detail::PlaneBase<T, Dim>(support, directions) {}
};

template <typename T>
struct Plane<T, 2> : public detail::PlaneBase<T, 2> {
    inline constexpr Plane() : detail::PlaneBase<T, 2>() {}
    inline constexpr Plane(Vector<T, 2> support, Matrix<T, 2, 2> directions) : detail::PlaneBase<T, 2>(support, directions) {}

    inline constexpr std::optional<Vector<T, 2>> orthoProj(Vector<T, 2> point) const
    {
        const auto& dx = this->directions[0];
        const auto& dy = this->directions[1];

        auto div = dx.cross(dy);
        if (div == T())
            return std::nullopt;

        point -= this->support;

        T resultx = point.cross(dy) / div;

        const auto& x = dy.x();
        const auto& y = dy.y();
        if ((x >= 0 ? x : -x) > (y >= 0 ? y : -y))
            return Vector<T, 2>(resultx, (point.x() - resultx * dx.x()) / x);
        if (y != 0)
            return Vector<T, 2>(resultx, (point.y() - resultx * dx.y()) / y);

        return std::nullopt;
    }

    inline constexpr std::optional<Vector<T, 2>> factorAt(const Vector<T, 2>& point) const
    {
        return orthoProj(point);
    }
};

template <typename T>
struct Plane<T, 3> : public detail::PlaneBase<T, 3> {
    inline constexpr Plane() : detail::PlaneBase<T, 3>() {}
    inline constexpr Plane(Vector<T, 3> support, Matrix<T, 2, 3> directions) : detail::PlaneBase<T, 3>(support, directions) {}
    
    inline constexpr Matrix<T, 4, 3> intersectionMatrix(const Line<T, 3>& line) const
    {
        return Matrix<T, 4, 3>({
            this->directions[0],
            this->directions[1],
            -line.direction(),
            line.support - this->support });
    }
            
    inline constexpr std::optional<Vector<T, 3>> intersectionFactors(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solve();
    }

    inline constexpr std::optional<T> intersectionLineFactor(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solveCol(2);
    }

    inline constexpr std::optional<Vector<T, 3>> intersectionPoint(const Line<T, 3>& line) const
    {
        if (auto factor = intersectionLineFactor(line))
            return line[*factor];
        return std::nullopt;
    }

    inline constexpr std::optional<Vector<T, 3>> intersectionPointViaPlane(const Line<T, 3>& line) const
    {
        if (auto factors = intersectionFactors(line))
            return (*this)[factors->xy()];
        return std::nullopt;
    }
};

using Plane1 = Plane<float, 1>;
using Plane2 = Plane<float, 2>;
using Plane3 = Plane<float, 3>;

template <typename T, std::size_t Dim>
struct Spat : public detail::SpatBase<T, Dim> {
    inline constexpr Spat() : detail::SpatBase<T, Dim>() {}
    inline constexpr Spat(Vector<T, Dim> support, Matrix<T, 3, Dim> directions) : detail::SpatBase<T, Dim>(support, directions) {}
};

template <typename T>
struct Spat<T, 3> : public detail::SpatBase<T, 3> {
    inline constexpr Spat() : detail::SpatBase<T, 3>() {}
    inline constexpr Spat(Vector<T, 3> support, Matrix<T, 3, 3> directions) : detail::SpatBase<T, 3>(support, directions) {}

    inline constexpr T tripleProduct() const
    {
        return this->directions.determinant();
    }
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

}
