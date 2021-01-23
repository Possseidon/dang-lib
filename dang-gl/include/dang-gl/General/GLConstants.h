#pragma once

#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Maps from enum types to the actual GLenum constants, used in OpenGL function calls.
template <typename T>
inline constexpr auto gl_constants = nullptr;

/// @brief Same as gl_constants, but allows for automatic type deduction.
template <typename T>
constexpr auto toGLConstant(T value)
{
    return gl_constants<T>[value];
}

} // namespace dang::gl
