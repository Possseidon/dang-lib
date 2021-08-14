#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief A list of all OpenGL objects, which can be created, some with multiple targets.
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

/// @brief The different buffer targets, which can be specified in glBindBuffer.
enum class BufferTarget {
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

    COUNT
};

/// @brief The different texture targets, which can be specified in glBindTexture.
enum class TextureTarget {
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture2DMultisample,
    Texture2DMultisampleArray,
    Texture3D,
    TextureCubeMap,
    TextureCubeMapArray,
    TextureRectangle,

    COUNT
};

/// @brief The different framebuffer targets, which can be specified in glBindFramebuffer.
/// @remark "Framebuffer" means both draw AND read framebuffers.
/// @remark If "both" doesn't make sense in the given context, it usually refers to the draw framebuffer.
enum class FramebufferTarget {
    Framebuffer,
    DrawFramebuffer,
    ReadFramebuffer,

    COUNT
};

/// @brief The different renderbuffer targets, which can be specified in glBindRenderbuffer.
/// @remark Currently the only "option" is simply GL_RENDERBUFFER.
enum class RenderbufferTarget {
    Renderbuffer,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::ObjectType> : default_enum_count<dang::gl::ObjectType> {};

template <>
struct enum_count<dang::gl::BufferTarget> : default_enum_count<dang::gl::BufferTarget> {};

template <>
struct enum_count<dang::gl::TextureTarget> : default_enum_count<dang::gl::TextureTarget> {};

template <>
struct enum_count<dang::gl::FramebufferTarget> : default_enum_count<dang::gl::FramebufferTarget> {};

template <>
struct enum_count<dang::gl::RenderbufferTarget> : default_enum_count<dang::gl::RenderbufferTarget> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief The GL-Constants for object types, which is mainly used to query the currently bound object.
template <>
inline constexpr dutils::EnumArray<ObjectType, GLenum> gl_constants<ObjectType> = {
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
    GL_FRAMEBUFFER,
};

/// @brief Maps from buffer targets to their respective constants, which need to be supplied to the glBindBuffer
/// function.
template <>
inline constexpr dutils::EnumArray<BufferTarget, GLenum> gl_constants<BufferTarget> = {
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
};

/// @brief Maps from texture targets to their respective constants, which need to be supplied to the glBindTexture
/// function.
template <>
inline constexpr dutils::EnumArray<TextureTarget, GLenum> gl_constants<TextureTarget> = {
    GL_TEXTURE_1D,
    GL_TEXTURE_1D_ARRAY,
    GL_TEXTURE_2D,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_2D_MULTISAMPLE,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    GL_TEXTURE_3D,
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_RECTANGLE};

/// @brief Maps from framebuffer targets to their respective constants, which need to be supplied to the
/// glBindFramebuffer function.
template <>
inline constexpr dutils::EnumArray<FramebufferTarget, GLenum> gl_constants<FramebufferTarget> = {
    GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER};

/// @brief Maps from renderbuffer targets to their respective constants, which need to be supplied to the
/// glBindRenderbuffer function.
template <>
inline constexpr dutils::EnumArray<RenderbufferTarget, GLenum> gl_constants<RenderbufferTarget> = {GL_RENDERBUFFER};

/// @brief Maps to the different enums for the various binding targets of the template specified object type.
/// @remark Not all bindable objects support multiple targets.
template <ObjectType>
struct object_target {
    using type = void;
};

template <>
struct object_target<ObjectType::Buffer> {
    using type = BufferTarget;
};

template <>
struct object_target<ObjectType::Texture> {
    using type = TextureTarget;
};

template <>
struct object_target<ObjectType::Renderbuffer> {
    using type = FramebufferTarget;
};

template <>
struct object_target<ObjectType::Framebuffer> {
    using type = RenderbufferTarget;
};

template <ObjectType v_type>
using object_target_t = typename object_target<v_type>::type;

namespace detail {

// Wraps OpenGL functions in a templated manner

template <ObjectType>
inline constexpr auto glGenObjects = nullptr;

template <>
inline constexpr auto& glGenObjects<ObjectType::Buffer> = glGenBuffers;
template <>
inline constexpr auto& glGenObjects<ObjectType::VertexArray> = glGenVertexArrays;
template <>
inline constexpr auto& glGenObjects<ObjectType::Query> = glGenQueries;
template <>
inline constexpr auto& glGenObjects<ObjectType::ProgramPipeline> = glGenProgramPipelines;
template <>
inline constexpr auto& glGenObjects<ObjectType::TransformFeedback> = glGenTransformFeedbacks;
template <>
inline constexpr auto& glGenObjects<ObjectType::Sampler> = glGenSamplers;
template <>
inline constexpr auto& glGenObjects<ObjectType::Texture> = glGenTextures;
template <>
inline constexpr auto& glGenObjects<ObjectType::Renderbuffer> = glGenRenderbuffers;
template <>
inline constexpr auto& glGenObjects<ObjectType::Framebuffer> = glGenFramebuffers;

template <ObjectType>
inline constexpr auto glCreateObject = nullptr;

template <>
inline constexpr auto& glCreateObject<ObjectType::Shader> = glCreateShader;
template <>
inline constexpr auto& glCreateObject<ObjectType::Program> = glCreateProgram;

template <ObjectType>
inline constexpr auto glDeleteObjects = nullptr;

template <>
inline constexpr auto& glDeleteObjects<ObjectType::Buffer> = glDeleteBuffers;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::VertexArray> = glDeleteVertexArrays;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::Query> = glDeleteQueries;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::ProgramPipeline> = glDeleteProgramPipelines;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::TransformFeedback> = glDeleteTransformFeedbacks;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::Sampler> = glDeleteSamplers;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::Texture> = glDeleteTextures;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::Renderbuffer> = glDeleteRenderbuffers;
template <>
inline constexpr auto& glDeleteObjects<ObjectType::Framebuffer> = glDeleteFramebuffers;

template <ObjectType>
inline constexpr auto glDeleteObject = nullptr;
template <>
inline constexpr auto& glDeleteObject<ObjectType::Shader> = glDeleteShader;
template <>
inline constexpr auto& glDeleteObject<ObjectType::Program> = glDeleteProgram;

template <ObjectType>
inline constexpr auto glBindObject = nullptr;

template <>
inline constexpr auto& glBindObject<ObjectType::Buffer> = glBindBuffer;
template <>
inline constexpr auto& glBindObject<ObjectType::Program> = glUseProgram;
template <>
inline constexpr auto& glBindObject<ObjectType::VertexArray> = glBindVertexArray;
template <>
inline constexpr auto& glBindObject<ObjectType::ProgramPipeline> = glBindProgramPipeline;
template <>
inline constexpr auto& glBindObject<ObjectType::TransformFeedback> = glBindTransformFeedback;
template <>
inline constexpr auto& glBindObject<ObjectType::Sampler> = glBindSampler;
template <>
inline constexpr auto& glBindObject<ObjectType::Texture> = glBindTexture;
template <>
inline constexpr auto& glBindObject<ObjectType::Renderbuffer> = glBindRenderbuffer;
template <>
inline constexpr auto& glBindObject<ObjectType::Framebuffer> = glBindFramebuffer;

/// @brief Whether the given function does not exist and is still a nullptr.
template <typename T>
constexpr bool canExecute(const T&)
{
    return !std::is_null_pointer_v<T>;
}

} // namespace detail

} // namespace dang::gl
