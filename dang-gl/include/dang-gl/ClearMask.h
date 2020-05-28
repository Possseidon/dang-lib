#pragma once

/// <summary>Wraps the bitfield passed to glClear.</summary>
enum class ClearMask : GLbitfield {
    NONE = 0,
    Color = GL_COLOR_BUFFER_BIT,
    Depth = GL_DEPTH_BUFFER_BIT,
    Stencil = GL_STENCIL_BUFFER_BIT,
    ALL = Color | Depth | Stencil
};
