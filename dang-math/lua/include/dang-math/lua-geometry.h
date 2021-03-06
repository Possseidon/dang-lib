#pragma once

#include "dang-lua/State.h"

#include "dang-math/geometry.h"

namespace dang::lua {

template <>
inline constexpr const char* enum_values<dang::math::LineSide>[] { "left", "hit", "right", nullptr };

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Line<T, v_dim>> {
    using Vector = dang::math::Vector<T, v_dim>;
    using Line = dang::math::Line<T, v_dim>;

    static const std::string base_class_name;
    static const std::string class_name;
    static const std::string class_name_ref;

    static const char* className();
    static const char* classNameRef();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();

    static Arg require(State& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Plane<T, v_dim>> {
    using Vector = dang::math::Vector<T, v_dim>;
    using Line = dang::math::Line<T, v_dim>;
    using Plane = dang::math::Plane<T, v_dim>;

    static const std::string base_class_name;
    static const std::string class_name;
    static const std::string class_name_ref;

    static const char* className();
    static const char* classNameRef();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();

    static Arg require(State& lua);
};

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Spat<T, v_dim>> {
    using Vector = dang::math::Vector<T, v_dim>;
    using Line = dang::math::Line<T, v_dim>;
    using Plane = dang::math::Plane<T, v_dim>;
    using Spat = dang::math::Spat<T, v_dim>;

    static const std::string base_class_name;
    static const std::string class_name;
    static const std::string class_name_ref;

    static const char* className();
    static const char* classNameRef();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();

    static Arg require(State& lua);
};

} // namespace dang::lua
