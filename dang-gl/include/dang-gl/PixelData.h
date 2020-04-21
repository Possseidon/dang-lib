#pragma once

#include "dang-utils/enum.h"

namespace dang::gl
{

enum class PixelDataFormat {
    UnsignedByte,
    Byte,
    UnsignedShort,
    Short,
    UnsignedInt,
    Int,
    HalfFloat,
    Float,

    UnsignedByte_3_3_2,
    UnsignedByte_2_3_3_rev,

    UnsignedShort_5_6_5,
    UnsignedShort_5_6_5_rev,
    UnsignedShort_4_4_4_4,
    UnsignedShort_4_4_4_4_rev,
    UnsignedShort_5_5_5_1,
    UnsignedShort_1_5_5_5_rev,

    UnsignedInt_8_8_8_8,
    UnsignedInt_8_8_8_8_rev,
    UnsignedInt_10_10_10_2,
    UnsignedInt_2_10_10_10_rev,

    // glReadPixels exclusive
    UnsignedInt_24_8,
    UnsignedInt_10f_11f_11f_rev,
    UnsignedInt_5_9_9_9_rev,
    Float_32_UnsignedInt_24_8_rev,

    COUNT
};

template <PixelDataFormat Format>
struct PixelDataInfo {};

template <>
struct PixelDataInfo<PixelDataFormat::UnsignedByte> {
    using Type = GLubyte;
};

template <>
struct PixelDataInfo<PixelDataFormat::Byte> {
    using Type = GLbyte;
};

template <>
struct PixelDataInfo<PixelDataFormat::UnsignedShort> {
    using Type = GLushort;
};
template <>
struct PixelDataInfo<PixelDataFormat::Short> {
    using Type = GLshort;
};

template <>
struct PixelDataInfo<PixelDataFormat::UnsignedInt> {
    using Type = GLuint;
};

template <>
struct PixelDataInfo<PixelDataFormat::Int> {
    using Type = GLint;
};

template <>
struct PixelDataInfo<PixelDataFormat::HalfFloat> {
    using Type = GLhalf;
};

template <>
struct PixelDataInfo<PixelDataFormat::Float> {
    using Type = GLfloat;
};

template <> struct PixelDataInfo<PixelDataFormat::UnsignedByte_3_3_2> : PixelDataInfo<PixelDataFormat::UnsignedByte> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedByte_2_3_3_rev> : PixelDataInfo<PixelDataFormat::UnsignedByte> {};

template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_5_6_5> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_5_6_5_rev> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_4_4_4_4> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_4_4_4_4_rev> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_5_5_5_1> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedShort_1_5_5_5_rev> : PixelDataInfo<PixelDataFormat::UnsignedShort> {};

template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_8_8_8_8> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_8_8_8_8_rev> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_10_10_10_2> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_2_10_10_10_rev> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};

template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_24_8> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_10f_11f_11f_rev> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};
template <> struct PixelDataInfo<PixelDataFormat::UnsignedInt_5_9_9_9_rev> : PixelDataInfo<PixelDataFormat::UnsignedInt> {};

template <> struct PixelDataInfo<PixelDataFormat::Float_32_UnsignedInt_24_8_rev> {
    struct Type {
        GLfloat depth;
        // stencil only has 8 bit actually
        GLuint stencil;
    };
};

constexpr dutils::EnumArray<PixelDataFormat, GLenum> PixelDataFormatsGL
{
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    GL_UNSIGNED_INT,
    GL_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,

    GL_UNSIGNED_BYTE_3_3_2,
    GL_UNSIGNED_BYTE_2_3_3_REV,

    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_5_6_5_REV,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_4_4_4_4_REV,
    GL_UNSIGNED_SHORT_5_5_5_1,
    GL_UNSIGNED_SHORT_1_5_5_5_REV,

    GL_UNSIGNED_INT_8_8_8_8,
    GL_UNSIGNED_INT_8_8_8_8_REV,
    GL_UNSIGNED_INT_10_10_10_2,
    GL_UNSIGNED_INT_2_10_10_10_REV,

    // glReadPixels exclusive
    GL_UNSIGNED_INT_24_8,
    GL_UNSIGNED_INT_10F_11F_11F_REV,
    GL_UNSIGNED_INT_5_9_9_9_REV,
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV
};

/// <summary>Stores pixel data in a template specified type.</summary>
template <std::size_t Dim, PixelDataFormat DataFormat>
class PixelData {
public:
    using DataType = typename PixelDataInfo<DataFormat>::Type;

    /// <summary>Initializes the pixel data with a width and height of zero.</summary>
    PixelData() = default;

    /// <summary>Initializes the pixel data using the given size with zero.</summary>
    explicit PixelData(dmath::svec<Dim> size)
        : data_(size.product())
        , size_(size)
    {
    }

    /// <summary>Initializes the pixel data using the given size and fills it with the value.</summary>
    PixelData(dmath::svec<Dim> size, DataType value)
        : data_(size.product(), value)
        , size_(size)
    {
    }

    /// <summary>Initializes the pixel data using the given size and data iterator.</summary>
    template <typename Iter>
    PixelData(dmath::svec<Dim> size, Iter first)
        : data_(first, std::next(first, size.area()))
        , size_(size)
    {
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

    /// <summary>Provides access for a single pixel at the given position.</summary>
    DataType& operator[](dmath::svec<Dim> pos)
    {
        return data_[posToIndex(pos)];
    }

    /// <summary>Provides access for a single pixel at the given position.</summary>
    const DataType& operator[](dmath::svec<Dim> pos) const
    {
        return data_[posToIndex(pos)];
    }

    /// <summary>Provides access to the raw underlying data.</summary>
    DataType* data()
    {
        return data_.data();
    }

    /// <summary>Provides access to the raw underlying data.</summary>
    const DataType* data() const
    {
        return data_.data();
    }

private:
    template <std::size_t First, std::size_t... Indices>
    std::size_t posToIndexHelperMul(dmath::svec<Dim> pos, std::index_sequence<Indices...>)
    {                               
        assert(pos[First] < size_[First]);
        return pos[First] * (size_[Indices] * ... * 1);
    }

    /// <summary>Recursive helper function, which takes Dim - 1 and an index sequence for Dim - 1 as start parameter.</summary>
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

    std::vector<DataType> data_;
    dmath::svec<Dim> size_;
};

template <PixelDataFormat DataFormat>
using PixelData1D = PixelData<1, DataFormat>;

template <PixelDataFormat DataFormat>
using PixelData2D = PixelData<2, DataFormat>;

template <PixelDataFormat DataFormat>
using PixelData3D = PixelData<3, DataFormat>;

}
