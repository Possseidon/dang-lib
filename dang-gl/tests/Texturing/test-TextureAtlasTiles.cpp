#include "dang-gl/Image/ImageBorder.h"
#include "dang-gl/Texturing/TextureAtlasTiles.h"

#include "dang-math/vector.h"

#include "dang-utils/catch2-stub-matcher.h"
#include "dang-utils/stub.h"
#include "dang-utils/utils.h"

#include "catch2/catch.hpp"

namespace dgl = dang::gl;
namespace dmath = dang::math;
namespace dutils = dang::utils;

using Catch::Generators::range;
using Catch::Matchers::Message;
using dutils::Stub;
using dutils::Matchers::Called;
using dutils::Matchers::CalledWith;

class TileData {
public:
    using Size = dgl::svec2;
    using Border = dgl::ImageBorder<>;

    TileData() = default;
    explicit TileData(const Size& size, const Border& border = {})
        : size_(size)
        , border_(border)
        , data_(true)
    {}

    explicit operator bool() const { return data_; }

    auto size() const { return static_cast<dmath::svec2>(size_); }

    void free() { data_ = false; }

    bool operator==(const TileData& other) const
    {
        return std::tie(size_, data_) == std::tie(other.size_, other.data_);
    }

    bool operator!=(const TileData& other) const { return !(*this == other); }

    const auto& border() const { return border_; }

private:
    Size size_;
    Border border_;
    bool data_ = false;
};

using TextureAtlasTiles = dgl::TextureAtlasTiles<TileData>;
using FrozenTextureAtlasTiles = dgl::FrozenTextureAtlasTiles<TileData>;

namespace Catch {

template <typename T, std::size_t v_dim>
struct is_range<dmath::Bounds<T, v_dim>> : std::false_type {};

template <>
struct StringMaker<TextureAtlasTiles::TileHandle> {
    static std::string convert(const TextureAtlasTiles::TileHandle& tile_handle)
    {
        using namespace std::literals;

        if (!tile_handle)
            return "empty tile handle"s;
        return "tile handle at "s + tile_handle.pos().format();
    }
};

} // namespace Catch

// Utility

auto atlasTiles() { return TextureAtlasTiles({16, 4}); }

auto atlasTilesWithTileHandle(const TileData::Border& border = {})
{
    auto atlas_tiles = TextureAtlasTiles({16, 4});
    auto tile_handle = atlas_tiles.add(TileData({4, 4}, border));
    return std::pair{std::move(atlas_tiles), std::move(tile_handle)};
}

auto freeze(TextureAtlasTiles atlas_tiles)
{
    auto resize = [](GLsizei, GLsizei, GLsizei) { return true; };
    auto modify = [](const TileData&, dgl::ivec3, GLint) {};
    return std::move(atlas_tiles).freeze(resize, modify);
}

auto frozenTiles() { return freeze(atlasTiles()); }

auto frozenTilesWithTileHandle()
{
    auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();
    return std::pair{freeze(std::move(atlas_tiles)), tile_handle};
}

