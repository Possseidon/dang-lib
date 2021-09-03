#pragma once

#include "dang-gl/Image/MipmapLevels.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/global.h"
#include "dang-utils/utils.h"

namespace dang::gl {

template <typename TBorderedImageData>
class FrozenTextureAtlasTiles;

/*

The BorderedImageData concept:

- Move-constructible
- dmath::svec2 padding()
    -> how much of the size is padding
- explicit operator bool() const
    -> if it contains any data
- dmath::svec2 size() const
    -> image size
- void free()
    -> frees all data, but leaves the size

*/

/// @brief Holds maximum size restrictions of the texture atlas.
struct TextureAtlasLimits {
    std::size_t max_texture_size;
    std::size_t max_layer_count;
};

/// @brief Can store a large number of texture tiles in multiple layers of grids.
/// @remark Meant for use with a 2D array texture, but has no hard dependency on it.
template <typename TBorderedImageData>
class TextureAtlasTiles {
public:
    using BorderedImageData = TBorderedImageData;
    using MipmapLevels = dang::gl::MipmapLevels<BorderedImageData>;

    /// @brief A function that is called with required size (width and height), layers and mipmap levels.
    /// @remark Returns whether actual resizing occurred.
    using TextureResizeFunction = std::function<bool(std::size_t, std::size_t, std::size_t)>;

    /// @brief A function that uploads the image to a specific position and mipmap level of a texture.
    using TextureModifyFunction = std::function<void(const BorderedImageData&, dmath::svec3, std::size_t)>;

    class TileHandle;

private:
    /// @brief Atlas information which is stored in a unique_ptr, so that it can be shared with all TileData instances.
    struct AtlasInfo {
        std::size_t atlas_size;
    };

    /// @brief Information about the placement of a tile, including whether it has been written to the texture yet.
    struct TilePlacement {
        /// @brief The index of this tile in the layer.
        std::size_t index = 0;
        /// @brief The position where the write this tile in the array texture.
        dmath::svec3 position;
        /// @brief Whether this tile has been written to the array texture yet.
        bool written = false;

        TilePlacement() = default;

        /// @param index The index of this tile in the layer.
        /// @param layer The index of the layer itself, determining the z position.
        /// @param position The x and y coordinates of the position.
        TilePlacement(std::size_t index, dmath::svec2 position, std::size_t layer)
            : index(index)
            , position(position.x(), position.y(), layer)
        {}

        /// @brief The position on the given mipmap layer.
        auto pixelPos(std::size_t mipmap_layer = 0) const { return position.xy() >> mipmap_layer; }

        /// @brief The position and atlas layer on the given mipmap layer.
        auto pixelPosAndLayer(std::size_t mipmap_layer = 0) const
        {
            auto [x, y] = pixelPos(mipmap_layer);
            return dmath::svec3(x, y, position.z());
        }
    };

    /// @brief Contains data about a single texture tile on a layer.
    struct TileData {
        MipmapLevels mipmap_levels;
        TilePlacement placement;
        std::shared_ptr<const AtlasInfo> atlas_info;

        TileData(MipmapLevels&& mipmap_levels, std::shared_ptr<const AtlasInfo> atlas_info)
            : mipmap_levels(std::move(mipmap_levels))
            , atlas_info(std::move(atlas_info))
        {}

        // Delete copy AND move, as pointers to TileData need to remain valid (TileHandle relies on this).
        TileData(const TileData&) = delete;
        TileData(TileData&&) = delete;
        TileData& operator=(const TileData&) = delete;
        TileData& operator=(TileData&&) = delete;
    };

    /// @brief A single layer in the array texture, storing a list of references to tiles.
    class Layer {
    public:
        /// @brief Creates a new layer with the given tile size, specified as log2.
        explicit Layer(const dmath::svec2& tile_size_log2, std::size_t max_texture_size)
            : tile_size_log2_(tile_size_log2)
            , max_tiles_(calculateMaxTiles(max_texture_size))
        {}

