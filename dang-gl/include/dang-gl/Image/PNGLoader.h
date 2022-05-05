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
    dmath::svec2 size(dmath::svec2 padding = {}) const;

    /// @brief Returns the total count of pixels.
    std::size_t count(dmath::svec2 padding = {}) const { return size(padding).product(); }

    /// @brief Converts the data into the specified format and returns it as a byte array.
    /// @remark Actual pixels are created inside of the byte array using placement new.
    /// @param flip Whether to flip the top and bottom of the PNG.
    /// @param pad_low Padding to add in the top left corner of the image.
    /// @param pad_low Padding to add in the bottom right corner of the image.
    template <PixelFormat v_pixel_format = PixelFormat::RGBA, std::size_t row_alignment = 4>
    std::unique_ptr<std::byte[]> read(bool flip = false, dmath::svec2 pad_low = {}, dmath::svec2 pad_high = {});

    /// @brief While errors throw an exception, warnings simply trigger this event.
    PNGWarningEvent onWarning;

private:
    /// @brief Expands or strips the bit depth to exactly 8 bit, potentially disabling palette or adding an alpha
    /// channel in the process.
    void handleBitDepth();
    /// @brief Converts between gray and rgb values, depending on the given pixel format.
    template <PixelFormat v_pixel_format>
    void handleGrayRGB();
    /// @brief Adds or strips the alpha channel, depending on the pixel format.
    template <PixelFormat v_pixel_format>
    void handleAlpha();
    /// @brief Converts between RGB(A) to BGR(A), depending on the given pixel format.
    template <PixelFormat v_pixel_format>
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

    /// @brief The width of the image in bytes.
    template <PixelFormat v_pixel_format>
    std::size_t byteWidth(std::size_t padding) const
    {
        return (size_[0] + padding) * sizeof(Pixel<v_pixel_format>);
    }

    /// @brief The width of the image in bytes, but aligned.
    template <PixelFormat v_pixel_format, std::size_t v_row_alignment>
    std::size_t alignedByteWidth(std::size_t padding) const
    {
        static_assert(v_row_alignment > 0);
        return (byteWidth<v_pixel_format>(padding) - 1) / v_row_alignment * v_row_alignment + v_row_alignment;
    }

    /// @brief The size of the image, but with width as the aligned byte width.
    template <PixelFormat v_pixel_format, std::size_t v_row_alignment>
    dmath::svec2 alignedByteSize(dmath::svec2 padding) const
    {
        auto result = size_;
        result[0] = alignedByteWidth<v_pixel_format, v_row_alignment>(padding.x());
        result[1] += padding.y();
        return result;
    }

    /// @brief The size of the image in bytes.
    template <PixelFormat v_pixel_format, std::size_t v_row_alignment>
    std::size_t byteCount(dmath::svec2 padding) const
    {
        return alignedByteSize<v_pixel_format, v_row_alignment>(padding).product();
    }

    /// @brief Cleans up the libpng handles.
    void cleanup();

    png_structp png_ptr_ = nullptr;
    png_infop info_ptr_ = nullptr;

    bool initialized_ = false;
    bool read_ = false;

    dmath::svec2 size_;

    // Keep track of modifications, as png_read_update_info can only be called once after the first call.
    png_byte color_type_ = 0;
    png_byte bit_depth_ = 0;
};

