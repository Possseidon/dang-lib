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

template <std::size_t v_dim>
struct axis_selector {
    enum type {};
};

template <>
struct axis_selector<1> {
    using type = Axis1;
};

template <>
struct axis_selector<2> {
    using type = Axis2;
};

template <>
struct axis_selector<3> {
    using type = Axis3;
};

template <std::size_t v_dim>
struct corner_selector {
    enum type {};
};

template <>
struct corner_selector<1> {
    using type = Corner1;
};

template <>
struct corner_selector<2> {
    using type = Corner2;
};

template <>
struct corner_selector<3> {
    using type = Corner3;
};

template <std::size_t v_dim>
struct edge_selector {
    enum type {};
};

template <>
struct edge_selector<2> {
    using type = Edge2;
};

template <>
struct edge_selector<3> {
    using type = Edge3;
};

template <std::size_t v_dim>
struct facing_selector {
    enum type {};
};

template <>
struct facing_selector<1> {
    using type = Facing1;
};

template <>
struct facing_selector<2> {
    using type = Facing2;
};

template <>
struct facing_selector<3> {
    using type = Facing3;
};

} // namespace detail

template <std::size_t v_dim>
using Axis = typename detail::axis_selector<v_dim>::type;

template <std::size_t v_dim>
using Corner = typename detail::corner_selector<v_dim>::type;

template <std::size_t v_dim>
using Edge = typename detail::edge_selector<v_dim>::type;

template <std::size_t v_dim>
using Facing = typename detail::facing_selector<v_dim>::type;

template <std::size_t v_dim>
using Axes = dutils::EnumSet<Axis<v_dim>>;

template <std::size_t v_dim>
using Corners = dutils::EnumSet<Corner<v_dim>>;

template <std::size_t v_dim>
using Edges = dutils::EnumSet<Edge<v_dim>>;

template <std::size_t v_dim>
using Facings = dutils::EnumSet<Facing<v_dim>>;

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
