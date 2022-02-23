#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"
#include "dang-utils/utils.h"

namespace dang::lua {

/// @brief Allows for conversion between Lua strings and std::string.
template <typename TString>
struct Convert<TString, std::enable_if_t<std::is_same_v<std::remove_cv_t<TString>, std::string>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "string"; }

    /// @brief Whether the value at the given stack position is a string.
    static bool isExact(lua_State* state, int pos) { return lua_type(state, pos) == LUA_TSTRING; }

    /// @brief Whether the value at the given stack position is a string or a number.
    static bool isValid(lua_State* state, int pos) { return lua_isstring(state, pos); }

    /// @brief Checks, whether the given argument stack position is a string or number and returns std::nullopt on
    /// failure.
    /// @remark Numbers are actually converted to a string in place.
    static std::optional<std::string> at(lua_State* state, int pos)
    {
        std::size_t length;
        const char* string = lua_tolstring(state, pos, &length);
        if (string)
            return std::string(string, length);
        return std::nullopt;
    }

    /// @brief Checks, whether the given argument stack position is a string or number and raises an error on failure.
    /// @remark Numbers are actually converted to a string in place.
    static std::string check(lua_State* state, int arg)
    {
        std::size_t length;
        const char* string = luaL_checklstring(state, arg, &length);
        return std::string(string, length);
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given string onto the stack.
    static void push(lua_State* state, const std::string& value)
    {
        lua_pushlstring(state, value.c_str(), value.size());
    }
};

/// @brief Allows for conversion between Lua strings and std::string_view.
template <typename TStringView>
struct Convert<TStringView, std::enable_if_t<std::is_same_v<std::remove_cv_t<TStringView>, std::string_view>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "string"; }

    /// @brief Whether the value at the given stack position is a string.
    static bool isExact(lua_State* state, int pos) { return lua_type(state, pos) == LUA_TSTRING; }

    /// @brief Whether the value at the given stack position is a string or a number.
    static bool isValid(lua_State* state, int pos) { return lua_isstring(state, pos); }

    /// @brief Checks, whether the given argument stack position is a string or number and returns std::nullopt on
    /// failure.
    /// @remark Numbers are actually converted to a string in place.
    static std::optional<std::string_view> at(lua_State* state, int pos)
    {
        std::size_t length;
        const char* string = lua_tolstring(state, pos, &length);
        if (string)
            return std::string_view(string, length);
        return std::nullopt;
    }

    /// @brief Checks, whether the given argument stack position is a string or number and raises an error on failure.
    /// @remark Numbers are actually converted to a string in place.
    static std::string_view check(lua_State* state, int arg)
    {
        std::size_t length;
        const char* string = luaL_checklstring(state, arg, &length);
        return std::string_view(string, length);
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given string onto the stack.
    static void push(lua_State* state, std::string_view value) { lua_pushlstring(state, value.data(), value.size()); }
};

/// @brief Allows pushing of char arrays as strings.
template <std::size_t v_count>
struct Convert<const char[v_count]> {
    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return "string"; }

    /// @brief Pushes the given string literal onto the stack.
    /// @remark The null terminator is removed and must exist.
    static void push(lua_State* state, const char (&value)[v_count])
    {
        assert(value[v_count - 1] == '\0');
        lua_pushlstring(state, value, v_count - 1);
    }
};

/// @brief Allows pushing of C-Style strings.
template <typename TCString>
struct Convert<TCString, std::enable_if_t<std::is_same_v<dutils::remove_cvref_t<TCString>, const char*>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "string"; }

    /// @brief Whether the value at the given stack position is a string.
    static bool isExact(lua_State* state, int pos) { return lua_type(state, pos) == LUA_TSTRING; }

    /// @brief Whether the value at the given stack position is a string or a number.
    static bool isValid(lua_State* state, int pos) { return lua_isstring(state, pos); }

    /// @brief Checks, whether the given argument stack position is a string or number and returns std::nullopt on
    /// failure.
    /// @remark Numbers are actually converted to a string in place.
    static std::optional<const char*> at(lua_State* state, int pos)
    {
        const char* string = lua_tostring(state, pos);
        if (string)
            return string;
        return std::nullopt;
    }

    /// @brief Checks, whether the given argument stack position is a string or number and raises an error on failure.
    /// @remark Numbers are actually converted to a string in place.
    static const char* check(lua_State* state, int arg) { return luaL_checkstring(state, arg); }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    /// @brief Pushes the given null-terminated string onto the stack.
    static void push(lua_State* state, const char* value) { lua_pushstring(state, value); }
};

/// @brief Allows pushing of mutable C-Style strings.
template <typename TCString>
struct Convert<TCString, std::enable_if_t<std::is_same_v<std::remove_cv_t<TCString>, char*>>> {
    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return "string"; }

    /// @brief Pushes the given null-terminated string onto the stack.
    static void push(lua_State* state, const char* value) { lua_pushstring(state, value); }
};

} // namespace dang::lua
