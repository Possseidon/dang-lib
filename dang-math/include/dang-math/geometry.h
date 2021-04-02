#pragma once

#include "dang-math/global.h"
#include "dang-math/matrix.h"
#include "dang-math/vector.h"

namespace dang::math {

/// @brief The side of a two-dimensional line.
enum class LineSide { Left, Hit, Right, COUNT };

/// @brief The side of a three-dimensional plane.
enum class PlaneSide { Top, Hit, Bottom, COUNT };

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

/// @brief Used as a base for axis-systems, consisting of one support vector and an arbitrary amount of direction
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
        : AxisSystemBase<T, v_dim, 1>(support, Matrix<T, 1, v_dim>::fromVector(directions))
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

    /// @brief Returns the factor of the point on the line, which lies closest to the given point.
    constexpr std::optional<T> closestFactorTo(const Vector<T, v_dim>& point) const
    {
        auto div = direction().sqrdot();
        if (div != T())
            return direction().dot(point - this->support) / div;
        return std::nullopt;
    }

    /// @brief Returns the point on the line, which lies closest to the given point.
    constexpr std::optional<Vector<T, v_dim>> closestPointTo(const Vector<T, v_dim>& point) const
    {
        if (auto factor = closestFactorTo(point))
            return (*this)[*factor];
        return std::nullopt;
    }

    /// @brief Returns the point mirrored on an imaginary plane, which has this line as its perpendicular.
    constexpr std::optional<Vector<T, v_dim>> mirror(const Vector<T, v_dim>& point) const
    {
        if (auto factor = closestFactorTo(point))
            return point - direction() * *factor * 2;
        return std::nullopt;
        // Version using reflect, which is roughly 2 - 3 times slower.
        // Probably, because this one uses std::sqrt, while the above does not.
        // return (point - support).reflect(direction().normalize()) + support;
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
    constexpr T area() const { return this->directions[0].length() * this->directions[1].length(); }

    /// @brief Returns the factors to the point on the plane, which lies closest to the given point.
    constexpr std::optional<Vector<T, 2>> closestFactorTo(Vector<T, v_dim> point) const
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

    /// @brief Returns the point on the plane, which lies closest to the given point.
    constexpr std::optional<Vector<T, v_dim>> closestPointTo(const Vector<T, v_dim>& point) const
    {
        if (auto factor = closestFactorTo(point))
            return (*this)[*factor];
        return std::nullopt;
    }

    /// @brief Returns one of the four quad points of the plane.
    constexpr Vector<T, v_dim> quadPoint(std::size_t index) const
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
        return T();
    }

