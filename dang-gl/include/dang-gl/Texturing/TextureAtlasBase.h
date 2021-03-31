#pragma once

#include "dang-gl/Texturing/TextureAtlasTiles.h"

namespace dang::gl {

/*

The TextureBase<TImageData> concept:

- Move-constructible
- bool resize(GLsizei required_size, GLsizei layers, GLsizei mipmap_levels)
    -> protected, resizes the texture
- void modify(const TImageData& image, ivec3 offset, GLint mipmap_level)
    -> protected, modifies the texture at a given spot

*/

template <typename TImageData, typename TTextureBase>
class BasicFrozenTextureAtlas;

template <typename TImageData, typename TTextureBase>
class TextureAtlasBase : public TTextureBase {
public:
    using Tiles = TextureAtlasTiles<TImageData>;
    using TileBorderGeneration = typename Tiles::TileBorderGeneration;
    using TileHandle = typename Tiles::TileHandle;
    using Frozen = BasicFrozenTextureAtlas<TImageData, TTextureBase>;

    TextureAtlasBase(GLsizei max_texture_size, GLsizei max_layer_count)
        : tiles_(max_texture_size, max_layer_count)
    {}

    TileBorderGeneration guessTileBorderGeneration(GLsizei size) const
    {
        return tiles_.guessTileBorderGeneration(size);
    }

    TileBorderGeneration guessTileBorderGeneration(svec2 size) const { return tiles_.guessTileBorderGeneration(size); }

    TileBorderGeneration defaultBorderGeneration() const { return tiles_.defaultBorderGeneration(); }
    void setDefaultBorderGeneration(TileBorderGeneration border) { tiles_.setDefaultBorderGeneration(border); }

    void add(std::string name, TImageData image_data, std::optional<TileBorderGeneration> border = std::nullopt)
    {
        tiles_.add(std::move(name), std::move(image_data), border);
    }

    [[nodiscard]] TileHandle addWithHandle(std::string name,
                                           TImageData image_data,
                                           std::optional<TileBorderGeneration> border = std::nullopt)
    {
        return tiles_.addWithHandle(std::move(name), std::move(image_data), border);
    }

    [[nodiscard]] bool exists(const std::string& name) const { return tiles_.exists(name); }
    [[nodiscard]] TileHandle operator[](const std::string& name) const { return tiles_[name]; }

    bool tryRemove(const std::string& name) { return tiles_.tryRemove(name); }
    void remove(const std::string& name) { return tiles_.remove(name); }

    void updateTexture() { return updateTextureHelper<false>(); }
    Frozen freeze() && { return updateTextureHelper<true>(); }

private:
    template <bool v_freeze>
    std::conditional_t<v_freeze, Frozen, void> updateTextureHelper()
    {
        // TODO: C++20 replace with std::bind_front
        using namespace std::placeholders;

        auto resize = std::bind(&TextureAtlasBase::resize, this, _1, _2, _3);
        auto modify = std::bind(&TextureAtlasBase::modify, this, _1, _2, _3);

        if constexpr (v_freeze)
            return Frozen(std::move(tiles_).freeze(resize, modify), std::move(*this));
        else
            tiles_.updateTexture(resize, modify);
    }

    Tiles tiles_;
};

template <typename TImageData, typename TTextureBase>
class BasicFrozenTextureAtlas : public TTextureBase {
public:
    using Tiles = FrozenTextureAtlasTiles<TImageData>;
    using TileHandle = typename Tiles::TileHandle;

    friend class TextureAtlasBase<TImageData, TTextureBase>;

    [[nodiscard]] bool exists(const std::string& name) const { return tiles_.exists(name); }
    [[nodiscard]] TileHandle operator[](const std::string& name) const { return tiles_[name]; }

private:
    BasicFrozenTextureAtlas(Tiles&& tiles, TTextureBase&& texture)
        : TTextureBase(std::move(texture))
        , tiles_(std::move(tiles))
    {}

    Tiles tiles_;
};

} // namespace dang::gl
