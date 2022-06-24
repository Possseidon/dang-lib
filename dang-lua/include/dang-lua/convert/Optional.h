#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

namespace detail {

template <typename TContained, typename SFINAE = void>
struct CheckOptional {
    static constexpr bool can_check = false;
};

template <typename TContained>
struct CheckOptional<TContained, std::enable_if_t<convert_checks_exactly_v<TContained, 1>>> {
    using Optional = std::optional<TContained>;
    using ConvertContained = Convert<TContained>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "optional " + ConvertContained::getCheckTypename(); }

    /// @brief Whether the value at the given stack position is nil or a valid value.
    static bool isExact(lua_State* state, int pos)
    {
        return lua_isnoneornil(state, pos) || ConvertContained::isExact(state, pos);
    }

    /// @brief Whether the value at the given stack position is nil or an exact value.
    static bool isValid(lua_State* state, int pos)
    {
        return lua_isnoneornil(state, pos) || ConvertContained::isValid(state, pos);
    }

    /// @brief Returns an optional containing a std::nullopt for nil values or a single std::nullopt for invalid values.
    static std::optional<Optional> at(lua_State* state, int pos)
    {
        if (lua_isnoneornil(state, pos))
            return Optional();
        if (auto result = ConvertContained::at(state, pos))
            return Optional(result);
        return std::nullopt;
    }

    /// @brief Returns std::nullopt for nil values or raises an error for invalid values.
    static Optional check(lua_State* state, int arg)
    {
        if (lua_isnoneornil(state, arg))
            return std::nullopt;
        if (auto result = ConvertContained::at(state, arg))
            return result;
        noreturn_luaL_typeerror(state, arg, getCheckTypename().c_str());
    }
};

template <typename TContained, typename SFINAE = void>
struct PushOptional {
    static constexpr bool can_push = false;
};

template <typename TContained>
struct PushOptional<TContained, std::enable_if_t<convert_pushes_exactly_v<TContained, 1>>> {
    using Optional = std::optional<TContained>;
    using ConvertContained = Convert<TContained>;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return ConvertContained::getPushTypename() + "?"; }

    /// @brief Pushes the given value or nil onto the stack.
    static void push(lua_State* state, const Optional& value)
    {
        if (value)
            ConvertContained::push(state, *value);
        else
            lua_pushnil(state);
    }

    /// @brief Pushes the given moved value or nil onto the stack.
    static void push(lua_State* state, Optional&& value)
    {
        if (value)
            ConvertContained::push(state, *std::move(value));
        else
            lua_pushnil(state);
    }
};

template <typename>
struct is_optional_helper : std::false_type {};

template <typename TContained>
struct is_optional_helper<std::optional<TContained>> : std::true_type {};

/// @brief Allows for conversion for possible nil values using std::optional.
template <typename>
struct ConvertOptional;

template <typename TContained>
struct ConvertOptional<std::optional<TContained>>
    : CheckOptional<TContained>
    , PushOptional<TContained> {};

} // namespace detail

template <typename T>
struct is_optional : detail::is_optional_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_optional_v = is_optional<T>::value;

template <typename TOptional>
struct Convert<TOptional, std::enable_if_t<is_optional_v<TOptional>>>
    : detail::ConvertOptional<std::remove_cv_t<TOptional>> {};

} // namespace dang::lua
