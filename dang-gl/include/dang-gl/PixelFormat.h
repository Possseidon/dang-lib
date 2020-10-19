#pragma once

#include "dang-utils/enum.h"

#include "GLConstants.h"
#include "PixelInternalFormat.h"

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

/// <summary>Provides info about a pixel format, like its component count, which is necessary to find out the storage size.</summary>
template <PixelFormat>
struct PixelFormatInfo {};

template <> struct PixelFormatInfo<PixelFormat::RED> {
    static constexpr std::size_t ComponentCount = 1;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::R8;
};

template <> struct PixelFormatInfo<PixelFormat::RG> {
    static constexpr std::size_t ComponentCount = 2;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RG8;
};

template <> struct PixelFormatInfo<PixelFormat::RGB> {
    static constexpr std::size_t ComponentCount = 3;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGB8;
};

template <> struct PixelFormatInfo<PixelFormat::BGR> {
    static constexpr std::size_t ComponentCount = 3;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGB8;
};

template <> struct PixelFormatInfo<PixelFormat::RGBA> {
    static constexpr std::size_t ComponentCount = 4;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGBA8;
};

template <> struct PixelFormatInfo<PixelFormat::BGRA> {
    static constexpr std::size_t ComponentCount = 4;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGBA8;
};

template <> struct PixelFormatInfo<PixelFormat::RED_INTEGER> {
    static constexpr std::size_t ComponentCount = 1;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::R8UI;
};

template <> struct PixelFormatInfo<PixelFormat::RG_INTEGER> {
    static constexpr std::size_t ComponentCount = 2;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RG8UI;
};

template <> struct PixelFormatInfo<PixelFormat::RGB_INTEGER> {
    static constexpr std::size_t ComponentCount = 3;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGB8UI;
};

template <> struct PixelFormatInfo<PixelFormat::BGR_INTEGER> {
    static constexpr std::size_t ComponentCount = 3;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGB8UI;
};

template <> struct PixelFormatInfo<PixelFormat::RGBA_INTEGER> {
    static constexpr std::size_t ComponentCount = 4;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGBA8UI;
};

template <> struct PixelFormatInfo<PixelFormat::BGRA_INTEGER> {
    static constexpr std::size_t ComponentCount = 4;
    static constexpr PixelInternalFormat Internal = PixelInternalFormat::RGBA8UI;
};

template <> struct PixelFormatInfo<PixelFormat::STENCIL_INDEX> { /* TODO: PixelFormatInfo<PixelFormat::STENCIL_INDEX> */ };
template <> struct PixelFormatInfo<PixelFormat::DEPTH_COMPONENT> { /* TODO: PixelFormatInfo<PixelFormat::DEPTH_COMPONENT> */ };
template <> struct PixelFormatInfo<PixelFormat::DEPTH_STENCIL> { /* TODO: PixelFormatInfo<PixelFormat::DEPTH_STENCIL> */ };

/// <summary>The GL-Constants for the pixel formats.</summary>
template <>
constexpr dutils::EnumArray<PixelFormat, GLenum> GLConstants<PixelFormat> = {
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
