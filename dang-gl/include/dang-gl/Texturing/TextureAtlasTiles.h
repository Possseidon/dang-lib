#pragma once

#include "dang-gl/Math/MathTypes.h"

#include "dang-utils/utils.h"

namespace dang::gl {

template <typename TImageData>
class FrozenTextureAtlasTiles;

/*

The ImageData concept:

- Move-constructible (preferably somewhat cheap)
- explicit operator bool() const
    -> if it contains any data
- svec2 size() const
    -> image size
- void free()
    -> frees all data, but leaves the size
- ImageData operator[](dmath::sbounds2) const
    -> returns a subsection of the image

*/

/// @brief Can store a large number of named textures in multiple layers of grids.
/// @remark Meant for use with a 2D array texture, but has no hard dependency on it.
/// @remark Supports automatic border generation on only positive or all sides.
template <typename TImageData>
class TextureAtlasTiles {
public:
    /// @brief On which sides of a texture to copy the opposite side for better tilling.
    enum class TileBorderGeneration { None, Positive, All };

    /// @brief A function that is called with required size (width and height), layers and mipmap levels.
    /// @remarks Returns whether actual resizing occurred.
    using TextureResizeFunction = std::function<bool(GLsizei, GLsizei, GLsizei)>;

    /// @brief A function that uploads the image to a specific position and mipmap level of a texture.
    using TextureModifyFunction = std::function<void(const TImageData&, ivec3, GLint)>;

    class TileHandle;

private:
    /// @brief Information about the placement of a tile, including whether it has been written to the texture yet.
    struct TilePlacement {
        /// @brief The index of this tile in the layer.
        std::size_t index = 0;
        /// @brief The position where the write this tile in the array texture.
        svec3 position;
        /// @brief Whether this tile has been written to the array texture yet.
        bool written = false;

        TilePlacement() = default;

        /// @param index The index of this tile in the layer.
        /// @param layer The index of the layer itself, determining the z position.
        /// @param position The x and y coordinates of the position.
        TilePlacement(std::size_t index, svec2 position, GLsizei layer)
            : index(index)
            , position(position.x(), position.y(), layer)
        {}
    };

    /// @brief Contains data about a single texture tile on a layer.
    struct TileData {
        using TileHandles = std::vector<TileHandle*>;

        mutable TileHandles handles;
        std::string name;
        TImageData image_data;
        TileBorderGeneration border;
        TilePlacement placement;

        TileData(std::string&& name, TImageData&& image_data, TileBorderGeneration border)
            : name(std::move(name))
            , image_data(std::move(image_data))
            , border(border)
        {}

        ~TileData()
        {
            for (auto& handle : handles)
                handle->data_ = nullptr;
            // handle->reset() also removes the entry from "handles"
            // just set data_ to nullptr instead manually
        }

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
        explicit Layer(const svec2& tile_size_log2, std::size_t max_texture_size)
            : tile_size_log2_(tile_size_log2)
            , max_tiles_(calculateMaxTiles(max_texture_size))
        {}

        Layer(const Layer&) = delete;
        Layer(Layer&&) = default;
        Layer& operator=(const Layer&) = delete;
        Layer& operator=(Layer&&) = default;

        /// @brief Returns the log2 of the pixel size of a tile.
        svec2 tileSizeLog2() const { return tile_size_log2_; }
        /// @brief Returns the pixel size of a single tile.
        svec2 tileSize() const { return {GLsizei{1} << tile_size_log2_.x(), GLsizei{1} << tile_size_log2_.y()}; }

        /// @brief Calculates the required grid size (for the longer side) to fit all tiles.
        GLsizei requiredGridSizeLog2() const
        {
            if (tiles_.empty())
                return 0;
            auto diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
            auto square_tiles = ((tiles_.size() - 1) >> diff_log2) + 1;
            return (dutils::ilog2ceil(square_tiles) + 1) >> 1;
        }

        /// @brief Calculates the required texture size to fit all tiles.
        GLsizei requiredTextureSize() const { return tileSize().maxValue() << requiredGridSizeLog2(); }

        /// @brief Whether the grid is empty.
        bool empty() const { return tiles_.empty(); }

        /// @brief Whether the grid is filled completely.
        bool full() const { return first_free_tile_ == max_tiles_; }

