#include "pch.h"

#include "PNGLoader.h"

namespace dang::gl {

PNGLoader::PNGLoader()
try
    : png_ptr_(initCheck(png_create_read_struct(PNG_LIBPNG_VER_STRING, this, errorCallback, warningCallback)))
    , info_ptr_(initCheck(png_create_info_struct(png_ptr_))) {
}
catch (...) {
    cleanup();
    throw;
}

PNGLoader::PNGLoader(std::istream& stream)
    : PNGLoader()
{
    init(stream);
}

PNGLoader::~PNGLoader() { cleanup(); }

void PNGLoader::init(std::istream& stream)
{
    if (initialized_)
        throw PNGError("PNG already initialized.");

    initialized_ = true;

    png_set_read_fn(png_ptr_, &stream, readCallback);
    png_read_info(png_ptr_, info_ptr_);

    size_.x() = png_get_image_width(png_ptr_, info_ptr_);
    size_.y() = png_get_image_height(png_ptr_, info_ptr_);

    png_set_interlace_handling(png_ptr_);
}

dmath::svec2 PNGLoader::size() const { return size_; }

void PNGLoader::handleBitDepth()
{
    if (color_type_ == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr_);
        color_type_ = PNG_COLOR_TYPE_RGB;
        bit_depth_ = 8;
        if (png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_ptr_);
            color_type_ |= PNG_COLOR_MASK_ALPHA;
        }
    }
    else if (bit_depth_ < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr_);
        bit_depth_ = 8;
    }
    else if (bit_depth_ == 16) {
        png_set_strip_16(png_ptr_);
        bit_depth_ = 8;
    }
}

void PNGLoader::errorCallback(png_structp, png_const_charp message) { throw PNGError(message); }

void PNGLoader::warningCallback(png_structp png_ptr, png_const_charp message)
{
    auto& png_image = *static_cast<PNGLoader*>(png_get_error_ptr(png_ptr));
    png_image.onWarning({png_image, message});
}

void PNGLoader::readCallback(png_structp png_ptr, png_bytep bytes, png_size_t size)
{
    auto& stream = *static_cast<std::ifstream*>(png_get_io_ptr(png_ptr));
    if (!stream.read(reinterpret_cast<char*>(bytes), size))
        throw PNGError("Unexpected eof while reading PNG.");
}

void PNGLoader::cleanup() { png_destroy_read_struct(&png_ptr_, &info_ptr_, nullptr); }

} // namespace dang::gl
