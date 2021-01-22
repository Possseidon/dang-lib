#pragma once

#include "dang-math/global.h"

#include "dang-utils/enum.h"

namespace dang::math {

/// @brief Represents the single x-axis of a one-dimensional system or an optional None value.
enum class Axis1 { None = -1, X, COUNT };

/// @brief Represents one of the two axes in a two-dimensional system or an optional None value.
enum class Axis2 { None = -1, X, Y, COUNT };

/// @brief Represents one of the three axes in a three-dimensional system or an optional None value.
enum class Axis3 { None = -1, X, Y, Z, COUNT };

/// @brief Represents one of the two corners in a one-dimensional system or an optional None value.
enum class Corner1 { None = -1, Left, Right, COUNT };

/// @brief Represents one of the four corners in a two-dimensional system or an optional None value.
enum class Corner2 { None = -1, LeftBottom, RightBottom, LeftTop, RightTop, COUNT };

/// @brief Represents one of the eight corners in a three-dimensional system or an optional None value.
enum class Corner3 {
    None = -1,
    LeftBottomBack,
    RightBottomBack,
    LeftTopBack,
    RightTopBack,
    LeftBottomFront,
    RightBottomFront,
    LeftTopFront,
    RightTopFront,
    COUNT
};

/// @brief Represents one of the four edges in a two-dimensional system or an optional None value.
enum class Edge2 { None = -1, Left, Right, Bottom, Top, COUNT };

/// @brief Represents one of the twelve edges in a three-dimensional system or an optional None value.
enum class Edge3 {
    None = -1,
    LeftBottom,
    RightBottom,
    LeftTop,
    RightTop,
    BottomBack,
    TopBack,
    BottomFront,
    TopFront,
    LeftFront,
    RightFront,
    LeftBack,
    RightBack,
    COUNT
};

/// @brief Represents one of the two facings in a one-dimensional system or an optional None value.
enum class Facing1 { None = -1, Left, Right, COUNT };

/// @brief Represents one of the four facings in a two-dimensional system or an optional None value.
enum class Facing2 { None = -1, Left, Right, Up, Down, COUNT };

/// @brief Represents one of the six facings in a three-dimensional system or an optional None value.
enum class Facing3 { None = -1, Left, Right, Up, Down, Back, Front, COUNT };

} // namespace dang::math

namespace dang::utils {

template <>
struct enum_count<dang::math::Axis1> : default_enum_count<dang::math::Axis1> {};

template <>
struct enum_count<dang::math::Axis2> : default_enum_count<dang::math::Axis2> {};

template <>
struct enum_count<dang::math::Axis3> : default_enum_count<dang::math::Axis3> {};

template <>
struct enum_count<dang::math::Corner1> : default_enum_count<dang::math::Corner1> {};

template <>
struct enum_count<dang::math::Corner2> : default_enum_count<dang::math::Corner2> {};

template <>
struct enum_count<dang::math::Corner3> : default_enum_count<dang::math::Corner3> {};

template <>
struct enum_count<dang::math::Edge2> : default_enum_count<dang::math::Edge2> {};

template <>
struct enum_count<dang::math::Edge3> : default_enum_count<dang::math::Edge3> {};

template <>
struct enum_count<dang::math::Facing1> : default_enum_count<dang::math::Facing1> {};

template <>
struct enum_count<dang::math::Facing2> : default_enum_count<dang::math::Facing2> {};

template <>
struct enum_count<dang::math::Facing3> : default_enum_count<dang::math::Facing3> {};

} // namespace dang::utils

namespace dang::math {

namespace detail {

template <std::size_t Dim>
struct AxisSelector {
    enum Type {};
};

template <>
struct AxisSelector<1> {
    using Type = Axis1;
};

template <>
struct AxisSelector<2> {
    using Type = Axis2;
};

template <>
struct AxisSelector<3> {
    using Type = Axis3;
};

template <std::size_t Dim>
struct CornerSelector {
    enum Type {};
};

template <>
struct CornerSelector<1> {
    using Type = Corner1;
};

template <>
struct CornerSelector<2> {
    using Type = Corner2;
};

template <>
struct CornerSelector<3> {
    using Type = Corner3;
};

template <std::size_t Dim>
struct EdgeSelector {
    enum Type {};
};

template <>
struct EdgeSelector<2> {
    using Type = Edge2;
};

template <>
struct EdgeSelector<3> {
    using Type = Edge3;
};

template <std::size_t Dim>
struct FacingSelector {
    enum Type {};
};

template <>
struct FacingSelector<1> {
    using Type = Facing1;
};

template <>
struct FacingSelector<2> {
    using Type = Facing2;
};

template <>
struct FacingSelector<3> {
    using Type = Facing3;
};

} // namespace detail

template <std::size_t Dim>
using Axis = typename detail::AxisSelector<Dim>::Type;

template <std::size_t Dim>
using Corner = typename detail::CornerSelector<Dim>::Type;

template <std::size_t Dim>
using Edge = typename detail::EdgeSelector<Dim>::Type;

template <std::size_t Dim>
using Facing = typename detail::FacingSelector<Dim>::Type;

template <std::size_t Dim>
using Axes = dutils::EnumSet<Axis<Dim>>;

template <std::size_t Dim>
using Corners = dutils::EnumSet<Corner<Dim>>;

template <std::size_t Dim>
using Edges = dutils::EnumSet<Edge<Dim>>;

template <std::size_t Dim>
using Facings = dutils::EnumSet<Facing<Dim>>;

using Axes1 = Axes<1>;
using Axes2 = Axes<2>;
using Axes3 = Axes<3>;

using Corners1 = Corners<1>;
using Corners2 = Corners<2>;
using Corners3 = Corners<3>;

using Edges2 = Edges<2>;
using Edges3 = Edges<3>;

using Facings1 = Facings<1>;
using Facings2 = Facings<2>;
using Facings3 = Facings<3>;

} // namespace dang::math
