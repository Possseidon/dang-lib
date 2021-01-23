#pragma once

#include "dang-gl/Image/Image.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelInternalFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/Objects/TextureContext.h"
#include "dang-gl/global.h"

#include "dang-math/vector.h"

#include "dang-utils/enum.h"

namespace dang::gl {

enum class TextureDepthStencilMode {
    DepthComponent,
    StencilIndex,

    COUNT
};

enum class TextureMagFilter {
    Nearest,
    Linear,

    COUNT
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

enum class TextureSwizzle {
    Red,
    Green,
    Blue,
    Alpha,
    Zero,
    One,

    COUNT
};

enum class TextureWrap {
    Repeat,
    ClampToBorder,
    ClampToEdge,
    MirroredRepeat,
    MirrorClampToEdge,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::TextureDepthStencilMode> : default_enum_count<dang::gl::TextureDepthStencilMode> {};

template <>
struct enum_count<dang::gl::TextureMagFilter> : default_enum_count<dang::gl::TextureMagFilter> {};

template <>
struct enum_count<dang::gl::TextureMinFilter> : default_enum_count<dang::gl::TextureMinFilter> {};

template <>
struct enum_count<dang::gl::TextureCompareFunc> : default_enum_count<dang::gl::TextureCompareFunc> {};

template <>
struct enum_count<dang::gl::TextureSwizzle> : default_enum_count<dang::gl::TextureSwizzle> {};

template <>
struct enum_count<dang::gl::TextureWrap> : default_enum_count<dang::gl::TextureWrap> {};

} // namespace dang::utils

namespace dang::gl {

template <>
inline constexpr dutils::EnumArray<TextureDepthStencilMode, GLenum> GLConstants<TextureDepthStencilMode> = {
    GL_DEPTH_COMPONENT, GL_STENCIL_INDEX};

template <>
inline constexpr dutils::EnumArray<TextureMagFilter, GLenum> GLConstants<TextureMagFilter> = {GL_NEAREST, GL_LINEAR};

template <>
inline constexpr dutils::EnumArray<TextureMinFilter, GLenum> GLConstants<TextureMinFilter> = {GL_NEAREST,
                                                                                              GL_LINEAR,
                                                                                              GL_NEAREST_MIPMAP_NEAREST,
                                                                                              GL_LINEAR_MIPMAP_NEAREST,
                                                                                              GL_NEAREST_MIPMAP_LINEAR,
                                                                                              GL_LINEAR_MIPMAP_LINEAR};

template <>
inline constexpr dutils::EnumArray<TextureCompareFunc, GLenum> GLConstants<TextureCompareFunc> = {
    GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};

template <>
inline constexpr dutils::EnumArray<TextureSwizzle, GLenum> GLConstants<TextureSwizzle> = {
    GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE};

template <>
inline constexpr dutils::EnumArray<TextureWrap, GLenum> GLConstants<TextureWrap> = {
    GL_REPEAT, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE};

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

/// @brief Serves as a base class for all texture classes.
class TextureBase : public Object<ObjectType::Texture> {
public:
    /// @brief Resets the bound texture of the context, in case of the texture still being bound.
    ~TextureBase()
    {
        if (*this)
            release();
    }

    TextureBase(const TextureBase&) = delete;
    TextureBase& operator=(const TextureBase&) = delete;

    /// @brief Binds the texture to the first free slot and returns its index or throws a TextureError, if all slots are
    /// occupied.
    std::size_t bind() const
    {
        auto slot = this->objectContext().bind(target_, handle(), active_slot_);
        active_slot_ = slot;
        return slot;
    }

    /// @brief If the texture is currently bound to a slot, makes that slot free for another texture to use.
    void release() const
    {
        this->objectContext().release(target_, active_slot_);
        active_slot_ = {};
    }

protected:
    /// @brief Initializes the texture base with the given texture handle, window and binding target.
    explicit TextureBase(TextureTarget target)
        : Object()
        , target_(target)
    {}

