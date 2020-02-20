#pragma once

#include "dang-utils/enum.h"

namespace dang::gl
{

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

constexpr dutils::EnumArray<ObjectType, GLenum> ObjectTypesGL
{
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
