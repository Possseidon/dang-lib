#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/convert/Nil.h"
#include "dang-lua/global.h"

namespace dang::lua {

namespace detail {

template <typename TVariant, typename SFINAE = void>
struct CheckVariant {
    static constexpr bool can_check = false;
};

template <typename... TOptions>
struct CheckVariant<std::variant<TOptions...>,
                    std::enable_if_t<std::conjunction_v<convert_checks_exactly<TOptions, 1>...>>> {
    using Variant = std::variant<TOptions...>;

    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = (Convert<TOptions>::check_count && ...);

    /// @brief Combines all possible options of the variant in the form: "a, b or c"
    static std::string getCheckTypename() { return getCheckTypenameHelper<TOptions...>(); }

    /// @brief Whether at least one option matches exactly.
    static constexpr bool isExact(lua_State* state, int pos) { return (Convert<TOptions>::isExact(state, pos) || ...); }

    /// @brief Whether at least one option is valid.
    static constexpr bool isValid(lua_State* state, int pos) { return (Convert<TOptions>::isValid(state, pos) || ...); }

    /// @brief Returns the first type that does not return std::nullopt or returns std::nullopt itself if none was
    /// found.
    static std::optional<Variant> at(lua_State* state, int pos)
    {
        return atHelper(state, pos, TypeList<TOptions...>());
    }

    /// @brief Returns the first type that does not return std::nullopt or raises and argument error if none was found.
    static Variant check(lua_State* state, int arg)
    {
        if (auto value = at(state, arg))
            return *value;
        noreturn_luaL_typeerror(state, arg, getCheckTypename().c_str());
    }

private:
    template <typename...>
    struct TypeList {};

    template <typename TFirst, typename... TRest>
    static std::optional<Variant> atHelper(lua_State* state, int pos, TypeList<TFirst, TRest...>)
    {
        auto value = Convert<TFirst>::at(state, pos);
        return value ? value : atHelper(state, pos, TypeList<TRest...>());
    }

    static std::optional<Variant> atHelper(lua_State*, int, TypeList<>) { return std::nullopt; }

    template <typename TFirst, typename... TRest>
    static std::string getCheckTypenameHelper()
    {
        std::string result{Convert<TFirst>::getCheckTypename()};
        if constexpr (sizeof...(TRest) == 0)
            return result;
        else if constexpr (sizeof...(TRest) == 1)
            return result + " or " + getCheckTypenameHelper<TRest...>();
        else
            return result + ", " + getCheckTypenameHelper<TRest...>();
    }
};

template <typename TVariant, typename SFINAE = void>
struct PushVariant {
    static constexpr bool can_push = false;
};

template <typename... TOptions>
struct PushVariant<std::variant<TOptions...>,
                   std::enable_if_t<std::conjunction_v<convert_pushes_exactly<TOptions, 1>...>>> {
    using Variant = std::variant<TOptions...>;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    /// @brief Combines all possible options of the variant in the form: "a|b|c"
    static std::string getPushTypename() { return getPushTypenameHelper<TOptions...>(); }

    /// @brief Pushes the value of the variant.
    static void push(lua_State* state, const Variant& variant)
    {
        std::visit(
            [state](const auto& value) {
                using ConvertValue = Convert<std::remove_reference_t<decltype(value)>>;
                ConvertValue::push(state, value);
            },
            variant);
    }

    static void push(lua_State* state, Variant&& variant)
    {
        std::visit(
            [state](auto&& value) {
                using ConvertValue = Convert<std::remove_reference_t<decltype(value)>>;
                ConvertValue::push(state, std::move(value));
            },
            std::move(variant));
    }

private:
    template <typename TFirst, typename... TRest>
    static std::string getPushTypenameHelper()
    {
        std::string result{Convert<TFirst>::getPushTypename()};
        if constexpr (sizeof...(TRest) == 0)
            return result;
        else
            return result + "|" + getPushTypenameHelper<TRest...>();
    }
};

template <typename>
struct is_variant_helper : std::false_type {};

template <typename... TOptions>
struct is_variant_helper<std::variant<TOptions...>> : std::true_type {};

/// @brief Allows for conversion of different values using std::variant.
template <typename TVariant>
struct ConvertVariant
    : CheckVariant<TVariant>
    , PushVariant<TVariant> {};

} // namespace detail

template <typename T>
struct is_variant : detail::is_variant_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_variant_v = is_variant<T>::value;

template <typename TVariant>
struct Convert<TVariant, std::enable_if_t<is_variant_v<TVariant>>>
    : detail::ConvertVariant<std::remove_cv_t<TVariant>> {};

template <typename TMonostate>
struct is_nil<TMonostate, std::enable_if_t<std::is_same_v<std::remove_cv_t<TMonostate>, std::monostate>>>
    : std::true_type {};

} // namespace dang::lua
