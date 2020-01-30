#pragma once

#include "utils.h"

namespace dang::math
{

enum class CoordAxis3 : char {
    None = -1,
    X,
    Y,
    Z
};

enum class CoordAxes3 : unsigned char {
    X = 1 << 0,
    Y = 1 << 1,
    Z = 1 << 2
};

enum class Corner1 : char {
    None = -1,
    Left,
    Right,
    Count
};

enum class Corners1 : unsigned char {
    Left = 1 << 0,
    Right = 1 << 1
};

enum class Corner2 : char {
    None = -1,
    LeftBottom,
    RightBottom,
    LeftTop,
    RightTop,
    Count
};

enum class Corners2 : unsigned char {
    LeftBottom = 1 << 0,
    RightBottom = 1 << 1,
    LeftTop = 1 << 2,
    RightTop = 1 << 3
};

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
    Count
};

enum class Corners3 : unsigned char {
    LeftBottomBack = 1 << 0,
    RightBottomBack = 1 << 1,
    LeftTopBack = 1 << 2,
    RightTopBack = 1 << 3,
    LeftBottomFront = 1 << 4,
    RightBottomFront = 1 << 5,
    LeftTopFront = 1 << 6,
    RightTopFront = 1 << 7
};

enum class Edge2 : char {    
    None = -1,
    Left,
    Right,
    Bottom,
    Top,
    Count
};

enum class Edges2 : unsigned char {
    Left = 1 << 0,
    Right = 1 << 1,
    Bottom = 1 << 2,
    Top = 1 << 3
};
       
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
    Count
};

enum class Edge3 : unsigned short {
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
    RightBack = 1 << 11
};

namespace detail
{

template <std::size_t Dim>
struct CornerSelector {
    using Type = void;
    using SetType = void;
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
    using Type = void;
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

}
                                                  
template <std::size_t Dim>
using Corner = detail::CornerSelector<Dim>::Type;

template <std::size_t Dim>
using Corners = detail::CornerSelector<Dim>::SetType;
                                                    
template <std::size_t Dim>
using Edge = detail::EdgeSelector<Dim>::Type;

template <std::size_t Dim>
using Edges = detail::EdgeSelector<Dim>::SetType;

}
