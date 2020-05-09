#pragma once

#include "Image.h"
#include "Object.h"
#include "ObjectBinding.h"
#include "PixelFormat.h"
#include "PixelInternalFormat.h"
#include "PixelType.h"

namespace dang::gl
{

/*

The concept of glActiveTexture and glBindTexture

Quote Khronos.org:
  "Binding textures for use in OpenGL is a little weird."

- There are a set number of texture slots, whose count can be queried using GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
- glActiveTexture sets the current slot to use, using GL_TEXTUREi
- glBindTexture binds a texture to that currently active slot
- Even though different texture types can be bound at the same time...
  -> It is NOT possible to bind different types of textures to the same active texture slot and USE them
  -> The spec explicitly disallows this
  -> Just binding, but not using is technically OK
- The active slot also identifies the texture in a shader sampler

*/

/// <summary>An error related to textures.</summary>
class TextureError : public std::runtime_error {
    using runtime_error::runtime_error;
};

class TextureBase;

// This current implementation is easy to use, but only allows a texture to be bound to a single slot
// Possibly consider modification, to allow a texture to be bound for multiple slots, as the spec does technically allows this
// -> This greatly complicates everything and might not be worth the cost (both run-time and possibly ease-of-use)

/// <summary>The context for all different texture types, as binding closely related all different types.</summary>
class TextureContext : public ObjectContext {
public:
    using ObjectContext::ObjectContext;

    /// <summary>Returns the currently active texture slot.</summary>
    GLint activeSlot();
    /// <summary>Sets the currently active texture slot.</summary>
    void setActiveSlot(GLint slot);

    /// <summary>Binds the texture to the first free slot and returns it or throws a TextureError, if all slots are occupied.</summary>
    GLint bind(const TextureBase& texture);
    /// <summary>If the texture is currently bound to a slot, makes that slot free for another texture to use.</summary>
    void release(const TextureBase& texture);
    /// <summary>Used for texture move semantics, to update the references of bound textures.</summary>
    void move(const TextureBase& from, const TextureBase& to);

private:
    GLint active_slot_;
    std::vector<const TextureBase*> active_textures_{ window().state().max_combined_texture_image_units };
    std::vector<const TextureBase*>::iterator first_free_slot_ = active_textures_.begin();
};

/// <summary>As texture types are closely related, this simply relays to the global texture context.</summary>
class TextureBinding : public ObjectBindingBase {
public:
    /// <summary>Initializes the binding with the given context.</summary>
    TextureBinding(TextureContext& context)
        : context_(context)
    {
    }

    /// <summary>Returns the associated context.</summary>
    TextureContext& context()
    {
        return context_;
    }

    /// <summary>Relays to the context, effectively binding the texture to the first free slot.</summary>
    template <typename TInfo>
    GLint bind(const TextureBase& object)
    {
        return context_.bind(object);
    }

    /// <summary>Relays to the context, effectively freeing up the slot, that the texture is currently occupying.</summary>
    void release(const TextureBase& object)
    {
        context_.release(object);
    }

    /// <summary>Relays to the context, updating a potentially bound texture.</summary>
    template <typename TInfo>
    void move(const TextureBase& from, const TextureBase& to)
    {
        context_.move(from, to);
    }

private:
    TextureContext& context_;
};

/// <summary>Info struct to create and destroy textures.</summary>
struct TextureInfo : public ObjectInfo {
    static GLuint create()
    {
        GLuint handle;
        glGenTextures(1, &handle);
        return handle;
    }

    static void destroy(GLuint handle)
    {
        glDeleteTextures(1, &handle);
    }

    using Context = TextureContext;
    static constexpr ObjectType ObjectType = ObjectType::Texture;

    using Binding = TextureBinding;
};

/// <summary>Serves as a base class for all texture classes, as texture types are closely related in binding.</summary>
class TextureBase : public Object<TextureInfo> {
public:
    friend TextureContext;

