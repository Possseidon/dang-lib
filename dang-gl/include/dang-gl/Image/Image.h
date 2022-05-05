#pragma once

#include "dang-gl/Image/PNGLoader.h"
#include "dang-gl/Image/Pixel.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"
#include "dang-math/bounds.h"
#include "dang-math/vector.h"

namespace dang::gl {

/// @brief Stores pixels data for an n-dimensional image in a template specified type.
template <std::size_t v_dim,
          PixelFormat v_pixel_format = PixelFormat::RGBA,
          PixelType v_pixel_type = PixelType::UNSIGNED_BYTE,
          std::size_t v_row_alignment = 4>
class Image {
public:
    static constexpr auto dim = v_dim;
    static constexpr auto pixel_format = v_pixel_format;
    static constexpr auto pixel_type = v_pixel_type;
    static constexpr auto row_alignment = v_row_alignment;

    using Pixel = dang::gl::Pixel<pixel_format, pixel_type>;
    using Size = dmath::svec<dim>;
    using Bounds = dmath::sbounds<dim>;

    static_assert(row_alignment > 0);

    static_assert(std::is_trivially_copy_assignable_v<Pixel>);
    static_assert(std::is_trivially_destructible_v<Pixel>);

    /// @brief Initializes the image with a size of zero without allocating any storage.
    Image() = default;

    Image(const Image& other)
        : size_(other.size_)
    {
        std::memcpy(data_.get(), other.data_.get(), byteCount());
    }

    Image(Image&&) = default;

    Image& operator=(const Image& other)
    {
        size_ = other.size_;
        auto byte_count = byteCount();
        data_ = std::make_unique<std::byte[]>(byte_count);
        std::memcpy(data_.get(), other.data_.get(), byte_count);
    }

    Image& operator=(Image&&) = default;

    /// @brief Initializes the image using the given size and fills it with the value.
    Image(const Size& size, const Pixel& value = {})
        : size_(size)
    {
        for (const auto& pos : Bounds(size_).xFirst())
            new (&(*this)[pos]) Pixel(value);
    }

    /// @brief Initializes the image using the given size and pixel iterator.
    template <typename TIter>
    Image(const Size& size, TIter first)
        : size_(size)
    {
        for (const auto& pos : Bounds(size_).xFirst())
            new (&(*this)[pos]) Pixel(first++);
    }

    /// @brief Initializes the image using the given size and preexisting chunk of data, which should match the size.
    /// @remark The array has to contain actual pixels that were created with placement new of Pixel.
    /// @remark Make sure, that the data is properly aligned.
    /// @remark For an empty image with either dimension being zero, data must be a nullptr.
    Image(const Size& size, std::unique_ptr<std::byte[]> data)
        : size_(size)
        , data_(std::move(data))
    {
        assert((count() == 0) == (data_.get() == nullptr));
    }

    /// @brief Creates a new image from a subsection of an existing image.
    Image(const Image& image, const Bounds& bounds)
        : size_(bounds.size())
    {
        for (const auto& pos : bounds.xFirst())
            new (&(*this)[pos - bounds.low]) Pixel(image[pos]);
    }

    /// @brief Loads a PNG image from the given stream and returns it.
    /// @exception PNGError if the stream does not contain a valid PNG.
    static Image loadFromPNG(std::istream& stream, Size pad_low = {}, Size pad_high = {})
    {
        static_assert(pixel_type == PixelType::UNSIGNED_BYTE, "Loading PNG images only supports unsigned bytes.");
        PNGLoader png_loader;
        // TODO: Better logging
        png_loader.onWarning.append([](const PNGWarningInfo& info) { std::cerr << info.message << '\n'; });
        png_loader.init(stream);
        auto data = png_loader.read<pixel_format, row_alignment>(true, pad_low, pad_high);
        return Image(png_loader.size(pad_low + pad_high), std::move(data));
    }

    /// @brief Loads a PNG image from the given file and returns it.
    /// @exception PNGError if the file cannot be opened.
    /// @exception PNGError if the file does not represent a valid PNG.
    static Image loadFromPNG(const fs::path& path, Size pad_low = {}, Size pad_high = {})
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
            throw PNGError("Cannot open PNG file: " + path.string());
        return loadFromPNG(stream, pad_low, pad_high);
    }

    /// @brief Returns the size of the image along each axis.
    const Size& size() const { return size_; }

    /// @brief Returns the total count of pixels.
    std::size_t count() const { return size_.product(); }

    /// @brief The width of the image in bytes.
    std::size_t byteWidth() const { return size_[0] * sizeof(Pixel); }

    /// @brief The width of the image in bytes, but aligned.
    std::size_t alignedByteWidth() const { return (byteWidth() - 1) / row_alignment * row_alignment + row_alignment; }

    /// @brief The size of the image, but with width as the aligned byte width.
    Size alignedByteSize() const
    {
        auto result = size();
        result[0] = alignedByteWidth();
        return result;
    }

    /// @brief Returns the byte count of the image.
    std::size_t byteCount() const { return alignedByteSize().product(); }

    /// @brief Provides access for a single pixel at the given position.
    Pixel& operator[](const Size& pos) { return *reinterpret_cast<Pixel*>(&data_[posToIndex(pos)]); }

    /// @brief Provides access for a single pixel at the given position.
    const Pixel& operator[](const Size& pos) const { return *reinterpret_cast<const Pixel*>(&data_[posToIndex(pos)]); }

    /// @brief Creates a new image from a subsection.
    Image operator[](const Bounds& bounds) const { return Image(*this, bounds); }

    /// @brief Copies pixels from a subsection of an existing image with a given offset.
    void setSubImage(const Size& offset, const Image& image, const Bounds& bounds)
    {
        for (const auto& pos : bounds.xFirst())
            (*this)[pos + offset] = image[pos];
    }

    /// @brief Copies pixels from an existing image with a given offset.
    void setSubImage(const Size& offset, const Image& image) { setSubImage(offset, image, Bounds(image.size())); }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    void* data() { return data_.get(); }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    const void* data() const { return data_.get(); }

    /// @brief Frees all image data, but leaves the size intact.
    void free() { data_ = nullptr; }

    /// @brief Frees all image data and sets the size to zero.
    void clear()
    {
        free();
        size_ = {};
    }

    /// @brief Whether the image contains any actual data.
    explicit operator bool() const { return bool{data_}; }

private:
    /// @brief A helper function, which calculates the position offset of a single dimension.
    template <std::size_t v_first, std::size_t... v_indices>
    std::size_t posToIndexHelperMul(const Size& pos, std::index_sequence<v_indices...>) const
    {
        return pos[v_first] * (alignedByteSize()[v_indices] * ... * 1);
    }

    /// @brief A helper function, which takes an index sequence of v_dim as start parameter.
    template <std::size_t... v_indices>
    std::size_t posToIndexHelper(const Size& pos, std::index_sequence<v_indices...>) const
    {
        return (posToIndexHelperMul<v_indices + 1>(pos, std::make_index_sequence<v_indices + 1>()) + ...);
    }

    /// @brief Converts the given pixel position into an index to the data.
    std::size_t posToIndex(const Size& pos) const
    {
        return pos[0] * sizeof(Pixel) + posToIndexHelper(pos, std::make_index_sequence<dim - 1>());
    }

    Size size_;
    std::unique_ptr<std::byte[]> data_ = count() > 0 ? std::make_unique<std::byte[]>(byteCount()) : nullptr;
};

using Image1D = Image<1>;
using Image2D = Image<2>;
using Image3D = Image<3>;

} // namespace dang::gl
