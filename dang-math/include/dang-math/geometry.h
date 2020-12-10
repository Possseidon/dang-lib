#pragma once

#include "dang-math/matrix.h"
#include "dang-math/utils.h"
#include "dang-math/vector.h"

namespace dang::math {

/// <summary>The side of a two-dimensional line.</summary>
enum class LineSide { Left, Hit, Right, COUNT };

} // namespace dang::math

namespace dang::utils {

template <>
struct EnumCount<dang::math::LineSide> : DefaultEnumCount<dang::math::LineSide> {};

} // namespace dang::utils

namespace dang::math {

template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystem;

template <typename T, std::size_t Dim>
struct Line;

template <typename T, std::size_t Dim>
struct Plane;

template <typename T, std::size_t Dim>
struct Spat;

namespace detail {

/// <summary>Used as a base for axis-sytems, consisting of one support vector and an arbitrary amount of direction vectors.</summary>
template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystemBase {
    Vector<T, Dim> support;
    Matrix<T, AxisCount, Dim> directions;

    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr AxisSystemBase() = default;

    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr AxisSystemBase(Vector<T, Dim> support, Matrix<T, AxisCount, Dim> directions)
        : support(support)
        , directions(directions)
    {}

    /// <summary>Converts a single given direction vector into a line.</summary>
    constexpr Line<T, Dim> line(std::size_t index) const { return Line<T, Dim>(support, directions[index]); }

    /// <summary>Converts two given direction vectors into a plane.</summary>
    constexpr Plane<T, Dim> plane(std::size_t index1, std::size_t index2) const
    {
        return Plane<T, Dim>(support, {directions[index1], directions[index2]});
    }

    /// <summary>Converts three given direction vectors into a spat.</summary>
    constexpr Spat<T, Dim> spat(std::size_t index1, std::size_t index2, std::size_t index3) const
    {
        return Spat<T, Dim>(support, {directions[index1], directions[index2], directions[index3]});
    }

    /// <summary>Returns a point in the axis-system by multiplying the factor onto the directions and adding the support vector onto it.</summary>
    constexpr Vector<T, Dim> operator[](const Vector<T, AxisCount>& factor) const
    {
        return support + directions * factor;
    }

    /// <summary>Returns the required factor to reach the specified point.</summary>
    constexpr std::optional<Vector<T, Dim>> factorAt(const Vector<T, Dim>& point) const
    {
        static_assert(Dim == AxisCount, "factorAt requires dimension and axis-count to be equal");
        return directions.solve(point - support);
    }

    /// <summary>Return true, when support and all direction vectors are equal.</summary>
    friend constexpr bool operator==(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support == rhs.support && lhs.directions == rhs.directions;
    }

    /// <summary>Return true, when support or any direction vector differs.</summary>
    friend constexpr bool operator!=(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support != rhs.support || lhs.directions != rhs.directions;
    }
};

/// <summary>Used as a base for lines, consisting of one support and one direction vector.</summary>
template <typename T, std::size_t Dim>
struct LineBase : AxisSystemBase<T, Dim, 1> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr LineBase()
        : AxisSystemBase<T, Dim, 1>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr LineBase(Vector<T, Dim> support, Vector<T, Dim> directions)
        : AxisSystemBase<T, Dim, 1>(support, directions)
    {}

    /// <summary>A simple shortcut, getting the only direction vector of the line.</summary>
    constexpr const Vector<T, Dim>& direction() const { return this->directions[0]; }

    /// <summary>A simple shortcut, getting the only direction vector of the line.</summary>
    constexpr Vector<T, Dim>& direction() { return this->directions[0]; }

    /// <summary>Returns the position of the head of the line. (support + direction)</summary>
    constexpr Vector<T, Dim> head() const { return this->support + direction(); }

    /// <summary>Changes the head of the line to the given position without modifying the tail position.</summary>
    void setHead(const Vector<T, Dim>& position) { direction() = position - this->support; }

    /// <summary>Returns the position of the tail of the line. (support)</summary>
    constexpr Vector<T, Dim> tail() const { return this->support; }

    /// <summary>Changes the tail of the line to the given position without modifying the head position.</summary>
    void setTail(const Vector<T, Dim>& position)
    {
        direction() += position - this->support;
        this->support = position;
    }

    /// <summary>Shortcut to get the length of the direction vector.</summary>
    constexpr T length() const { return direction().length(); }

    /// <summary>Returns the factor of the closest point on the line for the given point.</summary>
    constexpr T orthoProj(const Vector<T, Dim>& point) const
    {
        return direction().dot(point - this->support) / direction().sqrdot();
    }
};

/// <summary>Used as a base for planes, consisting of one support and two direction vectors.</summary>
template <typename T, std::size_t Dim>
struct PlaneBase : AxisSystemBase<T, Dim, 2> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr PlaneBase()
        : AxisSystemBase<T, Dim, 2>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr PlaneBase(Vector<T, Dim> support, Matrix<T, 2, Dim> directions)
        : AxisSystemBase<T, Dim, 2>(support, directions)
    {}

