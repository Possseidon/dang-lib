#pragma once

#include "dang-lua/Convert.h"
#include "dang-lua/State.h"
#include "dang-math/geometry.h"

namespace dang::lua {

template <>
struct EnumInfo<dang::math::LineSide> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "LineSide"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[4]{"left", "hit", "right"};
};

template <>
struct EnumInfo<dang::math::PlaneSide> : DefaultEnumInfo {
    static std::string getCheckTypename() { return "PlaneSide"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[4]{"top", "hit", "bottom"};
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Line<T, v_dim>> : DefaultClassInfo {
    static constexpr auto allow_table_initialization = true;

    using Line = dang::math::Line<T, v_dim>;

    using Point = typename Line::Point;
    using Direction = typename Line::Direction;

    static std::string getCheckTypename();
    static std::string getPushTypename() { return getCheckTypename(); }

    static std::vector<luaL_Reg> methods();
    static std::vector<luaL_Reg> metamethods();
    static std::vector<Property> properties();

    static Arg require(StateRef& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Plane<T, v_dim>> : DefaultClassInfo {
    static constexpr auto allow_table_initialization = true;

    using Plane = dang::math::Plane<T, v_dim>;

    using Point = typename Plane::Point;
    using Direction = typename Plane::Direction;
    using Directions = typename Plane::Directions;
    using Factor = typename Plane::Factor;
    using Factors = typename Plane::Factors;

    static std::string getCheckTypename();
    static std::string getPushTypename() { return getCheckTypename(); }

    static std::vector<luaL_Reg> methods();
    static std::vector<luaL_Reg> metamethods();
    static std::vector<Property> properties();

    static Arg require(StateRef& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Spat<T, v_dim>> : DefaultClassInfo {
    static constexpr auto allow_table_initialization = true;

    using Spat = dang::math::Spat<T, v_dim>;

    using Point = typename Spat::Point;
    using Direction = typename Spat::Direction;
    using Directions = typename Spat::Directions;
    using Factor = typename Spat::Factor;

    static std::string getCheckTypename();
    static std::string getPushTypename() { return getCheckTypename(); }

    static std::vector<luaL_Reg> methods();
    static std::vector<luaL_Reg> metamethods();
    static std::vector<Property> properties();

    static Arg require(StateRef& lua);
};

} // namespace dang::lua
