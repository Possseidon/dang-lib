#pragma once

#include "dang-gl/Image/Pixel.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Describes an image border that does nothing.
struct ImageBorderNone {};

/// @brief Describes an image border of a single solid color all around.
template <PixelFormat v_pixel_format = PixelFormat::RGBA, PixelType v_pixel_type = PixelType::UNSIGNED_BYTE>
struct ImageBorderSolid {
    Pixel<v_pixel_format, v_pixel_type> color;
};

/// @brief Describes an image border, for which both sides are a copy from the opposite side.
struct ImageBorderWrapBoth {};

/// @brief Describes an image border, for which only the positive side is copied from the opposite negative sides.
struct ImageBorderWrapPositive {};

/// @brief A variant of the different image border styles.
template <PixelFormat v_pixel_format = PixelFormat::RGBA, PixelType v_pixel_type = PixelType::UNSIGNED_BYTE>
using ImageBorder = std::variant<ImageBorderNone,
                                 ImageBorderSolid<v_pixel_format, v_pixel_type>,
                                 ImageBorderWrapBoth,
                                 ImageBorderWrapPositive>;

} // namespace dang::gl
