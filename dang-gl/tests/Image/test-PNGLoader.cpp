#include "dang-gl/Image/PNGLoader.h"

#include "catch2/catch.hpp"

namespace dgl = dang::gl;
namespace fs = std::filesystem;

template <dgl::PixelFormat format, std::size_t row_alignment>
void loadImages()
{
    auto flip = GENERATE(false, true);

    for (const auto& entry : fs::directory_iterator("PngSuite")) {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();
        if (path.extension() != ".png")
            continue;

        const auto& filename = path.filename().string();
        auto should_fail = filename.find("x") == 0;

        INFO("Loading " << filename << (should_fail ? " (should fail)" : ""))
        CAPTURE(flip, format, row_alignment);

        dgl::PNGLoader png_loader;

        png_loader.onWarning.append([&](const dgl::PNGWarningInfo& info) {
            CHECK(&info.image == &png_loader);
            if (should_fail)
                UNSCOPED_INFO(info.message);
            else
                WARN(info.message);
        });

        std::ifstream png_stream(path, std::ios::binary);

        auto read_data = [&] {
            png_loader.init(png_stream);
            png_loader.read<format, row_alignment>(flip);
        };

        if (should_fail)
            CHECK_THROWS_AS(read_data(), dgl::PNGError);
        else
            CHECK_NOTHROW(read_data());
    }
}

TEMPLATE_TEST_CASE_SIG("PNGLoader can read .png images in any format.",
                       "[image]",
                       ((dgl::PixelFormat format), format),
                       dgl::PixelFormat::RED,
                       dgl::PixelFormat::RG,
                       dgl::PixelFormat::RGB,
                       dgl::PixelFormat::BGR,
                       dgl::PixelFormat::RGBA,
                       dgl::PixelFormat::BGRA,
                       dgl::PixelFormat::RED_INTEGER,
                       dgl::PixelFormat::RG_INTEGER,
                       dgl::PixelFormat::RGB_INTEGER,
                       dgl::PixelFormat::BGR_INTEGER,
                       dgl::PixelFormat::RGBA_INTEGER,
                       dgl::PixelFormat::BGRA_INTEGER)
{
    loadImages<format, 1>();
    loadImages<format, 2>();
    loadImages<format, 4>();
    loadImages<format, 8>();
}
