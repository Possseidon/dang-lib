#pragma once

#include "dang-gl/Texturing/TextureAtlasTiles.h"
#include "dang-gl/global.h"

namespace dang::gl::TextureAtlasUtils {

std::size_t checkMaxTextureSize(std::optional<std::size_t> max_texture_size);
std::size_t checkMaxLayerCount(std::optional<std::size_t> max_layer_count);

TextureAtlasLimits checkLimits(std::optional<std::size_t> max_texture_size, std::optional<std::size_t> max_layer_count);

} // namespace dang::gl::TextureAtlasUtils
