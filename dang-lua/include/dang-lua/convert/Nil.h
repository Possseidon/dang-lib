#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Tag struct for a Lua nil value.
struct Nil {};

/// @brief A Lua nil value.
inline constexpr Nil nil;

inline constexpr bool operator==(Nil, Nil) { return true; }
inline constexpr bool operator!=(Nil, Nil) { return false; }
inline constexpr bool operator<(Nil, Nil) { return false; }
inline constexpr bool operator<=(Nil, Nil) { return true; }
inline constexpr bool operator>(Nil, Nil) { return false; }
inline constexpr bool operator>=(Nil, Nil) { return true; }

/// @brief True for types that should be treated as "nil" in Lua.
template <typename T, typename = void>
struct is_nil : std::false_type {};

template <typename T>
inline constexpr auto is_nil_v = is_nil<T>::value;

template <typename TNil>
struct is_nil<TNil, std::enable_if_t<std::is_same_v<std::remove_cv_t<TNil>, Nil>>> : std::true_type {};

/// @brief Converts nil values.
template <typename TNil>
struct Convert<TNil, std::enable_if_t<is_nil_v<TNil>>> {
    using Nil = std::remove_cv_t<TNil>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "nil"; }

    /// @brief Whether the given stack position is nil.
    static bool isExact(lua_State* state, int pos) { return lua_isnil(state, pos); }

    /// @brief Whether the given stack position is nil or none.
    static bool isValid(lua_State* state, int pos) { return lua_isnoneornil(state, pos); }

    /// @brief Returns an instance of TNil for nil and none values, and std::nullopt otherwise.
    static std::optional<Nil> at(lua_State* state, int pos)
    {
        if (lua_isnoneornil(state, pos))
            return Nil();
        return std::nullopt;
    }

    /// @brief Returns an instance of TNil and raises an error if the value is neither nil nor none.
    static Nil check(lua_State* state, int arg)
    {
        if (lua_isnoneornil(state, arg))
            return Nil();
        noreturn_luaL_typeerror(state, arg, "nil");
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes a nil value on the stack.
    static void push(lua_State* state, const Nil&) { lua_pushnil(state); }
};

} // namespace dang::lua
