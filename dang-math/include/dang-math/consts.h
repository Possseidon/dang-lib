#pragma once

#include "dang-math/enums.h"
#include "dang-math/global.h"
#include "dang-math/vector.h"

#include "dang-utils/enum.h"

namespace dang::math {

// --- axis_vector

template <std::size_t Dim>
inline constexpr auto axis_vector = nullptr;

template <>
inline constexpr dutils::EnumArray<Axis1, ivec1> axis_vector<1> = {ivec1(1)};

template <>
inline constexpr dutils::EnumArray<Axis2, ivec2> axis_vector<2> = {ivec2(1, 0), ivec2(0, 1)};

template <>
inline constexpr dutils::EnumArray<Axis3, ivec3> axis_vector<3> = {ivec3(1, 0, 0), ivec3(0, 1, 0), ivec3(0, 0, 1)};

inline constexpr auto axis_vector_1 = axis_vector<1>;
inline constexpr auto axis_vector_2 = axis_vector<2>;
inline constexpr auto axis_vector_3 = axis_vector<3>;

// --- facing_vector

template <std::size_t Dim>
inline constexpr auto facing_vector = nullptr;

template <>
inline constexpr dutils::EnumArray<Facing1, ivec1> facing_vector<1> = {ivec1(-1), ivec1(+1)};

template <>
inline constexpr dutils::EnumArray<Facing2, ivec2> facing_vector<2> = {
    ivec2(-1, 0), ivec2(+1, 0), ivec2(0, -1), ivec2(0, +1)};

template <>
inline constexpr dutils::EnumArray<Facing3, ivec3> facing_vector<3> = {
    ivec3(-1, 0, 0), ivec3(+1, 0, 0), ivec3(0, -1, 0), ivec3(0, +1, 0), ivec3(0, 0, -1), ivec3(0, 0, +1)};

inline constexpr auto facing_vector_1 = facing_vector<1>;
inline constexpr auto facing_vector_2 = facing_vector<2>;
inline constexpr auto facing_vector_3 = facing_vector<3>;

// --- corner_vector

template <std::size_t Dim>
inline constexpr auto corner_vector = nullptr;

template <>
inline constexpr dutils::EnumArray<Corner1, ivec1> corner_vector<1> = {ivec1(0), ivec1(1)};

template <>
inline constexpr dutils::EnumArray<Corner2, ivec2> corner_vector<2> = {
    ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1)};

template <>
inline constexpr dutils::EnumArray<Corner3, ivec3> corner_vector<3> = {ivec3(0, 0, 0),
                                                                       ivec3(1, 0, 0),
                                                                       ivec3(0, 1, 0),
                                                                       ivec3(1, 1, 0),
                                                                       ivec3(0, 0, 1),
                                                                       ivec3(1, 0, 1),
                                                                       ivec3(0, 1, 1),
                                                                       ivec3(1, 1, 1)};

inline constexpr auto corner_vector_1 = corner_vector<1>;
inline constexpr auto corner_vector_2 = corner_vector<2>;
inline constexpr auto corner_vector_3 = corner_vector<3>;

// --- axis_facing

template <std::size_t Dim>
inline constexpr auto axis_facing = nullptr;

template <>
inline constexpr dutils::EnumArray<Axis1, Facing1> axis_facing<1> = {Facing1::Right};

template <>
inline constexpr dutils::EnumArray<Axis2, Facing2> axis_facing<2> = {Facing2::Right, Facing2::Up};

template <>
inline constexpr dutils::EnumArray<Axis3, Facing3> axis_facing<3> = {Facing3::Right, Facing3::Up, Facing3::Front};

inline constexpr auto axis_facing_1 = axis_facing<1>;
inline constexpr auto axis_facing_2 = axis_facing<2>;
inline constexpr auto axis_facing_3 = axis_facing<3>;

// --- facing_axis

template <std::size_t Dim>
inline constexpr auto facing_axis = nullptr;

template <>
inline constexpr dutils::EnumArray<Facing1, Axis1> facing_axis<1> = {Axis1::X, Axis1::X};

template <>
inline constexpr dutils::EnumArray<Facing2, Axis2> facing_axis<2> = {Axis2::X, Axis2::X, Axis2::Y, Axis2::Y};

template <>
inline constexpr dutils::EnumArray<Facing3, Axis3> facing_axis<3> = {
    Axis3::X, Axis3::X, Axis3::Y, Axis3::Y, Axis3::Z, Axis3::Z};

inline constexpr auto facing_axis_1 = facing_axis<1>;
inline constexpr auto facing_axis_2 = facing_axis<2>;
inline constexpr auto facing_axis_3 = facing_axis<3>;

// --- facing_flipped

template <std::size_t Dim>
inline constexpr auto facing_flipped = nullptr;

template <>
inline constexpr dutils::EnumArray<Facing1, Facing1> facing_flipped<1> = {Facing1::Right, Facing1::Left};

template <>
inline constexpr dutils::EnumArray<Facing2, Facing2> facing_flipped<2> = {
    Facing2::Right, Facing2::Left, Facing2::Up, Facing2::Down};

template <>
inline constexpr dutils::EnumArray<Facing3, Facing3> facing_flipped<3> = {
    Facing3::Right, Facing3::Left, Facing3::Up, Facing3::Down, Facing3::Front, Facing3::Back};

inline constexpr auto facing_flipped_1 = facing_flipped<1>;
inline constexpr auto facing_flipped_2 = facing_flipped<2>;
inline constexpr auto facing_flipped_3 = facing_flipped<3>;

// --- facing_corners

template <std::size_t Dim>
inline constexpr auto facing_corners = nullptr;

template <>
inline constexpr dutils::EnumArray<Facing1, Corners1> facing_corners<1> = {Corners1{Corner1::Left},
                                                                          Corners1{Corner1::Right}};

template <>
inline constexpr dutils::EnumArray<Facing2, Corners2> facing_corners<2> = {
    Corners2{Corner2::LeftBottom, Corner2::LeftTop},
    Corners2{Corner2::RightBottom, Corner2::RightTop},
    Corners2{Corner2::LeftBottom, Corner2::RightBottom},
    Corners2{Corner2::LeftTop, Corner2::RightTop}};

template <>
inline constexpr dutils::EnumArray<Facing3, Corners3> facing_corners<3> = {
    Corners3{Corner3::LeftBottomBack, Corner3::LeftTopBack, Corner3::LeftBottomFront, Corner3::LeftTopFront},
    Corners3{Corner3::RightBottomBack, Corner3::RightTopBack, Corner3::RightBottomFront, Corner3::RightTopFront},
    Corners3{Corner3::LeftBottomBack, Corner3::RightBottomBack, Corner3::LeftBottomFront, Corner3::RightBottomFront},
    Corners3{Corner3::LeftTopBack, Corner3::RightTopBack, Corner3::LeftTopFront, Corner3::RightTopFront},
    Corners3{Corner3::LeftBottomBack, Corner3::RightBottomBack, Corner3::LeftTopBack, Corner3::RightTopBack},
    Corners3{Corner3::LeftBottomFront, Corner3::RightBottomFront, Corner3::LeftTopFront, Corner3::RightTopFront}};

inline constexpr auto facing_corners_1 = facing_corners<1>;
inline constexpr auto facing_corners_2 = facing_corners<2>;
inline constexpr auto facing_corners_3 = facing_corners<3>;

} // namespace dang::math
