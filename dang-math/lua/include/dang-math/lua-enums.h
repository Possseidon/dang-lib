#pragma once

#include "dang-lua/Convert.h"

#include "dang-math/enums.h"

namespace dang::lua {

template <>
inline constexpr const char* enum_values<dang::math::Axis1>[2] = {"x"};

template <>
inline constexpr const char* enum_values<dang::math::Axis2>[3] = {"x", "y"};

template <>
inline constexpr const char* enum_values<dang::math::Axis3>[4] = {"x", "y", "z"};

template <>
inline constexpr const char* enum_values<dang::math::Axis4>[5] = {"x", "y", "z", "w"};

template <>
inline constexpr const char* enum_values<dang::math::Corner1>[3] = {"left", "right"};

template <>
inline constexpr const char* enum_values<dang::math::Corner2>[5] = {"leftBottom", "rightBottom", "leftTop", "rightTop"};

template <>
inline constexpr const char* enum_values<dang::math::Corner3>[9] = {"leftBottomBack",
                                                                    "rightBottomBack",
                                                                    "leftTopBack",
                                                                    "rightTopBack",
                                                                    "leftBottomFront",
                                                                    "rightBottomFront",
                                                                    "leftTopFront",
                                                                    "rightTopFront"};

template <>
inline constexpr const char* enum_values<dang::math::Edge2>[5] = {"left", "right", "bottom", "top"};

template <>
inline constexpr const char* enum_values<dang::math::Edge3>[13] = {"leftBottom",
                                                                   "rightBottom",
                                                                   "leftTop",
                                                                   "rightTop",
                                                                   "bottomBack",
                                                                   "topBack",
                                                                   "bottomFront",
                                                                   "topFront",
                                                                   "leftFront",
                                                                   "rightFront",
                                                                   "leftBack",
                                                                   "rightBack"};

template <>
inline constexpr const char* enum_values<dang::math::Facing1>[3] = {"left", "right"};

template <>
inline constexpr const char* enum_values<dang::math::Facing2>[5] = {"left", "right", "up", "down"};

template <>
inline constexpr const char* enum_values<dang::math::Facing3>[7] = {"left", "right", "up", "down", "back", "front"};

} // namespace dang::lua