        Layer(const Layer&) = delete;
        Layer(Layer&&) = default;
        Layer& operator=(const Layer&) = delete;
        Layer& operator=(Layer&&) = default;

        /// @brief Returns the log2 of the pixel size of a tile.
        dmath::svec2 tileSizeLog2() const { return tile_size_log2_; }
        /// @brief Returns the pixel size of a single tile.
        dmath::svec2 tileSize() const
        {
            return {std::size_t{1} << tile_size_log2_.x(), std::size_t{1} << tile_size_log2_.y()};
        }

        /// @brief Calculates the required grid size (for the longer side) to fit all tiles.
        std::size_t requiredGridSizeLog2() const
        {
            if (tiles_.empty())
                return 0;
            auto diff_log2 = dutils::absoluteDifference(tile_size_log2_.x(), tile_size_log2_.y());
            auto square_tiles = ((tiles_.size() - 1) >> diff_log2) + 1;
            return (dutils::ilog2ceil(square_tiles) + 1) >> 1;
        }

        /// @brief Calculates the required texture size to fit all tiles.
        std::size_t requiredTextureSize() const { return tileSize().maxValue() << requiredGridSizeLog2(); }

        /// @brief Whether the grid is empty.
        bool empty() const { return tiles_.empty(); }

        /// @brief Whether the grid is filled completely.
        bool full() const { return first_free_tile_ == max_tiles_; }

        /// @brief Places a single tile in the grid, filling potential gaps that appeared after removing tiles.
        void addTile(TileData& tile, std::size_t layer)
        {
            assert(!full());
            tile.placement = TilePlacement(first_free_tile_, tileSize() * indexToPosition(first_free_tile_), layer);

            // find next free tile
            if (first_free_tile_ < tiles_.size()) {
                auto first = begin(tiles_) + first_free_tile_ + 1;
                auto iter = std::find(first, end(tiles_), &tile);
                first_free_tile_ = static_cast<std::size_t>(std::distance(begin(tiles_), iter));
            }
            else {
                tiles_.push_back(&tile);
                first_free_tile_++;
            }
        }

        /// @brief Removes a single tile, opening a gap, as all other tiles stay untouched.
        void removeTile(TileData& tile)
        {
            auto index = tile.placement.index;
            first_free_tile_ = std::min(first_free_tile_, index);
            tiles_[index] = nullptr;
            auto last_not_null = std::find_if(rbegin(tiles_), rend(tiles_), [](auto tile) { return tile != nullptr; });
            tiles_.erase(last_not_null.base(), end(tiles_));
        }

        /// @brief Draws all tiles that haven't been written yet and optionally frees their resources.
        void drawTiles(const TextureModifyFunction& modify, bool freeze) const
        {
            for (auto tile : tiles_) {
                if (tile == nullptr)
                    continue;
                if (!tile->placement.written) {
                    drawTile(*tile, modify);
                    assert(tile->placement.written);
                }
                if (freeze) {
                    for (auto& mipmap_level : tile->mipmap_levels)
                        mipmap_level.free();
                }
            }
        }

        /// @brief Shifts all tiles in the layer down by one.
        void shiftDown()
        {
            for (auto tile : tiles_)
                if (tile)
                    tile->placement.position.z()--;
        }

    private:
        /// @brief Draws a single tile onto the texture.
        void drawTile(TileData& tile, const TextureModifyFunction& modify) const
        {
            for (std::size_t mipmap_level = 0; mipmap_level < tile.mipmap_levels.count(); mipmap_level++) {
                const auto& bordered_image = tile.mipmap_levels[mipmap_level];
                assert(bordered_image);
                modify(bordered_image, tile.placement.pixelPosAndLayer(mipmap_level), mipmap_level);
            }
            tile.placement.written = true;
        }

        /// @brief Returns the maximum number of tiles, that can fit in a square texture of the given size.
        std::size_t calculateMaxTiles(std::size_t max_texture_size) const
        {
            assert(tileSize().maxValue() <= max_texture_size);
            auto x_tiles = std::size_t{1} << dutils::ilog2(max_texture_size >> tile_size_log2_.x());
            auto y_tiles = std::size_t{1} << dutils::ilog2(max_texture_size >> tile_size_log2_.y());
            return x_tiles * y_tiles;
        }

