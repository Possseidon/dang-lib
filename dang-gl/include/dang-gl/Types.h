#pragma once

#include "dang-math/vector.h"
#include "dang-math/matrix.h"

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

DMATH_MATRIX_DEFINE(mat, GLfloat)
DMATH_MATRIX_DEFINE(dmat, GLdouble)

};
