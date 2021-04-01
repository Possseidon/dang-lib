#pragma once

#include "dang-gl/Texturing/TextureAtlasTiles.h"
#include "dang-gl/global.h"

namespace dang::gl::TextureAtlasUtils {

GLsizei checkMaxTextureSize(std::optional<GLsizei> max_texture_size);
GLsizei checkMaxLayerCount(std::optional<GLsizei> max_layer_count);

TextureAtlasLimits checkLimits(std::optional<GLsizei> max_texture_size, std::optional<GLsizei> max_layer_count);

} // namespace dang::gl::TextureAtlasUtils