    TextureBase(TextureBase&&) = default;
    TextureBase& operator=(TextureBase&&) = default;

private:
    TextureTarget target_;
    mutable std::optional<std::size_t> active_slot_;
};

namespace detail {

template <std::size_t v_dim>
inline constexpr auto glTexStorage = nullptr;

template <>
inline constexpr auto& glTexStorage<1> = glTexStorage1D;
template <>
inline constexpr auto& glTexStorage<2> = glTexStorage2D;
template <>
inline constexpr auto& glTexStorage<3> = glTexStorage3D;

template <std::size_t v_dim>
inline constexpr auto glTexStorageMultisample = nullptr;

template <>
inline constexpr auto& glTexStorageMultisample<2> = glTexStorage2DMultisample;
template <>
inline constexpr auto& glTexStorageMultisample<3> = glTexStorage3DMultisample;

template <std::size_t v_dim>
inline constexpr auto glTexSubImage = nullptr;

template <>
inline constexpr auto& glTexSubImage<1> = glTexSubImage1D;
template <>
inline constexpr auto& glTexSubImage<2> = glTexSubImage2D;
template <>
inline constexpr auto& glTexSubImage<3> = glTexSubImage3D;

/// @brief A base for all textures with template parameters for the dimension and texture target.
template <std::size_t v_dim, TextureTarget v_target>
class TextureBaseTyped : public TextureBase {
public:
    template <PixelFormat v_format>
    static constexpr PixelInternalFormat DefaultInternal = PixelFormatInfo<v_format>::Internal;

    ~TextureBaseTyped() = default;

    TextureBaseTyped(const TextureBaseTyped&) = delete;
    TextureBaseTyped& operator=(const TextureBaseTyped&) = delete;

    /// @brief Returns the size of the image along each axis.
    dmath::svec<v_dim> size() const { return size_; }

    /// @brief Modifies a part of the stored texture at the optional given offset and mipmap level.
    template <std::size_t v_image_dim, PixelFormat v_format, PixelType v_type>
    void modify(const Image<v_image_dim, v_format, v_type>& image,
                dmath::svec<v_dim> offset = {},
                GLint mipmap_level = 0)
    {
        this->bind();
        subImage(std::make_index_sequence<v_dim>(), image, offset, mipmap_level);
    }

    /// @brief Regenerates all mipmaps from the top level.
    void generateMipmap()
    {
        this->bind();
        glGenerateMipmap(toGLConstant(v_target));
    }

    const vec4& borderColor() const { return border_color_; }

    void setBorderColor(const vec4& color)
    {
        if (border_color_ == color)
            return;
        glTexParameterfv(toGLConstant(v_target), GL_TEXTURE_BORDER_COLOR, &color[0]);
        border_color_ = color;
    }

    TextureDepthStencilMode depthStencilMode() const { return depth_stencil_mode_; }

    void setDepthStencilMode(TextureDepthStencilMode mode)
    {
        if (depth_stencil_mode_ == mode)
            return;
        glTexParameteri(toGLConstant(v_target), GL_DEPTH_STENCIL_TEXTURE_MODE, toGLConstant(mode));
        depth_stencil_mode_ = mode;
    }

    TextureCompareFunc compareFunc() const { return compare_func_; }

    void setCompareFunc(TextureCompareFunc func)
    {
        if (compare_func_ == func)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_COMPARE_FUNC, toGLConstant(func));
        compare_func_ = func;
    }

    GLfloat minLevelOfDetail() const { return min_level_of_detail_; }

    void setMinLevelOfDetail(GLfloat level)
    {
        if (min_level_of_detail_ == level)
            return;
        glTexParameterf(toGLConstant(v_target), GL_TEXTURE_MIN_LOD, level);
        min_level_of_detail_ = level;
    }

    GLfloat maxLevelOfDetail() const { return max_level_of_detail_; }

    void setMaxLevelOfDetail(GLfloat level)
    {
        if (max_level_of_detail_ == level)
            return;
        glTexParameterf(toGLConstant(v_target), GL_TEXTURE_MAX_LOD, level);
        max_level_of_detail_ = level;
    }

