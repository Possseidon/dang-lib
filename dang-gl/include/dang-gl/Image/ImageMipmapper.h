#pragma once

#include "dang-gl/Image/Image.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-math/vector.h"

namespace dang::gl {

/// @brief Mipmaps a regular image using a 2x2 box filter.
/// @remark Odd pixels are copied over, which might result in bad mipmaps, if the size is odd on a lot of levels.
template <typename TCalcType = float>
struct ImageMipmapper {
    template <std::size_t v_dim, PixelFormat v_pixel_format, PixelType v_pixel_type, std::size_t v_row_alignment>
    auto operator()(const Image<v_dim, v_pixel_format, v_pixel_type, v_row_alignment>& image) const
    {
        using Image = Image<v_dim, v_pixel_format, v_pixel_type, v_row_alignment>;
        using Pixel = typename Image::Pixel;
        using Size = typename Image::Size;
        using CalcPixel = dmath::Vector<TCalcType, Pixel::dim>;

        // It is not possible to change box_size at will.
        // To get that working more changes would be necessary.
        constexpr std::size_t box_size = 2;
        constexpr auto box_pixels = box_size * box_size;

        auto floor_size = image.size() / box_size;
        auto ceil_size = (image.size() - 1) / box_size + 1;
        Image result(ceil_size);
        for (const auto& pos : dmath::sbounds2(floor_size)) {
            CalcPixel color;
            for (const auto& offset : dmath::sbounds2(box_size))
                color += static_cast<CalcPixel>(image[pos * box_size + offset]);
            result[pos] = static_cast<Pixel>(color / box_pixels);
        }

        auto odd_width = floor_size.x() != ceil_size.x();
        auto odd_height = floor_size.y() != ceil_size.y();

        if (odd_width) {
            auto x = floor_size.x();
            for (std::size_t y = 0; y < floor_size.y(); y++) {
                CalcPixel color;
                for (std::size_t offset = 0; offset < box_size; offset++)
                    color += static_cast<CalcPixel>(image[Size(x * box_size, y * box_size + offset)]);
                result[Size(x, y)] = static_cast<Pixel>(color / box_size);
            }
        }

        if (odd_height) {
            auto y = floor_size.y();
            for (std::size_t x = 0; x < floor_size.x(); x++) {
                CalcPixel color;
                for (std::size_t offset = 0; offset < box_size; offset++)
                    color += static_cast<CalcPixel>(image[Size(x * box_size + offset, y * box_size)]);
                result[Size(x, y)] = static_cast<Pixel>(color / box_size);
            }
        }

        if (odd_width && odd_height)
            result[floor_size] = image[floor_size * box_size];

        return result;
    }
};

template <typename TCalcType = float>
inline constexpr ImageMipmapper<TCalcType> image_mipmapper;

} // namespace dang::gl
