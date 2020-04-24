#pragma once

#include "dang-math/vector.h"

#include "PixelFormat.h"
#include "PixelType.h"

namespace dang::gl
{

/// <summary>A pixel of specified format and type, currently represented as a dmath::Vector.</summary>
template <PixelFormat Format = PixelFormat::RGBA, PixelType Type = PixelType::UNSIGNED_BYTE>
using Pixel = dmath::Vector<typename PixelTypeInfo<Type>::Type, PixelFormatInfo<Format>::ComponentCount>;

}
