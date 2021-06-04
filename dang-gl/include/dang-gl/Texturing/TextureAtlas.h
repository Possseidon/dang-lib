#pragma once

#include "dang-gl/Image/BorderedImage.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Texturing/TextureAtlasBase.h"
#include "dang-gl/Texturing/TextureAtlasUtils.h"
#include "dang-gl/global.h"

namespace dang::gl {

namespace detail {

template <PixelFormat v_pixel_format, PixelType v_pixel_type, std::size_t v_row_alignment>
class TextureAtlasSingleTexture {
public:
    using BorderedImageData = BorderedImage<2, v_pixel_format, v_pixel_type, v_row_alignment>;

    // TODO: Some Texture2DArray related delegates for e.g. min/mag filter.
    //      -> Only a select few are probably important.
    //      -> Do not expose Texture2DArray completely.
    //      -> Add a way to send the texture to a shader uniform.

    // TODO: Temporary; remove this.
    Texture2DArray& texture() { return texture_; }

protected:
    bool resize(GLsizei required_size, GLsizei layers, GLsizei mipmap_levels)
    {
        assert(texture_.size().x() == texture_.size().y());
        if (required_size == texture_.size().x() && layers == texture_.size().z())
            return false;
        // /!\ Resets all texture parameters!
        texture_ = Texture2DArray(
            {required_size, required_size, layers}, mipmap_levels, pixel_format_internal_v<v_pixel_format>);
        return true;
    };

    void modify(const BorderedImageData& bordered_image_data, ivec3 offset, GLint mipmap_level)
    {
        texture_.modify(bordered_image_data.image(), offset, mipmap_level);
    };

private:
    Texture2DArray texture_ = empty_object;
};

} // namespace detail

template <PixelFormat v_pixel_format = Image2D::pixel_format,
          PixelType v_pixel_type = Image2D::pixel_type,
          std::size_t v_row_alignment = 4>
class TextureAtlas
    : public TextureAtlasBase<detail::TextureAtlasSingleTexture<v_pixel_format, v_pixel_type, v_row_alignment>> {
public:
    using Base = TextureAtlasBase<detail::TextureAtlasSingleTexture<v_pixel_format, v_pixel_type, v_row_alignment>>;

    explicit TextureAtlas(std::optional<GLsizei> max_texture_size = std::nullopt,
                          std::optional<GLsizei> max_layer_count = std::nullopt)
        : Base(TextureAtlasUtils::checkLimits(max_texture_size, max_layer_count))
    {}
};

template <PixelFormat v_pixel_format = Image2D::pixel_format,
          PixelType v_pixel_type = Image2D::pixel_type,
          std::size_t v_row_alignment = 4>
using FrozenTextureAtlas =
    BasicFrozenTextureAtlas<detail::TextureAtlasSingleTexture<v_pixel_format, v_pixel_type, v_row_alignment>>;

} // namespace dang::gl