        /// @brief Inverse pairing function, returning only even/odd bits as x/y.
        dmath::svec2 indexToPosition(std::size_t index)
        {
            auto size_diff_log2 = dutils::absoluteDifference(tile_size_log2_.x(), tile_size_log2_.y());
            auto flip = tile_size_log2_.x() < tile_size_log2_.y();
            auto x = dutils::removeOddBits(index >> size_diff_log2);
            auto y = dutils::removeOddBits(index >> (size_diff_log2 + 1));
            y <<= size_diff_log2;
            y |= index & ~(~std::size_t{0} << size_diff_log2);
            return flip ? dmath::svec2(y, x) : dmath::svec2(x, y);
        }

        /// @brief Pairing function, interleaving x/y as odd/even bits of the resulting integer.
        std::size_t positionToIndex(dmath::svec2 position)
        {
            auto size_diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
            auto flip = tile_size_log2_.x() < tile_size_log2_.y();
            position = flip ? position.yx() : position;
            auto result = position.y() & ~(~std::size_t{0} << size_diff_log2);
            position.y() >>= size_diff_log2;
            result |= dutils::interleaveZeros(position.x()) << size_diff_log2;
            result |= dutils::interleaveZeros(position.y()) << (size_diff_log2 + 1);
            return result;
        }

        dmath::svec2 tile_size_log2_;
        std::vector<TileData*> tiles_;
        std::size_t first_free_tile_ = 0;
        std::size_t max_tiles_;
    };

public:
    /// @brief A smart handle to a tile, which is invalidated when the tile is removed.
    class TileHandle {
    public:
        friend TextureAtlasTiles;
        friend TileData;

        TileHandle() = default;

        void reset() noexcept { data_.reset(); }

        [[nodiscard]] explicit operator bool() const noexcept { return data_ != nullptr; }

        [[nodiscard]] friend bool operator==(const TileHandle& lhs, const TileHandle& rhs) noexcept
        {
            return lhs.data_ == rhs.data_;
        }

        [[nodiscard]] friend bool operator!=(const TileHandle& lhs, const TileHandle& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        auto atlasPixelSize(std::size_t mipmap_layer = 0) const
        {
            return dataOrThrow().atlas_info->atlas_size >> mipmap_layer;
        }

        auto pixelPos(std::size_t mipmap_layer = 0) const { return dataOrThrow().placement.pixelPos(mipmap_layer); }

        auto pixelSize(std::size_t mipmap_layer = 0) const { return dataOrThrow().mipmap_levels[mipmap_layer].size(); }

        auto pixelPosAndLayer(std::size_t mipmap_layer = 0) const
        {
            return dataOrThrow().placement.pixelPosAndLayer(mipmap_layer);
        }

        auto pos() const { return static_cast<dmath::vec2>(pixelPos()) / static_cast<float>(atlasPixelSize()); }
        auto size() const { return static_cast<dmath::vec2>(pixelSize()) / static_cast<float>(atlasPixelSize()); }

        bounds2 bounds() const
        {
            auto padding = dataOrThrow().mipmap_levels[0].padding();
            auto inset = static_cast<dmath::vec2>(padding) / (2.0f * atlasPixelSize());
            auto top_left = pos();
            auto bottom_right = top_left + size();
            return bounds2(top_left, bottom_right).inset(inset);
        }

        auto layer() const { return dataOrThrow().placement.position.z(); }

        auto mipmapLevels() const { return data_->mipmap_levels.size(); }

    private:
        TileHandle(const std::shared_ptr<const TileData>& data)
            : data_(data)
        {}

        const TileData& dataOrThrow() const
        {
            if (!*this)
                throw std::invalid_argument("Tile handle is empty.");
            return *data_;
        }

        std::shared_ptr<const TileData> data_;
    };

