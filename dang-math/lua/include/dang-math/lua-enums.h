#pragma once

#include "dang-lua/Convert.h"
#include "dang-math/enums.h"

namespace dang::lua {

template <>
struct EnumInfo<dang::math::Axis1> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Axis1"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[2]{"x"};
};

template <>
struct EnumInfo<dang::math::Axis2> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Axis2"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[3]{"x", "y"};
};

template <>
struct EnumInfo<dang::math::Axis3> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Axis3"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[4]{"x", "y", "z"};
};

template <>
struct EnumInfo<dang::math::Axis4> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Axis4"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[5]{"x", "y", "z", "w"};
};

template <>
struct EnumInfo<dang::math::Corner1> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Corner1"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[3]{"left", "right"};
};

template <>
struct EnumInfo<dang::math::Corner2> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Corner2"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[5]{"leftBottom", "rightBottom", "leftTop", "rightTop"};
};

template <>
struct EnumInfo<dang::math::Corner3> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Corner3"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[9]{
        "leftBottomBack",
        "rightBottomBack",
        "leftTopBack",
        "rightTopBack",
        "leftBottomFront",
        "rightBottomFront",
        "leftTopFront",
        "rightTopFront",
    };
};

template <>
struct EnumInfo<dang::math::Edge2> : DefaultEnumInfo {
    static constexpr auto specialized = true;

    static std::string getCheckTypename() { return "Edge2"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[5]{"left", "right", "bottom", "top"};
};

template <>
struct EnumInfo<dang::math::Edge3> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Edge3"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[13]{
        "leftBottom",
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
        "rightBack",
    };
};

template <>
struct EnumInfo<dang::math::Facing1> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Facing1"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[3]{"left", "right"};
};

template <>
struct EnumInfo<dang::math::Facing2> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Facing2"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[5]{"left", "right", "up", "down"};
};

template <>
struct EnumInfo<dang::math::Facing3> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "Facing3"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[7]{"left", "right", "up", "down", "back", "front"};
};

} // namespace dang::lua