    /// <summary>Binds the texture to the first free slot and returns its index or throws a TextureError, if all slots are occupied.</summary>
    GLint bind() const
    {
        return this->context().bind(*this);
    }

    /// <summary>If the texture is currently bound to a slot, makes that slot free for another texture to use.</summary>
    void release() const
    {
        this->context().release(*this);
    }

protected:
    /// <summary>Initializes the texture base with the given texture handle, window and binding point.</summary>
    explicit TextureBase(BindingPoint binding_point)
        : Object()
        , binding_point_(binding_point)
    {
    }

private:
    BindingPoint binding_point_;
    mutable std::optional<GLint> active_slot_;
};

enum class TextureDepthStencilMode {
    DepthComponent,
    StencilIndex,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureDepthStencilMode, GLenum> GLConstants<TextureDepthStencilMode> = {
    GL_DEPTH_COMPONENT,
    GL_STENCIL_INDEX
};

enum class TextureMagFilter {
    Nearest,
    Linear,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureMagFilter, GLenum> GLConstants<TextureMagFilter> = {
    GL_NEAREST,
    GL_LINEAR
};

enum class TextureMinFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureMinFilter, GLenum> GLConstants<TextureMinFilter> = {
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureCompareFunc {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureCompareFunc, GLenum> GLConstants<TextureCompareFunc> = {
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS
};

enum class TextureSwizzle {
    Red,
    Green,
    Blue,
    Alpha,
    Zero,
    One,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureSwizzle, GLenum> GLConstants<TextureSwizzle> = {
    GL_RED,
    GL_GREEN,
    GL_BLUE,
    GL_ALPHA,
    GL_ZERO,
    GL_ONE
};

enum class TextureWrap {
    Repeat,
    ClampToBorder,
    ClampToEdge,
    MirroredRepeat,
    MirrorClampToEdge,

    COUNT
};

template <>
constexpr dutils::EnumArray<TextureWrap, GLenum> GLConstants<TextureWrap> = {
    GL_REPEAT,
    GL_CLAMP_TO_BORDER,
    GL_CLAMP_TO_EDGE,
    GL_MIRRORED_REPEAT,
    GL_MIRROR_CLAMP_TO_EDGE
};

namespace detail
{

template <std::size_t Dim>
constexpr auto glTexStorage = nullptr;

template <> constexpr auto& glTexStorage<1> = glTexStorage1D;
template <> constexpr auto& glTexStorage<2> = glTexStorage2D;
template <> constexpr auto& glTexStorage<3> = glTexStorage3D;

template <std::size_t Dim>
constexpr auto glTexStorageMultisample = nullptr;

template <> constexpr auto& glTexStorageMultisample<2> = glTexStorage2DMultisample;
template <> constexpr auto& glTexStorageMultisample<3> = glTexStorage3DMultisample;

template <std::size_t Dim>
constexpr auto glTexSubImage = nullptr;

template <> constexpr auto& glTexSubImage<1> = glTexSubImage1D;
template <> constexpr auto& glTexSubImage<2> = glTexSubImage2D;
template <> constexpr auto& glTexSubImage<3> = glTexSubImage3D;

/// <summary>A base for all textures with template parameters for the dimension and binding point.</summary>
template <std::size_t Dim, BindingPoint TextureBindingPoint>
class TextureBaseTyped : public TextureBase {
public:
    static constexpr GLenum Target = toGLConstant(TextureBindingPoint);
    template <PixelFormat Format>
    static constexpr PixelInternalFormat DefaultInternal = PixelFormatInfo<Format>::Internal;

    /// <summary>Simply calls the base constructor with the templated binding point.</summary>
    TextureBaseTyped()
        : TextureBase(TextureBindingPoint)
    {
    }

    /// <summary>Modifies a part of the stored texture at the optional given offset and mipmap level.</summary>
    template <std::size_t ImageDim, PixelFormat Format, PixelType Type>
    void modify(
        const Image<ImageDim, Format, Type>& image,
        dmath::svec<Dim> offset = {},
        GLint mipmap_level = 0)
    {
        this->bind();
        subImage(std::make_index_sequence<Dim>(), image, offset, mipmap_level);
    }

    /// <summary>Regenerates all mipmaps from the top level.</summary>
    void generateMipmap()
    {
        this->bind();
        glGenerateMipmap(Target);
    }

    const vec4& borderColor() const
    {
        return border_color_;
    }

    void setBorderColor(const vec4& color)
    {
        if (border_color_ == color)
            return;
        glTexParameterfv(Target, GL_TEXTURE_BORDER_COLOR, &color[0]);
        border_color_ = color;
    }

    TextureDepthStencilMode depthStencilMode() const
    {
        return depth_stencil_mode_;
    }

    void setDepthStencilMode(TextureDepthStencilMode mode)
    {
        if (depth_stencil_mode_ == mode)
            return;
        glTexParameteri(Target, GL_DEPTH_STENCIL_TEXTURE_MODE, toGLConstant(mode));
        depth_stencil_mode_ = mode;
    }

    TextureCompareFunc compareFunc() const
    {
        return compare_func_;
    }

    void setCompareFunc(TextureCompareFunc func)
    {
        if (compare_func_ == func)
            return;
        glTexParameteri(Target, GL_TEXTURE_COMPARE_FUNC, toGLConstant(func));
        compare_func_ = func;
    }

    GLfloat minLevelOfDetail() const
    {
        return min_level_of_detail_;
    }

    void setMinLevelOfDetail(GLfloat level)
    {
        if (min_level_of_detail_ == level)
            return;
        glTexParameterf(Target, GL_TEXTURE_MIN_LOD, level);
        min_level_of_detail_ = level;
    }

    GLfloat maxLevelOfDetail() const
    {
        return max_level_of_detail_;
    }

    void setMaxLevelOfDetail(GLfloat level)
    {
        if (max_level_of_detail_ == level)
            return;
        glTexParameterf(Target, GL_TEXTURE_MAX_LOD, level);
        max_level_of_detail_ = level;
    }

    GLfloat levelOfDetailBias() const
    {
        return level_of_detail_bias_;
    }

    void setLevelOfDetailBias(GLfloat bias)
    {
        if (level_of_detail_bias_ == bias)
            return;
        glTexParameterf(Target, GL_TEXTURE_LOD_BIAS, bias);
        level_of_detail_bias_ = bias;
    }

    TextureMagFilter magFilter() const
    {
        return mag_filter_;
    }

    void setMagFilter(TextureMagFilter mag_filter)
    {
        if (mag_filter_ == mag_filter)
            return;
        glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, toGLConstant(mag_filter));
        mag_filter_ = mag_filter;
    }

    TextureMinFilter minFilter() const
    {
        return min_filter_;
    }

    void setMinFilter(TextureMinFilter min_filter)
    {
        if (min_filter_ == min_filter)
            return;
        glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, toGLConstant(min_filter));
        min_filter_ = min_filter;
    }

    GLint baseLevel() const
    {
        return base_level_;
    }

    void setBaseLevel(GLint base_level)
    {
        if (base_level_ == base_level)
            return;
        glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, base_level);
        base_level_ = base_level;
    }

    GLint maxLevel() const
    {
        return max_level_;
    }

    void setMaxLevel(GLint max_level)
    {
        if (max_level_ == max_level)
            return;
        glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, max_level);
        max_level_ = max_level;
    }

