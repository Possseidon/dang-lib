#pragma once

#include "dang-gl/Image/Pixel.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"

#include "dang-math/vector.h"

#include "dang-utils/event.h"

namespace dang::gl {

/// @brief Thrown by the PNGLoader if libpng reports any error.
class PNGError : public std::runtime_error {
    using runtime_error::runtime_error;
};

class PNGLoader;

/// @brief A warning message with a reference to the associated PNGLoader.
struct PNGWarningInfo {
    PNGLoader& image;
    std::string message;
};

using PNGWarningEvent = dutils::Event<PNGWarningInfo>;

/// @brief Capable of loading any PNG into a given format using libpng.
class PNGLoader {
public:
    /// @brief Creates a new PNG loader without an associated stream.
    PNGLoader();
    /// @brief Immediately calls init with the given stream.
    explicit PNGLoader(std::istream& stream);
    /// @brief Cleans up the libpng handles.
    ~PNGLoader();

    PNGLoader(const PNGLoader&) = delete;
    PNGLoader(PNGLoader&&) = delete;
    PNGLoader& operator=(const PNGLoader&) = delete;
    PNGLoader& operator=(PNGLoader&&) = delete;

    /// @brief Initializes the info struct with various informations like width and height.
    /// @remark The same stream is reused for a likely read call and must therefore life long enough.
    void init(std::istream& stream);

    /// @brief After initialization, returns the width and height of the image.
    dmath::svec2 size() const;

    /// @brief Converts the data into the specified format and returns a consecutive vector of pixels.
    /// @remark Use the size method to query the width and height of the returned data.
    /// @param flip Whether to flip the top and bottom of the PNG.
    template <PixelFormat Format = PixelFormat::RGBA>
    std::vector<Pixel<Format>> read(bool flip = false);

    /// @brief While errors throw an exception, warnings simply trigger this event.
    PNGWarningEvent onWarning;

private:
    /// @brief Expands or strips the bit depth to exactly 8 bit, potentially disabling palette or adding an alpha channel in the process.
    void handleBitDepth();
    /// @brief Converts between gray and rgb values, depending on the given pixel format.
    template <PixelFormat Format>
    void handleGrayRGB();
    /// @brief Adds or strips the alpha channel, depending on the pixel format.
    template <PixelFormat Format>
    void handleAlpha();
    /// @brief Converts between RGB(A) to BGR(A), depending on the given pixel format.
    template <PixelFormat Format>
    void handleBGR();

    /// @brief Called by libpng, when an unrecoverable error occurs.
    static void errorCallback(png_structp png_ptr, png_const_charp message);
    /// @brief Called by libpng for warning messages.
    static void warningCallback(png_structp png_ptr, png_const_charp message);
    /// @brief Called by libpng to read a chunk of data from the PNG file.
    static void readCallback(png_structp png_ptr, png_bytep bytes, png_size_t size);

    /// @brief Used in initialization to check the libpng pointers.
    template <typename T>
    static T* initCheck(T* ptr)
    {
        if (ptr == nullptr)
            throw PNGError("Could not initialize libpng.");
        return ptr;
    }

    /// @brief Cleans up the libpng handles.
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
inline std::vector<Pixel<Format>> PNGLoader::read(bool flip)
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
    auto fill = [&] { return std::exchange(current, current + rowbytes); };

    if (flip)
        std::generate(offsets.rbegin(), offsets.rend(), fill);
    else
        std::generate(offsets.begin(), offsets.end(), fill);

    png_read_image(png_ptr_, offsets.data());
    png_read_end(png_ptr_, nullptr);

    return image;
}

template <PixelFormat Format>
inline void PNGLoader::handleGrayRGB()
{
    bool is_gray = color_type_ == PNG_COLOR_TYPE_GRAY || color_type_ == PNG_COLOR_TYPE_GA;

    if constexpr (Format == PixelFormat::RGB || Format == PixelFormat::BGR || Format == PixelFormat::RGBA ||
                  Format == PixelFormat::BGRA || Format == PixelFormat::RGB_INTEGER ||
                  Format == PixelFormat::BGR_INTEGER || Format == PixelFormat::RGBA_INTEGER ||
                  Format == PixelFormat::BGRA_INTEGER) {
        if (is_gray) {
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
        if (!is_gray) {
            png_set_rgb_to_gray(png_ptr_, 1, -1, -1);
            color_type_ &= ~PNG_COLOR_MASK_COLOR;
        }
    }
}

template <PixelFormat Format>
inline void PNGLoader::handleAlpha()
{
    bool has_alpha = color_type_ & PNG_COLOR_MASK_ALPHA;

    if constexpr (Format == PixelFormat::RG || Format == PixelFormat::RGBA || Format == PixelFormat::BGRA ||
                  Format == PixelFormat::RG_INTEGER || Format == PixelFormat::RGBA_INTEGER ||
                  Format == PixelFormat::BGRA_INTEGER) {
        if (!has_alpha) {
            png_set_add_alpha(png_ptr_, 0xFF, PNG_FILLER_AFTER);
            color_type_ |= PNG_COLOR_MASK_ALPHA;
        }
    }
    else {
        if (has_alpha) {
            png_set_strip_alpha(png_ptr_);
            color_type_ &= ~PNG_COLOR_MASK_ALPHA;
        }
    }
}

template <PixelFormat Format>
inline void PNGLoader::handleBGR()
{
    if constexpr (Format == PixelFormat::BGR || Format == PixelFormat::BGRA || Format == PixelFormat::BGR_INTEGER ||
                  Format == PixelFormat::BGRA_INTEGER) {
        assert(color_type_ == PNG_COLOR_TYPE_RGB || color_type_ == PNG_COLOR_TYPE_RGBA);
        png_set_bgr(png_ptr_);
    }
}

} // namespace dang::gl