    /// @brief Creates a new instance of TextureAtlasTiles with the given maximum dimensions.
    /// @exception std::invalid_argument if either maximum is less than zero.
    TextureAtlasTiles(const TextureAtlasLimits& limits)
        : limits_(limits)
    {}

    TextureAtlasTiles(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles(TextureAtlasTiles&&) = default;
    TextureAtlasTiles& operator=(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles& operator=(TextureAtlasTiles&&) = default;

    /// @brief Adds a new tile.
    /// @exception std::invalid_argument if the image does not contain any data.
    /// @exception std::invalid_argument if the image is too big.
    /// @exception std::length_error if a new layer would exceed the maximum layer count.
    [[nodiscard]] TileHandle add(MipmapLevels mipmap_levels)
    {
        return TileHandle(emplaceTile(std::move(mipmap_levels)));
    }

    /// @brief Checks if a tile with the given name exists.
    /// @exception std::invalid_argument if the handle is empty.
    [[nodiscard]] bool contains(const TileHandle& tile_handle) const
    {
        if (!tile_handle)
            throw std::invalid_argument("Tile handle is empty.");
        return std::find(tiles_.begin(), tiles_.end(), tile_handle.data_) != tiles_.end();
    }

    /// @brief Removes the given tile, returning false if it does not belong to this atlas.
    /// @exception std::invalid_argument if the handle is empty.
    bool tryRemove(const TileHandle& tile_handle)
    {
        if (!tile_handle)
            throw std::invalid_argument("Tile handle is empty.");
        return removeTile(tile_handle.data_);
    }

    /// @brief Removes the tile with the given handle.
    /// @exception std::invalid_argument if the handle is empty.
    /// @exception std::invalid_argument if the tile does not belong to this atlas.
    void remove(const TileHandle& tile_handle)
    {
        if (!tryRemove(tile_handle))
            throw std::invalid_argument("Tile does not belong to this atlas.");
    }

    /// @brief Calls "resize" with the current size and uses "modify" to upload the texture data.
    void updateTexture(const TextureResizeFunction& resize, const TextureModifyFunction& modify)
    {
        ensureTextureSize(resize);
        for (auto& layer : layers_)
            layer.drawTiles(modify, false);
    }

    /// @brief Similar to updateTexture, but also frees image data and returns a frozen atlas.
    [[nodiscard]] FrozenTextureAtlasTiles<BorderedImageData> freeze(const TextureResizeFunction& resize,
                                                                    const TextureModifyFunction& modify) &&
    {
        ensureTextureSize(resize);
        for (auto& layer : layers_)
            layer.drawTiles(modify, true);
        return FrozenTextureAtlasTiles<BorderedImageData>(std::move(*this));
    }

    // Not really useful outside of debugging and unit-test:

    /// @brief The total number of tiles on the atlas.
    std::size_t size() const { return tiles_.size(); }

    /// @brief Whether there are no tiles on the atlas.
    bool empty() const { return tiles_.empty(); }

private:
    /// @brief Calls "resize" to resize the texture and invalidates all tiles if a resize occurred.
    void ensureTextureSize(const TextureResizeFunction& resize)
    {
        auto required_size = maxLayerSize();
        if (required_size == 0)
            return;
        auto layers = layers_.size();
        auto mipmap_levels = maxMipmapLevels(required_size);
        if (!resize(required_size, layers, mipmap_levels))
            return;
        for (const auto& tile : tiles_)
            tile->placement.written = false;
    }

    /// @brief Finds the maximum layer size.
    std::size_t maxLayerSize() const
    {
        std::size_t result = 0;
        for (auto& layer : layers_)
            result = std::max(result, layer.requiredTextureSize());
        return result;
    }

    using LayerResult = std::pair<TextureAtlasTiles<BorderedImageData>::Layer*, std::size_t>;

    /// @brief Returns a pointer to a (possibly newly created) layer for the given tile size and its index.
    /// @remark The pointer can be null, in which case a new layer would have exceeded the maximum layer count.
    LayerResult layerForTile(const dmath::svec2& size)
    {
        auto tile_size_log2 = dmath::svec2(static_cast<std::size_t>(dutils::ilog2ceil(size.x())),
                                           static_cast<std::size_t>(dutils::ilog2ceil(size.y())));
        auto layer_iter = std::find_if(begin(layers_), end(layers_), [&](const Layer& layer) {
            return !layer.full() && layer.tileSizeLog2() == tile_size_log2;
        });
        auto layer_index = static_cast<std::size_t>(std::distance(begin(layers_), layer_iter));
        if (layer_iter != end(layers_))
            return {&*layer_iter, layer_index};
        if (layer_index >= limits_.max_layer_count)
            return {nullptr, 0};
        return {&layers_.emplace_back(tile_size_log2, limits_.max_texture_size), layer_index};
    }

    /// @brief Creates a new tile and adds it to a (possibly newly created) layer.
    /// @exception std::invalid_argument if the image does not contain any data.
    /// @exception std::invalid_argument if the image is too big.
    /// @exception std::length_error if a new layer would exceed the maximum layer count.
    const std::shared_ptr<TileData>& emplaceTile(MipmapLevels&& mipmap_levels)
    {
        if (!mipmap_levels.fullImage())
            throw std::invalid_argument("Image does not contain data.");

        auto size = mipmap_levels.fullImage().size();

        if (size.greaterThan(limits_.max_texture_size).any())
            throw std::invalid_argument("Image is too big for texture atlas. (" + size.format() + " > " +
                                        std::to_string(limits_.max_texture_size) + ")");

        auto [layer, index] = layerForTile(size);
        if (!layer)
            throw std::length_error("Too many texture atlas layers. (max " + std::to_string(limits_.max_layer_count) +
                                    ")");

        const auto& tile = tiles_.emplace_back(std::make_shared<TileData>(std::move(mipmap_levels), atlas_info_));
        layer->addTile(*tile, index);
        atlas_info_->atlas_size = maxLayerSize();
        return tile;
    }

    /// @brief Removes a tile from the atlas and returns true if it existed.
    bool removeTile(const std::shared_ptr<const TileData>& tile_data)
    {
        auto iter = std::find(tiles_.begin(), tiles_.end(), tile_data);
        if (iter == tiles_.end())
            return false;
        auto tile_ptr = *iter;
        auto layer_index = tile_ptr->placement.position.z();
        auto& layer = layers_[layer_index];
        layer.removeTile(*tile_ptr);
        tiles_.erase(iter);
        if (layer.empty()) {
            layers_.erase(begin(layers_) + layer_index);
            for (auto layer_iter = begin(layers_) + layer_index; layer_iter != end(layers_); layer_iter++)
                layer_iter->shiftDown();
        }
        atlas_info_->atlas_size = maxLayerSize();
        return true;
    }

    TextureAtlasLimits limits_;
    std::shared_ptr<AtlasInfo> atlas_info_ = std::make_shared<AtlasInfo>();
    std::vector<std::shared_ptr<TileData>> tiles_;
    std::vector<Layer> layers_;
};

/// @brief A facade over a texture atlas, whose image data has been freed, preventing further modifications.
template <typename TBorderedImageData>
class FrozenTextureAtlasTiles {
public:
    using BorderedImageData = TBorderedImageData;
    using TileHandle = typename TextureAtlasTiles<BorderedImageData>::TileHandle;

    FrozenTextureAtlasTiles(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles(FrozenTextureAtlasTiles&&) = default;
    FrozenTextureAtlasTiles& operator=(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles& operator=(FrozenTextureAtlasTiles&&) = default;

    friend class TextureAtlasTiles<BorderedImageData>;

    [[nodiscard]] bool contains(const TileHandle& tile_handle) const { return tiles_.contains(tile_handle); }

private:
    FrozenTextureAtlasTiles(TextureAtlasTiles<BorderedImageData>&& tiles)
        : tiles_(std::move(tiles))
    {}

    TextureAtlasTiles<BorderedImageData> tiles_;
};

} // namespace dang::gl
