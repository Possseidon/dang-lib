#include "dang-gl/Texturing/TextureAtlasUtils.h"

#include "dang-gl/Context/Context.h"

namespace dang::gl::TextureAtlasUtils {

std::size_t checkMaxTextureSize(std::optional<std::size_t> max_texture_size)
{
    auto context_max_texture_size = static_cast<std::size_t>(context()->max_3d_texture_size);
    if (!max_texture_size)
        return context_max_texture_size;
    if (*max_texture_size < 1)
        throw std::invalid_argument("Maximum texture size must be at least one.");
    if (*max_texture_size > context_max_texture_size)
        throw std::invalid_argument("Maximum texture size must be at most " + std::to_string(context_max_texture_size) +
                                    ". (got " + std::to_string(*max_texture_size) + ")");
    return *max_texture_size;
}

std::size_t checkMaxLayerCount(std::optional<std::size_t> max_layer_count)
{
    auto context_max_layer_count = static_cast<std::size_t>(context()->max_array_texture_layers);
    if (!max_layer_count)
        return context_max_layer_count;
    if (*max_layer_count < 1)
        throw std::invalid_argument("Maximum layer count must be at least one.");
    if (*max_layer_count > context_max_layer_count)
        throw std::invalid_argument("Maximum layer count must be at most " + std::to_string(context_max_layer_count) +
                                    ". (got " + std::to_string(*max_layer_count) + ")");
    return *max_layer_count;
}

TextureAtlasLimits checkLimits(std::optional<std::size_t> max_texture_size, std::optional<std::size_t> max_layer_count)
{
    return {checkMaxTextureSize(max_texture_size), checkMaxLayerCount(max_layer_count)};
}

} // namespace dang::gl::TextureAtlasUtils