    GLfloat levelOfDetailBias() const { return level_of_detail_bias_; }

    void setLevelOfDetailBias(GLfloat bias)
    {
        if (level_of_detail_bias_ == bias)
            return;
        glTexParameterf(toGLConstant(v_target), GL_TEXTURE_LOD_BIAS, bias);
        level_of_detail_bias_ = bias;
    }

    TextureMagFilter magFilter() const { return mag_filter_; }

    void setMagFilter(TextureMagFilter mag_filter)
    {
        if (mag_filter_ == mag_filter)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_MAG_FILTER, toGLConstant(mag_filter));
        mag_filter_ = mag_filter;
    }

    TextureMinFilter minFilter() const { return min_filter_; }

    void setMinFilter(TextureMinFilter min_filter)
    {
        if (min_filter_ == min_filter)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_MIN_FILTER, toGLConstant(min_filter));
        min_filter_ = min_filter;
    }

    GLint baseLevel() const { return base_level_; }

    void setBaseLevel(GLint base_level)
    {
        if (base_level_ == base_level)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_BASE_LEVEL, base_level);
        base_level_ = base_level;
    }

    GLint maxLevel() const { return max_level_; }

    void setMaxLevel(GLint max_level)
    {
        if (max_level_ == max_level)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_MAX_LEVEL, max_level);
        max_level_ = max_level;
    }

    TextureSwizzle swizzleRed() const { return swizzle_red_; }

    void setSwizzleRed(TextureSwizzle swizzle)
    {
        if (swizzle_red_ == swizzle)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_SWIZZLE_R, toGLConstant(swizzle));
        swizzle_red_ = swizzle;
    }

    TextureSwizzle swizzleGreen() const { return swizzle_green_; }

    void setSwizzleGreen(TextureSwizzle swizzle)
    {
        if (swizzle_green_ == swizzle)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_SWIZZLE_G, toGLConstant(swizzle));
        swizzle_green_ = swizzle;
    }

    TextureSwizzle swizzleBlue() const { return swizzle_blue_; }

    void setSwizzleBlue(TextureSwizzle swizzle)
    {
        if (swizzle_blue_ == swizzle)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_SWIZZLE_B, toGLConstant(swizzle));
        swizzle_blue_ = swizzle;
    }

    TextureSwizzle swizzleAlpha() const { return swizzle_alpha_; }

    void setSwizzleAlpha(TextureSwizzle swizzle)
    {
        if (swizzle_alpha_ == swizzle)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_SWIZZLE_A, toGLConstant(swizzle));
        swizzle_alpha_ = swizzle;
    }

    TextureWrap wrapS() const { return wrap_s_; }

    void setWrapS(TextureWrap wrap)
    {
        if (wrap_s_ == wrap)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_WRAP_S, toGLConstant(wrap));
        wrap_s_ = wrap;
    }

    TextureWrap wrapT() const { return wrap_t_; }

    void setWrapT(TextureWrap wrap)
    {
        if (wrap_t_ == wrap)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_WRAP_T, toGLConstant(wrap));
        wrap_t_ = wrap;
    }

    TextureWrap wrapR() const { return wrap_r_; }

    void setWrapR(TextureWrap wrap)
    {
        if (wrap_r_ == wrap)
            return;
        glTexParameteri(toGLConstant(v_target), GL_TEXTURE_WRAP_R, toGLConstant(wrap));
        wrap_r_ = wrap;
    }

