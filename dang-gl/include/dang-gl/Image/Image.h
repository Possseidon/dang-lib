#pragma once

#include "dang-gl/Image/PNGLoader.h"
#include "dang-gl/Image/Pixel.h"
#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Stores pixels data for an n-dimensional image in a template specified type.
template <std::size_t Dim, PixelFormat Format = PixelFormat::RGBA, PixelType Type = PixelType::UNSIGNED_BYTE>
class Image {
public:
    using Pixel = Pixel<Format, Type>;

    /// @brief Initializes the image with a width and height of zero.
    Image() = default;

    /// @brief Initializes the image using the given size with zero.
    explicit Image(dmath::svec<Dim> size)
        : size_(size)
        , data_(size)
    {}

    /// @brief Initializes the image using the given size and fills it with the value.
    Image(dmath::svec<Dim> size, const Pixel& value)
        : size_(size)
        , data_(size, value)
    {}

    /// @brief Initializes the image using the given size and data iterator.
    template <typename Iter>
    Image(dmath::svec<Dim> size, Iter first)
        : size_(size)
        , data_(first, std::next(first, size))
    {}

    /// @brief Initializes the image using the given size and pre-existing vector of data, which should match the size.
    /// @remark Highly consider passing the data as an r-value using std::move to avoid a copy.
    Image(dmath::svec<Dim> size, std::vector<Pixel> data)
        : size_(size)
        , data_(std::move(data))
    {
        assert(data_.size() == size.product());
    }

    /// @brief Loads a PNG image from the given stream and returns it.
    /// @remark Throws a PNGError if the stream does not contain a valid PNG.
    static Image loadFromPNG(std::istream& stream)
    {
        static_assert(Type == PixelType::UNSIGNED_BYTE, "Loading PNG images only supports unsigned bytes.");
        PNGLoader png_loader;
        // TODO: Better logging
        png_loader.onWarning.append([](const PNGWarningInfo& info) { std::cerr << info.message << '\n'; });
        png_loader.init(stream);
        std::vector<Pixel> data = png_loader.read<Format>(true);
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
    dmath::svec<Dim> size() const { return size_; }

    /// @brief Returns the total count of pixels.
    std::size_t count() const { return size_.product(); }

    /// @brief Returns the actual size of the image in bytes.
    std::size_t byteSize() const { return count() * sizeof(Pixel); }

    /// @brief Provides access for a single pixel at the given position.
    Pixel& operator[](dmath::svec<Dim> pos) { return data_[posToIndex(pos)]; }

    /// @brief Provides access for a single pixel at the given position.
    const Pixel& operator[](dmath::svec<Dim> pos) const { return data_[posToIndex(pos)]; }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    Pixel* data() { return data_.data(); }

    /// @brief Provides access to the raw underlying data, which can be used to provide OpenGL the data.
    const Pixel* data() const { return data_.data(); }

private:
    /// @brief A helper function, which calculates the position offset of a single dimension.
    template <std::size_t First, std::size_t... Indices>
    std::size_t posToIndexHelperMul(dmath::svec<Dim> pos, std::index_sequence<Indices...>)
    {
        assert(pos[First] < size_[First]);
        return pos[First] * (size_[Indices] * ... * 1);
    }

    /// @brief A helper function, which takes an index sequence of Dim as start parameter.
    template <std::size_t... Indices>
    std::size_t posToIndexHelper(dmath::svec<Dim> pos, std::index_sequence<Indices...>)
    {
        return (posToIndexHelperMul<Indices>(pos, std::make_index_sequence<Indices>()) + ...);
    }

    /// @brief Converts the given pixel position into an index to the data.
    std::size_t posToIndex(dmath::svec<Dim> pos) { return posToIndexHelper(pos, std::make_index_sequence<Dim>()); }

    dmath::svec<Dim> size_;
    std::vector<Pixel> data_;
};

using Image1D = Image<1>;
using Image2D = Image<2>;
using Image3D = Image<3>;

} // namespace dang::gl
