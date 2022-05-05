#pragma once

#include "dang-gl/global.h"
#include "dang-math/enums.h"
#include "dang-math/geometry.h"
#include "dang-math/matrix.h"
#include "dang-math/vector.h"
#include "dang-utils/enum.h"

namespace dang::gl {

inline constexpr dutils::EnumArray<dmath::Facing3, dmath::Plane3> cube_planes = {
    dmath::Plane3({0, 0, 0}, dmath::mat2x3({{0, 0, 1}, {0, 1, 0}})),
    dmath::Plane3({1, 0, 1}, dmath::mat2x3({{0, 0, -1}, {0, 1, 0}})),
    dmath::Plane3({0, 0, 0}, dmath::mat2x3({{1, 0, 0}, {0, 0, 1}})),
    dmath::Plane3({0, 1, 1}, dmath::mat2x3({{1, 0, 0}, {0, 0, -1}})),
    dmath::Plane3({1, 0, 0}, dmath::mat2x3({{-1, 0, 0}, {0, 1, 0}})),
    dmath::Plane3({0, 0, 1}, dmath::mat2x3({{1, 0, 0}, {0, 1, 0}}))};

inline constexpr std::array quad_tex_coords = {
    dmath::vec2(0, 0), dmath::vec2(1, 0), dmath::vec2(1, 1), dmath::vec2(1, 1), dmath::vec2(0, 1), dmath::vec2(0, 0)};

inline constexpr std::array centered_tex_coords = {dmath::vec2(-1, -1),
                                                   dmath::vec2(+1, -1),
                                                   dmath::vec2(+1, +1),
                                                   dmath::vec2(+1, +1),
                                                   dmath::vec2(-1, +1),
                                                   dmath::vec2(-1, -1)};

} // namespace dang::gl
