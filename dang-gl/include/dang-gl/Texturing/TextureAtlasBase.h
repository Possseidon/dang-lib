#pragma once

#include "dang-gl/Texturing/TextureAtlasTiles.h"
#include "dang-gl/global.h"

namespace dang::gl {

/*

The TextureBase concept:

- Move-constructible
- using BorderedImageData = ...;
- bool resize(GLsizei required_size, GLsizei layers, GLsizei mipmap_levels)
    -> protected, resizes the texture
- void modify(const BorderedImageData& bordered_image_data, ivec3 offset, GLint mipmap_level)
    -> protected, modifies the texture at a given spot

*/

template <typename TTextureBase>
class BasicFrozenTextureAtlas;

template <typename TTextureBase>
class TextureAtlasBase : public TTextureBase {
public:
    using BorderedImageData = typename TTextureBase::BorderedImageData;
    using Tiles = TextureAtlasTiles<BorderedImageData>;
    using TileHandle = typename Tiles::TileHandle;
    using Frozen = BasicFrozenTextureAtlas<TTextureBase>;

    TextureAtlasBase(const TextureAtlasLimits& limits)
        : tiles_(limits)
    {}

    [[nodiscard]] TileHandle add(BorderedImageData bordered_image_data)
    {
        return tiles_.add(std::move(bordered_image_data));
    }

    [[nodiscard]] bool contains(const TileHandle& tile_handle) const { return tiles_.contains(tile_handle); }

    bool tryRemove(const TileHandle& tile_handle) { return tiles_.tryRemove(tile_handle); }
    void remove(const TileHandle& tile_handle) { return tiles_.remove(tile_handle); }

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

template <typename TTextureBase>
class BasicFrozenTextureAtlas : public TTextureBase {
public:
    using BorderedImageData = typename TTextureBase::BorderedImageData;
    using Tiles = FrozenTextureAtlasTiles<BorderedImageData>;
    using TileHandle = typename Tiles::TileHandle;

    friend class TextureAtlasBase<TTextureBase>;

    [[nodiscard]] bool exists(const TileHandle& tile_handle) const { return tiles_.exists(tile_handle); }

private:
    BasicFrozenTextureAtlas(Tiles&& tiles, TTextureBase&& texture)
        : TTextureBase(std::move(texture))
        , tiles_(std::move(tiles))
    {}

    Tiles tiles_;
};

} // namespace dang::gl
