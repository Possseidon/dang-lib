#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/Image/PixelInternalFormat.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"
#include "dang-utils/utils.h"

namespace dang::gl {

/// @brief Specifies, which components make up a pixel, containing red, green, blue, alpha, stencil and depth.
enum class PixelFormat {
    RED,
    RG,
    RGB,
    BGR,
    RGBA,
    BGRA,

    RED_INTEGER,
    RG_INTEGER,
    RGB_INTEGER,
    BGR_INTEGER,
    RGBA_INTEGER,
    BGRA_INTEGER,

    STENCIL_INDEX,
    DEPTH_COMPONENT,
    DEPTH_STENCIL,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::PixelFormat> : default_enum_count<dang::gl::PixelFormat> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief The GL-Constants for the pixel formats.
template <>
inline constexpr dutils::EnumArray<PixelFormat, GLenum> gl_constants<PixelFormat> = {GL_RED,
                                                                                     GL_RG,
                                                                                     GL_RGB,
                                                                                     GL_BGR,
                                                                                     GL_RGBA,
                                                                                     GL_BGRA,

                                                                                     GL_RED_INTEGER,
                                                                                     GL_RG_INTEGER,
                                                                                     GL_RGB_INTEGER,
                                                                                     GL_BGR_INTEGER,
                                                                                     GL_RGBA_INTEGER,
                                                                                     GL_BGRA_INTEGER,

                                                                                     GL_STENCIL_INDEX,
                                                                                     GL_DEPTH_COMPONENT,
                                                                                     GL_DEPTH_STENCIL};

/// @brief Provides the internal format to use for a given pixel format.
template <PixelFormat>
struct pixel_format_internal {};

template <PixelFormat v_format>
inline constexpr auto pixel_format_internal_v = pixel_format_internal<v_format>::value;

template <>
struct pixel_format_internal<PixelFormat::RED> : dutils::constant<PixelInternalFormat::R8> {};

template <>
struct pixel_format_internal<PixelFormat::RG> : dutils::constant<PixelInternalFormat::RG8> {};

template <>
struct pixel_format_internal<PixelFormat::RGB> : dutils::constant<PixelInternalFormat::RGB8> {};

template <>
struct pixel_format_internal<PixelFormat::BGR> : dutils::constant<PixelInternalFormat::RGB8> {};

template <>
struct pixel_format_internal<PixelFormat::RGBA> : dutils::constant<PixelInternalFormat::RGBA8> {};

template <>
struct pixel_format_internal<PixelFormat::BGRA> : dutils::constant<PixelInternalFormat::RGBA8> {};

template <>
struct pixel_format_internal<PixelFormat::RED_INTEGER> : dutils::constant<PixelInternalFormat::R8UI> {};

template <>
struct pixel_format_internal<PixelFormat::RG_INTEGER> : dutils::constant<PixelInternalFormat::RG8UI> {};

template <>
struct pixel_format_internal<PixelFormat::RGB_INTEGER> : dutils::constant<PixelInternalFormat::RGB8UI> {};

template <>
struct pixel_format_internal<PixelFormat::BGR_INTEGER> : dutils::constant<PixelInternalFormat::RGB8UI> {};

template <>
struct pixel_format_internal<PixelFormat::RGBA_INTEGER> : dutils::constant<PixelInternalFormat::RGBA8UI> {};

template <>
struct pixel_format_internal<PixelFormat::BGRA_INTEGER> : dutils::constant<PixelInternalFormat::RGBA8UI> {};

/// @brief Provides the count of (usually color) components for the given pixel format.
template <PixelFormat>
struct pixel_format_component_count {};

template <PixelFormat v_format>
inline constexpr auto pixel_format_component_count_v = pixel_format_component_count<v_format>::value;

template <>
struct pixel_format_component_count<PixelFormat::RED> : std::integral_constant<std::size_t, 1> {};

template <>
struct pixel_format_component_count<PixelFormat::RG> : std::integral_constant<std::size_t, 2> {};

template <>
struct pixel_format_component_count<PixelFormat::RGB> : std::integral_constant<std::size_t, 3> {};

template <>
struct pixel_format_component_count<PixelFormat::BGR> : std::integral_constant<std::size_t, 3> {};

template <>
struct pixel_format_component_count<PixelFormat::RGBA> : std::integral_constant<std::size_t, 4> {};

template <>
struct pixel_format_component_count<PixelFormat::BGRA> : std::integral_constant<std::size_t, 4> {};

template <>
struct pixel_format_component_count<PixelFormat::RED_INTEGER> : std::integral_constant<std::size_t, 1> {};

template <>
struct pixel_format_component_count<PixelFormat::RG_INTEGER> : std::integral_constant<std::size_t, 2> {};

template <>
struct pixel_format_component_count<PixelFormat::RGB_INTEGER> : std::integral_constant<std::size_t, 3> {};

template <>
struct pixel_format_component_count<PixelFormat::BGR_INTEGER> : std::integral_constant<std::size_t, 3> {};

template <>
struct pixel_format_component_count<PixelFormat::RGBA_INTEGER> : std::integral_constant<std::size_t, 4> {};

template <>
struct pixel_format_component_count<PixelFormat::BGRA_INTEGER> : std::integral_constant<std::size_t, 4> {};

// TODO: PixelFormat::STENCIL_INDEX
// TODO: PixelFormat::DEPTH_COMPONENT
// TODO: PixelFormat::DEPTH_STENCIL

} // namespace dang::gl
