#pragma once

#include <array>
#include <cstddef>
#include <numeric>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Returns the combined push count of all types or std::nullopt if any count is not known at compile-time.
template <typename... TValues>
inline constexpr auto combined_push_count = (Convert<TValues>::push_count && ...)
                                                ? std::optional((0 + ... + *Convert<TValues>::push_count))
                                                : std::nullopt;

/// @brief Returns the combined push count of all values.
template <typename... TValues>
static constexpr int combinedPushCount([[maybe_unused]] const TValues&... values)
{
    return (0 + ... + [&] {
        if constexpr (Convert<TValues>::push_count)
            return *Convert<TValues>::push_count;
        else
            return Convert<TValues>::getPushCount(values);
    }());
}

namespace detail {

/// @brief Allows pushing multiple values using tuple like types.
template <typename TTuple, typename... TValues>
struct PushTupleImpl {
    using Tuple = TTuple;

    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = combined_push_count<TValues...>;

    /// @brief Returns the total push count of all values in the tuple.
    static constexpr int getPushCount(const Tuple& tuple)
    {
        static_assert(!push_count);
        return std::apply(combinedPushCount<TValues...>, tuple);
    }

    /// @brief Combines all values in the form: "a, b, c"
    static std::string getPushTypename()
    {
        if constexpr (sizeof...(TValues) > 0)
            return getPushTypenameHelper<TValues...>();
        else
            return "";
    }

    /// @brief Pushes all values in the tuple onto the stack.
    static void push(lua_State* state, const Tuple& tuple)
    {
        pushAll(state, std::index_sequence_for<TValues...>(), tuple);
    }

    /// @brief Pushes all values in the tuple onto the stack.
    static void push(lua_State* state, Tuple&& tuple)
    {
        pushAll(state, std::index_sequence_for<TValues...>(), std::move(tuple));
    }

private:
    template <std::size_t... v_indices>
    static void pushAll(lua_State* state, std::index_sequence<v_indices...>, Tuple&& tuple)
    {
        (Convert<TValues>::push(state, std::move(std::get<v_indices>(tuple))), ...);
    }

    template <std::size_t... v_indices>
    static void pushAll(lua_State* state, std::index_sequence<v_indices...>, const Tuple& tuple)
    {
        (Convert<TValues>::push(state, std::get<v_indices>(tuple)), ...);
    }

    template <typename TFirst, typename... TRest>
    static std::string getPushTypenameHelper()
    {
        std::string result{Convert<TFirst>::getPushTypename()};
        if constexpr (sizeof...(TRest) == 0)
            return result;
        else
            return result + ", " + getPushTypenameHelper<TRest...>();
    }
};

template <typename TValue, std::size_t v_count>
struct PushArrayImpl {
    using Array = std::array<TValue, v_count>;
    using ConvertValue = Convert<TValue>;

    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count =
        ConvertValue::push_count ? *ConvertValue::push_count * static_cast<int>(v_count) : std::optional<int>();

    /// @brief Returns the total push count of all values in the array.
    static constexpr int getPushCount(const Array& array)
    {
        static_assert(!push_count);
        return std::accumulate(begin(array), end(array), ConvertValue::getPushCount);
    }

    /// @brief Creates a name in the form: "type<size>"
    static std::string getPushTypename()
    {
        return ConvertValue::getPushTypename() + "<" + std::to_string(v_count) + ">";
    }

    /// @brief Pushes all values in the array onto the stack.
    static void push(lua_State* state, const Array& array)
    {
        for (const auto& value : array)
            ConvertValue::push(state, value);
    }

    /// @brief Pushes all values in the array onto the stack.
    static void push(lua_State* state, Array&& array)
    {
        for (auto&& value : array)
            ConvertValue::push(state, std::move(value));
    }
};

template <typename, typename SFINAE = void>
struct ConvertTuple {
    static constexpr bool can_check = false;
    static constexpr bool can_push = false;
};

template <typename TFirst, typename TSecond>
struct ConvertTuple<std::pair<TFirst, TSecond>,
                    std::enable_if_t<convert_can_push_v<TFirst> && convert_can_push_v<TSecond>>>
    : PushTupleImpl<std::pair<TFirst, TSecond>, TFirst, TSecond> {};

template <typename... TValues>
struct ConvertTuple<std::tuple<TValues...>, std::enable_if_t<std::conjunction_v<convert_can_push<TValues>...>>>
    : PushTupleImpl<std::tuple<TValues...>, TValues...> {};

template <typename TValue, std::size_t v_count>
struct ConvertTuple<std::array<TValue, v_count>, std::enable_if_t<convert_can_push_v<TValue>>>
    : PushArrayImpl<TValue, v_count> {};

template <typename>
struct is_tuple_helper : std::false_type {};

template <typename TFirst, typename TSecond>
struct is_tuple_helper<std::pair<TFirst, TSecond>> : std::true_type {};

template <typename... TValues>
struct is_tuple_helper<std::tuple<TValues...>> : std::true_type {};

template <typename TValue, std::size_t v_size>
struct is_tuple_helper<std::array<TValue, v_size>> : std::true_type {};

} // namespace detail

template <typename T>
struct is_tuple : detail::is_tuple_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_tuple_v = is_tuple<T>::value;

template <typename TTuple>
struct Convert<TTuple, std::enable_if_t<is_tuple_v<TTuple>>> : detail::ConvertTuple<std::remove_cv_t<TTuple>> {};

} // namespace dang::lua
