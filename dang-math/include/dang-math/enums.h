#pragma once

#include "utils.h"

#include "dang-utils/enum.h"

namespace dang::math {

/// <summary>Represents the single x-axis of a one-dimensional system or an optional None value.</summary>
enum class Axis1 : char { None = -1, X, COUNT };

/// <summary>Represents a set of the single x-axis of a one-dimensional system.</summary>
enum class Axes1 : unsigned char { NONE = 0, X = 1 << 0, ALL = ~(~0u << 1) };

/// <summary>Represents one of the two axes in a two-dimensional system or an optional None value.</summary>
enum class Axis2 : char { None = -1, X, Y, COUNT };

/// <summary>Represents a set of the two axes in a two-dimensional system.</summary>
enum class Axes2 : unsigned char { NONE = 0, X = 1 << 0, Y = 1 << 1, ALL = ~(~0u << 2) };

/// <summary>Represents one of the three axes in a three-dimensional system or an optional None value.</summary>
enum class Axis3 : char { None = -1, X, Y, Z, COUNT };

/// <summary>Represents a set of the three axes in a three-dimensional system.</summary>
enum class Axes3 : unsigned char { NONE = 0, X = 1 << 0, Y = 1 << 1, Z = 1 << 2, ALL = ~(~0u << 3) };

/// <summary>Represents one of the two corners in a one-dimensional system or an optional None value.</summary>
enum class Corner1 : char { None = -1, Left, Right, COUNT };

/// <summary>Represents a set of the two corners in a one-dimensional system.</summary>
enum class Corners1 : unsigned char { NONE = 0, Left = 1 << 0, Right = 1 << 1, ALL = ~(~0u << 2) };

/// <summary>Represents one of the four corners in a two-dimensional system or an optional None value.</summary>
enum class Corner2 : char { None = -1, LeftBottom, RightBottom, LeftTop, RightTop, COUNT };

/// <summary>Represents a set of the four corners in a two-dimensional system.</summary>
enum class Corners2 : unsigned char {
    NONE = 0,
    LeftBottom = 1 << 0,
    RightBottom = 1 << 1,
    LeftTop = 1 << 2,
    RightTop = 1 << 3,
    ALL = ~(~0u << 4)
};

/// <summary>Represents one of the eight corners in a three-dimensional system or an optional None value.</summary>
enum class Corner3 : char {
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

/// <summary>Represents a set of the eight corners in a three-dimensional system.</summary>
enum class Corners3 : unsigned char {
    NONE = 0,
    LeftBottomBack = 1 << 0,
    RightBottomBack = 1 << 1,
    LeftTopBack = 1 << 2,
    RightTopBack = 1 << 3,
    LeftBottomFront = 1 << 4,
    RightBottomFront = 1 << 5,
    LeftTopFront = 1 << 6,
    RightTopFront = 1 << 7,
    ALL = ~(~0u << 8)
};

/// <summary>Represents one of the four edges in a two-dimensional system or an optional None value.</summary>
enum class Edge2 : char { None = -1, Left, Right, Bottom, Top, COUNT };

/// <summary>Represents a set of the four edges in a two-dimensional system.</summary>
enum class Edges2 : unsigned char {
    NONE = 0,
    Left = 1 << 0,
    Right = 1 << 1,
    Bottom = 1 << 2,
    Top = 1 << 3,
    ALL = ~(~0u << 4)
};

/// <summary>Represents one of the twelve edges in a three-dimensional system or an optional None value.</summary>
enum class Edge3 : char {
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

/// <summary>Represents a set of the twelve edges in a three-dimensional system.</summary>
enum class Edges3 : unsigned short {
    NONE = 0,
    LeftBottom = 1 << 0,
    RightBottom = 1 << 1,
    LeftTop = 1 << 2,
    RightTop = 1 << 3,
    BottomBack = 1 << 4,
    TopBack = 1 << 5,
    BottomFront = 1 << 6,
    TopFront = 1 << 7,
    LeftFront = 1 << 8,
    RightFront = 1 << 9,
    LeftBack = 1 << 10,
    RightBack = 1 << 11,
    ALL = ~(~0u << 12)
};

/// <summary>Represents one of the two facings in a one-dimensional system or an optional None value.</summary>
enum class Facing1 : char { None = -1, Left, Right, COUNT };

/// <summary>Represents a set of the two facings in a one-dimensional system.</summary>
enum class Facings1 : unsigned char { NONE = 0, Left = 1 << 0, Right = 1 << 1, ALL = ~(~0u << 2) };

/// <summary>Represents one of the four facings in a two-dimensional system or an optional None value.</summary>
enum class Facing2 : char { None = -1, Left, Right, Up, Down, COUNT };

/// <summary>Represents a set of the four facings in a two-dimensional system.</summary>
enum class Facings2 : unsigned char {
    NONE = 0,
    Left = 1 << 0,
    Right = 1 << 1,
    Up = 1 << 2,
    Down = 1 << 3,
    ALL = ~(~0u << 4)
};

/// <summary>Represents one of the six facings in a three-dimensional system or an optional None value.</summary>
enum class Facing3 : char { None = -1, Left, Right, Up, Down, Back, Front, COUNT };

/// <summary>Represents a set of the six facings in a three-dimensional system.</summary>
enum class Facings3 : unsigned char {
    NONE = 0,
    Left = 1 << 0,
    Right = 1 << 1,
    Up = 1 << 2,
    Down = 1 << 3,
    Back = 1 << 4,
    Front = 1 << 5,
    ALL = ~(~0u << 6)
};

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
struct AxisSelector {
    enum class Type {};
    enum class SetType {};
};

template <>
struct AxisSelector<1> {
    using Type = Axis1;
    using SetType = Axes1;
};

template <>
struct AxisSelector<2> {
    using Type = Axis2;
    using SetType = Axes2;
};

template <>
struct AxisSelector<3> {
    using Type = Axis3;
    using SetType = Axes3;
};

template <std::size_t Dim>
struct CornerSelector {
    enum class Type {};
    enum class SetType {};
};

template <>
struct CornerSelector<1> {
    using Type = Corner1;
    using SetType = Corners1;
};

template <>
struct CornerSelector<2> {
    using Type = Corner2;
    using SetType = Corners2;
};

template <>
struct CornerSelector<3> {
    using Type = Corner3;
    using SetType = Corners3;
};

template <std::size_t Dim>
struct EdgeSelector {
    enum class Type {};
    enum class SetType {};
};

template <>
struct EdgeSelector<2> {
    using Type = Edge2;
    using SetType = Edges2;
};

template <>
struct EdgeSelector<3> {
    using Type = Edge3;
    using SetType = Edges3;
};

template <std::size_t Dim>
struct FacingSelector {
    enum class Type {};
    enum class SetType {};
};

template <>
struct FacingSelector<1> {
    using Type = Facing1;
    using SetType = Facings1;
};

template <>
struct FacingSelector<2> {
    using Type = Facing2;
    using SetType = Facings2;
};

template <>
struct FacingSelector<3> {
    using Type = Facing3;
    using SetType = Facings3;
};

} // namespace detail

template <std::size_t Dim>
using Axis = typename detail::AxisSelector<Dim>::Type;

template <std::size_t Dim>
using Axes = typename detail::AxisSelector<Dim>::SetType;

template <std::size_t Dim>
using Corner = typename detail::CornerSelector<Dim>::Type;

template <std::size_t Dim>
using Corners = typename detail::CornerSelector<Dim>::SetType;

template <std::size_t Dim>
using Edge = typename detail::EdgeSelector<Dim>::Type;

template <std::size_t Dim>
using Edges = typename detail::EdgeSelector<Dim>::SetType;

template <std::size_t Dim>
using Facing = typename detail::FacingSelector<Dim>::Type;

template <std::size_t Dim>
using Facings = typename detail::FacingSelector<Dim>::SetType;

} // namespace dang::math
