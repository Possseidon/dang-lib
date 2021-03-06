#pragma once

#include "dang-math/global.h"
#include "dang-math/matrix.h"
#include "dang-math/vector.h"

namespace dang::math {

/// @brief The side of a two-dimensional line.
enum class LineSide { Left, Hit, Right, COUNT };

} // namespace dang::math

namespace dang::utils {

template <>
struct enum_count<dang::math::LineSide> : default_enum_count<dang::math::LineSide> {};

} // namespace dang::utils

namespace dang::math {

template <typename T, std::size_t v_dim, std::size_t v_axis_count>
struct AxisSystem;

template <typename T, std::size_t v_dim>
struct Line;

template <typename T, std::size_t v_dim>
struct Plane;

template <typename T, std::size_t v_dim>
struct Spat;

namespace detail {

/// @brief Used as a base for axis-sytems, consisting of one support vector and an arbitrary amount of direction
/// vectors.
template <typename T, std::size_t v_dim, std::size_t v_axis_count>
struct AxisSystemBase {
    Vector<T, v_dim> support;
    Matrix<T, v_axis_count, v_dim> directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr AxisSystemBase() = default;

    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr AxisSystemBase(Vector<T, v_dim> support, Matrix<T, v_axis_count, v_dim> directions)
        : support(support)
        , directions(directions)
    {}

    /// @brief Converts a single given direction vector into a line.
    constexpr Line<T, v_dim> line(std::size_t index) const { return Line<T, v_dim>(support, directions[index]); }

    /// @brief Converts two given direction vectors into a plane.
    constexpr Plane<T, v_dim> plane(std::size_t index1, std::size_t index2) const
    {
        return Plane<T, v_dim>(support, Matrix<T, 2, v_dim>({directions[index1], directions[index2]}));
    }

    /// @brief Converts three given direction vectors into a spat.
    constexpr Spat<T, v_dim> spat(std::size_t index1, std::size_t index2, std::size_t index3) const
    {
        return Spat<T, v_dim>(support,
                              Matrix<T, 3, v_dim>({directions[index1], directions[index2], directions[index3]}));
    }

    /// @brief Returns a point in the axis-system by multiplying the factor onto the directions and adding the support
    /// vector onto it.
    constexpr Vector<T, v_dim> operator[](const Vector<T, v_axis_count>& factor) const
    {
        return support + directions * factor;
    }

    /// @brief Returns the required factor to reach the specified point.
    constexpr std::optional<Vector<T, v_dim>> factorAt(const Vector<T, v_dim>& point) const
    {
        static_assert(v_dim == v_axis_count, "factorAt requires dimension and axis-count to be equal");
        return directions.solve(point - support);
    }

    /// @brief Return true, when support and all direction vectors are equal.
    friend constexpr bool operator==(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support == rhs.support && lhs.directions == rhs.directions;
    }

    /// @brief Return true, when support or any direction vector differs.
    friend constexpr bool operator!=(const AxisSystemBase& lhs, const AxisSystemBase& rhs)
    {
        return lhs.support != rhs.support || lhs.directions != rhs.directions;
    }
};

/// @brief Used as a base for lines, consisting of one support and one direction vector.
template <typename T, std::size_t v_dim>
struct LineBase : AxisSystemBase<T, v_dim, 1> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr LineBase()
        : AxisSystemBase<T, v_dim, 1>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr LineBase(Vector<T, v_dim> support, Vector<T, v_dim> directions)
        : AxisSystemBase<T, v_dim, 1>(support, Matrix<T, 1, v_dim>(directions))
    {}

    /// @brief A simple shortcut, getting the only direction vector of the line.
    constexpr const Vector<T, v_dim>& direction() const { return this->directions[0]; }

    /// @brief A simple shortcut, getting the only direction vector of the line.
    constexpr Vector<T, v_dim>& direction() { return this->directions[0]; }

    /// @brief Returns the position of the head of the line. (support + direction)
    constexpr Vector<T, v_dim> head() const { return this->support + direction(); }

    /// @brief Changes the head of the line to the given position without modifying the tail position.
    void setHead(const Vector<T, v_dim>& position) { direction() = position - this->support; }

    /// @brief Returns the position of the tail of the line. (support)
    constexpr Vector<T, v_dim> tail() const { return this->support; }

    /// @brief Changes the tail of the line to the given position without modifying the head position.
    void setTail(const Vector<T, v_dim>& position)
    {
        direction() += position - this->support;
        this->support = position;
    }

    /// @brief Shortcut to get the length of the direction vector.
    constexpr T length() const { return direction().length(); }

