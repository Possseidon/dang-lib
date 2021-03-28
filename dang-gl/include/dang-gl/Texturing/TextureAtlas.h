#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Texturing/TextureAtlasTiles.h"

namespace dang::gl {

class TextureAtlas {
public:
    using TileBorderGeneration = TextureAtlasTiles::TileBorderGeneration;
    using TileHandle = TextureAtlasTiles::TileHandle;

    explicit TextureAtlas(std::optional<GLsizei> max_texture_size = std::nullopt,
                          std::optional<GLsizei> max_layer_count = std::nullopt);

    TileBorderGeneration guessTileBorderGeneration(GLsizei size) const;
    TileBorderGeneration guessTileBorderGeneration(svec2 size) const;

    TileBorderGeneration defaultBorderGeneration() const;
    void setDefaultBorderGeneration(TileBorderGeneration border);

    bool add(std::string name, Image2D image, std::optional<TileBorderGeneration> border = std::nullopt);
    [[nodiscard]] TileHandle addWithHandle(std::string name,
                                           Image2D image,
                                           std::optional<TileBorderGeneration> border = std::nullopt);

    [[nodiscard]] bool exists(const std::string& name) const;
    [[nodiscard]] TileHandle operator[](const std::string& name) const;

    bool remove(const std::string& name);

    void updateTexture();

    // TODO: Some Texture2DArray related delegates for e.g. min/mag filter.
    //      -> Only a select few are probably important.
    //      -> Do not expose Texture2DArray completely.
    //      -> Add a way to send the texture to a shader uniform.

    // TODO: Temporary; remove this.
    Texture2DArray& texture() { return texture_; }

private:
    TextureAtlasTiles tiles_;
    Texture2DArray texture_;
};

} // namespace dang::gl
