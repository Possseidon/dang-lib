#pragma once

#include "dang-math/global.h"

#include "dang-utils/enum.h"

namespace dang::math {

/// <summary>Represents the single x-axis of a one-dimensional system or an optional None value.</summary>
enum class Axis1 { None = -1, X, COUNT };

/// <summary>Represents one of the two axes in a two-dimensional system or an optional None value.</summary>
enum class Axis2 { None = -1, X, Y, COUNT };

/// <summary>Represents one of the three axes in a three-dimensional system or an optional None value.</summary>
enum class Axis3 { None = -1, X, Y, Z, COUNT };

/// <summary>Represents one of the two corners in a one-dimensional system or an optional None value.</summary>
enum class Corner1 { None = -1, Left, Right, COUNT };

/// <summary>Represents one of the four corners in a two-dimensional system or an optional None value.</summary>
enum class Corner2 { None = -1, LeftBottom, RightBottom, LeftTop, RightTop, COUNT };

/// <summary>Represents one of the eight corners in a three-dimensional system or an optional None value.</summary>
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

/// <summary>Represents one of the four edges in a two-dimensional system or an optional None value.</summary>
enum class Edge2 { None = -1, Left, Right, Bottom, Top, COUNT };

/// <summary>Represents one of the twelve edges in a three-dimensional system or an optional None value.</summary>
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

/// <summary>Represents one of the two facings in a one-dimensional system or an optional None value.</summary>
enum class Facing1 { None = -1, Left, Right, COUNT };

/// <summary>Represents one of the four facings in a two-dimensional system or an optional None value.</summary>
enum class Facing2 { None = -1, Left, Right, Up, Down, COUNT };

/// <summary>Represents one of the six facings in a three-dimensional system or an optional None value.</summary>
enum class Facing3 { None = -1, Left, Right, Up, Down, Back, Front, COUNT };

} // namespace dang::math

namespace dang::utils {

template <>
struct EnumCount<dang::math::Axis1> : DefaultEnumCount<dang::math::Axis1> {};

template <>
struct EnumCount<dang::math::Axis2> : DefaultEnumCount<dang::math::Axis2> {};

template <>
struct EnumCount<dang::math::Axis3> : DefaultEnumCount<dang::math::Axis3> {};

template <>
struct EnumCount<dang::math::Corner1> : DefaultEnumCount<dang::math::Corner1> {};

template <>
struct EnumCount<dang::math::Corner2> : DefaultEnumCount<dang::math::Corner2> {};

template <>
struct EnumCount<dang::math::Corner3> : DefaultEnumCount<dang::math::Corner3> {};

template <>
struct EnumCount<dang::math::Edge2> : DefaultEnumCount<dang::math::Edge2> {};

template <>
struct EnumCount<dang::math::Edge3> : DefaultEnumCount<dang::math::Edge3> {};

template <>
struct EnumCount<dang::math::Facing1> : DefaultEnumCount<dang::math::Facing1> {};

template <>
struct EnumCount<dang::math::Facing2> : DefaultEnumCount<dang::math::Facing2> {};

template <>
struct EnumCount<dang::math::Facing3> : DefaultEnumCount<dang::math::Facing3> {};

} // namespace dang::utils

namespace dang::math {

namespace detail {

template <std::size_t Dim>
struct AxisSelector {};

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
struct CornerSelector {};

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
struct EdgeSelector {};

template <>
struct EdgeSelector<2> {
    using Type = Edge2;
};

template <>
struct EdgeSelector<3> {
    using Type = Edge3;
};

template <std::size_t Dim>
struct FacingSelector {};

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
