#pragma once

namespace dang::gl {

/// <summary>Maps from enum types to the actual GLenum constants, used in OpenGL function calls.</summary>
template <typename T>
constexpr auto GLConstants = nullptr;

/// <summary>Same as GLConstants, but allows for automatic type deduction.</summary>
template <typename T>
constexpr auto toGLConstant(T value)
{
    return GLConstants<T>[value];
}

} // namespace dang::gl
