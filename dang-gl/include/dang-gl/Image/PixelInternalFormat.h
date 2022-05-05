#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief Formats, for how OpenGL stores its pixel data.
enum class PixelInternalFormat {
    // base formats
    DEPTH_COMPONENT,
    DEPTH_STENCIL,
    RED,
    RG,
    RGB,
    RGBA,

    // sized formats
    R8,
    R8_SNORM,
    R16,
    R16_SNORM,
    RG8,
    RG8_SNORM,
    RG16,
    RG16_SNORM,
    R3_G3_B2,
    RGB4,
    RGB5,
    RGB8,
    RGB8_SNORM,
    RGB10,
    RGB12,
    RGB16_SNORM,
    RGBA2,
    RGBA4,
    RGB5_A1,
    RGBA8,
    RGBA8_SNORM,
    RGB10_A2,
    RGB10_A2UI,
    RGBA12,
    RGBA16,
    SRGB8,
    SRGB8_ALPHA8,
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
    R11F_G11F_B10F,
    RGB9_E5,
    R8I,
    R8UI,
    R16I,
    R16UI,
    R32I,
    R32UI,
    RG8I,
    RG8UI,
    RG16I,
    RG16UI,
    RG32I,
    RG32UI,
    RGB8I,
    RGB8UI,
    RGB16I,
    RGB16UI,
    RGB32I,
    RGB32UI,
    RGBA8I,
    RGBA8UI,
    RGBA16I,
    RGBA16UI,
    RGBA32I,
    RGBA32UI,

    // compressed formats
    COMPRESSED_RED,
    COMPRESSED_RG,
    COMPRESSED_RGB,
    COMPRESSED_RGBA,
    COMPRESSED_SRGB,
    COMPRESSED_SRGB_ALPHA,
    COMPRESSED_RED_RGTC1,
    COMPRESSED_SIGNED_RED_RGTC1,
    COMPRESSED_RG_RGTC2,
    COMPRESSED_SIGNED_RG_RGTC2,
    COMPRESSED_RGBA_BPTC_UNORM,
    COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
    COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
    COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,

    // stencil formats
    STENCIL_INDEX1,
    STENCIL_INDEX4,
    STENCIL_INDEX8,
    STENCIL_INDEX16,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::PixelInternalFormat> : default_enum_count<dang::gl::PixelInternalFormat> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief GL-Constants for internal pixel formats.
template <>
inline constexpr dutils::EnumArray<PixelInternalFormat, GLenum> gl_constants<PixelInternalFormat> = {
    // base formats
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
    GL_RED,
    GL_RG,
    GL_RGB,
    GL_RGBA,

    // sized formats
    GL_R8,
    GL_R8_SNORM,
    GL_R16,
    GL_R16_SNORM,
    GL_RG8,
    GL_RG8_SNORM,
    GL_RG16,
    GL_RG16_SNORM,
    GL_R3_G3_B2,
    GL_RGB4,
    GL_RGB5,
    GL_RGB8,
    GL_RGB8_SNORM,
    GL_RGB10,
    GL_RGB12,
    GL_RGB16_SNORM,
    GL_RGBA2,
    GL_RGBA4,
    GL_RGB5_A1,
    GL_RGBA8,
    GL_RGBA8_SNORM,
    GL_RGB10_A2,
    GL_RGB10_A2UI,
    GL_RGBA12,
    GL_RGBA16,
    GL_SRGB8,
    GL_SRGB8_ALPHA8,
    GL_R16F,
    GL_RG16F,
    GL_RGB16F,
    GL_RGBA16F,
    GL_R32F,
    GL_RG32F,
    GL_RGB32F,
    GL_RGBA32F,
    GL_R11F_G11F_B10F,
    GL_RGB9_E5,
    GL_R8I,
    GL_R8UI,
    GL_R16I,
    GL_R16UI,
    GL_R32I,
    GL_R32UI,
    GL_RG8I,
    GL_RG8UI,
    GL_RG16I,
    GL_RG16UI,
    GL_RG32I,
    GL_RG32UI,
    GL_RGB8I,
    GL_RGB8UI,
    GL_RGB16I,
    GL_RGB16UI,
    GL_RGB32I,
    GL_RGB32UI,
    GL_RGBA8I,
    GL_RGBA8UI,
    GL_RGBA16I,
    GL_RGBA16UI,
    GL_RGBA32I,
    GL_RGBA32UI,

    // compressed formats
    GL_COMPRESSED_RED,
    GL_COMPRESSED_RG,
    GL_COMPRESSED_RGB,
    GL_COMPRESSED_RGBA,
    GL_COMPRESSED_SRGB,
    GL_COMPRESSED_SRGB_ALPHA,
    GL_COMPRESSED_RED_RGTC1,
    GL_COMPRESSED_SIGNED_RED_RGTC1,
    GL_COMPRESSED_RG_RGTC2,
    GL_COMPRESSED_SIGNED_RG_RGTC2,
    GL_COMPRESSED_RGBA_BPTC_UNORM,
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,

    // stencil formats
    GL_STENCIL_INDEX1,
    GL_STENCIL_INDEX4,
    GL_STENCIL_INDEX8,
    GL_STENCIL_INDEX16};

} // namespace dang::gl
