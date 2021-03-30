#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Image/Image.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Texturing/TextureAtlasTiles.h"

namespace dang::gl {

class FrozenTextureAtlas;

class TextureAtlas {
public:
    using Tiles = TextureAtlasTiles<Image2D>;
    using TileBorderGeneration = Tiles::TileBorderGeneration;
    using TileHandle = Tiles::TileHandle;

    explicit TextureAtlas(std::optional<GLsizei> max_texture_size = std::nullopt,
                          std::optional<GLsizei> max_layer_count = std::nullopt);

    TileBorderGeneration guessTileBorderGeneration(GLsizei size) const;
    TileBorderGeneration guessTileBorderGeneration(svec2 size) const;

    TileBorderGeneration defaultBorderGeneration() const;
    void setDefaultBorderGeneration(TileBorderGeneration border);

    void add(std::string name, Image2D image, std::optional<TileBorderGeneration> border = std::nullopt);
    [[nodiscard]] TileHandle addWithHandle(std::string name,
                                           Image2D image,
                                           std::optional<TileBorderGeneration> border = std::nullopt);

    [[nodiscard]] bool exists(const std::string& name) const;
    [[nodiscard]] TileHandle operator[](const std::string& name) const;

    bool tryRemove(const std::string& name);
    void remove(const std::string& name);

    void updateTexture();
    FrozenTextureAtlas freeze() &&;

    // TODO: Some Texture2DArray related delegates for e.g. min/mag filter.
    //      -> Only a select few are probably important.
    //      -> Do not expose Texture2DArray completely.
    //      -> Add a way to send the texture to a shader uniform.

    // TODO: Temporary; remove this.
    Texture2DArray& texture() { return texture_; }

private:
    template <bool v_freeze>
    std::conditional_t<v_freeze, FrozenTextureAtlas, void> updateTextureHelper();

    Tiles tiles_;

    // TODO: Move texture and related functions into a base class.
    Texture2DArray texture_ = empty_object;
};

class FrozenTextureAtlas {
public:
    using Tiles = FrozenTextureAtlasTiles<Image2D>;
    using TileBorderGeneration = TextureAtlas::TileBorderGeneration;
    using TileHandle = TextureAtlas::TileHandle;

    friend class TextureAtlas;

    [[nodiscard]] bool exists(const std::string& name) const;
    [[nodiscard]] TileHandle operator[](const std::string& name) const;

    // TODO: Temporary; remove this.
    Texture2DArray& texture() { return texture_; }

private:
    FrozenTextureAtlas(Tiles&& tiles, Texture2DArray&& texture);

    Tiles tiles_;
    Texture2DArray texture_;
};

} // namespace dang::gl