protected:
    /// @brief Simply calls the base constructor with the templated texture target.
    TextureBaseTyped()
        : TextureBase(v_target)
    {}

    TextureBaseTyped(TextureBaseTyped&&) = default;
    TextureBaseTyped& operator=(TextureBaseTyped&&) = default;

    /// @brief Sets the internal size to the given value.
    void setSize(dmath::svec<v_dim> size) { size_ = size; }

    /// @brief Calls glTexSubImage with the provided parameters and index sequence of the textures dimension.
    template <std::size_t v_image_dim, PixelFormat v_format, PixelType v_type, std::size_t... v_indices>
    void subImage(std::index_sequence<v_indices...>,
                  const Image<v_image_dim, v_format, v_type>& image,
                  dmath::svec<v_dim> offset = {},
                  GLint mipmap_level = 0)
    {
        glTexSubImage<v_dim>(toGLConstant(v_target),
                             mipmap_level,
                             static_cast<GLint>(offset[v_indices])...,
                             static_cast<GLsizei>(v_indices < image.size().size() ? image.size()[v_indices] : 1)...,
                             toGLConstant(v_format),
                             toGLConstant(v_type),
                             image.data());
    }

private:
    dmath::svec<v_dim> size_;

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

/// @brief Base class for all regluar, non-multisampled textures.
template <std::size_t v_dim, TextureTarget v_target>
class TextureBaseRegular : public TextureBaseTyped<v_dim, v_target> {
public:
    /// @brief Creates an empty texture.
    TextureBaseRegular() = default;

    /// @brief Initializes a new texture with the given size, optional mipmap level count and internal format.
    /// @param mipmap_levels Defaults to generating a full mipmap down to 1x1.
    explicit TextureBaseRegular(dmath::svec<v_dim> size,
                                std::optional<GLsizei> mipmap_levels = std::nullopt,
                                PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
        : TextureBaseRegular()
    {
        generate(size, mipmap_levels, internal_format);
    }

    /// @brief Initializes a new texture with the given image data, optional mipmap level count and internal format.
    /// @param mipmap_levels Defaults to generating a full mipmap down to 1x1.
    /// @param internal_format Defaults to being chosen, based on the format of the provided image.
    template <PixelFormat v_format, PixelType v_type>
    explicit TextureBaseRegular(const Image<v_dim, v_format, v_type>& image,
                                std::optional<GLsizei> mipmap_levels = std::nullopt,
                                PixelInternalFormat internal_format = PixelFormatInfo<v_format>::Internal)
        : TextureBaseRegular()
    {
        generate(image, mipmap_levels, internal_format);
    }

    ~TextureBaseRegular() = default;

    TextureBaseRegular(const TextureBaseRegular&) = delete;
    TextureBaseRegular& operator=(const TextureBaseRegular&) = delete;

    /// @brief Generates storage for the specified size with optional mipmap level count and internal format.
    /// @param mipmap_levels Defaults to generating a full mipmap down to 1x1.
    void generate(dmath::svec<v_dim> size,
                  std::optional<GLsizei> mipmap_levels = std::nullopt,
                  PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        this->bind();
        storage(std::make_index_sequence<v_dim>(), size, mipmap_levels, internal_format);
    }

    /// @brief Generates texture storage and fills it with the provided image.
    /// @param mipmap_levels Defaults to generating a full mipmap down to 1x1.
    /// @param internal_format Defaults to being chosen, based on the format of the provided image.
    template <PixelFormat v_format, PixelType v_type>
    void generate(const Image<v_dim, v_format, v_type>& image,
                  std::optional<GLsizei> mipmap_levels = std::nullopt,
                  PixelInternalFormat internal_format = PixelFormatInfo<v_format>::Internal)
    {
        this->bind();
        storage(std::make_index_sequence<v_dim>(), image.size(), mipmap_levels, internal_format);
        this->subImage(std::make_index_sequence<v_dim>(), image);
        glGenerateMipmap(toGLConstant(v_target));
    }

protected:
    TextureBaseRegular(TextureBaseRegular&&) = default;
    TextureBaseRegular& operator=(TextureBaseRegular&&) = default;

private:
    /// @brief Returns the biggest component of a given vector.
    template <std::size_t... v_indices>
    static std::size_t maxSize(dmath::svec<v_dim> size, std::index_sequence<v_indices...>)
    {
        std::size_t result = 0;
        ((result = std::max(result, size[v_indices])), ...);
        return result;
    }

    /// @brief Calculates the integer log2 plus one of the given value, which is the required mipmap count for a given
    /// size.
    static std::size_t mipmapCount(std::size_t value)
    {
        // TODO: Use std::bit_width in C++20
        std::size_t result = 1;
        while (value >>= 1)
            result++;
        return result;
    }

    /// @brief Returns the required count to generate a full mipmap down to 1x1 for the given size.
    GLsizei maxMipmapLevelsFor(dmath::svec<v_dim> size)
    {
        return static_cast<GLsizei>(mipmapCount(maxSize(size, std::make_index_sequence<v_dim>())));
    }

    /// @brief Calls glTexStorage with the provided parameters and index sequence of the textures dimension.
    template <std::size_t... v_indices>
    void storage(std::index_sequence<v_indices...>,
                 dmath::svec<v_dim> size,
                 std::optional<GLsizei> mipmap_levels = std::nullopt,
                 PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        glTexStorage<v_dim>(toGLConstant(v_target),
                            mipmap_levels.value_or(maxMipmapLevelsFor(size)),
                            toGLConstant(internal_format),
                            static_cast<GLsizei>(size[v_indices])...);
        this->setSize(size);
    }
};

/// @brief Base class for all multisampled textures.
template <std::size_t v_dim, TextureTarget v_target>
class TextureBaseMultisample : public TextureBaseTyped<v_dim, v_target> {
public:
    /// @brief Creates an empty multisampled texture.
    TextureBaseMultisample() = default;

