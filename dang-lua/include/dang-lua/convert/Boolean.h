#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Allows for conversion between Lua boolean and C++ bool.
template <typename TBool>
struct Convert<TBool, std::enable_if_t<std::is_same_v<std::remove_cv_t<TBool>, bool>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "boolean"; }

    /// @brief Whether the given stack position contains an actual boolean.
    static bool isExact(lua_State* state, int pos) { return lua_isboolean(state, pos); }

    /// @brief Always returns true, as everything is convertible to boolean.
    static constexpr bool isValid(lua_State*, int) { return true; }

    /// @brief Converts the given stack position and never returns std::nullopt.
    static std::optional<bool> at(lua_State* state, int pos) { return lua_toboolean(state, pos); }

    /// @brief Converts the given stack position and never raises an error.
    static bool check(lua_State* state, int arg) { return lua_toboolean(state, arg); }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given boolean on the stack.
    static void push(lua_State* state, bool value) { lua_pushboolean(state, value); }
};

} // namespace dang::lua
