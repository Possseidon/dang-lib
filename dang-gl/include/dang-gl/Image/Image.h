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
template <std::size_t v_dim, PixelFormat v_format = PixelFormat::RGBA, PixelType v_type = PixelType::UNSIGNED_BYTE>
class Image {
public:
    using Pixel = Pixel<v_format, v_type>;
    using Size = dmath::svec<v_dim>;
    using Bounds = dmath::sbounds<v_dim>;

    /// @brief Initializes the image with a width and height of zero.
    Image() = default;

    /// @brief Initializes the image using the given size with zero.
    explicit Image(const Size& size)
        : size_(size)
        , data_(count())
    {}

    /// @brief Initializes the image using the given size and fills it with the value.
    Image(const Size& size, const Pixel& value)
        : size_(size)
        , data_(count(), value)
    {}

    /// @brief Initializes the image using the given size and data iterator.
    template <typename TIter>
    Image(const Size& size, TIter first)
        : size_(size)
        , data_(first, std::next(first, size))
    {}

    /// @brief Initializes the image using the given size and preexisting vector of data, which should match the size.
    /// @remark Highly consider passing the data as an r-value using std::move to avoid a copy.
    Image(const Size& size, std::vector<Pixel> data)
        : size_(size)
        , data_(std::move(data))
    {
        assert(data_.size() == size.product());
    }

    /// @brief Creates a new image from a subsection of an existing image.
    Image(const Image& image, const Bounds& bounds)
        : size_(bounds.size())
    {
        data_.reserve(count());
        for (const auto& pos : bounds)
            data_.push_back(image[pos]);
    }

    /// @brief Loads a PNG image from the given stream and returns it.
    /// @remark Throws a PNGError if the stream does not contain a valid PNG.
    static Image loadFromPNG(std::istream& stream)
    {
        static_assert(v_type == PixelType::UNSIGNED_BYTE, "Loading PNG images only supports unsigned bytes.");
        PNGLoader png_loader;
        // TODO: Better logging
        png_loader.onWarning.append([](const PNGWarningInfo& info) { std::cerr << info.message << '\n'; });
        png_loader.init(stream);
        std::vector<Pixel> data = png_loader.read<v_format>(true);
        return Image(png_loader.size(), data);
    }

    /// @brief Loads a PNG image from the given file and returns it.
    /// @remark Throws a PNGError if the file does not represent a valid PNG.
    static Image loadFromPNG(const fs::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
            throw PNGError("Cannot open PNG file: " + path.string());
        return loadFromPNG(stream);
    }

    /// @brief Returns the size of the image along each axis.
    const Size& size() const { return size_; }

    /// @brief Returns the total count of pixels.
    std::size_t count() const { return size_.product(); }

    /// @brief Returns the actual size of the image in bytes.
    std::size_t byteSize() const { return count() * sizeof(Pixel); }

    /// @brief Provides access for a single pixel at the given position.
    Pixel& operator[](const Size& pos) { return data_[posToIndex(pos)]; }

    /// @brief Provides access for a single pixel at the given position.
    const Pixel& operator[](const Size& pos) const { return data_[posToIndex(pos)]; }

    /// @brief Creates a new image from a subsection.
    Image operator[](const Bounds& bounds) const { return Image(*this, bounds); }

    /// @brief Copies pixels from a subsection of an existing image with a given offset.
    void setSubImage(const Size& offset, const Image& image, const Bounds& bounds)
    {
        for (const auto& pos : bounds)
            (*this)[pos + offset] = image[pos];
    }

    /// @brief Copies pixels from an existing image with a given offset.
    void setSubImage(const Size& offset, const Image& image) { setSubImage(offset, image, Bounds(image.size())); }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    Pixel* data() { return data_.data(); }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    const Pixel* data() const { return data_.data(); }

    /// @brief Frees all image data, but leaves texture width and height intact.
    void free() { data_.clear(); }

    /// @brief Frees all image data and sets the texture width and height to zero.
    void clear()
    {
        free();
        size_ = {};
    }

    /// @brief Whether the image contains any actual data.
    explicit operator bool() const { return !data_.empty(); }

private:
    /// @brief A helper function, which calculates the position offset of a single dimension.
    template <std::size_t v_first, std::size_t... v_indices>
    std::size_t posToIndexHelperMul(const Size& pos, std::index_sequence<v_indices...>) const
    {
        assert(pos[v_first] < size_[v_first]);
        return pos[v_first] * (size_[v_indices] * ... * 1);
    }

    /// @brief A helper function, which takes an index sequence of v_dim as start parameter.
    template <std::size_t... v_indices>
    std::size_t posToIndexHelper(const Size& pos, std::index_sequence<v_indices...>) const
    {
        return (posToIndexHelperMul<v_indices>(pos, std::make_index_sequence<v_indices>()) + ...);
    }

    /// @brief Converts the given pixel position into an index to the data.
    std::size_t posToIndex(const Size& pos) const { return posToIndexHelper(pos, std::make_index_sequence<v_dim>()); }

    Size size_;
    std::vector<Pixel> data_;
};

using Image1D = Image<1>;
using Image2D = Image<2>;
using Image3D = Image<3>;

} // namespace dang::gl
