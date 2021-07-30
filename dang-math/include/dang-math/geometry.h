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
    using Type = T;
    static constexpr auto dim = v_dim;
    static constexpr auto axis_count = v_axis_count;

    using Point = Vector<T, v_dim>;
    using Direction = Vector<T, v_dim>;
    using Directions = Matrix<T, axis_count, dim>;
    using Factor = T;
    using Factors = Vector<T, axis_count>;

    using Line = dang::math::Line<T, dim>;
    using Plane = dang::math::Plane<T, dim>;
    using Spat = dang::math::Spat<T, dim>;

    Point support;
    Directions directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr AxisSystemBase() = default;

    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr AxisSystemBase(const Point& support, const Directions& directions)
        : support(support)
        , directions(directions)
    {}

    /// @brief Converts a single given direction vector into a line.
    constexpr Line line(std::size_t index) const { return Line(support, directions[index]); }

    /// @brief Converts two given direction vectors into a plane.
    constexpr Plane plane(std::size_t index1, std::size_t index2) const
    {
        return Plane(support, typename Plane::Directions({directions[index1], directions[index2]}));
    }

    /// @brief Converts three given direction vectors into a spat.
    constexpr Spat spat(std::size_t index1, std::size_t index2, std::size_t index3) const
    {
        return Spat(support, typename Spat::Directions({directions[index1], directions[index2], directions[index3]}));
    }

    /// @brief Returns a point in the axis-system by multiplying the factor onto the directions and adding the support
    /// vector onto it.
    constexpr Point operator[](const Factors& factors) const { return support + directions * factors; }

    /// @brief Returns the required factor to reach the specified point.
    constexpr std::optional<Factors> factorAt(const Point& point) const
    {
        static_assert(dim == axis_count, "factorAt requires dimension and axis-count to be equal");
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
    using Base = AxisSystemBase<T, v_dim, 1>;

    using Point = typename Base::Point;
    using Direction = typename Base::Direction;
    using Directions = typename Base::Directions;
    using Factor = typename Base::Factor;

    using Length = T;

    /// @brief Initializes support and direction vectors with zero.
    constexpr LineBase()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr LineBase(const Point& support, const Direction& directions)
        : Base(support, Directions::fromVector(directions))
    {}

    /// @brief A simple shortcut, getting the only direction vector of the line.
    constexpr const Direction& direction() const { return this->directions[0]; }

    /// @brief A simple shortcut, getting the only direction vector of the line.
    constexpr Direction& direction() { return this->directions[0]; }

    /// @brief Returns the position of the head of the line. (support + direction)
    constexpr Point head() const { return this->support + direction(); }

    /// @brief Changes the head of the line to the given position without modifying the tail position.
    void setHead(const Point& point) { direction() = point - this->support; }

    /// @brief Returns the position of the tail of the line. (support)
    constexpr Point tail() const { return this->support; }

    /// @brief Changes the tail of the line to the given position without modifying the head position.
    void setTail(const Point& point)
    {
        direction() += point.vectorTo(this->support);
        this->support = point;
    }

    /// @brief Shortcut to get the length of the direction vector.
    constexpr Length length() const { return direction().length(); }

    /// @brief Returns the factor of the point on the line, which lies closest to the given point.
    constexpr std::optional<Factor> closestFactorTo(const Point& point) const
    {
        auto div = direction().sqrdot();
        if (div != decltype(div)())
            return direction().dot(point - this->support) / div;
        return std::nullopt;
    }

    /// @brief Returns the point on the line, which lies closest to the given point.
    constexpr std::optional<Point> closestPointTo(const Point& point) const
    {
        if (auto factor = closestFactorTo(point))
            return (*this)[*factor];
        return std::nullopt;
    }

    /// @brief Returns the point mirrored on an imaginary plane, which has this line as its perpendicular.
    constexpr std::optional<Point> mirror(const Point& point) const
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
    using Base = AxisSystemBase<T, v_dim, 2>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;
    using Factors = typename Base::Factors;

    using Area = T;
    using Radians = T;
    using Degrees = T;

    /// @brief Initializes support and direction vectors with zero.
    constexpr PlaneBase()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr PlaneBase(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}

    /// @brief Returns the area of the plane, seen as an n-dimensional parallelogram.
    constexpr Area area() const { return this->directions[0].length() * this->directions[1].length(); }

    /// @brief Returns the factors to the point on the plane, which lies closest to the given point.
    constexpr std::optional<Factors> closestFactorTo(const Point& point) const
    {
        auto dxs = this->directions[0].sqrdot();
        auto dys = this->directions[1].sqrdot();
        auto dxy = this->directions[0].dot(this->directions[1]);

        auto div = dxs * dys - dxy * dxy;
        if (div == decltype(div)())
            return std::nullopt;

        auto point_relative = point - this->support;
        auto dxp = this->directions[0].dot(point_relative);
        auto dyp = this->directions[1].dot(point_relative);
        return Factors(dys * dxp - dxy * dyp, dxs * dyp - dxy * dxp) / div;
    }

    /// @brief Returns the point on the plane, which lies closest to the given point.
    constexpr std::optional<Point> closestPointTo(const Point& point) const
    {
        if (auto factor = closestFactorTo(point))
            return (*this)[*factor];
        return std::nullopt;
    }

    /// @brief Returns one of the four quad points of the plane.
    constexpr Point quadPoint(std::size_t index) const
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
    constexpr Point trianglePoint(std::size_t index) const
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
    constexpr Radians innerRadians(std::size_t index) const
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
    constexpr Degrees innerDegrees(std::size_t index) const { return degrees(innerRadians(index)); }
};

