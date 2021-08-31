#pragma once

#include "dang-gl/Image/Image.h"
#include "dang-gl/Image/ImageBorder.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Adds or replaces a border around an image.
template <std::size_t v_dim,
          PixelFormat v_pixel_format = PixelFormat::RGBA,
          PixelType v_pixel_type = PixelType::UNSIGNED_BYTE,
          std::size_t v_row_alignment = 4>
class BorderedImage {
public:
    static constexpr auto dim = v_dim;
    static constexpr auto pixel_format = v_pixel_format;
    static constexpr auto pixel_type = v_pixel_type;
    static constexpr auto row_alignment = v_row_alignment;

    using BorderNone = ImageBorderNone;
    using BorderSolid = ImageBorderSolid<v_pixel_format, v_pixel_type>;
    using BorderWrapPositive = ImageBorderWrapPositive;
    using BorderWrapBoth = ImageBorderWrapBoth;

    using Border = ImageBorder<v_pixel_format, v_pixel_type>;

    using Image = dang::gl::Image<v_dim, v_pixel_format, v_pixel_type, v_row_alignment>;

    using Pixel = typename Image::Pixel;
    using Size = typename Image::Size;
    using Bounds = typename Image::Bounds;

    /// @brief Constructs an empty image without a border.
    BorderedImage() = default;

    /// @brief Uses the given image directly without a border.
    BorderedImage(Image image)
        : image_(std::move(image))
    {}

    /// @brief Creates a padded copy of the image and applies the border to it.
    static BorderedImage addBorder(const Border& border, const Image& image)
    {
        return {border, std::visit(AddBorder{image}, border)};
    }

    /// @brief Assumes the image to be padded already, allowing modifying it in place when moved in.
    static BorderedImage replaceBorder(const Border& border, Image image)
    {
        return {border, std::visit(ReplaceBorder{std::move(image)}, border)};
    }

    /// @brief The image with the now applied border.
    const auto& image() const& { return image_; }
    /// @brief The image with the now applied border.
    auto image() && { return std::move(image_); }

    // --- BorderedImageData concept:

    /// @brief The border that the image now has.
    const auto& border() const { return border_; }

    /// @brief How much of the size is padding for the border.
    auto padding() const { return std::visit(imageBorderPadding, border_); }

    /// @brief Whether the image contains any actual data.
    explicit operator bool() const { return bool{image_}; }

    /// @brief Returns the size of the image.
    const auto& size() const { return image_.size(); }

    /// @brief Frees all image data, but leaves the size intact.
    void free() { image_.free(); }

private:
    /// @brief Assumes the given image already has the specified border style.
    BorderedImage(const Border& border, Image image)
        : border_(border)
        , image_(std::move(image))
    {}

    struct AddBorder {
        const Image& image;

        Image operator()(BorderNone) const
        {
            // Copy could be avoided in this rare case...
            return image;
        }

        Image operator()(BorderSolid border) const
        {
            // This could be optimized by not filling it with the color completely.
            // Maybe add an additional Image ctor taking size, image, offset and fill color.
            Image new_image(image.size() + 2, border.color);
            new_image.setSubImage({1}, image);
            return new_image;
        }

        Image operator()(BorderWrapBoth border) const
        {
            Image new_image(image.size() + 2);
            new_image.setSubImage({1}, image);
            return ReplaceBorder{std::move(new_image)}(border);
        }

        Image operator()(BorderWrapPositive border) const
        {
            Image new_image(image.size() + 1);
            new_image.setSubImage({0}, image);
            return ReplaceBorder{std::move(new_image)}(border);
        }
    };

    struct ReplaceBorder {
        Image image;

        Image operator()(BorderNone) && { return std::move(image); }

        Image operator()(BorderSolid border) &&
        {
            Bounds bounds(image.size());
            for (std::size_t facing = 0; facing < dim * 2; facing++)
                for (const auto& pos : bounds.facing(facing, typename Bounds::ClipInfo{false, true}).xFirst())
                    image[pos] = border.color;
            return std::move(image);
        }

        Image operator()(BorderWrapBoth) &&
        {
            auto size = image.size();
            Bounds bounds(size);
            for (std::size_t facing = 0; facing < dim * 2; facing++)
                for (const auto& pos : bounds.facing(facing, typename Bounds::ClipInfo{false, true}).xFirst())
                    image[pos] = image[(pos + size - 3) % (size - 2) + 1];
            return std::move(image);
        }

        Image operator()(BorderWrapPositive) &&
        {
            Bounds bounds(image.size());
            auto inner_size = image.size() - 1;
            for (std::size_t facing = 1; facing < dim * 2; facing += 2)
                for (const auto& pos : bounds.facing(facing, typename Bounds::ClipInfo{false, false}).xFirst())
                    image[pos] = image[(pos + inner_size) % inner_size];
            return std::move(image);
        }
    };

    Border border_;
    Image image_;
};

} // namespace dang::gl
