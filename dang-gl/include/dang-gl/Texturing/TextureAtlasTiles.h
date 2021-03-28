#pragma once

#include "dang-gl/Image/Image.h"
#include "dang-gl/Math/MathTypes.h"

namespace dang::gl {

class FrozenTextureAtlasTiles;

/// @brief Can store a large number of named textures in multiple layers of grids.
/// @remark Meant for use with a 2D array texture, but has no hard dependency on it.
/// @remark Supports automatic border generation on only positive or all sides.
class TextureAtlasTiles {
public:
    /// @brief On which sides of a texture to copy the opposite side for better tilling.
    enum class TileBorderGeneration { None, Positive, All };

    /// @brief A function that is called with required size (width and height), layers and mipmap levels.
    /// @remarks Returns whether actual resizing occurred.
    using TextureResizeFunction = std::function<bool(GLsizei, GLsizei, GLsizei)>;

    /// @brief A function that uploads the image to a specific position and mipmap level of a texture.
    using TextureModifyFunction = std::function<void(const Image2D&, ivec3, GLint)>;

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
        TilePlacement(std::size_t index, svec2 position, GLsizei layer);
    };

    /// @brief Contains data about a single texture tile on a layer.
    struct TileData {
        using TileHandles = std::vector<TileHandle*>;

        mutable TileHandles handles;
        std::string name;
        Image2D image;
        TileBorderGeneration border;
        TilePlacement placement;

        TileData(std::string&& name, Image2D&& image, TileBorderGeneration border);
        ~TileData();

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
        explicit Layer(const svec2& tile_size_log2, std::size_t max_texture_size);

        Layer(const Layer&) = delete;
        Layer(Layer&&) = default;
        Layer& operator=(const Layer&) = delete;
        Layer& operator=(Layer&&) = default;

        /// @brief Returns the log2 of the pixel size of a tile.
        svec2 tileSizeLog2() const;
        /// @brief Returns the pixel size of a single tile.
        svec2 tileSize() const;
        /// @brief Calculates the required grid size (for the longer side) to fit all tiles.
        GLsizei requiredGridSizeLog2() const;
        /// @brief Calculates the required texture size to fit all tiles.
        GLsizei requiredTextureSize() const;

        /// @brief Whether the grid is filled completely.
        bool full() const;

        /// @brief Places a single tile in the grid, filling potential gaps that appeared after removing tiles.
        void addTile(TileData& tile, GLsizei layer);
        /// @brief Removes a single tile, opening a gap, as all other tiles stay untouched.
        void removeTile(TileData& tile);

        /// @brief Draws all tiles that haven't been written yet and optionally frees their resources.
        void drawTiles(const TextureModifyFunction& modify, bool freeze) const;

    private:
        /// @brief Draws a single tile onto the texture, also taking the tiles border generation into account.
        void drawTile(TileData& tile, const TextureModifyFunction& modify) const;

        /// @brief Returns the maximum number of tiles, that can fit in a square texture of the given size.
        std::size_t calculateMaxTiles(std::size_t max_texture_size) const;

        /// @brief Inverse pairing function, returning only even/odd bits as x/y.
        svec2 indexToPosition(std::size_t index);
        /// @brief Pairing function, interleaving x/y as odd/even bits of the resulting integer.
        std::size_t positionToIndex(svec2 position);

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
        ~TileHandle() noexcept;

        TileHandle(const TileHandle& tile_handle);
        TileHandle(TileHandle&& tile_handle) noexcept;
        TileHandle& operator=(const TileHandle& tile_handle);
        TileHandle& operator=(TileHandle&& tile_handle) noexcept;

        void reset() noexcept;

        [[nodiscard]] operator bool() const noexcept;

        [[nodiscard]] const std::string& name() const noexcept;

        friend bool operator==(const TileHandle& lhs, const TileHandle& rhs) noexcept;
        friend bool operator!=(const TileHandle& lhs, const TileHandle& rhs) noexcept;

    private:
        TileData::TileHandles::iterator find() const;

        TileHandle(const TileData* data);

        const TileData* data_ = nullptr;
    };

    /// @brief Creates a new instance of TextureAtlasTiles with the given maximum dimensions.
    TextureAtlasTiles(GLsizei max_texture_size, GLsizei max_layer_count);