    TextureSwizzle swizzleRed() const
    {
        return swizzle_red_;
    }

    void setSwizzleRed(TextureSwizzle swizzle)
    {
        if (swizzle_red_ == swizzle)
            return;
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, toGLConstant(swizzle));
        swizzle_red_ = swizzle;
    }

    TextureSwizzle swizzleGreen() const
    {
        return swizzle_green_;
    }

    void setSwizzleGreen(TextureSwizzle swizzle)
    {
        if (swizzle_green_ == swizzle)
            return;
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, toGLConstant(swizzle));
        swizzle_green_ = swizzle;
    }

    TextureSwizzle swizzleBlue() const
    {
        return swizzle_blue_;
    }

    void setSwizzleBlue(TextureSwizzle swizzle)
    {
        if (swizzle_blue_ == swizzle)
            return;
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, toGLConstant(swizzle));
        swizzle_blue_ = swizzle;
    }

    TextureSwizzle swizzleAlpha() const
    {
        return swizzle_alpha_;
    }

    void setSwizzleAlpha(TextureSwizzle swizzle)
    {
        if (swizzle_alpha_ == swizzle)
            return;
        glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, toGLConstant(swizzle));
        swizzle_alpha_ = swizzle;
    }

    TextureWrap wrapS() const
    {
        return wrap_s_;
    }

    void setWrapS(TextureWrap wrap)
    {
        if (wrap_s_ == wrap)
            return;
        glTexParameteri(Target, GL_TEXTURE_WRAP_S, toGLConstant(wrap));
        wrap_s_ = wrap;
    }

    TextureWrap wrapT() const
    {
        return wrap_t_;
    }

    void setWrapT(TextureWrap wrap)
    {
        if (wrap_t_ == wrap)
            return;
        glTexParameteri(Target, GL_TEXTURE_WRAP_T, toGLConstant(wrap));
        wrap_t_ = wrap;
    }

    TextureWrap wrapR() const
    {
        return wrap_r_;
    }

    void setWrapR(TextureWrap wrap)
    {
        if (wrap_r_ == wrap)
            return;
        glTexParameteri(Target, GL_TEXTURE_WRAP_R, toGLConstant(wrap));
        wrap_r_ = wrap;
    }

