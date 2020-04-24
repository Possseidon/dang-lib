#pragma once

#include "dang-math/vector.h"

#include "dang-utils/event.h"

#include "Pixel.h"

namespace dang::gl
{

/// <summary>Thrown by the PNGLoader if libpng reports any error.</summary>
class PNGError : public std::runtime_error {
    using runtime_error::runtime_error;
};

class PNGLoader;

/// <summary>A warning message with a reference to the associated PNGLoader.</summary>
struct PNGWarningInfo {
    PNGLoader& image;
    std::string message;
};

using PNGWarningEvent = dutils::Event<PNGWarningInfo>;

/// <summary>Capable of loading any PNG into a given format using libpng.</summary>
class PNGLoader {
public:
    /// <summary>Creates a new PNG loader without an associated stream.</summary>
    PNGLoader();
    /// <summary>Immediately calls init with the given stream.</summary>
    explicit PNGLoader(std::istream& stream);
    /// <summary>Cleans up the libpng handles.</summary>
    ~PNGLoader();

    PNGLoader(const PNGLoader&) = delete;
    PNGLoader(PNGLoader&&) = delete;
    PNGLoader& operator=(const PNGLoader&) = delete;
    PNGLoader& operator=(PNGLoader&&) = delete;

    /// <summary>Initializes the info struct with various informations like width and height.</summary>
    /// <remarks>The same stream is reused for a likely read call and must therefore life long enough.</remarks>
    void init(std::istream& stream);

    /// <summary>After initialization, returns the width and height of the image.</summary>
    dmath::svec2 size() const;

    /// <summary>Converts the data into the specified format and returns a consecutive vector of pixels.</summary>
    /// <remarks>Use the size method to query the width and height of the returned data.</remarks>
    template <PixelFormat Format = PixelFormat::RGBA>
    std::vector<Pixel<Format>> read();

    /// <summary>While errors throw an exception, warnings simply trigger this event.</summary>
    PNGWarningEvent onWarning;

private:
    /// <summary>Expands or strips the bit depth to exactly 8 bit, potentially disabling palette or adding an alpha channel in the process.</summary>
    void handleBitDepth();
    /// <summary>Converts between gray and rgb values, depending on the given pixel format.</summary>
    template <PixelFormat Format>
    void handleGrayRGB();
    /// <summary>Adds or strips the alpha channel, depending on the pixel format.</summary>
    template <PixelFormat Format>
    void handleAlpha();
    /// <summary>Converts between RGB(A) to BGR(A), depending on the given pixel format.</summary>
    template <PixelFormat Format>
    void handleBGR();

    /// <summary>Called by libpng, when an unrecoverable error occurs.</summary>
    static void errorCallback(png_structp png_ptr, png_const_charp message);
    /// <summary>Called by libpng for warning messages.</summary>
    static void warningCallback(png_structp png_ptr, png_const_charp message);
    /// <summary>Called by libpng to read a chunk of data from the PNG file.</summary>
    static void readCallback(png_structp png_ptr, png_bytep bytes, png_size_t size);

    /// <summary>Used in initialization to check the libpng pointers.</summary>
    template <typename T>
    static T* initCheck(T* ptr)
    {
        if (ptr == nullptr)
            throw PNGError("Could not initialize libpng.");
        return ptr;
    }

    /// <summary>Cleans up the libpng handles.</summary>
    void cleanup();

    png_structp png_ptr_ = nullptr;
    png_infop info_ptr_ = nullptr;

    bool initialized_ = false;
    bool read_ = false;

    dmath::svec2 size_;