    /// @brief Initializes a new multisampled texture with the given size, sample count and optional internal format.
    TextureBaseMultisample(dmath::svec<v_dim> size,
                           GLsizei samples,
                           bool fixed_sample_locations = true,
                           PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
        : TextureBaseMultisample()
    {
        generate(size, samples, internal_format);
    }

    /// @brief Initializes a new multisampled texture with the given image data, sample count and optional internal
    /// format.
    /// @param internal_format Defaults to being chosen, based on the format of the provided image.
    template <PixelFormat v_format, PixelType v_type>
    explicit TextureBaseMultisample(const Image<v_dim, v_format, v_type>& image,
                                    GLsizei samples,
                                    bool fixed_sample_locations = true,
                                    PixelInternalFormat internal_format = PixelFormatInfo<v_format>::Internal)
        : TextureBaseMultisample()
    {
        generate(image, samples, fixed_sample_locations, internal_format);
    }

    ~TextureBaseMultisample() = default;

    TextureBaseMultisample(const TextureBaseMultisample&) = delete;
    TextureBaseMultisample& operator=(const TextureBaseMultisample&) = delete;

    /// @brief Generates storage for the specified size, samples and optional internal format.
    void generate(dmath::svec<v_dim> size,
                  GLsizei samples,
                  bool fixed_sample_locations = true,
                  PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        this->bind();
        storageMultisample(std::make_index_sequence<v_dim>(), size, samples, fixed_sample_locations, internal_format);
    }

    /// @brief Generates texture storage and fills it with the provided image.
    /// @param internal_format Defaults to being chosen, based on the format of the provided image.
    template <PixelFormat v_format, PixelType v_type>
    void generate(const Image<v_dim, v_format, v_type>& image,
                  GLint samples,
                  bool fixed_sample_locations = true,
                  PixelInternalFormat internal_format = PixelFormatInfo<v_format>::Internal)
    {
        this->bind();
        storageMultisample(
            std::make_index_sequence<v_dim>(), image.size(), samples, fixed_sample_locations, internal_format);
        this->texSubImage(std::make_index_sequence<v_dim>(), image);
    }

protected:
    TextureBaseMultisample(TextureBaseMultisample&&) = default;
    TextureBaseMultisample& operator=(TextureBaseMultisample&&) = default;

private:
    /// @brief Calls glTexStorageMultisample with the provided parameters and index sequence of the textures dimension.
    template <std::size_t... v_indices>
    void storageMultisample(std::index_sequence<v_indices...>,
                            dmath::svec<v_dim> size,
                            GLsizei samples,
                            bool fixed_sample_locations = true,
                            PixelInternalFormat internal_format = PixelInternalFormat::RGBA8)
    {
        glTexStorageMultisample<v_dim>(toGLConstant(v_target),
                                       samples,
                                       toGLConstant(internal_format),
                                       static_cast<GLsizei>(size[v_indices])...,
                                       static_cast<GLboolean>(fixed_sample_locations));
        this->setSize(size);
    }
};

} // namespace detail

class Texture1D : public detail::TextureBaseRegular<1, TextureTarget::Texture1D> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    Texture1D(const Texture1D&) = delete;
    Texture1D(Texture1D&&) = default;
    Texture1D& operator=(const Texture1D&) = delete;
    Texture1D& operator=(Texture1D&&) = default;
};

