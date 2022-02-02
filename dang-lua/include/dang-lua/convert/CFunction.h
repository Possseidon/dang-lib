#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Allows for conversion of C functions.
template <typename TCFunction>
struct Convert<TCFunction,
               std::enable_if_t<std::is_same_v<std::remove_cv_t<TCFunction>, lua_CFunction> ||
                                std::is_same_v<std::remove_cv_t<TCFunction>, std::remove_pointer_t<lua_CFunction>>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "C function"; }

    /// @brief Whether the value at the given stack position is a C function.
    static bool isExact(lua_State* state, int pos) { return lua_iscfunction(state, pos); }

    /// @brief Whether the value at the given stack position is a C function.
    static bool isValid(lua_State* state, int pos) { return isExact(state, pos); }

    /// @brief Checks, whether the given argument stack position is a C function and returns std::nullopt on failure.
    static std::optional<lua_CFunction> at(lua_State* state, int pos)
    {
        if (auto result = lua_tocfunction(state, pos))
            return result;
        return std::nullopt;
    }

    /// @brief Checks, whether the given argument stack position is a C function and raises an error on failure.
    static lua_CFunction check(lua_State* state, int arg)
    {
        if (auto result = lua_tocfunction(state, arg))
            return result;
        noreturn_luaL_typeerror(state, arg, "C function");
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given C function onto the stack.
    static void push(lua_State* state, lua_CFunction value) { lua_pushcfunction(state, value); }
};

} // namespace dang::lua
