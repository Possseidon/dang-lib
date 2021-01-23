#pragma once

#include "dang-gl/Image/PixelFormat.h"
#include "dang-gl/Image/PixelType.h"
#include "dang-gl/global.h"

#include "dang-math/vector.h"

namespace dang::gl {

/// @brief A pixel of specified format and type, currently represented as a dmath::Vector.
template <PixelFormat v_format = PixelFormat::RGBA, PixelType v_type = PixelType::UNSIGNED_BYTE>
using Pixel = dmath::Vector<typename PixelTypeInfo<v_type>::Type, PixelFormatInfo<v_format>::ComponentCount>;

} // namespace dang::gl