    /// @brief Returns the factor of the closest point on the line for the given point.
    constexpr T orthoProj(const Vector<T, v_dim>& point) const
    {
        return direction().dot(point - this->support) / direction().sqrdot();
    }
};

/// @brief Used as a base for planes, consisting of one support and two direction vectors.
template <typename T, std::size_t v_dim>
struct PlaneBase : AxisSystemBase<T, v_dim, 2> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr PlaneBase()
        : AxisSystemBase<T, v_dim, 2>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr PlaneBase(Vector<T, v_dim> support, Matrix<T, 2, v_dim> directions)
        : AxisSystemBase<T, v_dim, 2>(support, directions)
    {}

    /// @brief Returns the area of the plane, seen as an n-dimensional parallelogram.
    constexpr T area() { return this->directions[0].length() * this->directions[1].length(); }

    /// @brief Returns the factors of the closest point on the plane for the given point.
    constexpr std::optional<Vector<T, 2>> orthoProj(Vector<T, v_dim> point) const
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

    /// @brief Returns one of the four quad points of the plane.
    constexpr Vector<T, v_dim> quadPoint(std::size_t index)
    {
        switch (index) {
        case 0:
            return this->support;
        case 1:
            return this->support + this->directions[0];
        case 2:
            return this->support + this->directions[0] + this->directions[1];
        case 3:
            return this->support + this->directions[1];
        }
        assert(false);
        return {};
    }

    /// @brief Returns one of the three triangle points of the plane.
    constexpr Vector<T, v_dim> trianglePoint(std::size_t index)
    {
        switch (index) {
        case 0:
            return this->support;
        case 1:
            return this->support + this->directions[0];
        case 2:
            return this->support + this->directions[1];
        }
        assert(false);
        return {};
    }

    /// @brief Returns one of the three inner angles in radians.
    constexpr T innerRadians(std::size_t index)
    {
        switch (index) {
        case 0:
            return this->directions[0].radiansTo(this->directions[1]);
        case 1:
            return trianglePoint(1).vectorTo(trianglePoint(2)).radiansTo(-this->directions[0]);
        case 2:
            return trianglePoint(2).vectorTo(trianglePoint(1)).radiansTo(-this->directions[1]);
        }
        assert(false);
        return {};
    }

    /// @brief Returns one of the three inner angles in degrees.
    constexpr T innerDegrees(std::size_t index) { return degrees(innerRadians(index)); }
};

/// @brief Used as a base for spats, consisting of one support and three direction vectors.
template <typename T, std::size_t v_dim>
struct SpatBase : AxisSystemBase<T, v_dim, 3> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr SpatBase()
        : AxisSystemBase<T, v_dim, 3>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr SpatBase(Vector<T, v_dim> support, Matrix<T, 3, v_dim> directions)
        : AxisSystemBase<T, v_dim, 3>(support, directions)
    {}
};

} // namespace detail

/// @brief An axis-system with one support and an arbitrary amount of direction vectors.
template <typename T, std::size_t v_dim, std::size_t v_axis_count>
struct AxisSystem : detail::AxisSystemBase<T, v_dim, v_axis_count> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr AxisSystem()
        : detail::AxisSystemBase<T, v_dim, v_axis_count>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr AxisSystem(Vector<T, v_dim> support, Matrix<T, v_axis_count, v_dim> directions)
        : detail::AxisSystemBase<T, v_dim, v_axis_count>(support, directions)
    {}
};

/// @brief A line with one support and one direction vector.
template <typename T, std::size_t v_dim>
struct Line : detail::LineBase<T, v_dim> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : detail::LineBase<T, v_dim>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(Vector<T, v_dim> support, Vector<T, v_dim> directions)
        : detail::LineBase<T, v_dim>(support, directions)
    {}
};

/// @brief A two-dimensional line with one support and one direction vector.
template <typename T>
struct Line<T, 2> : detail::LineBase<T, 2> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : detail::LineBase<T, 2>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(Vector<T, 2> support, Vector<T, 2> directions)
        : detail::LineBase<T, 2>(support, directions)
    {}

    /// @brief Returns the distance between the (infinite) line and given point.
    constexpr T distanceTo(const Vector<T, 2>& point) const
    {
        if (this->direction() == T())
            return this->support.distanceTo(point);
        Line<T, 2> rotated{this->support, this->direction().cross().normalize()};
        return rotated.orthoProj(point);
    }

    /// @brief Returns the side of the line, where the point is positioned.
    constexpr LineSide sideOf(const Vector<T, 2>& point) const
    {
        T sideFactor = distanceTo(point);
        if (sideFactor > 0)
            return LineSide::Left;
        else if (sideFactor != 0)
            return LineSide::Right;
        return LineSide::Hit;
    }

    /// @brief Builds a matrix, which can be used to calculate the intersection with another line.
    constexpr Matrix<T, 3, 2> intersectionMatrix(const Line<T, 2>& other) const
    {
        return Matrix<T, 3, 2>({this->direction(), -other.direction(), other.support - this->support});
    }

    /// @brief Returns the factor to reach the intersection point with the given line.
    constexpr std::optional<T> intersectionFactor(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solveCol(0);
    }

    /// @brief Returns both factors to reach the intersection point with the given line.
    constexpr std::optional<Vector<T, 2>> intersectionFactors(const Line<T, 2>& other) const
    {
        return intersectionMatrix(other).solve();
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point.
    constexpr std::optional<Vector<T, 2>> intersectionPoint(const Line<T, 2>& other) const
    {
        if (auto factor = intersectionFactor(other))
            return (*this)[*factor];
        return std::nullopt;
    }
};

