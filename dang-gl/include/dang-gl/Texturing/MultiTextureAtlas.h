#pragma once

#include "dang-gl/Image/BorderedImage.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Texturing/TextureAtlasBase.h"
#include "dang-gl/Texturing/TextureAtlasUtils.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

namespace detail {

template <typename TSubTextureEnum, PixelFormat v_pixel_format, PixelType v_pixel_type, std::size_t v_row_alignment>
class TextureAtlasMultiTexture {
public:
    using BorderedImage = BorderedImage<2, v_pixel_format, v_pixel_type, v_row_alignment>;

    class BorderedImageData {
    public:
        BorderedImageData(dutils::EnumArray<TSubTextureEnum, BorderedImage> bordered_images)
            : bordered_images_((ensureCompatible(bordered_images), std::move(bordered_images)))
        {}

        BorderedImage& operator[](TSubTextureEnum sub_texture) { return bordered_images_[sub_texture]; }

        const BorderedImage& operator[](TSubTextureEnum sub_texture) const { return bordered_images_[sub_texture]; }

        // --- BorderedImageData concept:

        const auto& border() const { return bordered_images_.front().border(); }

        explicit operator bool() const { return bool{bordered_images_.front()}; }

        const auto& size() const { return bordered_images_.front().size(); }

        void free()
        {
            for (auto& image : bordered_images_)
                image.free();
        }

    private:
        void ensureCompatible(const dutils::EnumArray<TSubTextureEnum, BorderedImage>& bordered_images)
        {
            auto size = bordered_images.front().size();
            auto border = bordered_images.font().border();
            for (auto& bordered_image : bordered_images) {
                if (!bordered_image)
                    throw std::invalid_argument("SubTexture image is empty.");
                if (bordered_image.size() != size)
                    throw std::invalid_argument("SubTexture images have varying sizes. (" +
                                                bordered_image.size().format() + " != " + size.format() + ")");
                if (imageBorderPadding(bordered_image.border()) != imageBorderPadding(border))
                    throw std::invalid_argument("SubTexture images have borders with varying padding.");
            }
        }

        dutils::EnumArray<TSubTextureEnum, BorderedImage> bordered_images_;
    };

    // TODO: Some Texture2DArray related delegates for e.g. min/mag filter.
    //      -> Only a select few are probably important.
    //      -> Do not expose Texture2DArray completely.
    //      -> Add a way to send the texture to a shader uniform.

    // TODO: Temporary; remove this.
    Texture2DArray& texture(TSubTextureEnum sub_texture) { return textures_[sub_texture]; }

protected:
    bool resize(GLsizei required_size, GLsizei layers, GLsizei mipmap_levels)
    {
        assert(textures_.front().size().x() == textures_.front().size().y());
        if (required_size == textures_.front().size().x() && layers == textures_.front().size().z())
            return false;
        // /!\ Resets all texture parameters!
        for (auto& texture : textures_)
            texture = Texture2DArray(
                {required_size, required_size, layers}, mipmap_levels, pixel_format_internal_v<v_pixel_format>);
        return true;
    };

    void modify(const BorderedImageData& bordered_image_data, ivec3 offset, GLint mipmap_level)
    {
        for (auto sub_texture : dutils::enumerate<TSubTextureEnum>)
            textures_[sub_texture].modify(bordered_image_data[sub_texture], offset, mipmap_level);
    };

private:
    template <TSubTextureEnum... v_sub_textures>
    dutils::EnumArray<TSubTextureEnum, Texture2DArray> emptyTextures(
        dutils::EnumSequence<TSubTextureEnum, v_sub_textures...>)
    {
        return {((void)v_sub_textures, empty_object)...};
    }

    dutils::EnumArray<TSubTextureEnum, Texture2DArray> textures_ =
        emptyTextures(dutils::makeEnumSequence<TSubTextureEnum>());
};

} // namespace detail

template <typename TSubTextureEnum,
          PixelFormat v_pixel_format = Image2D::pixel_format,
          PixelType v_pixel_type = Image2D::pixel_type,
          std::size_t v_row_alignment = 4>
class MultiTextureAtlas
    : public TextureAtlasBase<
          detail::TextureAtlasMultiTexture<TSubTextureEnum, v_pixel_format, v_pixel_type, v_row_alignment>> {
public:
    using Base = TextureAtlasBase<
        detail::TextureAtlasMultiTexture<TSubTextureEnum, v_pixel_format, v_pixel_type, v_row_alignment>>;

    explicit MultiTextureAtlas(std::optional<GLsizei> max_texture_size = std::nullopt,
                               std::optional<GLsizei> max_layer_count = std::nullopt)
        : Base(TextureAtlasUtils::checkLimits(max_texture_size, max_layer_count))
    {}
};

template <typename TSubTextureEnum,
          PixelFormat v_pixel_format = Image2D::pixel_format,
          PixelType v_pixel_type = Image2D::pixel_type,
          std::size_t v_row_alignment = 4>
using FrozenMultiTextureAtlas = BasicFrozenTextureAtlas<
    detail::TextureAtlasMultiTexture<TSubTextureEnum, v_pixel_format, v_pixel_type, v_row_alignment>>;

} // namespace dang::gl