/// @brief Used as a base for spats, consisting of one support and three direction vectors.
template <typename T, std::size_t v_dim>
struct SpatBase : AxisSystemBase<T, v_dim, 3> {
    using Base = AxisSystemBase<T, v_dim, 3>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr SpatBase()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr SpatBase(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}
};

} // namespace detail

/// @brief An axis-system with one support and an arbitrary amount of direction vectors.
template <typename T, std::size_t v_dim, std::size_t v_axis_count>
struct AxisSystem : detail::AxisSystemBase<T, v_dim, v_axis_count> {
    using Base = detail::AxisSystemBase<T, v_dim, v_axis_count>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr AxisSystem()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr AxisSystem(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}
};

/// @brief A line with one support and one direction vector.
template <typename T, std::size_t v_dim>
struct Line : detail::LineBase<T, v_dim> {
    using Base = detail::LineBase<T, v_dim>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}
};

/// @brief A two-dimensional line with one support and one direction vector.
template <typename T>
struct Line<T, 2> : detail::LineBase<T, 2> {
    using Base = detail::LineBase<T, 2>;

    using Point = typename Base::Point;
    using Direction = typename Base::Direction;
    using Factor = typename Base::Factor;

    using Height = T;
    using Distance = T;
    using LineFactors = Vector<T, 2>;
    using IntersectionMatrix = Matrix<T, 3, 2>;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(const Point& support, const Direction& direction)
        : Base(support, direction)
    {}

    /// @brief Returns the positive (left) or negative (right) distance between the (infinite) line and given point.
    constexpr Height heightTo(const Point& point) const
    {
        if (auto distance = Line(this->support, this->direction().cross().normalize()).closestFactorTo(point))
            return *distance;
        return this->support.distanceTo(point);
    }

    /// @brief Returns the distance between the (infinite) line and given point.
    constexpr Distance distanceTo(const Point& point) const
    {
        auto height = heightTo(point);
        return height >= 0 ? height : -height;
    }

    /// @brief Returns the side of the line, where the point is positioned.
    constexpr LineSide sideOf(const Point& point) const
    {
        auto distance = heightTo(point);
        if (distance > 0)
            return LineSide::Left;
        else if (distance != 0)
            return LineSide::Right;
        return LineSide::Hit;
    }

    /// @brief Builds a matrix, which can be used to calculate the intersection with another line.
    constexpr IntersectionMatrix intersectionMatrix(const Line& other) const
    {
        return IntersectionMatrix({this->direction(), -other.direction(), other.support - this->support});
    }

    /// @brief Returns the factor to reach the intersection point with the given line.
    constexpr std::optional<Factor> intersectionFactor(const Line& other) const
    {
        return intersectionMatrix(other).solveCol(0);
    }

    /// @brief Returns both factors to reach the intersection point with the given line.
    constexpr std::optional<LineFactors> intersectionFactors(const Line& other) const
    {
        return intersectionMatrix(other).solve();
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point.
    constexpr std::optional<Point> intersectionPoint(const Line& other) const
    {
        if (auto factor = intersectionFactor(other))
            return (*this)[*factor];
        return std::nullopt;
    }
};

/// @brief A three-dimensional line with one support and one direction vector.
template <typename T>
struct Line<T, 3> : detail::LineBase<T, 3> {
    using Base = detail::LineBase<T, 3>;

