#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief The type of the components in a pixel, which also contains various non-byte aligned variants.
enum class PixelType {
    UNSIGNED_BYTE,
    BYTE,
    UNSIGNED_SHORT,
    SHORT,
    UNSIGNED_INT,
    INT,
    HALF_FLOAT,
    FLOAT,

    UNSIGNED_BYTE_3_3_2,
    UNSIGNED_BYTE_2_3_3_REV,

    UNSIGNED_SHORT_5_6_5,
    UNSIGNED_SHORT_5_6_5_REV,
    UNSIGNED_SHORT_4_4_4_4,
    UNSIGNED_SHORT_4_4_4_4_REV,
    UNSIGNED_SHORT_5_5_5_1,
    UNSIGNED_SHORT_1_5_5_5_REV,

    UNSIGNED_INT_8_8_8_8,
    UNSIGNED_INT_8_8_8_8_REV,
    UNSIGNED_INT_10_10_10_2,
    UNSIGNED_INT_2_10_10_10_REV,

    // glReadPixels exclusive
    UNSIGNED_INT_24_8,
    UNSIGNED_INT_10F_11F_11F_REV,
    UNSIGNED_INT_5_9_9_9_REV,
    FLOAT_32_UNSIGNED_INT_24_8_REV,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::PixelType> : default_enum_count<dang::gl::PixelType> {};

} // namespace dang::utils

namespace dang::gl {

template <>
inline constexpr dutils::EnumArray<PixelType, GLenum> gl_constants<PixelType> = {GL_UNSIGNED_BYTE,
                                                                                 GL_BYTE,
                                                                                 GL_UNSIGNED_SHORT,
                                                                                 GL_SHORT,
                                                                                 GL_UNSIGNED_INT,
                                                                                 GL_INT,
                                                                                 GL_HALF_FLOAT,
                                                                                 GL_FLOAT,

                                                                                 GL_UNSIGNED_BYTE_3_3_2,
                                                                                 GL_UNSIGNED_BYTE_2_3_3_REV,

                                                                                 GL_UNSIGNED_SHORT_5_6_5,
                                                                                 GL_UNSIGNED_SHORT_5_6_5_REV,
                                                                                 GL_UNSIGNED_SHORT_4_4_4_4,
                                                                                 GL_UNSIGNED_SHORT_4_4_4_4_REV,
                                                                                 GL_UNSIGNED_SHORT_5_5_5_1,
                                                                                 GL_UNSIGNED_SHORT_1_5_5_5_REV,

                                                                                 GL_UNSIGNED_INT_8_8_8_8,
                                                                                 GL_UNSIGNED_INT_8_8_8_8_REV,
                                                                                 GL_UNSIGNED_INT_10_10_10_2,
                                                                                 GL_UNSIGNED_INT_2_10_10_10_REV,

                                                                                 // glReadPixels exclusive
                                                                                 GL_UNSIGNED_INT_24_8,
                                                                                 GL_UNSIGNED_INT_10F_11F_11F_REV,
                                                                                 GL_UNSIGNED_INT_5_9_9_9_REV,
                                                                                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV};

template <PixelType>
struct underlying_pixel_type {};

template <PixelType v_type>
using underlying_pixel_type_t = typename underlying_pixel_type<v_type>::type;

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_BYTE> {
    using type = GLubyte;
};

template <>
struct underlying_pixel_type<PixelType::BYTE> {
    using type = GLbyte;
};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT> {
    using type = GLushort;
};
template <>
struct underlying_pixel_type<PixelType::SHORT> {
    using type = GLshort;
};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT> {
    using type = GLuint;
};

template <>
struct underlying_pixel_type<PixelType::INT> {
    using type = GLint;
};

template <>
struct underlying_pixel_type<PixelType::HALF_FLOAT> {
    using type = GLhalf;
};

template <>
struct underlying_pixel_type<PixelType::FLOAT> {
    using type = GLfloat;
};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_BYTE_3_3_2> : underlying_pixel_type<PixelType::UNSIGNED_BYTE> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_BYTE_2_3_3_REV> : underlying_pixel_type<PixelType::UNSIGNED_BYTE> {};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_5_6_5> : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_5_6_5_REV> : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_4_4_4_4> : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_4_4_4_4_REV>
    : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_5_5_5_1> : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_SHORT_1_5_5_5_REV>
    : underlying_pixel_type<PixelType::UNSIGNED_SHORT> {};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_8_8_8_8> : underlying_pixel_type<PixelType::UNSIGNED_INT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_8_8_8_8_REV> : underlying_pixel_type<PixelType::UNSIGNED_INT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_10_10_10_2> : underlying_pixel_type<PixelType::UNSIGNED_INT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_2_10_10_10_REV>
    : underlying_pixel_type<PixelType::UNSIGNED_INT> {};

template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_24_8> : underlying_pixel_type<PixelType::UNSIGNED_INT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_10F_11F_11F_REV>
    : underlying_pixel_type<PixelType::UNSIGNED_INT> {};
template <>
struct underlying_pixel_type<PixelType::UNSIGNED_INT_5_9_9_9_REV> : underlying_pixel_type<PixelType::UNSIGNED_INT> {};

template <>
struct underlying_pixel_type<PixelType::FLOAT_32_UNSIGNED_INT_24_8_REV> {
    struct type {
        GLfloat depth;
        // stencil only has 8 bit actually
        GLuint stencil;
    };
};

} // namespace dang::gl