class Texture1DArray : public detail::TextureBaseRegular<2, TextureTarget::Texture1DArray> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    Texture1DArray(const Texture1DArray&) = delete;
    Texture1DArray(Texture1DArray&&) = default;
    Texture1DArray& operator=(const Texture1DArray&) = delete;
    Texture1DArray& operator=(Texture1DArray&&) = default;
};

class Texture2D : public detail::TextureBaseRegular<2, TextureTarget::Texture2D> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    Texture2D(const Texture2D&) = delete;
    Texture2D(Texture2D&&) = default;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D& operator=(Texture2D&&) = default;
};

class Texture2DArray : public detail::TextureBaseRegular<3, TextureTarget::Texture2DArray> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    Texture2DArray(const Texture2DArray&) = delete;
    Texture2DArray(Texture2DArray&&) = default;
    Texture2DArray& operator=(const Texture2DArray&) = delete;
    Texture2DArray& operator=(Texture2DArray&&) = default;
};

class Texture2DMultisample : public detail::TextureBaseMultisample<2, TextureTarget::Texture2DMultisample> {
public:
    using TextureBaseMultisample::TextureBaseMultisample;

    Texture2DMultisample(const Texture2DMultisample&) = delete;
    Texture2DMultisample(Texture2DMultisample&&) = default;
    Texture2DMultisample& operator=(const Texture2DMultisample&) = delete;
    Texture2DMultisample& operator=(Texture2DMultisample&&) = default;
};

class Texture2DMultisampleArray : public detail::TextureBaseMultisample<3, TextureTarget::Texture2DMultisampleArray> {
public:
    using TextureBaseMultisample::TextureBaseMultisample;

    Texture2DMultisampleArray(const Texture2DMultisampleArray&) = delete;
    Texture2DMultisampleArray(Texture2DMultisampleArray&&) = default;
    Texture2DMultisampleArray& operator=(const Texture2DMultisampleArray&) = delete;
    Texture2DMultisampleArray& operator=(Texture2DMultisampleArray&&) = default;
};

class Texture3D : public detail::TextureBaseRegular<3, TextureTarget::Texture3D> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    Texture3D(const Texture3D&) = delete;
    Texture3D(Texture3D&&) = default;
    Texture3D& operator=(const Texture3D&) = delete;
    Texture3D& operator=(Texture3D&&) = default;
};

class TextureCubeMap : public detail::TextureBaseRegular<2, TextureTarget::TextureCubeMap> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    TextureCubeMap(const TextureCubeMap&) = delete;
    TextureCubeMap(TextureCubeMap&&) = default;
    TextureCubeMap& operator=(const TextureCubeMap&) = delete;
    TextureCubeMap& operator=(TextureCubeMap&&) = default;
};

class TextureRectangle : public detail::TextureBaseRegular<2, TextureTarget::TextureRectangle> {
public:
    using TextureBaseRegular::TextureBaseRegular;

    TextureRectangle(const TextureRectangle&) = delete;
    TextureRectangle(TextureRectangle&&) = default;
    TextureRectangle& operator=(const TextureRectangle&) = delete;
    TextureRectangle& operator=(TextureRectangle&&) = default;
};

} // namespace dang::gl
