#pragma once

#include "dang-gl/Image/Image.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/Math/MathTypes.h"
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
    using Image = Image<2, v_pixel_format, v_pixel_type, v_row_alignment>;

    class ImageData {
    public:
        ImageData(dutils::EnumArray<TSubTextureEnum, Image> images)
            : images_((ensureSameSize(images), std::move(images)))
        {}

        Image& operator[](TSubTextureEnum sub_texture) { return images_[sub_texture]; }

        const Image& operator[](TSubTextureEnum sub_texture) const { return images_[sub_texture]; }

        // --- ImageData concept:

        explicit operator bool() const { return bool{images_.front()}; }

        auto size() const { return images_.front().size(); }

        void free()
        {
            for (auto& image : images_)
                image.free();
        }

        ImageData operator[](const dmath::sbounds2& bounds) const
        {
            return subsectionHelper(bounds, dutils::makeEnumSequence<TSubTextureEnum>());
        }

    private:
        template <TSubTextureEnum... v_sub_textures>
        ImageData subsectionHelper(const dmath::sbounds2& bounds,
                                   dutils::EnumSequence<TSubTextureEnum, v_sub_textures...>) const
        {
            return {{images_[v_sub_textures][bounds]...}};
        }

        void ensureSameSize(const dutils::EnumArray<TSubTextureEnum, Image>& images)
        {
            auto size = images.front().size();
            for (auto& image : images) {
                if (!image)
                    throw std::invalid_argument("SubTexture image is empty.");
                if (image.size() != size)
                    throw std::invalid_argument("SubTexture images have varying sizes. (" + image.size().format() +
                                                " != " + size.format() + ")");
            }
        }

        dutils::EnumArray<TSubTextureEnum, Image> images_;
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

    void modify(const ImageData& image_data, ivec3 offset, GLint mipmap_level)
    {
        for (auto sub_texture : dutils::enumerate<TSubTextureEnum>)
            textures_[sub_texture].modify(image_data[sub_texture], offset, mipmap_level);
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
