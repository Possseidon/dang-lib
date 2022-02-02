#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Allows for conversion between Lua numbers and C++ floating point types.
template <typename TNumber>
struct Convert<TNumber, std::enable_if_t<std::is_floating_point_v<TNumber>>> {
    using Number = std::remove_cv_t<TNumber>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "number"; }

    /// @brief Whether the stack position contains an actual number.
    static bool isExact(lua_State* state, int pos) { return lua_type(state, pos) == LUA_TNUMBER; }

    /// @brief Whether the stack position contains a number or a string, convertible to a number.
    static bool isValid(lua_State* state, int pos) { return lua_isnumber(state, pos); }

    /// @brief Converts the given argument stack position into a Lua number and returns std::nullopt on failure.
    static std::optional<Number> at(lua_State* state, int pos)
    {
        int isnum;
        lua_Number result = lua_tonumberx(state, pos, &isnum);
        if (isnum)
            return static_cast<Number>(result);
        return std::nullopt;
    }

    /// @brief Converts the given argument stack position into a floating point type and raises an error on failure.
    static Number check(lua_State* state, int arg)
    {
        int isnum;
        lua_Number result = lua_tonumberx(state, arg, &isnum);
        if (isnum)
            return static_cast<Number>(result);
        if (lua_type(state, arg) == LUA_TSTRING)
            noreturn_luaL_argerror(state, arg, "string cannot be converted to a number");
        noreturn_luaL_typeerror(state, arg, "number");
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given number on the stack.
    static void push(lua_State* state, Number value) { lua_pushnumber(state, static_cast<lua_Number>(value)); }
};

} // namespace dang::lua