    using Point = typename Base::Point;
    using Direction = typename Base::Direction;
    using Factor = typename Base::Factor;

    using Plane = typename Base::Plane;

    using Distance = T;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Line()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Line(const Point& support, const Direction& directions)
        : Base(support, directions)
    {}

    /// @brief Returns the distance between the (infinite) line and given point.
    constexpr Distance distanceTo(const Point& point) const
    {
        if (this->direction() == Direction())
            return this->support.distanceTo(point);
        return this->direction().cross(point.vectorTo(this->support)).length() / this->direction().length();
    }

    using Base::closestFactorTo;

    /// @brief Returns the factor to the point on this line, which lies closest to the given line.
    constexpr std::optional<Factor> closestFactorTo(const Line& other) const
    {
        return Plane(this->support,
                     typename Plane::Directions({this->direction(), this->direction().cross(other.direction())}))
            .intersectionLineFactor(other);
    }

    using Base::closestPointTo;

    /// @brief Returns the point on this line, which lies closest to the given line.
    constexpr std::optional<Point> closestPointTo(const Line& other) const
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
    using Base = detail::PlaneBase<T, v_dim>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}
};

/// @brief A two-dimensional plane with one support and two direction vectors.
template <typename T>
struct Plane<T, 2> : detail::PlaneBase<T, 2> {
    using Base = detail::PlaneBase<T, 2>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;
    using Factors = typename Base::Factors;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}

    /// @brief Returns the required factor to reach the specified point.
    constexpr Factors factorAt(const Point& point) const
    {
        const auto& dx = this->directions[0];
        const auto& dy = this->directions[1];

        auto div = dx.cross(dy);

        auto point_relative = point - this->support;

        auto resultx = point_relative.cross(dy) / div;

        const auto& x = dy.x();
        const auto& y = dy.y();
        if ((x >= 0 ? x : -x) > (y >= 0 ? y : -y))
            return Factors(resultx, (point_relative.x() - resultx * dx.x()) / x);
        return Factors(resultx, (point_relative.y() - resultx * dx.y()) / y);
    }
};

/// @brief A three-dimensional plane with one support and two direction vectors.
template <typename T>
struct Plane<T, 3> : detail::PlaneBase<T, 3> {
    using Base = detail::PlaneBase<T, 3>;

    using Point = typename Base::Point;
    using Direction = typename Base::Direction;
    using Directions = typename Base::Directions;
    using Factor = typename Base::Factor;

    using Line = typename Base::Line;

    using Height = T;
    using Distance = T;
    using CosAngle = T;
    using Radians = T;
    using Degrees = T;
    using PlaneLineFactors = Vector<T, 3>;
    using IntersectionMatrix = Matrix<T, 4, 3>;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Plane()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Plane(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}

    /// @brief Returns the perpendicular of the plane using the cross-product.
    /// @remark The length of the result is the area of the plane.
    constexpr Direction perpendicular() const { return this->directions[0].cross(this->directions[1]); }

    /// @brief Returns the perpendicular of the plane as a line with the same support vector using the cross-product.
    /// @remark The length of the result is the area of the plane.
    constexpr Line perpendicularLine() const { return Line(this->support, perpendicular()); }

    /// @brief Returns a normalized perpendicular of the plane.
    constexpr Direction normal() const { return perpendicular().normalize(); }

    /// @brief Returns a normalized perpendicular of the plane as a line with the same support vector.
    constexpr Line normalLine() const { return Line(this->support, normal()); }

    /// @brief Returns the positive (top) or negative (bottom) distance between the (infinite) plane and given point.
    constexpr Height heightTo(const Point& point) const
    {
        if (auto distance = normalLine().closestFactorTo(point))
            return *distance;
        return this->support.distanceTo(point);
    }

    /// @brief Returns the distance between the (infinite) plane and given point.
    constexpr Distance distanceTo(const Point& point) const
    {
        auto height = heightTo(point);
        return height >= 0 ? height : -height;
    }

    /// @brief Returns the side of the plane, where the point is positioned.
    constexpr PlaneSide sideOf(const Point& point) const
    {
        auto distance = heightTo(point);
        if (distance > 0)
            return PlaneSide::Top;
        else if (distance != 0)
            return PlaneSide::Bottom;
        return PlaneSide::Hit;
    }

    /// @brief Builds a matrix, which can be used to calculate the intersection with a line.
    constexpr IntersectionMatrix intersectionMatrix(const Line& line) const
    {
        return IntersectionMatrix(
            {this->directions[0], this->directions[1], -line.direction(), line.support - this->support});
    }

