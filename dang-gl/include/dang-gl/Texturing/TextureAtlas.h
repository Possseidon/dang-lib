#pragma once

#include "dang-gl/Context/State.h"
#include "dang-gl/Objects/Texture.h"

#include "dang-utils/utils.h"

namespace dang::gl {

// TODO: Create GitHub issues for these:

// TODO: Support mip-mapping.

// TODO: Support unnamed tiles.
//      -> These tiles can only be used via handle (or using SubTile, see below).
//      -> Own tiles in a vector of unique_ptr instead.
//      -> Current named tile map only references these.

// TODO: Support SubTiles (e.g. for fonts).
//      -> Tiles have an optional reference to a list of other related tiles.
//      -> These lists are owned by the atlas itself.

// TODO: Similar facility to ".info" files in Pengine to copy specific subsections from a larger image into SubTiles.
//      -> Do not add file related functions to atlas though.
//      -> Keep this ".info" thing separate and not rely on actual files at all.

// TODO: Support SubTextures (e.g. for diffuse, ambient, specular, ...).
//      -> Simply have a vector of Texture2DArray, where all SubTextures share the same coordinates.

// TODO: Actually use "max_layer_count_" and throw an error or something.

// TODO: Allow for custom "max_3d_texture_size_" to allow enforcing of smaller layers.

// TODO: Add a way to free all image data.
//      -> This has to freeze the atlas, as it won't be able to regenerate anymore.
//      -> Theoretically it might be possible to add more images until a layer is full and needs resizing...
//      -> But this might be very unpredictable, therefore just freeze it right away.
//      -> Add a function to query if it is frozen.
//      -> Throw an error if a new tile is added while the atlas is frozen.

/// @brief Can store a large number of named textures in multiple layers of grids.
/// @remark Implemented using a 2D array texture.
/// @remark Supports automatic border generation on only positive or all sides.
class TextureAtlas {
public:
    enum class TileBorderGeneration { None, Positive, All };

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
        explicit Layer(GLsizei tile_size_log2, std::size_t max_texture_size);

        Layer(const Layer&) = delete;
        Layer(Layer&&) = default;
        Layer& operator=(const Layer&) = delete;
        Layer& operator=(Layer&&) = default;

        /// @brief Returns the log2 of the pixel size of a tile.
        GLsizei tileSizeLog2() const;
        /// @brief Returns the pixel size of a single tile.
        GLsizei tileSize() const;
        /// @brief Calculates the required grid size to fit all tiles.
        GLsizei requiredGridSize() const;
        /// @brief Calculates the required texture size to fit all tiles.
        GLsizei requiredTextureSize() const;

        /// @brief Whether the grid is filled completely.
        bool full() const;

        /// @brief Places a single tile in the grid, filling potential gaps that appeared after removing tiles.
        void addTile(TileData& tile, GLsizei layer);
        /// @brief Removes a single tile, opening a gap, as all other tiles stay untouched.
        void removeTile(TileData& tile);

        /// @brief Draws all tiles that haven't been written on the given layer of the array texture.
        void drawTiles(Texture2DArray& texture) const;

    private:
        /// @brief Draws a single tile onto the texture, also taking the tiles border generation into account.
        void drawTile(TileData& tile, Texture2DArray& texture) const;

        /// @brief Inverse pairing function, returning only even/odd bits as x/y.
        static constexpr svec2 indexToPosition(std::size_t index);
        /// @brief Pairing function, interleaving x/y as odd/even bits of the resulting integer.
        static constexpr std::size_t positionToIndex(svec2 position);

        GLsizei tile_size_log2_;
        std::vector<TileData*> tiles_;
        std::size_t first_free_tile_ = 0;
        std::size_t max_tiles_;
    };

public:
    /// @brief A smart handle to a tile, which is invalidated when the tile is removed.
    class TileHandle {
    public:
        friend TextureAtlas;
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

        // TODO: More functionality, most importantly querying the texture bounds for VBO generation.

    private:
        TileData::TileHandles::iterator find() const;

        TileHandle(const TileData* data);

        const TileData* data_ = nullptr;
    };

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

    /// @brief Draws all tiles that haven't yet been written to the array texture.
    void drawTiles();

    // TODO: Some Texture2DArray related delegates for e.g. min/mag filter.
    //      -> Only a select few are probably important.
    //      -> Do not expose Texture2DArray completely.
    //      -> Add a way to send the texture to a shader uniform.

    // TODO: Temporary; remove this.
    Texture2DArray& texture() { return texture_; }

private:
    /// @brief Ensures that the texture is big enough for all layers.
    void ensureTextureSize();

    /// @brief Finds the maximum layer size.
    GLsizei maxLayerSize() const;

    /// @brief Returns a index and pointer to a (possibly newly created) layer for the given tile.
    std::pair<std::size_t, TextureAtlas::Layer*> layerForTile(const TileData& tile);

    using Tiles = std::unordered_map<std::string, TileData>;
    using EmplaceResult = std::pair<TileData*, bool>;

    /// @brief Creates a new tile and adds it to a (possibly newly created) layer.
    EmplaceResult emplaceTile(std::string&& name,
                              Image2D&& image,
                              std::optional<TileBorderGeneration> border = std::nullopt);

    GLsizei max_layer_count_ = context()->max_array_texture_layers;
    GLsizei max_texture_size_ = context()->max_3d_texture_size;
    Texture2DArray texture_;
    Tiles tiles_;
    std::vector<Layer> layers_;
    TileBorderGeneration default_border_ = TileBorderGeneration::None;
};

[[nodiscard]] bool operator==(const TextureAtlas::TileHandle& lhs, const TextureAtlas::TileHandle& rhs) noexcept;
[[nodiscard]] bool operator!=(const TextureAtlas::TileHandle& lhs, const TextureAtlas::TileHandle& rhs) noexcept;

} // namespace dang::gl