    // png_read_update_info can only be called once after the first call, so we have to keep track of modifications ourself    
    png_byte color_type_ = 0;
    png_byte bit_depth_ = 0;
};

template <PixelFormat Format>
inline std::vector<Pixel<Format>> PNGLoader::read()
{
    if (!initialized_)
        throw PNGError("PNG not initialized.");

    if (read_)
        throw PNGError("PNG already read.");

    read_ = true;

    color_type_ = png_get_color_type(png_ptr_, info_ptr_);
    bit_depth_ = png_get_bit_depth(png_ptr_, info_ptr_);

    handleBitDepth();
    handleGrayRGB<Format>();
    handleAlpha<Format>();
    handleBGR<Format>();

    png_read_update_info(png_ptr_, info_ptr_);

    if (color_type_ != png_get_color_type(png_ptr_, info_ptr_))
        throw PNGError("PNG color_type mismatch");
    if (bit_depth_ != png_get_bit_depth(png_ptr_, info_ptr_))
        throw PNGError("PNG bit_depth mismatch");

    png_size_t rowbytes = png_get_rowbytes(png_ptr_, info_ptr_);
    if (rowbytes != size_.x() * PixelFormatInfo<Format>::ComponentCount)
        throw PNGError("Cannot convert PNG to correct format.");

    std::vector<Pixel<Format>> image(size_.product());

    // fill offsets with the row-pointers to the actual image data
    std::vector<png_bytep> offsets(size_.y());
    png_bytep current = reinterpret_cast<png_bytep>(image.data());
    std::generate(offsets.begin(), offsets.end(), [&] { return std::exchange(current, current + rowbytes); });

    png_read_image(png_ptr_, offsets.data());
    png_read_end(png_ptr_, nullptr);

    return image;
}

template <PixelFormat Format>
inline void PNGLoader::handleGrayRGB()
{
    auto is_gray = [&]() -> bool {
        return color_type_ == PNG_COLOR_TYPE_GRAY || color_type_ == PNG_COLOR_TYPE_GA;
    };

    if constexpr (
        Format == PixelFormat::RGB || Format == PixelFormat::BGR ||
        Format == PixelFormat::RGBA || Format == PixelFormat::BGRA ||
        Format == PixelFormat::RGB_INTEGER || Format == PixelFormat::BGR_INTEGER ||
        Format == PixelFormat::RGBA_INTEGER || Format == PixelFormat::BGRA_INTEGER) {
        if (is_gray()) {
            png_set_gray_to_rgb(png_ptr_);
            color_type_ |= PNG_COLOR_MASK_COLOR;
        }
    }
    else {
        // error_action:
        //  1 -> ignore mismatched RGB
        //  2 -> warning
        //  3 -> error
        // for 1 and 2 checkable with png_get_rgb_to_gray_status

        // red/green weight:
        // -1 -> default values for good rgb to gray conversion
        if (!is_gray()) {
            png_set_rgb_to_gray(png_ptr_, 1, -1, -1);
            color_type_ &= ~PNG_COLOR_MASK_COLOR;
        }
    }
}

template <PixelFormat Format>
inline void PNGLoader::handleAlpha()
{
    auto has_alpha = [&]() -> bool {
        return color_type_ & PNG_COLOR_MASK_ALPHA;
    };

    if constexpr (
        Format == PixelFormat::RG || Format == PixelFormat::RGBA || Format == PixelFormat::BGRA ||
        Format == PixelFormat::RG_INTEGER || Format == PixelFormat::RGBA_INTEGER || Format == PixelFormat::BGRA_INTEGER) {
        if (!has_alpha()) {
            png_set_add_alpha(png_ptr_, 0xFF, PNG_FILLER_AFTER);
            color_type_ |= PNG_COLOR_MASK_ALPHA;
        }
    }
    else {
        if (has_alpha()) {
            png_set_strip_alpha(png_ptr_);
            color_type_ &= ~PNG_COLOR_MASK_ALPHA;
        }
    }
}

template <PixelFormat Format>
inline void PNGLoader::handleBGR()
{
    if constexpr (
        Format == PixelFormat::BGR ||
        Format == PixelFormat::BGRA ||
        Format == PixelFormat::BGR_INTEGER ||
        Format == PixelFormat::BGRA_INTEGER) {
        assert(color_type_ == PNG_COLOR_TYPE_RGB || color_type_ == PNG_COLOR_TYPE_RGBA);
        png_set_bgr(png_ptr_);
    }
}

}
