#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Tag struct for Lua's "fail" value.
struct Fail {};

/// @brief The Lua "fail" value.
inline constexpr Fail fail;

inline constexpr bool operator==(Fail, Fail) { return true; }
inline constexpr bool operator!=(Fail, Fail) { return false; }
inline constexpr bool operator<(Fail, Fail) { return false; }
inline constexpr bool operator<=(Fail, Fail) { return true; }
inline constexpr bool operator>(Fail, Fail) { return false; }
inline constexpr bool operator>=(Fail, Fail) { return true; }

/// @brief True for types that should be treated as "fail" in Lua.
template <typename, typename SFINAE = void>
struct is_fail : std::false_type {};

template <typename TFail>
inline constexpr auto is_fail_v = is_fail<TFail>::value;

template <typename TFail>
struct is_fail<TFail, std::enable_if_t<std::is_same_v<std::remove_cv_t<TFail>, Fail>>> : std::true_type {};

/// @brief Allows for pushing of the "fail" value, which is currently just "nil".
template <typename TFail>
struct Convert<TFail, std::enable_if_t<is_fail_v<TFail>>> {
    using Fail = std::remove_cv_t<TFail>;

    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return "fail"; }

    /// @brief Pushes the "fail" value on the stack.
    static void push(lua_State* state, const Fail&) { luaL_pushfail(state); }
};

} // namespace dang::lua