        /// @brief Places a single tile in the grid, filling potential gaps that appeared after removing tiles.
        void addTile(TileData& tile, GLsizei layer)
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
                if (tile->placement.written)
                    continue;
                drawTile(*tile, modify);
                assert(tile->placement.written);
                if (freeze)
                    tile->image_data.free();
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
        /// @brief Draws a single tile onto the texture, also taking the tiles border generation into account.
        void drawTile(TileData& tile, const TextureModifyFunction& modify) const
        {
            const auto& image_data = tile.image_data;
            assert(image_data);

            const auto& position = tile.placement.position;

            auto width = image_data.size().x();
            auto height = image_data.size().y();

            auto s_width = static_cast<GLsizei>(width);
            auto s_height = static_cast<GLsizei>(height);

            switch (tile.border) {
            case TileBorderGeneration::Positive: {
                // left top -> right bottom
                modify(image_data[dmath::sbounds2({0, 0}, {1, 1})], position + svec3{s_width, s_height, 0}, 0);
                // left -> right
                modify(image_data[dmath::sbounds2({0, 0}, {1, height})], position + svec3{s_width, 0, 0}, 0);
                // top -> bottom
                modify(image_data[dmath::sbounds2({0, 0}, {width, 1})], position + svec3{0, s_height, 0}, 0);

                [[fallthrough]];
            }
            case TileBorderGeneration::None: {
                // full image
                modify(image_data, position, 0);
                break;
            }
            case TileBorderGeneration::All: {
                // full image (offset by 1)
                modify(image_data, position + svec3{1, 1, 0}, 0);

                // left top -> right bottom
                modify(image_data[dmath::sbounds2({0, 0}, {1, 1})], position + svec3{s_width + 1, s_height + 1, 0}, 0);
                // right top -> left bottom
                modify(
                    image_data[dmath::sbounds2({width - 1, 0}, {width, 1})], position + svec3{0, s_height + 1, 0}, 0);
                // left bottom -> right top
                modify(
                    image_data[dmath::sbounds2({0, height - 1}, {1, height})], position + svec3{s_width + 1, 0, 0}, 0);
                // right bottom -> left top
                modify(image_data[dmath::sbounds2({width - 1, height - 1}, {width, height})],
                       position + svec3{0, 0, 0},
                       0);

                // left -> right
                modify(image_data[dmath::sbounds2({0, 0}, {1, height})], position + svec3{s_width + 1, 1, 0}, 0);
                // right -> left
                modify(image_data[dmath::sbounds2({width - 1, 0}, {width, height})], position + svec3{0, 1, 0}, 0);
                // top -> bottom
                modify(image_data[dmath::sbounds2({0, 0}, {width, 1})], position + svec3{1, s_height + 1, 0}, 0);
                // bottom -> top
                modify(image_data[dmath::sbounds2({0, height - 1}, {width, height})], position + svec3{1, 0, 0}, 0);

                break;
            }
            default:
                assert(false);
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
        svec2 indexToPosition(std::size_t index)
        {
            auto size_diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
            auto flip = tile_size_log2_.x() < tile_size_log2_.y();
            auto x = static_cast<GLsizei>(dutils::removeOddBits(index >> size_diff_log2));
            auto y = static_cast<GLsizei>(dutils::removeOddBits(index >> (size_diff_log2 + 1)));
            y <<= size_diff_log2;
            y |= index & ~(~std::size_t{0} << size_diff_log2);
            return flip ? svec2(y, x) : svec2(x, y);
        }

        /// @brief Pairing function, interleaving x/y as odd/even bits of the resulting integer.
        std::size_t positionToIndex(svec2 position)
        {
            auto size_diff_log2 = std::abs(tile_size_log2_.x() - tile_size_log2_.y());
            auto flip = tile_size_log2_.x() < tile_size_log2_.y();
            position = flip ? position.yx() : position;
            auto result = position.y() & ~(~std::size_t{0} << size_diff_log2);
            position.y() >>= size_diff_log2;
            result |= dutils::interleaveZeros(static_cast<std::size_t>(position.x())) << size_diff_log2;
            result |= dutils::interleaveZeros(static_cast<std::size_t>(position.y())) << (size_diff_log2 + 1);
            return result;
        }

        svec2 tile_size_log2_;
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
        ~TileHandle() noexcept { reset(); }

        TileHandle(const TileHandle& tile_handle)
            : TileHandle(tile_handle.data_)
        {}

        TileHandle(TileHandle&& tile_handle) noexcept
            : data_(tile_handle.data_)
        {
            if (!tile_handle)
                return;
            // Just replace the existing entry.
            *tile_handle.find() = this;
            tile_handle.data_ = nullptr;
        }

        TileHandle& operator=(const TileHandle& tile_handle)
        {
            reset();
            if (!tile_handle)
                return *this;
            data_ = tile_handle.data_;
            data_->handles.push_back(this);
            return *this;
        }

        TileHandle& operator=(TileHandle&& tile_handle) noexcept
        {
            reset();
            if (!tile_handle)
                return *this;
            data_ = tile_handle.data_;
            // Just replace the existing entry.
            *tile_handle.find() = this;
            tile_handle.data_ = nullptr;
            return *this;
        }

        void reset() noexcept
        {
            if (!*this)
                return;
            auto iter = find();
            assert(iter != end(data_->handles));
            data_->handles.erase(iter);
            data_ = nullptr;
        }

        [[nodiscard]] operator bool() const noexcept { return data_ != nullptr; }

        [[nodiscard]] const std::string& name() const noexcept { return data_->name; }

        friend bool operator==(const TileHandle& lhs, const TileHandle& rhs) noexcept;
        friend bool operator!=(const TileHandle& lhs, const TileHandle& rhs) noexcept;

    private:
        typename TileData::TileHandles::iterator find() const
        {
            return std::find(begin(data_->handles), end(data_->handles), this);
        }

        TileHandle(const TileData* data)
            : data_(data)
        {
            if (data)
                data->handles.push_back(this);
        }

        const TileData* data_ = nullptr;
    };

    /// @brief Creates a new instance of TextureAtlasTiles with the given maximum dimensions.
    /// @exception std::invalid_argument if either maximum is less than zero.
    TextureAtlasTiles(GLsizei max_texture_size, GLsizei max_layer_count)
        : max_texture_size_(max_texture_size)
        , max_layer_count_(max_layer_count)
    {
        if (max_texture_size < 0)
            throw std::invalid_argument("Maximum texture size cannot be negative.");
        if (max_layer_count < 0)
            throw std::invalid_argument("Maximum layer count cannot be negative.");
    }

    TextureAtlasTiles(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles(TextureAtlasTiles&&) = default;
    TextureAtlasTiles& operator=(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles& operator=(TextureAtlasTiles&&) = default;

    /// @brief Guesses a generation method for a given image size.
    /// @remark Gives the method that will result in a final power of two size.
    /// @exception std::invalid_argument if the size is negative.
    TileBorderGeneration guessTileBorderGeneration(GLsizei size) const
    {
        if (size < 0)
            throw std::invalid_argument("Size cannot be negative.");
        auto usize = static_cast<std::make_unsigned_t<GLsizei>>(size);
        if (dutils::popcount(usize) == 1)
            return TileBorderGeneration::None;
        if (dutils::popcount(usize + 1) == 1)
            return TileBorderGeneration::Positive;
        if (dutils::popcount(usize + 2) == 1)
            return TileBorderGeneration::All;
        return default_border_;
    }

    /// @brief Guesses a generation method for a given image size.
    /// @remark Gives the method that will result in a final power of two size.
    /// @exception std::invalid_argument if either value of size is negative.
    TileBorderGeneration guessTileBorderGeneration(svec2 size) const
    {
        return guessTileBorderGeneration(size.maxValue());
    }

    /// @brief Adds the given border generation to the size.
    static GLsizei sizeWithBorder(GLsizei size, TileBorderGeneration border)
    {
        switch (border) {
        case TileBorderGeneration::None:
            return size;
        case TileBorderGeneration::Positive:
            return size + 1;
        case TileBorderGeneration::All:
            return size + 2;
        default:
            assert(false);
            return 0;
        }
    }

    /// @brief Adds the given border generation to both components of the size.
    static svec2 sizeWithBorder(svec2 size, TileBorderGeneration border)
    {
        return {sizeWithBorder(size.x(), border), sizeWithBorder(size.y(), border)};
    }

    /// @brief The current default border generation method.
    TileBorderGeneration defaultBorderGeneration() const { return default_border_; }
    /// @brief Sets the default border generation method.
    void setDefaultBorderGeneration(TileBorderGeneration border) { default_border_ = border; }

    /// @brief Adds a new tile with a given name and border generation.
    /// @exception std::invalid_argument if the name is empty.
    /// @exception std::invalid_argument if the name already exists.
    /// @exception std::invalid_argument if the image does not contain any data.
    /// @exception std::invalid_argument if the image is too big.
    /// @exception std::length_error if a new layer would exceed the maximum layer count.
    void add(std::string name, TImageData image_data, std::optional<TileBorderGeneration> border = std::nullopt)
    {
        emplaceTile(std::move(name), std::move(image_data), border);
    }

    /// @brief Adds a new tile with a given name and border generation and returns a handle to it.
    /// @remark Returns an empty handle if the given name is already in use.
    /// @exception std::invalid_argument if the name is empty.
    /// @exception std::invalid_argument if the name already exists.
    /// @exception std::invalid_argument if the image does not contain any data.
    /// @exception std::invalid_argument if the image is too big.
    /// @exception std::length_error if a new layer would exceed the maximum layer count.
    [[nodiscard]] TileHandle addWithHandle(std::string name,
                                           TImageData image_data,
                                           std::optional<TileBorderGeneration> border = std::nullopt)
    {
        return TileHandle(emplaceTile(std::move(name), std::move(image_data), border));
    }

    /// @brief Checks if a tile with the given name exists.
    /// @exception std::invalid_argument if the name is empty.
    [[nodiscard]] bool exists(const std::string& name) const
    {
        if (name.empty())
            throw std::invalid_argument("Tile name is empty.");
        auto iter = tiles_.find(name);
        return iter != tiles_.end();
    }

    /// @brief Returns a (possibly empty) handle to the tile with the given name.
    /// @exception std::invalid_argument if the name is empty.
    [[nodiscard]] TileHandle operator[](const std::string& name) const
    {
        if (name.empty())
            throw std::invalid_argument("Tile name is empty.");
        auto iter = tiles_.find(name);
        if (iter != tiles_.end())
            return TileHandle(&iter->second);
        return TileHandle();
    }

    /// @brief Removes the tile with the given name.
    /// @remark Returns false if there is no tile with the given name.
    /// @exception std::invalid_argument if the name is empty.
    bool tryRemove(const std::string& name)
    {
        if (name.empty())
            throw std::invalid_argument("Tile name is empty.");
        auto iter = tiles_.find(name);
        if (iter == tiles_.end())
            return false;
        auto layer_index = iter->second.placement.position.z();
        auto& layer = layers_[layer_index];
        layer.removeTile(iter->second);
        tiles_.erase(iter);
        if (layer.empty()) {
            layers_.erase(begin(layers_) + layer_index);
            for (auto layer_iter = begin(layers_) + layer_index; layer_iter != end(layers_); layer_iter++)
                layer_iter->shiftDown();
        }
        return true;
    }

    /// @brief Removes the tile with the given name.
    /// @exception std::invalid_argument if the name is empty.
    /// @exception std::invalid_argument if the name doesn't exist.
    void remove(const std::string& name)
    {
        if (!tryRemove(name))
            throw std::invalid_argument("Tile with name \"" + name + "\" doesn't exist.");
    }

    /// @brief Calls "resize" with the current size and uses "modify" to upload the texture data.
    void updateTexture(const TextureResizeFunction& resize, const TextureModifyFunction& modify)
    {
        ensureTextureSize(resize);
        for (auto& layer : layers_)
            layer.drawTiles(modify, false);
    }

    /// @brief Similar to updateTexture, but also frees image data and returns a frozen atlas.
    [[nodiscard]] FrozenTextureAtlasTiles<TImageData> freeze(const TextureResizeFunction& resize,
                                                             const TextureModifyFunction& modify) &&
    {
        ensureTextureSize(resize);
        for (auto& layer : layers_)
            layer.drawTiles(modify, true);
        return FrozenTextureAtlasTiles<TImageData>(std::move(*this));
    }

private:
    /// @brief Calls "resize" to resize the texture and invalidates all tiles if a resize occurred.
    void ensureTextureSize(const TextureResizeFunction& resize)
    {
        auto required_size = maxLayerSize();
        auto layers = static_cast<GLsizei>(layers_.size());
        if (!resize(required_size, layers, 1))
            return;
        for (auto& [name, tile] : tiles_)
            tile.placement.written = false;
    }

    /// @brief Finds the maximum layer size.
    GLsizei maxLayerSize() const
    {
        GLsizei result = 0;
        for (auto& layer : layers_)
            result = std::max(result, layer.requiredTextureSize());
        return result;
    }

    using LayerResult = std::pair<TextureAtlasTiles<TImageData>::Layer*, std::size_t>;

    /// @brief Returns a pointer to a (possibly newly created) layer for the given tile size and its index.
    /// @remark The pointer can be null, in which case a new layer would have exceeded the maximum layer count.
    LayerResult layerForTile(const svec2& size)
    {
        auto unsigned_width = static_cast<std::make_unsigned_t<GLsizei>>(size.x());
        auto unsigned_height = static_cast<std::make_unsigned_t<GLsizei>>(size.y());
        auto tile_size_log2 = svec2(static_cast<GLsizei>(dutils::ilog2ceil(unsigned_width)),
                                    static_cast<GLsizei>(dutils::ilog2ceil(unsigned_height)));
        auto layer_iter = std::find_if(begin(layers_), end(layers_), [&](const Layer& layer) {
            return !layer.full() && layer.tileSizeLog2() == tile_size_log2;
        });
        auto layer_index = std::distance(begin(layers_), layer_iter);
        if (layer_iter != end(layers_))
            return {&*layer_iter, layer_index};
        if (layer_index >= max_layer_count_)
            return {nullptr, 0};
        return {&layers_.emplace_back(tile_size_log2, max_texture_size_), layer_index};
    }

    /// @brief Creates a new tile and adds it to a (possibly newly created) layer.
    /// @exception std::invalid_argument if the name is empty.
    /// @exception std::invalid_argument if the name already exists.
    /// @exception std::invalid_argument if the image does not contain any data.
    /// @exception std::invalid_argument if the image is too big.
    /// @exception std::length_error if a new layer would exceed the maximum layer count.
    const TileData* emplaceTile(std::string&& name,
                                TImageData&& image_data,
                                std::optional<TileBorderGeneration> border = std::nullopt)
    {
        if (name.empty())
            throw std::invalid_argument("Tile name is empty.");

        if (!image_data)
            throw std::invalid_argument("Image does not contain data.");
        if (image_data.size().greaterThan(max_texture_size_).any())
            throw std::invalid_argument("Image is too big for texture atlas. (" + image_data.size().format() + " > " +
                                        std::to_string(max_texture_size_) + ")");

        auto actual_border = border ? *border : guessTileBorderGeneration(static_cast<svec2>(image_data.size()));
        auto [layer, index] = layerForTile(sizeWithBorder(static_cast<svec2>(image_data.size()), actual_border));
        if (!layer)
            throw std::length_error("Too many texture atlas layers. (max " + std::to_string(max_layer_count_) + ")");

        // Explicit copy to have two strings to move from.
        std::string key = name;
        auto [iter, ok] = tiles_.try_emplace(std::move(key), std::move(name), std::move(image_data), actual_border);
        if (!ok)
            throw std::invalid_argument("Tile with name \"" + iter->first + "\" already exists.");
        auto& tile = iter->second;
        layer->addTile(tile, index);
        return &tile;
    }

    GLsizei max_texture_size_;
    GLsizei max_layer_count_;
    std::unordered_map<std::string, TileData> tiles_;
    std::vector<Layer> layers_;
    TileBorderGeneration default_border_ = TileBorderGeneration::None;
};

/// @brief A facade over a texture atlas, whose image data has been freed, preventing further modifications.
template <typename TImageData>
class FrozenTextureAtlasTiles {
public:
    using TileHandle = typename TextureAtlasTiles<TImageData>::TileHandle;

    FrozenTextureAtlasTiles(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles(FrozenTextureAtlasTiles&&) = default;
    FrozenTextureAtlasTiles& operator=(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles& operator=(FrozenTextureAtlasTiles&&) = default;

    friend class TextureAtlasTiles<TImageData>;

    [[nodiscard]] bool exists(const std::string& name) const { return tiles_.exists(name); }
    [[nodiscard]] TileHandle operator[](const std::string& name) const { return tiles_[name]; }

private:
    FrozenTextureAtlasTiles(TextureAtlasTiles<TImageData>&& tiles)
        : tiles_(std::move(tiles))
    {}

    TextureAtlasTiles<TImageData> tiles_;
};

template <typename TImageData>
[[nodiscard]] bool operator==(const typename TextureAtlasTiles<TImageData>::TileHandle& lhs,
                              const typename TextureAtlasTiles<TImageData>::TileHandle& rhs) noexcept
{
    return lhs.data_ == rhs.data_;
}

template <typename TImageData>
[[nodiscard]] bool operator!=(const typename TextureAtlasTiles<TImageData>::TileHandle& lhs,
                              const typename TextureAtlasTiles<TImageData>::TileHandle& rhs) noexcept
{
    return !(lhs == rhs);
}

} // namespace dang::gl