protected:
    /// <summary>Calls glTexSubImage with the provided parameters and index sequence of the textures dimension.</summary>
    template <std::size_t ImageDim, PixelFormat Format, PixelType Type, std::size_t... Indices>
    void subImage(
        std::index_sequence<Indices...>,
        const Image<ImageDim, Format, Type>& image,
        dmath::svec<Dim> offset = {},
        GLint mipmap_level = 0)
    {
        glTexSubImage<Dim>(
            Target,
            mipmap_level,
            static_cast<GLint>(offset[Indices])...,
            static_cast<GLsizei>(Indices < image.size().size() ? image.size()[Indices] : 1)...,
            toGLConstant(Format),
            toGLConstant(Type),
            image.data());
    }

private:
    vec4 border_color_;

    TextureDepthStencilMode depth_stencil_mode_ = TextureDepthStencilMode::DepthComponent;
    TextureCompareFunc compare_func_ = TextureCompareFunc::LessEqual;

    GLfloat min_level_of_detail_ = -1000.0f;
    GLfloat max_level_of_detail_ = 1000.0f;
    GLfloat level_of_detail_bias_ = 0.0f;

    TextureMagFilter mag_filter_ = TextureMagFilter::Linear;
    TextureMinFilter min_filter_ = TextureMinFilter::NearestMipmapLinear;

    GLint base_level_ = 0;
    GLint max_level_ = 1000;

    TextureSwizzle swizzle_red_ = TextureSwizzle::Red;
    TextureSwizzle swizzle_green_ = TextureSwizzle::Green;
    TextureSwizzle swizzle_blue_ = TextureSwizzle::Blue;
    TextureSwizzle swizzle_alpha_ = TextureSwizzle::Alpha;

    TextureWrap wrap_s_ = TextureWrap::Repeat;
    TextureWrap wrap_t_ = TextureWrap::Repeat;
    TextureWrap wrap_r_ = TextureWrap::Repeat;
};

/// <summary>Base class for all regluar, non-multisampled textures.</summary>
template <std::size_t Dim, BindingPoint TextureBindingPoint>
class TextureBaseRegular : public TextureBaseTyped<Dim, TextureBindingPoint> {
public:
    /// <summary>Creates an empty texture.</summary>
    TextureBaseRegular() = default;

