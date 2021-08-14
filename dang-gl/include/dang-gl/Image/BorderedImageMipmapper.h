#pragma once

#include "dang-gl/Image/BorderedImage.h"
#include "dang-gl/Image/ImageMipmapper.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"

#include "dang-math/vector.h"

namespace dang::gl {

/// @brief Mipmaps a bordered image into a new bordered image (with border set to "none").
template <typename TCalcType = float>
struct BorderedImageMipmapper {
    template <std::size_t v_dim, PixelFormat v_pixel_format, PixelType v_pixel_type, std::size_t v_row_alignment>
    auto operator()(const BorderedImage<v_dim, v_pixel_format, v_pixel_type, v_row_alignment>& bordered_image) const
    {
        // Mipmaps entire image including border, but sets border of result to "none".
        using BorderedImage = BorderedImage<v_dim, v_pixel_format, v_pixel_type, v_row_alignment>;
        return BorderedImage(image_mipmapper<TCalcType>(bordered_image.image()));
    }
};

template <typename TCalcType = float>
inline constexpr BorderedImageMipmapper<TCalcType> bordered_image_mipmapper;

} // namespace dang::gl