    TextureAtlasTiles(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles(TextureAtlasTiles&&) = default;
    TextureAtlasTiles& operator=(const TextureAtlasTiles&) = delete;
    TextureAtlasTiles& operator=(TextureAtlasTiles&&) = default;

    /// @brief Guesses a generation method for a given image size.
    /// @remark Gives the method that will result in a final power of two size.
    TileBorderGeneration guessTileBorderGeneration(GLsizei size) const;

    /// @brief Guesses a generation method for a given image size.
    /// @remark Gives the method that will result in a final power of two size.
    TileBorderGeneration guessTileBorderGeneration(svec2 size) const;

    /// @brief Adds the given border generation to the size.
    static GLsizei sizeWithBorder(GLsizei size, TileBorderGeneration border);
    /// @brief Adds the given border generation to both components of the size.
    static svec2 sizeWithBorder(svec2 size, TileBorderGeneration border);

    /// @brief The current default border generation method.
    TileBorderGeneration defaultBorderGeneration() const;
    /// @brief Sets the default border generation method.
    void setDefaultBorderGeneration(TileBorderGeneration border);

    /// @brief Adds a new tile with a given name and border generation.
    /// @remark Returns false if the given name is already in use.
    bool add(std::string name, Image2D image, std::optional<TileBorderGeneration> border = std::nullopt);
    /// @brief Adds a new tile with a given name and border generation and returns a handle to it.
    /// @remark Returns an empty handle if the given name is already in use.
    [[nodiscard]] TileHandle addWithHandle(std::string name,
                                           Image2D image,
                                           std::optional<TileBorderGeneration> border = std::nullopt);

    /// @brief Checks if a tile with the given name exists.
    [[nodiscard]] bool exists(const std::string& name) const;
    /// @brief Returns a (possibly empty) handle to the tile with the given name.
    [[nodiscard]] TileHandle operator[](const std::string& name) const;

    /// @brief Removes the tile with the given name.
    /// @remark Returns false if there is no tile with the given name.
    bool remove(const std::string& name);

    /// @brief Calls "resize" with the current size and uses "modify" to upload the texture data.
    void updateTexture(const TextureResizeFunction& resize, const TextureModifyFunction& modify);
    /// @brief Similar to updateTexture, but also frees image data and returns a frozen atlas.
    FrozenTextureAtlasTiles freeze(const TextureResizeFunction& resize, const TextureModifyFunction& modify) &&;

private:
    /// @brief Calls "resize" to resize the texture and invalidates all tiles if a resize occurred.
    void ensureTextureSize(const TextureResizeFunction& resize);

    /// @brief Finds the maximum layer size.
    GLsizei maxLayerSize() const;

    /// @brief Returns a index and pointer to a (possibly newly created) layer for the given tile.
    std::pair<std::size_t, TextureAtlasTiles::Layer*> layerForTile(const TileData& tile);

    using Tiles = std::unordered_map<std::string, TileData>;
    using EmplaceResult = std::pair<TileData*, bool>;

    /// @brief Creates a new tile and adds it to a (possibly newly created) layer.
    EmplaceResult emplaceTile(std::string&& name,
                              Image2D&& image,
                              std::optional<TileBorderGeneration> border = std::nullopt);

    GLsizei max_texture_size_;
    GLsizei max_layer_count_;
    Tiles tiles_;
    std::vector<Layer> layers_;
    TileBorderGeneration default_border_ = TileBorderGeneration::None;
};

/// @brief A facade over a texture atlas, whose image data has been freed, preventing further modifications.
class FrozenTextureAtlasTiles {
public:
    using TileHandle = TextureAtlasTiles::TileHandle;

    FrozenTextureAtlasTiles(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles(FrozenTextureAtlasTiles&&) = default;
    FrozenTextureAtlasTiles& operator=(const FrozenTextureAtlasTiles&) = delete;
    FrozenTextureAtlasTiles& operator=(FrozenTextureAtlasTiles&&) = default;

    friend class TextureAtlasTiles;

    [[nodiscard]] bool exists(const std::string& name) const;
    [[nodiscard]] TileHandle operator[](const std::string& name) const;

private:
    FrozenTextureAtlasTiles(TextureAtlasTiles&& tiles);

    TextureAtlasTiles tiles_;
};

[[nodiscard]] bool operator==(const TextureAtlasTiles::TileHandle& lhs,
                              const TextureAtlasTiles::TileHandle& rhs) noexcept;
[[nodiscard]] bool operator!=(const TextureAtlasTiles::TileHandle& lhs,
                              const TextureAtlasTiles::TileHandle& rhs) noexcept;

} // namespace dang::gl