    /// <summary>Initializes a new texture with the given size, optional mipmap level count and internal format.</summary>
    /// <param name="mipmap_levels">Defaults to generating a full mipmap down to 1x1.</param>
    explicit TextureBaseRegular(
        dmath::svec<Dim> size,
        std::optional<GLsizei> mipmap_levels = std::nullopt,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
        : TextureBaseRegular()
    {
        generate(size, mipmap_levels, internal_format);
    }

    /// <summary>Initializes a new texture with the given image data, optional mipmap level count and internal format.</summary>
    /// <param name="mipmap_levels">Defaults to generating a full mipmap down to 1x1.</param>
    /// <param name="internal_format">Defaults to being chosen, based on the format of the provided image.</param>
    template <PixelFormat Format, PixelType Type>
    explicit TextureBaseRegular(
        const Image<Dim, Format, Type>& image,
        std::optional<GLsizei> mipmap_levels = std::nullopt,
        PixelInternalFormat internal_format = PixelFormatInfo<Format>::Internal)
        : TextureBaseRegular()
    {
        generate(image, mipmap_levels, internal_format);
    }

    /// <summary>Generates storage for the specified size with optional mipmap level count and internal format.</summary>
    /// <param name="mipmap_levels">Defaults to generating a full mipmap down to 1x1.</param>
    void generate(
        dmath::svec<Dim> size,
        std::optional<GLsizei> mipmap_levels = std::nullopt,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        this->bind();
        storage(std::make_index_sequence<Dim>(), size, mipmap_levels, internal_format);
    }

    /// <summary>Generates texture storage and fills it with the provided image.</summary>
    /// <param name="mipmap_levels">Defaults to generating a full mipmap down to 1x1.</param>
    /// <param name="internal_format">Defaults to being chosen, based on the format of the provided image.</param>
    template <PixelFormat Format, PixelType Type>
    void generate(
        const Image<Dim, Format, Type>& image,
        std::optional<GLsizei> mipmap_levels = std::nullopt,
        PixelInternalFormat internal_format = PixelFormatInfo<Format>::Internal)
    {
        this->bind();
        storage(std::make_index_sequence<Dim>(), image.size(), mipmap_levels, internal_format);
        this->subImage(std::make_index_sequence<Dim>(), image);
        glGenerateMipmap(TextureBaseTyped<Dim, TextureBindingPoint>::Target);
    }

private:
    /// <summary>Returns the biggest component of a given vector.</summary>
    template <std::size_t... Indices>
    std::size_t maxSize(dmath::svec<Dim> size, std::index_sequence<Indices...>)
    {
        std::size_t result = 0;
        ((result = std::max(result, size[Indices])), ...);
        return result;
    }

    /// <summary>Calculates the integer log2 of the given value, which is the required mipmap count for a given size.</summary>
    std::size_t ilog2PlusOne(std::size_t value)
    {
        // TODO: Use std::bit_width in C++20
        std::size_t result = 1;
        while (value >>= 1)
            result++;
        return result;
    }

    /// <summary>Returns the required count to generate a full mipmap down to 1x1 for the given size.</summary>
    GLsizei maxMipmapLevelsFor(dmath::svec<Dim> size)
    {
        return static_cast<GLsizei>(ilog2PlusOne(maxSize(size, std::make_index_sequence<Dim>())));
    }

    /// <summary>Calls glTexStorage with the provided parameters and index sequence of the textures dimension.</summary>
    template <std::size_t... Indices>
    void storage(
        std::index_sequence<Indices...>,
        dmath::svec<Dim> size,
        std::optional<GLsizei> mipmap_levels = std::nullopt,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        GLsizei actual_mipmap_levels = mipmap_levels.value_or(maxMipmapLevelsFor(size));
        glTexStorage<Dim>(
            TextureBaseTyped<Dim, TextureBindingPoint>::Target,
            actual_mipmap_levels,
            toGLConstant(internal_format),
            static_cast<GLsizei>(size[Indices])...);
    }
};

/// <summary>Base class for all multisampled textures.</summary>
template <std::size_t Dim, BindingPoint TextureBindingPoint>
class TextureBaseMultisample : public TextureBaseTyped<Dim, TextureBindingPoint> {
public:
    /// <summary>Creates an empty multisampled texture.</summary>
    TextureBaseMultisample() = default;

