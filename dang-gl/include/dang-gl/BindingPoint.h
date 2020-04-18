#pragma once

#include "dang-utils/enum.h"

namespace dang::gl
{

/// <summary>A list of all separately bindable targets for glBind functions.</summary>
enum class BindingPoint {
    // glBindBuffer
    ArrayBuffer,
    AtomicCounterBuffer,
    CopyReadBuffer,
    CopyWriteBuffer,
    DispatchIndirectBuffer,
    DrawIndirectBuffer,
    ElementArrayBuffer,
    PixelPackBuffer,
    PixelUnpackBuffer,
    QueryBuffer,
    ShaderStorageBuffer,
    TextureBuffer,
    TransformFeedbackBuffer,
    UniformBuffer,

    // glBindTexture
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture2DMultisample,
    Texture2DMultisampleArray,
    Texture3D,
    TextureCubeMap,
    TextureRectangle,

    // glBindFramebuffer
    DrawFramebuffer,
    ReadFramebuffer,

    // glBindRenderbuffer
    Renderbuffer,

    // glBindProgramPipeline
    ProgramPipeline,

    // glBindSampler
    Sampler,

    // glBindVertexArray
    VertexArray,

    // glUseProgram
    Program,

    COUNT
};

/// <summary>Maps from binding points to their respective target constant, which needs to be supplied to the glBind function.</summary>
constexpr dutils::EnumArray<BindingPoint, GLenum> BindingPointTargets
{
    // glBindBuffer
    GL_ARRAY_BUFFER,
    GL_ATOMIC_COUNTER_BUFFER,
    GL_COPY_READ_BUFFER,
    GL_COPY_WRITE_BUFFER,
    GL_DISPATCH_INDIRECT_BUFFER,
    GL_DRAW_INDIRECT_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
    GL_PIXEL_PACK_BUFFER,
    GL_PIXEL_UNPACK_BUFFER,
    GL_QUERY_BUFFER,
    GL_SHADER_STORAGE_BUFFER,
    GL_TEXTURE_BUFFER,
    GL_TRANSFORM_FEEDBACK_BUFFER,
    GL_UNIFORM_BUFFER,

    // glBindTexture
    GL_TEXTURE_1D,
    GL_TEXTURE_1D_ARRAY,
    GL_TEXTURE_2D,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_2D_MULTISAMPLE,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    GL_TEXTURE_3D,
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_RECTANGLE,

    // glBindFramebuffer
    GL_DRAW_FRAMEBUFFER,
    GL_READ_FRAMEBUFFER,

    // glBindRenderbuffer
    0,

    // glBindProgramPipeline
    0,

    // glBindSampler
    0,

    // glBindVertexArray
    0,

    // glUseProgram
    0
};

}