template <PixelFormat v_pixel_format, std::size_t v_row_alignment>
inline std::unique_ptr<std::byte[]> PNGLoader::read(bool flip, dmath::svec2 pad_low, dmath::svec2 pad_high)
{
    if (!initialized_)
        throw PNGError("PNG not initialized.");

    if (read_)
        throw PNGError("PNG already read.");

    read_ = true;

    color_type_ = png_get_color_type(png_ptr_, info_ptr_);
    bit_depth_ = png_get_bit_depth(png_ptr_, info_ptr_);

    handleBitDepth();
    handleGrayRGB<v_pixel_format>();
    handleAlpha<v_pixel_format>();
    handleBGR<v_pixel_format>();

    png_read_update_info(png_ptr_, info_ptr_);

    if (color_type_ != png_get_color_type(png_ptr_, info_ptr_))
        throw PNGError("PNG color_type mismatch");
    if (bit_depth_ != png_get_bit_depth(png_ptr_, info_ptr_))
        throw PNGError("PNG bit_depth mismatch");

    png_size_t rowbytes = png_get_rowbytes(png_ptr_, info_ptr_);
    if (rowbytes != size_.x() * pixel_format_component_count_v<v_pixel_format>)
        throw PNGError("Cannot convert PNG to correct format.");

    auto padding = pad_low + pad_high;
    auto padded_size = size(padding);

    using Pixel = Pixel<v_pixel_format>;

    auto aligned_width = alignedByteWidth<v_pixel_format, v_row_alignment>(padding.x());
    auto data_offset = pad_low.y() * aligned_width + pad_low.x() * sizeof(Pixel);
    auto image = std::make_unique<std::byte[]>(byteCount<v_pixel_format, v_row_alignment>(padding));
    auto base_ptr = reinterpret_cast<png_bytep>(image.get());

    // Fill offsets with the row-pointers to the actual image data.
    std::vector<png_bytep> offsets(size().y());

    auto fill = [&, ptr = base_ptr + data_offset]() mutable { return std::exchange(ptr, ptr + aligned_width); };

    if (flip)
        std::generate(offsets.rbegin(), offsets.rend(), fill);
    else
        std::generate(offsets.begin(), offsets.end(), fill);

    for (std::size_t y = 0; y < padded_size.y(); y++)
        std::uninitialized_default_construct_n(reinterpret_cast<Pixel*>(base_ptr + y * aligned_width), padded_size.x());

    // Make sure, the caller doesn't need to call the destructor on each pixel.
    static_assert(std::is_trivially_destructible_v<Pixel>);

    png_read_image(png_ptr_, offsets.data());
    png_read_end(png_ptr_, nullptr);

    return image;
}

template <PixelFormat v_pixel_format>
inline void PNGLoader::handleGrayRGB()
{
    bool is_gray = color_type_ == PNG_COLOR_TYPE_GRAY || color_type_ == PNG_COLOR_TYPE_GA;

    if constexpr (v_pixel_format == PixelFormat::RGB || v_pixel_format == PixelFormat::RGB_INTEGER ||
                  v_pixel_format == PixelFormat::BGR || v_pixel_format == PixelFormat::BGR_INTEGER ||
                  v_pixel_format == PixelFormat::RGBA || v_pixel_format == PixelFormat::RGBA_INTEGER ||
                  v_pixel_format == PixelFormat::BGRA || v_pixel_format == PixelFormat::BGRA_INTEGER) {
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

template <PixelFormat v_pixel_format>
inline void PNGLoader::handleAlpha()
{
    bool has_alpha = color_type_ & PNG_COLOR_MASK_ALPHA;

    if constexpr (v_pixel_format == PixelFormat::RG || v_pixel_format == PixelFormat::RG_INTEGER ||
                  v_pixel_format == PixelFormat::RGBA || v_pixel_format == PixelFormat::RGBA_INTEGER ||
                  v_pixel_format == PixelFormat::BGRA || v_pixel_format == PixelFormat::BGRA_INTEGER) {
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

template <PixelFormat v_pixel_format>
inline void PNGLoader::handleBGR()
{
    if constexpr (v_pixel_format == PixelFormat::BGR || v_pixel_format == PixelFormat::BGR_INTEGER ||
                  v_pixel_format == PixelFormat::BGRA || v_pixel_format == PixelFormat::BGRA_INTEGER) {
        assert(color_type_ == PNG_COLOR_TYPE_RGB || color_type_ == PNG_COLOR_TYPE_RGBA);
        png_set_bgr(png_ptr_);
    }
}

} // namespace dang::gl