    /// <summary>Initializes a new multisampled texture with the given size, sample count and optional internal format.</summary>
    TextureBaseMultisample(
        dmath::svec<Dim> size,
        GLsizei samples,
        bool fixed_sample_locations = true,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
        : TextureBaseMultisample()
    {
        generate(size, samples, internal_format);
    }

    /// <summary>Initializes a new multisampled texture with the given image data, sample count and optional internal format.</summary>
    /// <param name="internal_format">Defaults to being chosen, based on the format of the provided image.</param>
    template <PixelFormat Format, PixelType Type>
    explicit TextureBaseMultisample(
        const Image<Dim, Format, Type>& image,
        GLsizei samples,
        bool fixed_sample_locations = true,
        PixelInternalFormat internal_format = PixelFormatInfo<Format>::Internal)
        : TextureBaseMultisample()
    {
        generate(image, samples, fixed_sample_locations, internal_format);
    }

    /// <summary>Generates storage for the specified size, samples and optional internal format.</summary>
    void generate(
        dmath::svec<Dim> size,
        GLsizei samples,
        bool fixed_sample_locations = true,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        this->bind();
        storageMultisample(std::make_index_sequence<Dim>(), size, samples, fixed_sample_locations, internal_format);
    }

    /// <summary>Generates texture storage and fills it with the provided image.</summary>
    /// <param name="internal_format">Defaults to being chosen, based on the format of the provided image.</param>
    template <PixelFormat Format, PixelType Type>
    void generate(
        const Image<Dim, Format, Type>& image,
        GLint samples,
        bool fixed_sample_locations = true,
        PixelInternalFormat internal_format = PixelFormatInfo<Format>::Internal)
    {
        this->bind();
        storageMultisample(std::make_index_sequence<Dim>(), image.size(), samples, fixed_sample_locations, internal_format);
        this->texSubImage(std::make_index_sequence<Dim>(), image);
    }

private:
    /// <summary>Calls glTexStorageMultisample with the provided parameters and index sequence of the textures dimension.</summary>
    template <std::size_t... Indices>
    void storageMultisample(
        std::index_sequence<Indices...>,
        dmath::svec<Dim> size,
        GLsizei samples,
        bool fixed_sample_locations = true,
        PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        glTexStorageMultisample<Dim>(
            TextureBaseTyped<Dim, TextureBindingPoint>::Target,
            samples,
            toGLConstant(internal_format),
            static_cast<GLsizei>(size[Indices])...,
            static_cast<GLboolean>(fixed_sample_locations));
    }
};

}

class Texture1D : public detail::TextureBaseRegular<1, BindingPoint::Texture1D> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class Texture1DArray : public detail::TextureBaseRegular<2, BindingPoint::Texture1DArray> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class Texture2D : public detail::TextureBaseRegular<2, BindingPoint::Texture2D> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class Texture2DArray : public detail::TextureBaseRegular<3, BindingPoint::Texture2DArray> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class Texture2DMultisample : public detail::TextureBaseMultisample<2, BindingPoint::Texture2DMultisample> {
public:
    using TextureBaseMultisample::TextureBaseMultisample;
};

class Texture2DMultisampleArray : public detail::TextureBaseMultisample<3, BindingPoint::Texture2DMultisampleArray> {
public:
    using TextureBaseMultisample::TextureBaseMultisample;
};

class Texture3D : public detail::TextureBaseRegular<3, BindingPoint::Texture3D> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class TextureCubeMap : public detail::TextureBaseRegular<2, BindingPoint::TextureCubeMap> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

class TextureRectangle : public detail::TextureBaseRegular<2, BindingPoint::TextureRectangle> {
public:
    using TextureBaseRegular::TextureBaseRegular;
};

}
