#pragma once

#include <cstddef>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief A full EnumInfo implementation, meant to be inherited for convenience.
struct DefaultEnumInfo {
    static constexpr auto specialized = true;

    static std::string getCheckTypename() { return "<enum>"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr const char* values[1]{};
};

/// @brief Can be specialized to allow an enum to be converted to and from Lua strings.
template <typename TEnum>
struct EnumInfo {
    /// @brief A flag to find out if a specific class is specialized and can be used with Convert.
    static constexpr auto specialized = false;
};

// The EnumInfo enum type may be cv-qualified.

template <typename TEnum>
struct EnumInfo<const TEnum> : EnumInfo<TEnum> {};

template <typename TEnum>
struct EnumInfo<volatile TEnum> : EnumInfo<TEnum> {};

template <typename TEnum>
struct EnumInfo<const volatile TEnum> : EnumInfo<TEnum> {};

/// @brief Converts enums to and from Lua as strings.
template <typename TEnum>
struct Convert<TEnum, std::enable_if_t<std::is_enum_v<TEnum>>> {
    using Enum = std::remove_cv_t<TEnum>;
    using Info = EnumInfo<TEnum>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return Info::getCheckTypename(); }

    /// @brief Whether the stack position is a valid enum string.
    static bool isExact(lua_State* state, int pos) { return at(state, pos).has_value(); }

    /// @brief Whether the stack position is a valid enum string.
    static bool isValid(lua_State* state, int pos) { return isExact(state, pos); }

    /// @brief Returns the enum value at the given stack position or std::nullopt on failure.
    static std::optional<Enum> at(lua_State* state, int pos)
    {
        if (lua_type(state, pos) != LUA_TSTRING)
            return std::nullopt;
        return findEnumValue(lua_tostring(state, pos));
    }

    /// @brief Returns the enum value at the given argument stack position and raises an argument error on failure.
    static Enum check(lua_State* state, int arg)
    {
        assertValuesValid();
        return static_cast<Enum>(luaL_checkoption(state, arg, nullptr, Info::values));
    }

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    /// @brief Returns the name of the enum.
    static std::string getPushTypename() { return Info::getPushTypename(); }

    /// @brief Pushes the string name of the enum value onto the stack.
    static void push(lua_State* state, Enum value)
    {
        assertValuesValid();
        lua_pushstring(state, Info::values[static_cast<std::size_t>(value)]);
    }

private:
    /// @brief Finds the given string's enum value and returns std::nullopt if not found.
    static std::optional<Enum> findEnumValue(const char* value)
    {
        assertValuesValid();
        for (std::size_t i = 0; Info::values[i]; i++)
            if (std::strcmp(Info::values[i], value) == 0)
                return static_cast<Enum>(i);
        return std::nullopt;
    }

    /// @brief Ensures, that the EnumInfo specialization is valid at compile-time.
    static constexpr void assertValuesValid()
    {
        static_assert(Info::values[std::size(Info::values) - 1] == nullptr, "enum values are not null-terminated");
        static_assert(std::size(Info::values) > 1, "enum values are empty");
    }
};

} // namespace dang::lua
