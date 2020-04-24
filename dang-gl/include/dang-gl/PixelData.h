#pragma once

#include "dang-utils/enum.h"

#include "Pixel.h"
#include "PixelFormat.h"
#include "PixelType.h"
#include "PNGLoader.h"

namespace dang::gl
{

/// <summary>Stores pixel data in a template specified type.</summary>
template <std::size_t Dim, PixelFormat Format = PixelFormat::RGBA, PixelType Type = PixelType::UNSIGNED_BYTE>
class PixelData {
public:
    using Pixel = Pixel<Format, Type>;

    /// <summary>Initializes the pixel data with a width and height of zero.</summary>
    PixelData() = default;

    /// <summary>Initializes the pixel data using the given size with zero.</summary>
    explicit PixelData(dmath::svec<Dim> size)
        : size_(size)
        , data_(size)
    {
    }

    /// <summary>Initializes the pixel data using the given size and fills it with the value.</summary>
    PixelData(dmath::svec<Dim> size, const Pixel& value)
        : size_(size)
        , data_(size, value)
    {
    }

    /// <summary>Initializes the pixel data using the given size and data iterator.</summary>
    template <typename Iter>
    PixelData(dmath::svec<Dim> size, Iter first)
        : size_(size)
        , data_(first, std::next(first, size))
    {
    }

    /// <summary>Initializes the pixel data using the given size and pre-existing vector of data, which should match the size.</summary>
    /// <remarks>Highly consider passing the data as an r-value using std::move to avoid a copy.</remarks>
    PixelData(dmath::svec<Dim> size, std::vector<Pixel> data)
        : size_(size)
        , data_(std::move(data))
    {
        assert(data_.size() == size.product());
    }

    /// <summary>Loads a PNG image from the given stream and returns it.</summary>
    /// <remarks>Throws a PNGError if the stream does not contain a valid PNG.</remarks>
    static PixelData loadFromPNG(std::istream& stream)
    {
        static_assert(Type == PixelType::UNSIGNED_BYTE, "Loading PNG images only supports unsigned bytes.");
        PNGLoader png_loader;
        // TODO: Better logging
        png_loader.onWarning.append([](const PNGWarningInfo& info) { std::cerr << info.message << std::endl; });
        png_loader.init(stream);
        std::vector<Pixel> data = png_loader.read<Format>();
        return PixelData(png_loader.size(), data);
    }

    /// <summary>Loads a PNG image from the given file and returns it.</summary>
    /// <remarks>Throws a PNGError if the file does not represent a valid PNG.</remarks>
    static PixelData loadFromPNG(const fs::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
            throw PNGError("Cannot open PNG file: " + path.string());
        return loadFromPNG(stream);
    }

    /// <summary>Returns the size of the pixel data along each axis.</summary>
    dmath::svec<Dim> size() const
    {
        return size_;
    }

    /// <summary>Returns the total count of pixels.</summary>
    std::size_t count() const
    {
        return size_.product();
    }

    /// <summary>Returns the actual size of the image in bytes.</summary>
    std::size_t byteSize() const
    {
        return count() * sizeof(Pixel);
    }

    /// <summary>Provides access for a single pixel at the given position.</summary>
    Pixel& operator[](dmath::svec<Dim> pos)
    {
        return data_[posToIndex(pos)];
    }

    /// <summary>Provides access for a single pixel at the given position.</summary>
    const Pixel& operator[](dmath::svec<Dim> pos) const
    {
        return data_[posToIndex(pos)];
    }

    /// <summary>Provides access to the raw underlying data, which can be used to provide OpenGL the data.</summary>
    Pixel* data()
    {
        return data_.data();
    }

    /// <summary>Provides access to the raw underlying data, which can be used to provide OpenGL the data.</summary>
    const Pixel* data() const
    {
        return data_.data();
    }

private:
    /// <summary>A helper function, which calculates the position offset of a single dimension.</summary>
    template <std::size_t First, std::size_t... Indices>
    std::size_t posToIndexHelperMul(dmath::svec<Dim> pos, std::index_sequence<Indices...>)
    {
        assert(pos[First] < size_[First]);
        return pos[First] * (size_[Indices] * ... * 1);
    }

    /// <summary>A helper function, which takes an index sequence of Dim as start parameter.</summary>
    template <std::size_t... Indices>
    std::size_t posToIndexHelper(dmath::svec<Dim> pos, std::index_sequence<Indices...>)
    {
        return (posToIndexHelperMul<Indices>(pos, std::make_index_sequence<Indices>()) + ...);
    }

    /// <summary>Converts the given pixel position into an index to the data.</summary>
    std::size_t posToIndex(dmath::svec<Dim> pos)
    {
        return posToIndexHelper(pos, std::make_index_sequence<Dim>());
    }

    dmath::svec<Dim> size_;
    std::vector<Pixel> data_;
};

using PixelData1D = PixelData<1>;
using PixelData2D = PixelData<2>;
using PixelData3D = PixelData<3>;

}
