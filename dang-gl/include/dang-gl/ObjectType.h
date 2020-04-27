#pragma once

#include "dang-utils/enum.h"

#include "GLConstants.h"

namespace dang::gl
{

/// <summary>A list of all OpenGL objects, which can be created, some with multiple binding points.</summary>
enum class ObjectType {
    Buffer,
    Shader,
    Program,
    VertexArray,
    Query,
    ProgramPipeline,
    TransformFeedback,
    Sampler,
    Texture,
    Renderbuffer,
    Framebuffer,

    COUNT
};

/// <summary>The GL-Constants for object types, which is mainly used to query the currently bound object.</summary>
template <>
constexpr dutils::EnumArray<ObjectType, GLenum> GLConstants<ObjectType> = {
   GL_BUFFER,
   GL_SHADER,
   GL_PROGRAM,
   GL_VERTEX_ARRAY,
   GL_QUERY,
   GL_PROGRAM_PIPELINE,
   GL_TRANSFORM_FEEDBACK,
   GL_SAMPLER,
   GL_TEXTURE,
   GL_RENDERBUFFER,
   GL_FRAMEBUFFER
};

}
