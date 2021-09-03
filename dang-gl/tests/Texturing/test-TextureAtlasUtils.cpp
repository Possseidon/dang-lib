#include "dang-gl/Texturing/TextureAtlasUtils.h"
#include "dang-glfw/GLFW.h"
#include "dang-glfw/Window.h"

#include "catch2/catch_test_macros.hpp"

namespace dgl = dang::gl;
namespace dglfw = dang::glfw;

TEST_CASE("TextureAtlasUtils can be used to query limits for texture atlases.",
          "[opengl][texturing][texture-atlas-utils]")
{
    dglfw::GLFW glfw;
    dglfw::WindowInfo window_info;
    window_info.visible = false;
    window_info.title = "dang-test: TextureAtlasUtils";
    dglfw::Window window(window_info);

    SECTION("For maximum texture size, if no value is given, GL_MAX_3D_TEXTURE_SIZE is returned.")
    {
        auto expected_size = static_cast<std::size_t>(dgl::context()->max_3d_texture_size);
        auto max_texture_size = dgl::TextureAtlasUtils::checkMaxTextureSize(std::nullopt);
        CHECK(max_texture_size == expected_size);

        SECTION("If the given value is in range, it is returned.")
        {
            auto checked_max_texture_size = dgl::TextureAtlasUtils::checkMaxTextureSize(max_texture_size);
            CHECK(checked_max_texture_size == max_texture_size);

            auto checked_1_texture_size = dgl::TextureAtlasUtils::checkMaxTextureSize(1);
            CHECK(checked_1_texture_size == 1);
        }
        SECTION("If the given value is out of range, an invalid_argument exception is thrown.")
        {
            CHECK_THROWS_AS(dgl::TextureAtlasUtils::checkMaxTextureSize(max_texture_size + 1), std::invalid_argument);
            CHECK_THROWS_AS(dgl::TextureAtlasUtils::checkMaxTextureSize(0), std::invalid_argument);
        }
    }

    SECTION("For maximum layer count, if no value is given, GL_MAX_ARRAY_TEXTURE_LAYERS is returned.")
    {
        auto expected_size = static_cast<std::size_t>(dgl::context()->max_3d_texture_size);
        auto max_layer_count = dgl::TextureAtlasUtils::checkMaxLayerCount(std::nullopt);
        CHECK(max_layer_count == expected_size);

        SECTION("If the given value is in range, it is returned.")
        {
            auto checked_max_layer_count = dgl::TextureAtlasUtils::checkMaxLayerCount(max_layer_count);
            CHECK(checked_max_layer_count == max_layer_count);

            auto checked_1_layer_count = dgl::TextureAtlasUtils::checkMaxLayerCount(1);
            CHECK(checked_1_layer_count == 1);
        }
        SECTION("If the given value is out of range, an invalid_argument exception is thrown.")
        {
            CHECK_THROWS_AS(dgl::TextureAtlasUtils::checkMaxLayerCount(max_layer_count + 1), std::invalid_argument);
            CHECK_THROWS_AS(dgl::TextureAtlasUtils::checkMaxLayerCount(0), std::invalid_argument);
        }
    }

    SECTION("Both functions can be combined in a single call.")
    {
        auto max_texture_size = dgl::TextureAtlasUtils::checkMaxTextureSize(std::nullopt);
        auto max_layer_count = dgl::TextureAtlasUtils::checkMaxLayerCount(std::nullopt);
        auto limits = dgl::TextureAtlasUtils::checkLimits(std::nullopt, std::nullopt);
        CHECK(limits.max_texture_size == max_texture_size);
        CHECK(limits.max_layer_count == max_layer_count);
    }
}