    /// <summary>Returns the area of the plane, seen as an n-dimensional parallelogram.</summary>
    constexpr T area() { return this->directions[0].length() * this->directions[1].length(); }

    /// <summary>Returns the factors of the closest point on the plane for the given point.</summary>
    constexpr std::optional<Vector<T, 2>> orthoProj(Vector<T, Dim> point) const
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

/// <summary>Used as a base for spats, consisting of one support and three direction vectors.</summary>
template <typename T, std::size_t Dim>
struct SpatBase : AxisSystemBase<T, Dim, 3> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr SpatBase()
        : AxisSystemBase<T, Dim, 3>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr SpatBase(Vector<T, Dim> support, Matrix<T, 3, Dim> directions)
        : AxisSystemBase<T, Dim, 3>(support, directions)
    {}
};

} // namespace detail

/// <summary>An axis-system with one support and an arbitrary amount of direction vectors.</summary>
template <typename T, std::size_t Dim, std::size_t AxisCount>
struct AxisSystem : detail::AxisSystemBase<T, Dim, AxisCount> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr AxisSystem()
        : detail::AxisSystemBase<T, Dim, AxisCount>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr AxisSystem(Vector<T, Dim> support, Matrix<T, AxisCount, Dim> directions)
        : detail::AxisSystemBase<T, Dim, AxisCount>(support, directions)
    {}
};

/// <summary>A line with one support and one direction vector.</summary>
template <typename T, std::size_t Dim>
struct Line : detail::LineBase<T, Dim> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Line()
        : detail::LineBase<T, Dim>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Line(Vector<T, Dim> support, Vector<T, Dim> directions)
        : detail::LineBase<T, Dim>(support, directions)
    {}
};

/// <summary>A two-dimensional line with one support and one direction vector.</summary>
template <typename T>
struct Line<T, 2> : detail::LineBase<T, 2> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Line()
        : detail::LineBase<T, 2>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Line(Vector<T, 2> support, Vector<T, 2> directions)
        : detail::LineBase<T, 2>(support, directions)
    {}

    /// <summary>Returns the distance between the (infinite) line and given point.</summary>
    constexpr T distanceTo(const Vector<T, 2>& point) const
    {
        if (this->direction() == T())
            return this->support.distanceTo(point);
        Line<T, 2> rotated{this->support, this->direction().cross().normalize()};
        return rotated.orthoProj(point);
    }

    /// <summary>Returns the side of the line, where the point is positioned.</summary>
    constexpr LineSide sideOf(const Vector<T, 2>& point) const
    {
        T sideFactor = distanceTo(point);
        if (sideFactor > 0)
            return LineSide::Left;
        else if (sideFactor != 0)
            return LineSide::Right;
        return LineSide::Hit;
    }

    /// <summary>Builds a matrix, which can be used to calculate the intersection with another line.</summary>
    constexpr Matrix<T, 3, 2> intersectionMatrix(const Line<T, 2>& other) const
    {
        return Matrix<T, 3, 2>({this->direction(), -other.direction(), other.support - this->support});
    }

    /// <summary>Returns the factor to reach the intersection point with the given line.</summary>
    constexpr std::optional<T> intersectionFactor(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solveCol(0);
    }

    /// <summary>Returns both factors to reach the intersection point with the given line.</summary>
    constexpr std::optional<Vector<T, 2>> intersectionFactors(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solve();
    }

    /// <summary>Calculates the intersection with the given line and returns the intersection point.</summary>
    constexpr std::optional<Vector<T, 2>> intersectionPoint(const Line<T, 2>& other) const
    {
        if (auto factor = intersectionFactor(other))
            return (*this)[*factor];
        return std::nullopt;
    }
};

/// <summary>A three-dimensional line with one support and one direction vector.</summary>
template <typename T>
struct Line<T, 3> : detail::LineBase<T, 3> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Line()
        : detail::LineBase<T, 3>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Line(Vector<T, 3> support, Vector<T, 3> directions)
        : detail::LineBase<T, 3>(support, directions)
    {}
};

using Line1 = Line<float, 1>;
using Line2 = Line<float, 2>;
using Line3 = Line<float, 3>;

/// <summary>A plane with one support and two direction vectors.</summary>
template <typename T, std::size_t Dim>
struct Plane : detail::PlaneBase<T, Dim> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Plane()
        : detail::PlaneBase<T, Dim>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Plane(Vector<T, Dim> support, Matrix<T, 2, Dim> directions)
        : detail::PlaneBase<T, Dim>(support, directions)
    {}
};