/// @brief A three-dimensional line with one support and one direction vector.
template <typename T>
struct Line<T, 3> : detail::LineBase<T, 3> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : detail::LineBase<T, 3>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(Vector<T, 3> support, Vector<T, 3> directions)
        : detail::LineBase<T, 3>(support, directions)
    {}
};

using Line1 = Line<float, 1>;
using Line2 = Line<float, 2>;
using Line3 = Line<float, 3>;

/// @brief A plane with one support and two direction vectors.
template <typename T, std::size_t v_dim>
struct Plane : detail::PlaneBase<T, v_dim> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : detail::PlaneBase<T, v_dim>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(Vector<T, v_dim> support, Matrix<T, 2, v_dim> directions)
        : detail::PlaneBase<T, v_dim>(support, directions)
    {}
};

/// @brief A two-dimensional plane with one support and two direction vectors.
template <typename T>
struct Plane<T, 2> : detail::PlaneBase<T, 2> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : detail::PlaneBase<T, 2>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(Vector<T, 2> support, Matrix<T, 2, 2> directions)
        : detail::PlaneBase<T, 2>(support, directions)
    {}

    /// @brief Returns the required factor to reach the specified point.
    constexpr std::optional<Vector<T, 2>> factorAt(Vector<T, 2> point) const
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

/// @brief A three-dimensional plane with one support and two direction vectors.
template <typename T>
struct Plane<T, 3> : detail::PlaneBase<T, 3> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : detail::PlaneBase<T, 3>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(Vector<T, 3> support, Matrix<T, 2, 3> directions)
        : detail::PlaneBase<T, 3>(support, directions)
    {}

    /// @brief Returns the perpendicular of the plane using the cross-product.
    /// @remark The length of the result is the area of the plane.
    constexpr Vector<T, 3> perpendicular() const { return this->directions[0].cross(this->directions[1]); }

    /// @brief Returns a normalized perpendicular of the plane.
    constexpr Vector<T, 3> normal() const { return perpendicular().normalize(); }

    /// @brief Returns the height from the plane to the given point.
    constexpr T height(const Vector<T, 3>& point) const { return Line<T, 3>(this->support, normal()).orthoProj(point); }

    /// @brief Builds a matrix, which can be used to calculate the intersection with a line.
    constexpr Matrix<T, 4, 3> intersectionMatrix(const Line<T, 3>& line) const
    {
        return Matrix<T, 4, 3>(
            {this->directions[0], this->directions[1], -line.direction(), line.support - this->support});
    }

    /// @brief Returns the factors to reach the intersection point with the given line for the plane (xy) and line (z).
    constexpr std::optional<Vector<T, 3>> intersectionFactors(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solve();
    }

    /// @brief Returns the factor to reach the intersection point with the given line for the line itself.
    constexpr std::optional<T> intersectionLineFactor(const Line<T, 3>& line) const
    {
        return intersectionMatrix(line).solveCol(2);
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point.
    constexpr std::optional<Vector<T, 3>> intersectionPoint(const Line<T, 3>& line) const
    {
        if (auto factor = intersectionLineFactor(line))
            return line[*factor];
        return std::nullopt;
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point, using the plane.
    constexpr std::optional<Vector<T, 3>> intersectionPointViaPlane(const Line<T, 3>& line) const
    {
        if (auto factors = intersectionFactors(line))
            return (*this)[factors->xy()];
        return std::nullopt;
    }

    /// @brief Returns the intersection with another plane in the form of a line of arbitrary position and length.
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

/// @brief A spat with one support and three direction vectors.
template <typename T, std::size_t v_dim>
struct Spat : detail::SpatBase<T, v_dim> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Spat()
        : detail::SpatBase<T, v_dim>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Spat(Vector<T, v_dim> support, Matrix<T, 3, v_dim> directions)
        : detail::SpatBase<T, v_dim>(support, directions)
    {}
};

/// @brief A three-dimensional spat with one support and three direction vectors.
template <typename T>
struct Spat<T, 3> : detail::SpatBase<T, 3> {
    /// @brief Initializes support and direction vectors with zero.
    constexpr Spat()
        : detail::SpatBase<T, 3>()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Spat(Vector<T, 3> support, Matrix<T, 3, 3> directions)
        : detail::SpatBase<T, 3>(support, directions)
    {}

    /// @brief Returns the triple product (aka Spat-Produkt) of the spat.
    constexpr T tripleProduct() const { return this->directions.determinant(); }
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

} // namespace dang::math
