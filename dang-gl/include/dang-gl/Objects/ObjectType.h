#pragma once

#include "dang-gl/General/GLConstants.h"

#include "dang-utils/enum.h"

namespace dang::gl {

/// <summary>A list of all OpenGL objects, which can be created, some with multiple targets.</summary>
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

/// <summary>The different buffer targets, which can be specified in glBindBuffer.</summary>
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

/// <summary>The different texture targets, which can be specified in glBindTexture.</summary>
enum class TextureTarget {
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture2DMultisample,
    Texture2DMultisampleArray,
    Texture3D,
    TextureCubeMap,
    TextureRectangle,

    COUNT
};

/// <summary>The different framebuffer targets, which can be specified in glBindFramebuffer.</summary>
/// <remarks>
/// <para>- "Framebuffer" means both draw AND read framebuffers.</para>
/// <para>- If "both" doesn't make sense in the given context, it usually refers to the draw framebuffer.</para>
/// </remarks>
enum class FramebufferTarget {
    Framebuffer,
    DrawFramebuffer,
    ReadFramebuffer,

    COUNT
};

/// <summary>The different renderbuffer targets, which can be specified in glBindRenderbuffer.</summary>
/// <remarks>Currently the only "option" is simply GL_RENDERBUFFER.</remarks>
enum class RenderbufferTarget {
    Renderbuffer,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct EnumCount<dang::gl::ObjectType> : DefaultEnumCount<dang::gl::ObjectType> {};

template <>
struct EnumCount<dang::gl::BufferTarget> : DefaultEnumCount<dang::gl::BufferTarget> {};

template <>
struct EnumCount<dang::gl::TextureTarget> : DefaultEnumCount<dang::gl::TextureTarget> {};

template <>
struct EnumCount<dang::gl::FramebufferTarget> : DefaultEnumCount<dang::gl::FramebufferTarget> {};

template <>
struct EnumCount<dang::gl::RenderbufferTarget> : DefaultEnumCount<dang::gl::RenderbufferTarget> {};

} // namespace dang::utils

namespace dang::gl {

/// <summary>The GL-Constants for object types, which is mainly used to query the currently bound object.</summary>
template <>
inline constexpr dutils::EnumArray<ObjectType, GLenum> GLConstants<ObjectType> = {GL_BUFFER,
                                                                                  GL_SHADER,
                                                                                  GL_PROGRAM,
                                                                                  GL_VERTEX_ARRAY,
                                                                                  GL_QUERY,
                                                                                  GL_PROGRAM_PIPELINE,
                                                                                  GL_TRANSFORM_FEEDBACK,
                                                                                  GL_SAMPLER,
                                                                                  GL_TEXTURE,
                                                                                  GL_RENDERBUFFER,
                                                                                  GL_FRAMEBUFFER};

/// <summary>Maps from buffer targets to their respective constants, which need to be supplied to the glBindBuffer function.</summary>
template <>
inline constexpr dutils::EnumArray<BufferTarget, GLenum> GLConstants<BufferTarget> = {GL_ARRAY_BUFFER,
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
                                                                                      GL_UNIFORM_BUFFER};

/// <summary>Maps from texture targets to their respective constants, which need to be supplied to the glBindTexture function.</summary>
template <>
inline constexpr dutils::EnumArray<TextureTarget, GLenum> GLConstants<TextureTarget> = {GL_TEXTURE_1D,
                                                                                        GL_TEXTURE_1D_ARRAY,
                                                                                        GL_TEXTURE_2D,
                                                                                        GL_TEXTURE_2D_ARRAY,
                                                                                        GL_TEXTURE_2D_MULTISAMPLE,
                                                                                        GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
                                                                                        GL_TEXTURE_3D,
                                                                                        GL_TEXTURE_CUBE_MAP,
                                                                                        GL_TEXTURE_RECTANGLE};

/// <summary>Maps from framebuffer targets to their respective constants, which need to be supplied to the glBindFramebuffer function.</summary>
template <>
inline constexpr dutils::EnumArray<FramebufferTarget, GLenum> GLConstants<FramebufferTarget> = {
    GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER};

/// <summary>Maps from renderbuffer targets to their respective constants, which need to be supplied to the glBindRenderbuffer function.</summary>
template <>
inline constexpr dutils::EnumArray<RenderbufferTarget, GLenum> GLConstants<RenderbufferTarget> = {GL_RENDERBUFFER};

namespace detail {

// Helper to select the matching target enum for a given object type

template <ObjectType>
struct TargetSelector {
    using Type = void;
};

template <>
struct TargetSelector<ObjectType::Buffer> {
    using Type = BufferTarget;
};

template <>
struct TargetSelector<ObjectType::Texture> {
    using Type = TextureTarget;
};

template <>
struct TargetSelector<ObjectType::Renderbuffer> {
    using Type = FramebufferTarget;
};

template <>
struct TargetSelector<ObjectType::Framebuffer> {
    using Type = RenderbufferTarget;
};

} // namespace detail

/// <summary>Maps to the different enums for the various binding targets of the template specified object type.</summary>
/// <remarks>Not all bindable objects support multiple targets.</remarks>
template <ObjectType Type>
using ObjectTarget = typename detail::TargetSelector<Type>::Type;

namespace detail {

// Wraps OpenGL functions in a templated manner

template <ObjectType Type>
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

template <ObjectType Type>
inline constexpr auto glCreateObject = nullptr;

template <>
inline constexpr auto& glCreateObject<ObjectType::Shader> = glCreateShader;
template <>
inline constexpr auto& glCreateObject<ObjectType::Program> = glCreateProgram;

template <ObjectType Type>
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

template <ObjectType Type>
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

/// <summary>Whether the given function does not exist and is still a nullptr.</summary>
template <typename T>
constexpr bool canExecute(const T&)
{
    return !std::is_null_pointer_v<T>;
}

} // namespace detail

} // namespace dang::gl