    /// @brief Returns one of the three triangle points of the plane.
    constexpr Vector<T, v_dim> trianglePoint(std::size_t index) const
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
        return T();
    }

    /// @brief Returns one of the three inner angles in radians.
    constexpr T innerRadians(std::size_t index) const
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
        return T();
    }

    /// @brief Returns one of the three inner angles in degrees.
    constexpr T innerDegrees(std::size_t index) const { return degrees(innerRadians(index)); }
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

    /// @brief Returns the positive (left) or negative (right) distance between the (infinite) line and given point.
    constexpr T heightTo(const Vector<T, 2>& point) const
    {
        if (auto distance = Line<T, 2>(this->support, this->direction().cross().normalize()).closestFactorTo(point))
            return *distance;
        return this->support.distanceTo(point);
    }

    /// @brief Returns the distance between the (infinite) line and given point.
    constexpr T distanceTo(const Vector<T, 2> point) const
    {
        auto height = heightTo(point);
        return height >= 0 ? height : -height;
    }

    /// @brief Returns the side of the line, where the point is positioned.
    constexpr LineSide sideOf(const Vector<T, 2>& point) const
    {
        auto distance = heightTo(point);
        if (distance > 0)
            return LineSide::Left;
        else if (distance != 0)
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

    /// @brief Returns the distance between the (infinite) line and given point.
    constexpr T distanceTo(const Vector<T, 3>& point) const
    {
        if (this->direction() == Vector<T, 3>())
            return this->support.distanceTo(point);
        return this->direction().cross(point.vectorTo(this->support)).length() / this->direction().length();
    }

    using detail::LineBase<T, 3>::closestFactorTo;

    /// @brief Returns the factor to the point on this line, which lies closest to the given line.
    constexpr std::optional<T> closestFactorTo(const Line& other) const
    {
        return Plane<T, 3>(this->support,
                           Matrix<T, 2, 3>({this->direction(), this->direction().cross(other.direction())}))
            .intersectionLineFactor(other);
    }

    using detail::LineBase<T, 3>::closestPointTo;

    /// @brief Returns the point on this line, which lies closest to the given line.
    constexpr std::optional<Vector<T, 3>> closestPointTo(const Line& other) const
    {
        if (auto factor = closestFactorTo(other))
            return (*this)[*factor];
        return std::nullopt;
    }
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
    constexpr Vector<T, 2> factorAt(Vector<T, 2> point) const
    {
        const auto& dx = this->directions[0];
        const auto& dy = this->directions[1];

        auto div = dx.cross(dy);

        point -= this->support;

        T resultx = point.cross(dy) / div;

        const auto& x = dy.x();
        const auto& y = dy.y();
        if ((x >= 0 ? x : -x) > (y >= 0 ? y : -y))
            return Vector<T, 2>(resultx, (point.x() - resultx * dx.x()) / x);
        return Vector<T, 2>(resultx, (point.y() - resultx * dx.y()) / y);
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

    /// @brief Returns the perpendicular of the plane as a line with the same support vector using the cross-product.
    /// @remark The length of the result is the area of the plane.
    constexpr Line<T, 3> perpendicularLine() const { return Line<T, 3>(this->support, perpendicular()); }

    /// @brief Returns a normalized perpendicular of the plane.
    constexpr Vector<T, 3> normal() const { return perpendicular().normalize(); }

    /// @brief Returns a normalized perpendicular of the plane as a line with the same support vector.
    constexpr Line<T, 3> normalLine() const { return Line<T, 3>(this->support, normal()); }

    /// @brief Returns the positive (top) or negative (bottom) distance between the (infinite) plane and given point.
    constexpr T heightTo(const Vector<T, 3>& point) const
    {
        if (auto distance = normalLine().closestFactorTo(point))
            return *distance;
        return this->support.distanceTo(point);
    }

    /// @brief Returns the distance between the (infinite) plane and given point.
    constexpr T distanceTo(const Vector<T, 3> point) const
    {
        auto height = heightTo(point);
        return height >= 0 ? height : -height;
    }

    /// @brief Returns the side of the plane, where the point is positioned.
    constexpr PlaneSide sideOf(const Vector<T, 3>& point) const
    {
        auto distance = heightTo(point);
        if (distance > 0)
            return PlaneSide::Top;
        else if (distance != 0)
            return PlaneSide::Bottom;
        return PlaneSide::Hit;
    }

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

    /// @brief Returns the cosine of the angle between the planes perpendicular and the given direction.
    constexpr T cosAngleToPerpendicular(const Vector<T, 3>& direction) const
    {
        return perpendicular().cosAngleTo(direction);
    }

    /// @brief Returns the angle between the planes perpendicular and the given direction in radians.
    constexpr T radiansToPerpendicular(const Vector<T, 3>& direction) const
    {
        return perpendicular().radiansTo(direction);
    }

    /// @brief Returns the angle between the planes perpendicular and the given direction in degrees.
    constexpr T degreesToPerpendicular(const Vector<T, 3>& direction) const
    {
        return perpendicular().degreesTo(direction);
    }

    /// @brief Returns the angle between the plane and the given direction in radians.
    constexpr T radiansTo(const Vector<T, 3>& direction) const
    {
        return pi_v<T> / T(2) - perpendicular().radiansTo(direction);
    }

    /// @brief Returns the angle between the plane and the given direction in degrees.
    constexpr T degreesTo(const Vector<T, 3>& direction) const { return T(90) - perpendicular().degreesTo(direction); }

    /// @brief Returns the cosine of the angle to the given plane.
    constexpr T cosAngleTo(const Plane<T, 3>& other) const { return perpendicular().cosAngleTo(other.perpendicular()); }

    /// @brief Returns the angle to the given plane in radians.
    constexpr T radiansTo(const Plane<T, 3>& other) const { return perpendicular().radiansTo(other.perpendicular()); }

    /// @brief Returns the angle to the given plane in degrees.
    constexpr T degreesTo(const Plane<T, 3>& other) const { return perpendicular().degreesTo(other.perpendicular()); }

    /// @brief Returns the point mirrored on the plane.
    constexpr std::optional<Vector<T, 3>> mirror(const Vector<T, 3>& point) const
    {
        return perpendicularLine().mirror(point);
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

    /// @brief Returns the triple product (aka Spatprodukt) of the spat.
    constexpr T tripleProduct() const { return this->directions.determinant(); }
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

} // namespace dang::math