TEST_CASE("TextureAtlasTiles can be constructed and moved.", "[texturing][texture-atlas-tiles]")
{
    SECTION("Constructing TextureAtlasTiles.")
    {
        CHECK_NOTHROW(TextureAtlasTiles({16, 4}));
        CHECK_NOTHROW(TextureAtlasTiles({0, 4}));
        CHECK_NOTHROW(TextureAtlasTiles({16, 0}));
        CHECK_NOTHROW(TextureAtlasTiles({0, 0}));
    }
    SECTION("Constructing TextureAtlasTiles with invalid limits throws an invalid_argument exception.")
    {
        CHECK_THROWS_MATCHES(
            TextureAtlasTiles({-1, 4}), std::invalid_argument, Message("Maximum texture size cannot be negative."));
        CHECK_THROWS_MATCHES(
            TextureAtlasTiles({16, -1}), std::invalid_argument, Message("Maximum layer count cannot be negative."));
    }
    SECTION("TextureAtlasTiles cannot be copied, but can be moved.")
    {
        STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<TextureAtlasTiles>);
        STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<TextureAtlasTiles>);

        STATIC_REQUIRE(std::is_move_assignable_v<TextureAtlasTiles>);
        STATIC_REQUIRE(std::is_move_constructible_v<TextureAtlasTiles>);

        auto [atlas_tiles1, tile_handle] = atlasTilesWithTileHandle();

        auto atlas_tiles2 = std::move(atlas_tiles1);

        CHECK(atlas_tiles2.size() == 1);
        CHECK(atlas_tiles2.contains(tile_handle));

        atlas_tiles1 = std::move(atlas_tiles2);

        CHECK(atlas_tiles1.size() == 1);
        CHECK(atlas_tiles1.contains(tile_handle));
    }
    SECTION("Newly created TextureAtlasTiles are empty.")
    {
        auto atlas_tiles = atlasTiles();

        CHECK(atlas_tiles.empty());
        CHECK(atlas_tiles.size() == 0);

        SECTION("Once a tile has been added, it will not be considered empty anymore.")
        {
            (void)atlas_tiles.add(TileData({4, 4}));

            CHECK_FALSE(atlas_tiles.empty());
            CHECK(atlas_tiles.size() == 1);

            (void)atlas_tiles.add(TileData({4, 4}));

            CHECK_FALSE(atlas_tiles.empty());
            CHECK(atlas_tiles.size() == 2);
        }
    }
}

TEST_CASE("TextureAtlasTiles can be filled with tiles.", "[texturing][texture-atlas-tiles]")
{
    auto atlas_tiles = TextureAtlasTiles({4, 2});
    auto tile = TileData({4, 4});
    auto wide_tile = TileData({5, 1});
    auto high_tile = TileData({1, 5});
    auto empty_tile = TileData();

    SECTION("Tiles can be added, returning a handle.")
    {
        auto tile_handle = atlas_tiles.add(tile);

        CHECK(tile_handle);
        CHECK(atlas_tiles.size() == 1);
    }
    SECTION("Adding a tile without data throws an invalid_argument exception.")
    {
        CHECK_THROWS_MATCHES(
            atlas_tiles.add(empty_tile), std::invalid_argument, Message("Image does not contain data."));

        CHECK(atlas_tiles.size() == 0);
    }
    SECTION("Adding a tile that is too big throws an invalid_argument exception.")
    {
        CHECK_THROWS_MATCHES(atlas_tiles.add(wide_tile),
                             std::invalid_argument,
                             Message("Image is too big for texture atlas. ([5, 1] > 4)"));

        CHECK_THROWS_MATCHES(atlas_tiles.add(high_tile),
                             std::invalid_argument,
                             Message("Image is too big for texture atlas. ([1, 5] > 4)"));

        CHECK(atlas_tiles.size() == 0);
    }
    SECTION("Adding a tile to an atlas that has no more free layers throws a length_error exception.")
    {
        (void)atlas_tiles.add(tile);
        (void)atlas_tiles.add(tile);

        CHECK_THROWS_MATCHES(
            atlas_tiles.add(tile), std::length_error, Message("Too many texture atlas layers. (max 2)"));

        CHECK(atlas_tiles.size() == 2);
    }
}

TEST_CASE("TextureAtlasTiles can check whether a tile handle belongs to it.", "[texturing][texture-atlas-tiles]")
{
    SECTION("Tiles only ever belong to the atlas that created them.")
    {
        auto [atlas_tiles1, tile_handle1] = atlasTilesWithTileHandle();
        auto [atlas_tiles2, tile_handle2] = atlasTilesWithTileHandle();

        CHECK(atlas_tiles1.contains(tile_handle1));
        CHECK_FALSE(atlas_tiles1.contains(tile_handle2));
        CHECK_FALSE(atlas_tiles2.contains(tile_handle1));
        CHECK(atlas_tiles2.contains(tile_handle2));
    }
    SECTION("Testing an empty tile throws an invalid_argument exception, as it cannot belong to any atlas.")
    {
        auto atlas_tiles = TextureAtlasTiles({16, 4});

        CHECK_THROWS_MATCHES(atlas_tiles.contains({}), std::invalid_argument, Message("Tile handle is empty."));
    }
}

