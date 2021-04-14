#pragma once

#include "dang-lua/State.h"

#include "dang-math/geometry.h"

namespace dang::lua {

template <>
inline constexpr const char* enum_values<dang::math::LineSide>[4] = {"left", "hit", "right"};

template <>
inline constexpr const char* enum_values<dang::math::PlaneSide>[4] = {"top", "hit", "bottom"};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Line<T, v_dim>> : DefaultClassInfo {
    using Line = dang::math::Line<T, v_dim>;

    using Point = typename Line::Point;
    using Direction = typename Line::Direction;

    static const std::string base_class_name;
    static const std::string class_name;

    static const char* className();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();
    static std::vector<Property> properties();

    static Arg require(State& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Plane<T, v_dim>> : DefaultClassInfo {
    using Plane = dang::math::Plane<T, v_dim>;

    using Point = typename Plane::Point;
    using Direction = typename Plane::Direction;
    using Directions = typename Plane::Directions;
    using Factor = typename Plane::Factor;
    using Factors = typename Plane::Factors;

    static const std::string base_class_name;
    static const std::string class_name;

    static const char* className();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();
    static std::vector<Property> properties();

    static Arg require(State& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Spat<T, v_dim>> : DefaultClassInfo {
    using Spat = dang::math::Spat<T, v_dim>;

    using Point = typename Spat::Point;
    using Direction = typename Spat::Direction;
    using Directions = typename Spat::Directions;
    using Factor = typename Spat::Factor;

    static const std::string base_class_name;
    static const std::string class_name;

    static const char* className();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();
    static std::vector<Property> properties();

    static Arg require(State& lua);
};

} // namespace dang::lua