    /// @brief Returns the factors to reach the intersection point with the given line for the plane (xy) and line (z).
    constexpr std::optional<PlaneLineFactors> intersectionFactors(const Line& line) const
    {
        return intersectionMatrix(line).solve();
    }

    /// @brief Returns the factor to reach the intersection point with the given line for the line itself.
    constexpr std::optional<Factor> intersectionLineFactor(const Line& line) const
    {
        return intersectionMatrix(line).solveCol(2);
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point.
    constexpr std::optional<Point> intersectionPoint(const Line& line) const
    {
        if (auto factor = intersectionLineFactor(line))
            return line[*factor];
        return std::nullopt;
    }

    /// @brief Calculates the intersection with the given line and returns the intersection point, using the plane.
    constexpr std::optional<Point> intersectionPointViaPlane(const Line& line) const
    {
        if (auto factors = intersectionFactors(line))
            return (*this)[factors->xy()];
        return std::nullopt;
    }

    /// @brief Returns the intersection with another plane in the form of a line of arbitrary position and length.
    constexpr std::optional<Line> intersectionLine(const Plane& plane) const
    {
        auto perp = perpendicular();
        auto dir = perp.cross(plane.perpendicular());
        Line line(this->support, dir.cross(perp));
        if (auto pos = plane.intersectionPoint(line))
            return Line(*pos, dir);
        return std::nullopt;
    }

    /// @brief Returns the cosine of the angle between the planes perpendicular and the given direction.
    constexpr CosAngle cosAngleToPerpendicular(const Direction& direction) const
    {
        return perpendicular().cosAngleTo(direction);
    }

    /// @brief Returns the angle between the planes perpendicular and the given direction in radians.
    constexpr Radians radiansToPerpendicular(const Direction& direction) const
    {
        return perpendicular().radiansTo(direction);
    }

    /// @brief Returns the angle between the planes perpendicular and the given direction in degrees.
    constexpr Degrees degreesToPerpendicular(const Direction& direction) const
    {
        return perpendicular().degreesTo(direction);
    }

    /// @brief Returns the angle between the plane and the given direction in radians.
    constexpr Radians radiansTo(const Direction& direction) const
    {
        return pi_v<Radians> / Radians(2) - perpendicular().radiansTo(direction);
    }

    /// @brief Returns the angle between the plane and the given direction in degrees.
    constexpr Degrees degreesTo(const Direction& direction) const
    {
        return Degrees(90) - perpendicular().degreesTo(direction);
    }

    /// @brief Returns the cosine of the angle to the given plane.
    constexpr CosAngle cosAngleTo(const Plane& other) const
    {
        return perpendicular().cosAngleTo(other.perpendicular());
    }

    /// @brief Returns the angle to the given plane in radians.
    constexpr Radians radiansTo(const Plane& other) const { return perpendicular().radiansTo(other.perpendicular()); }

    /// @brief Returns the angle to the given plane in degrees.
    constexpr Degrees degreesTo(const Plane& other) const { return perpendicular().degreesTo(other.perpendicular()); }

    /// @brief Returns the point mirrored on the plane.
    constexpr std::optional<Point> mirror(const Point& point) const { return perpendicularLine().mirror(point); }
};

using Plane1 = Plane<float, 1>;
using Plane2 = Plane<float, 2>;
using Plane3 = Plane<float, 3>;

/// @brief A spat with one support and three direction vectors.
template <typename T, std::size_t v_dim>
struct Spat : detail::SpatBase<T, v_dim> {
    using Base = detail::SpatBase<T, v_dim>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Spat()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Spat(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}
};

/// @brief A three-dimensional spat with one support and three direction vectors.
template <typename T>
struct Spat<T, 3> : detail::SpatBase<T, 3> {
    using Base = detail::SpatBase<T, 3>;

    using Point = typename Base::Point;
    using Directions = typename Base::Directions;

    using TripleProduct = T;

    /// @brief Initializes support and direction vectors with zero.
    constexpr Spat()
        : Base()
    {}
    /// @brief Initializes support and direction vectors with the given vectors.
    constexpr Spat(const Point& support, const Directions& directions)
        : Base(support, directions)
    {}

    /// @brief Returns the triple product (aka Spatprodukt) of the spat.
    constexpr TripleProduct tripleProduct() const { return this->directions.determinant(); }
};

using Spat1 = Spat<float, 1>;
using Spat2 = Spat<float, 2>;
using Spat3 = Spat<float, 3>;

} // namespace dang::math