TEST_CASE("TextureAtlasTiles allows arbitrary removal of tiles that were added previously.",
          "[texturing][texture-atlas-tiles]")
{
    auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();

    SECTION("Tiles can be removed.")
    {
        SECTION("... using the remove method.") { atlas_tiles.remove(tile_handle); }
        SECTION("... using the tryRemove method.") { CHECK(atlas_tiles.tryRemove(tile_handle)); }

        CHECK_FALSE(atlas_tiles.contains(tile_handle));
        CHECK(atlas_tiles.size() == 0);
    }
    SECTION("Removing an empty tile handle throws an invalid_argument exception.")
    {
        CHECK_THROWS_MATCHES(atlas_tiles.remove({}), std::invalid_argument, Message("Tile handle is empty."));
        CHECK_THROWS_MATCHES(atlas_tiles.tryRemove({}), std::invalid_argument, Message("Tile handle is empty."));

        CHECK(atlas_tiles.contains(tile_handle));
        CHECK(atlas_tiles.size() == 1);
    }
    SECTION("Removing a tile handle that doesn't belong to the atlas.")
    {
        auto [other_atlas_tiles, other_tile_handle] = atlasTilesWithTileHandle();

        SECTION("Throws an invalid_argument exception when using the remove method.")
        {
            CHECK_THROWS_MATCHES(atlas_tiles.remove(other_tile_handle),
                                 std::invalid_argument,
                                 Message("Tile does not belong to this atlas."));
        }
        SECTION("Returns false when using the tryRemove method.")
        {
            CHECK_FALSE(atlas_tiles.tryRemove(other_tile_handle));
        }

        CHECK(atlas_tiles.contains(tile_handle));
        CHECK(atlas_tiles.size() == 1);
    }

    INFO("Regardless, tile handles stay valid.");
    CHECK(tile_handle);
}

TEST_CASE("TextureAtlasTiles can be filled with tiles of the same size, spanning multiple layers.",
          "[texturing][texture-atlas-tiles]")
{
    auto max_texture_size = GENERATE(0, 1, 2, 4);
    auto max_layer_count = GENERATE(range(0, 5));
    auto tile_width = GENERATE(range(1, 5));
    auto tile_height = GENERATE(range(1, 5));

    auto tile_size = dgl::svec2(tile_width, tile_height);

    // Tiles are aligned on powers of two:
    auto tile_width_log2 = dutils::ilog2ceil(static_cast<unsigned>(tile_width));
    auto tile_height_log2 = dutils::ilog2ceil(static_cast<unsigned>(tile_height));
    auto tile_size_log2 = dgl::svec2(tile_width_log2, tile_height_log2);
    auto tile_size_pow2 = 1 << tile_size_log2;
    auto tile_area_pow2 = tile_size_pow2.product();

    auto layer_pixel_count = dutils::sqr(max_texture_size);
    auto tiles_per_layer = layer_pixel_count / tile_area_pow2;
    auto total_tiles = tiles_per_layer * max_layer_count;

    auto atlas_tiles = TextureAtlasTiles({max_texture_size, max_layer_count});

    if (tile_size.greaterThan(max_texture_size).any()) {
        DYNAMIC_SECTION("Atlas of size " << max_texture_size << " cannot fit any tile of size " << tile_size
                                         << ".\nAdding a single tile should throw an invalid_argument exception.")
        {
            CHECK_THROWS_AS(atlas_tiles.add(TileData(tile_size)), std::invalid_argument);
        }
    }
    else {
        DYNAMIC_SECTION("Atlas of size " << max_texture_size << " should fit " << tiles_per_layer << " tiles of size "
                                         << tile_size << " on each layer.\n"
                                         << "Atlas has " << max_layer_count << " layers and should therefore fit "
                                         << total_tiles << " tiles.")
        {
            for (GLsizei layer = 0; layer < max_layer_count; layer++) {
                std::set<dgl::svec2> tile_positions;
                for (GLsizei i = 0; i < tiles_per_layer; i++) {
                    auto tile = atlas_tiles.add(TileData(tile_size));
                    auto tile_pos = tile.pixelPos();
                    auto [_, position_is_unique] = tile_positions.insert(tile_pos);

                    CHECK(tile.layer() == layer);
                    CHECK(position_is_unique);
                    {
                        INFO("Tile position is on atlas.");
                        CHECK(tile_pos.greaterThanEqual(0).all());
                        CHECK(tile_pos.lessThan(max_texture_size).all());
                    }
                    UNSCOPED_INFO("Tile is aligned on " << tile_size_pow2 << ".");
                    CHECK(tile_pos % tile_size_pow2 == 0);
                }
            }

            SECTION("Adding one more tile should throw a length_error exception.")
            {
                CHECK_THROWS_AS(atlas_tiles.add(TileData(tile_size)), std::length_error);
            }
        }
    }
}

