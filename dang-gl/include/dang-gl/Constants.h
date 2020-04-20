#pragma once

#include "dang-utils/enum.h"

#include "dang-math/enums.h"
#include "dang-math/geometry.h"
#include "dang-math/matrix.h"
#include "dang-math/vector.h"

namespace dang::gl
{

constexpr dutils::EnumArray<dmath::Facing3, dmath::Plane3> CubePlanes
{
    dmath::Plane3({ 0, 0, 0 }, dmath::mat2x3({{  0, 0,  1 }, { 0, 1,  0 }})),
    dmath::Plane3({ 1, 0, 1 }, dmath::mat2x3({{  0, 0, -1 }, { 0, 1,  0 }})),
    dmath::Plane3({ 0, 0, 0 }, dmath::mat2x3({{  1, 0,  0 }, { 0, 0,  1 }})),
    dmath::Plane3({ 0, 1, 1 }, dmath::mat2x3({{  1, 0,  0 }, { 0, 0, -1 }})),
    dmath::Plane3({ 1, 0, 0 }, dmath::mat2x3({{ -1, 0,  0 }, { 0, 1,  0 }})),
    dmath::Plane3({ 0, 0, 1 }, dmath::mat2x3({{  1, 0,  0 }, { 0, 1,  0 }}))
};

constexpr std::array QuadTexCoords
{
    dmath::vec2(0, 0),
    dmath::vec2(1, 0),
    dmath::vec2(1, 1),
    dmath::vec2(1, 1),
    dmath::vec2(0, 1),
    dmath::vec2(0, 0)
};

constexpr std::array CenteredTexCoords
{
    dmath::vec2(-1, -1),
    dmath::vec2(+1, -1),
    dmath::vec2(+1, +1),
    dmath::vec2(+1, +1),
    dmath::vec2(-1, +1),
    dmath::vec2(-1, -1)
};

}
