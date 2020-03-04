#pragma once

#include "dang-math/matrix.h"
#include "dang-math/quaternion.h"
#include "dang-math/vector.h"

namespace dang::gl
{

template <std::size_t Dim>
using vec = dmath::Vector<GLfloat, Dim>;

template <std::size_t Dim>
using dvec = dmath::Vector<GLdouble, Dim>;

template <std::size_t Dim>
using ivec = dmath::Vector<GLint, Dim>;

template <std::size_t Dim>
using uvec = dmath::Vector<GLuint, Dim>;

template <std::size_t Dim>
using bvec = dmath::Vector<GLboolean, Dim>;

using vec1 = vec<1>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

using dvec1 = dvec<1>;
using dvec2 = dvec<2>;
using dvec3 = dvec<3>;
using dvec4 = dvec<4>;

using ivec1 = ivec<1>;
using ivec2 = ivec<2>;
using ivec3 = ivec<3>;
using ivec4 = ivec<4>;

using uvec1 = uvec<1>;
using uvec2 = uvec<2>;
using uvec3 = uvec<3>;
using uvec4 = uvec<4>;

using bvec1 = bvec<1>;
using bvec2 = bvec<2>;
using bvec3 = bvec<3>;
using bvec4 = bvec<4>;

template <std::size_t Dim>
using bounds = dmath::Bounds<GLfloat, Dim>;

template <std::size_t Dim>
using dbounds = dmath::Bounds<GLdouble, Dim>;

template <std::size_t Dim>
using ibounds = dmath::Bounds<GLint, Dim>;

template <std::size_t Dim>
using ubounds = dmath::Bounds<GLuint, Dim>;

using bounds1 = bounds<1>;
using bounds2 = bounds<2>;
using bounds3 = bounds<3>;

using dbounds1 = dbounds<1>;
using dbounds2 = dbounds<2>;
using dbounds3 = dbounds<3>;

using ibounds1 = ibounds<1>;
using ibounds2 = ibounds<2>;
using ibounds3 = ibounds<3>;

using ubounds1 = ubounds<1>;
using ubounds2 = ubounds<2>;
using ubounds3 = ubounds<3>;

DMATH_MATRIX_DEFINE(mat, GLfloat)
DMATH_MATRIX_DEFINE(dmat, GLdouble)

using quat = dmath::Quaternion<GLfloat>;
using dquat = dmath::DualQuaternion<GLfloat>;

};