TEST_CASE("TextureAtlasTiles can be used to update a texture.", "[texturing]")
{
    auto resize = dutils::Stub<bool(GLsizei, GLsizei, GLsizei)>();
    resize.setInfo({"resize", {"required_size", "layer_count", "mipmap_levels"}});

    dutils::Stub<void(const TileData&, dgl::ivec3, GLint)> modify;
    modify.setInfo({"modify", {"tile_data", "offset", "mipmap_level"}});

    auto size = GENERATE(4, 8, 16);
    auto tile_size = GENERATE(1, 2, 4);
    auto layers = GENERATE(1, 2, 4);

    auto tile_count = dutils::sqr(size) / dutils::sqr(tile_size) * layers;

    auto atlas_tiles = TextureAtlasTiles({size, layers});
    for (auto i = 0; i < tile_count; i++)
        (void)atlas_tiles.add(TileData({tile_size, tile_size}));

    SECTION("Using the updateTexture method, which allows further modifications.")
    {
        atlas_tiles.updateTexture(resize, modify);
    }
    SECTION("Using the freeze method, which prevents further modifications.")
    {
        auto frozen_atlas = std::move(atlas_tiles).freeze(resize, modify);
    }

    CHECK_THAT(resize, CalledWith(resize, size, layers, 1));
    CHECK_THAT(modify, Called(modify, tile_count));

    std::set<std::pair<dgl::ivec3, GLint>> positions_and_mipmap_levels;
    for (const auto& [tile_data, offset, mipmap_level] : modify.invocations()) {
        auto [_, position_and_mipmap_level_is_unique] = positions_and_mipmap_levels.insert({offset, mipmap_level});
        CHECK(position_and_mipmap_level_is_unique);
    }
}

TEST_CASE("FrozenTextureAtlasTiles represents a frozen state of TextureAtlasTiles.",
          "[texturing][frozen-texture-atlas-tiles]")
{
    SECTION("Manually constructing FrozenTextureAtlasTiles is not allowed.")
    {
        STATIC_REQUIRE_FALSE(std::is_constructible_v<FrozenTextureAtlasTiles, TextureAtlasTiles&&>);
    }
    SECTION("It must be constructed using the freeze method.")
    {
        auto atlas_tiles = atlasTiles();

        auto frozen_tiles = freeze(std::move(atlas_tiles));
    }
    SECTION("Once frozen, related tile handles belong to the frozen atlas.")
    {
        auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();

        auto frozen_tiles = freeze(std::move(atlas_tiles));

        CHECK(frozen_tiles.contains(tile_handle));
    }
    SECTION("Like TextureAtlasTiles, it cannot be copied, but can be moved.")
    {
        STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<FrozenTextureAtlasTiles>);
        STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<FrozenTextureAtlasTiles>);

        STATIC_REQUIRE(std::is_move_constructible_v<FrozenTextureAtlasTiles>);
        STATIC_REQUIRE(std::is_move_assignable_v<FrozenTextureAtlasTiles>);

        auto [frozen_tiles, tile_handle] = frozenTilesWithTileHandle();

        auto other_frozen_tiles = std::move(frozen_tiles);
        CHECK(other_frozen_tiles.contains(tile_handle));

        frozen_tiles = std::move(other_frozen_tiles);
        CHECK(frozen_tiles.contains(tile_handle));
    }
}

