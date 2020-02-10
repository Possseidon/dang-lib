#pragma once

#include "dang-utils/enum.h"

namespace dang::gl
{

enum class ObjectType {
    Texture,
    VertexArray,
    Buffer,
    Shader,
    Program,
    Query,
    ProgramPipeline,
    Sampler,
    DisplayList,
    Framebuffer,
    Renderbuffer,
    TransformFeedback,
    COUNT
};

constexpr dutils::EnumArray<ObjectType, GLenum> ObjectTypesGL
{
   GL_TEXTURE,
   GL_VERTEX_ARRAY,
   GL_BUFFER,
   GL_SHADER,
   GL_PROGRAM,
   GL_QUERY,
   GL_PROGRAM_PIPELINE,
   GL_SAMPLER,
   GL_DISPLAY_LIST,
   GL_FRAMEBUFFER,
   GL_RENDERBUFFER,
   GL_TRANSFORM_FEEDBACK
};

}
