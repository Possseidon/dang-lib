#pragma once

#include "dang-utils/enum.h"

namespace dang::gl
{

/// <summary>Specifies, which components make up a pixel, containing red, green, blue, alpha, stencil and depth.</summary>
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

namespace detail
{

/// <summary>Helper struct, which simply sets the component count to the template specified value.</summary>
template <std::size_t ComponentCount>
struct PixelFormatCompnentCount {
    static constexpr std::size_t ComponentCount = ComponentCount;
};

}

/// <summary>Provides info about a pixel format, like its component count, which is necessary to find out the storage size.</summary>
template <PixelFormat>
struct PixelFormatInfo {};

template <> struct PixelFormatInfo<PixelFormat::RED> : detail::PixelFormatCompnentCount<1> {};
template <> struct PixelFormatInfo<PixelFormat::RG> : detail::PixelFormatCompnentCount<2> {};
template <> struct PixelFormatInfo<PixelFormat::RGB> : detail::PixelFormatCompnentCount<3> {};
template <> struct PixelFormatInfo<PixelFormat::BGR> : detail::PixelFormatCompnentCount<3> {};
template <> struct PixelFormatInfo<PixelFormat::RGBA> : detail::PixelFormatCompnentCount<4> {};
template <> struct PixelFormatInfo<PixelFormat::BGRA> : detail::PixelFormatCompnentCount<4> {};

template <> struct PixelFormatInfo<PixelFormat::RED_INTEGER> : detail::PixelFormatCompnentCount<1> {};
template <> struct PixelFormatInfo<PixelFormat::RG_INTEGER> : detail::PixelFormatCompnentCount<2> {};
template <> struct PixelFormatInfo<PixelFormat::RGB_INTEGER> : detail::PixelFormatCompnentCount<3> {};
template <> struct PixelFormatInfo<PixelFormat::BGR_INTEGER> : detail::PixelFormatCompnentCount<3> {};
template <> struct PixelFormatInfo<PixelFormat::RGBA_INTEGER> : detail::PixelFormatCompnentCount<4> {};
template <> struct PixelFormatInfo<PixelFormat::BGRA_INTEGER> : detail::PixelFormatCompnentCount<4> {};

// TODO: What are the actual component counts?
/*
template <> struct PixelFormatInfo<PixelFormat::STENCIL_INDEX> : detail::PixelFormatCompnentCount<4> {};
template <> struct PixelFormatInfo<PixelFormat::DEPTH_COMPONENT> : detail::PixelFormatCompnentCount<4> {};
template <> struct PixelFormatInfo<PixelFormat::DEPTH_STENCIL> : detail::PixelFormatCompnentCount<4> {};
*/

/// <summary>The GL-Constants for the pixel formats.</summary>
constexpr dutils::EnumArray<PixelFormat, GLenum> PixelFormatsGL
{
    GL_RED,
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
    GL_DEPTH_STENCIL
};

}