TEST_CASE("TextureAtlasTiles::TileHandle provides information about a created tile.")
{
    auto empty_handle = TextureAtlasTiles::TileHandle();

    SECTION("Tile handles can be checked for validity.")
    {
        CHECK_FALSE(empty_handle);

        auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();
        CHECK(tile_handle);
    }
    SECTION("An empty tile handle throws an invalid_argument exception for most operations.")
    {
        auto msg = Message("Tile handle is empty.");
        CHECK_THROWS_MATCHES(empty_handle.atlasPixelSize(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.pixelPos(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.pixelSize(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.pos(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.size(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.bounds(), std::invalid_argument, msg);
        CHECK_THROWS_MATCHES(empty_handle.layer(), std::invalid_argument, msg);
    }
    SECTION("A valid tile handle provides information about its tile data.")
    {
        struct BorderInfo {
            dgl::ImageBorder<> border;
            dgl::bounds2 expected_bounds;
        };

        auto border_info = GENERATE(BorderInfo{dgl::ImageBorderNone{}, dgl::bounds2(0, 1)},
                                    BorderInfo{dgl::ImageBorderSolid<>{}, dgl::bounds2(0.25f, 0.75f)},
                                    BorderInfo{dgl::ImageBorderWrapBoth{}, dgl::bounds2(0.25f, 0.75f)},
                                    BorderInfo{dgl::ImageBorderWrapPositive{}, dgl::bounds2(0.125f, 0.875f)});

        auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle(border_info.border);

        CHECK(tile_handle.atlasPixelSize() == 4);
        CHECK(tile_handle.pixelPos() == dgl::svec2());
        CHECK(tile_handle.pixelSize() == dmath::svec2(4));
        CHECK(tile_handle.pos() == dgl::vec2());
        CHECK(tile_handle.size() == dgl::vec2(1));
        CHECK(tile_handle.bounds() == border_info.expected_bounds);
        CHECK(tile_handle.layer() == 0);
    }
    SECTION("Tile handles can be compared for equality.")
    {
        auto atlas_tiles = atlasTiles();
        auto tile_handle1 = atlas_tiles.add(TileData({4, 4}));
        auto tile_handle2 = atlas_tiles.add(TileData({4, 4}));

        CHECK(tile_handle1 == tile_handle1);
        CHECK(tile_handle2 == tile_handle2);
        CHECK(empty_handle == empty_handle);

        CHECK(tile_handle1 != tile_handle2);
        CHECK(tile_handle2 != tile_handle1);
        CHECK(tile_handle1 != empty_handle);
        CHECK(empty_handle != tile_handle1);
        CHECK(tile_handle2 != empty_handle);
        CHECK(empty_handle != tile_handle2);
    }
    SECTION("Tile handles behave like a reference type.")
    {
        STATIC_REQUIRE(std::is_copy_constructible_v<TextureAtlasTiles::TileHandle>);
        STATIC_REQUIRE(std::is_copy_assignable_v<TextureAtlasTiles::TileHandle>);

        STATIC_REQUIRE(std::is_move_constructible_v<TextureAtlasTiles::TileHandle>);
        STATIC_REQUIRE(std::is_move_assignable_v<TextureAtlasTiles::TileHandle>);

        auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();

        SECTION("They can be copied.")
        {
            auto other_tile_handle = tile_handle;
            CHECK(other_tile_handle == tile_handle);
            CHECK(atlas_tiles.contains(other_tile_handle));
        }
        SECTION("They can be moved.")
        {
            auto other_tile_handle = std::move(tile_handle);
            CHECK(other_tile_handle);
            CHECK(atlas_tiles.contains(other_tile_handle));
        }
    }
    SECTION("Tile handles can be reset.")
    {
        auto [atlas_tiles, tile_handle] = atlasTilesWithTileHandle();
        tile_handle.reset();
        CHECK_FALSE(tile_handle);

        SECTION("Reseting an empty tile handle does nothing.")
        {
            tile_handle.reset();
            CHECK_FALSE(tile_handle);
        }
    }
}
