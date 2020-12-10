#pragma once

#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Wraps the bitfield representing the three buffers of framebuffer objects.
enum class BufferMask : GLbitfield {
    NONE = 0,
    Color = GL_COLOR_BUFFER_BIT,
    Depth = GL_DEPTH_BUFFER_BIT,
    Stencil = GL_STENCIL_BUFFER_BIT,
    ALL = Color | Depth | Stencil
};

} // namespace dang::gl