/// <summary>A two-dimensional plane with one support and two direction vectors.</summary>
template <typename T>
struct Plane<T, 2> : detail::PlaneBase<T, 2> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Plane()
        : detail::PlaneBase<T, 2>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Plane(Vector<T, 2> support, Matrix<T, 2, 2> directions)
        : detail::PlaneBase<T, 2>(support, directions)
    {}

    /// <summary>Returns the required factor to reach the specified point.</summary>
    constexpr std::optional<Vector<T, 2>> factorAt(const Vector<T, 2>& point) const
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
};

/// <summary>A three-dimensional plane with one support and two direction vectors.</summary>
template <typename T>
struct Plane<T, 3> : detail::PlaneBase<T, 3> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Plane()
        : detail::PlaneBase<T, 3>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Plane(Vector<T, 3> support, Matrix<T, 2, 3> directions)
        : detail::PlaneBase<T, 3>(support, directions)
    {}

    /// <summary>Returns the perpendicular of the plane using the cross-product.</summary>
    /// <remarks>The length of the result is the area of the plane.</remarks>
    constexpr Vector<T, 3> perpendicular() const { return this->directions[0].cross(this->directions[1]); }

    /// <summary>Returns a normalized perpendicular of the plane.</summary>
    constexpr Vector<T, 3> normal() const { return perpendicular().normalize(); }

    /// <summary>Returns the height from the plane to the given point.</summary>
    constexpr T height(const Vector<T, 3>& point) const { return Line<T, 3>(this->support, normal()).orthoProj(point); }

    /// <summary>Builds a matrix, which can be used to calculate the intersection with a line.</summary>
    constexpr Matrix<T, 4, 3> intersectionMatrix(const Line<T, 3>& line) const
    {
        return Matrix<T, 4, 3>(
            {this->directions[0], this->directions[1], -line.direction(), line.support - this->support});
    }

    /// <summary>Returns the factors to reach the intersection point with the given line for the plane (xy) and line (z).</summary>
    constexpr std::optional<Vector<T, 3>> intersectionFactors(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solve();
    }

    /// <summary>Returns the factor to reach the intersection point with the given line for the line itself.</summary>
    constexpr std::optional<T> intersectionLineFactor(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solveCol(2);
    }

    /// <summary>Calculates the intersection with the given line and returns the intersection point.</summary>
    constexpr std::optional<Vector<T, 3>> intersectionPoint(const Line<T, 3>& line) const
    {
        if (auto factor = intersectionLineFactor(line))
            return line[*factor];
        return std::nullopt;
    }

    /// <summary>Calculates the intersection with the given line and returns the intersection point, using the plane.</summary>
    constexpr std::optional<Vector<T, 3>> intersectionPointViaPlane(const Line<T, 3>& line) const
    {
        if (auto factors = intersectionFactors(line))
            return (*this)[factors->xy()];
        return std::nullopt;
    }

    /// <summary>Returns the intersection with another plane in the form of a line of arbitrary position and length.</summary>
    constexpr std::optional<Line<T, 3>> intersectionLine(const Plane<T, 3>& plane) const
    {
        Vector<T, 3> perp = perpendicular();
        Vector<T, 3> dir = perp.cross(plane.perpendicular());
        Line<T, 3> line(this->support, dir.cross(perp));
        if (auto pos = plane.intersectionPoint(line))
            return Line<T, 3>(*pos, dir);
        return std::nullopt;
    }
};

using Plane1 = Plane<float, 1>;
using Plane2 = Plane<float, 2>;
using Plane3 = Plane<float, 3>;

/// <summary>A spat with one support and three direction vectors.</summary>
template <typename T, std::size_t Dim>
struct Spat : detail::SpatBase<T, Dim> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Spat()
        : detail::SpatBase<T, Dim>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Spat(Vector<T, Dim> support, Matrix<T, 3, Dim> directions)
        : detail::SpatBase<T, Dim>(support, directions)
    {}
};

/// <summary>A three-dimensional spat with one support and three direction vectors.</summary>
template <typename T>
struct Spat<T, 3> : detail::SpatBase<T, 3> {
    /// <summary>Initializes support and direction vectors with zero.</summary>
    constexpr Spat()
        : detail::SpatBase<T, 3>()
    {}
    /// <summary>Initializes support and direction vectors with the given vectors.</summary>
    constexpr Spat(Vector<T, 3> support, Matrix<T, 3, 3> directions)
        : detail::SpatBase<T, 3>(support, directions)
    {}

    /// <summary>Returns the triple product (aka Spat-Produkt) of the spat.</summary>
    constexpr T tripleProduct() const { return this->directions.determinant(); }
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

} // namespace dang::math
