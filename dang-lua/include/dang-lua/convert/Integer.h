#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Allows for conversion between Lua integers and C++ integral types.
template <typename TInteger>
struct Convert<TInteger,
               std::enable_if_t<std::is_integral_v<TInteger> && !std::is_same_v<std::remove_cv_t<TInteger>, bool>>> {
    using Integer = std::remove_cv_t<TInteger>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "integer"; }

    // TODO: Some special case to allow for conversion between uint64 -> lua_Integer

    // TODO: C++20 replace with std::in_range <3
    /// @brief Checks, whether the given Lua integer fits into the range of the C++ integral type.
    static constexpr bool checkRange([[maybe_unused]] lua_Integer value)
    {
        if constexpr (std::is_same_v<Integer, std::uint64_t>) {
            return value >= 0;
        }
        else {
            constexpr auto int_min = lua_Integer{std::numeric_limits<Integer>::min()};
            constexpr auto int_max = lua_Integer{std::numeric_limits<Integer>::max()};
            constexpr auto lua_min = std::numeric_limits<lua_Integer>::min();
            constexpr auto lua_max = std::numeric_limits<lua_Integer>::max();
            if constexpr (int_max < lua_max) {
                if (value > int_max)
                    return false;
            }
            if constexpr (int_min > lua_min) {
                if (value < int_min)
                    return false;
            }
            return true;
        }
    }

    /// @brief Whether the value at the given stack position is an integer and fits the C++ integral type.
    static bool isExact(lua_State* state, int pos)
    {
        if (lua_type(state, pos) != LUA_TNUMBER)
            return false;
        int isnum;
        lua_Integer value = lua_tointegerx(state, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// @brief Whether the value at the given stack position is an integer or a string convertible to an integer and
    /// fits the C++ integral type.
    static bool isValid(lua_State* state, int pos)
    {
        int isnum;
        lua_Integer value = lua_tointegerx(state, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// @brief Returns an error message for the given number not being in the correct range.
    static std::string getRangeErrorMessage(lua_Integer value)
    {
        return "value " + std::to_string(value) + " must be in range " +
               std::to_string(std::numeric_limits<Integer>::min()) + " .. " +
               std::to_string(std::numeric_limits<Integer>::max());
    }

    /// @brief Converts the given argument stack position into an integral type and returns std::nullopt on failure.
    static std::optional<Integer> at(lua_State* state, int pos)
    {
        int isnum;
        lua_Integer result = lua_tointegerx(state, pos, &isnum);
        if (isnum && checkRange(result))
            return static_cast<Integer>(result);
        return std::nullopt;
    }

    /// @brief Converts the given argument stack position into an integral type and raises an error on failure.
    static Integer check(lua_State* state, int arg)
    {
        int isnum;
        lua_Integer result = lua_tointegerx(state, arg, &isnum);
        if (!isnum) {
            auto type = lua_type(state, arg);
            if (type == LUA_TNUMBER)
                noreturn_luaL_argerror(state, arg, "number has no integer representation");
            if (type == LUA_TSTRING)
                noreturn_luaL_argerror(state, arg, "string cannot be converted to an integer");
            noreturn_luaL_typeerror(state, arg, "integer");
        }
        if (!checkRange(result))
            noreturn_luaL_argerror(state, arg, getRangeErrorMessage(result).c_str());
        return static_cast<Integer>(result);
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given integer on the stack.
    static void push(lua_State* state, Integer value) { lua_pushinteger(state, static_cast<lua_Integer>(value)); }
};

} // namespace dang::lua
