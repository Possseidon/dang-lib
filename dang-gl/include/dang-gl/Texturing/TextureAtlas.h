#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Image/Image.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Texturing/TextureAtlasBase.h"

namespace dang::gl {

namespace detail {

template <PixelFormat v_format, PixelType v_type>
class TextureAtlasSingleTexture {
public:
    using ImageData = Image<2, v_format, v_type>;

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
        texture_ =
            Texture2DArray({required_size, required_size, layers}, mipmap_levels, pixel_format_internal_v<v_format>);
        return true;
    };

    void modify(const ImageData& image, ivec3 offset, GLint mipmap_level)
    {
        texture_.modify(image, offset, mipmap_level);
    };

    Texture2DArray texture_ = empty_object;
};

} // namespace detail

template <PixelFormat v_format = Image2D::pixel_format, PixelType v_type = Image2D::pixel_type>
class TextureAtlas
    : public TextureAtlasBase<Image<2, v_format, v_type>, detail::TextureAtlasSingleTexture<v_format, v_type>> {
public:
    using Base = TextureAtlasBase<Image<2, v_format, v_type>, detail::TextureAtlasSingleTexture<v_format, v_type>>;

    explicit TextureAtlas(std::optional<GLsizei> max_texture_size = std::nullopt,
                          std::optional<GLsizei> max_layer_count = std::nullopt)
        : Base(max_texture_size.value_or(context()->max_3d_texture_size),
               max_layer_count.value_or(context()->max_array_texture_layers))
    {
        assert(!max_texture_size || max_texture_size >= 1 && max_texture_size <= context()->max_3d_texture_size);
        assert(!max_layer_count || max_layer_count >= 1 && max_layer_count <= context()->max_array_texture_layers);
    }
};

template <PixelFormat v_format = Image2D::pixel_format, PixelType v_type = Image2D::pixel_type>
using FrozenTextureAtlas =
    BasicFrozenTextureAtlas<Image<2, v_format, v_type>, detail::TextureAtlasSingleTexture<v_format, v_type>>;

} // namespace dang::gl
