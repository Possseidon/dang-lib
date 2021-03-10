#pragma once

#include "dang-utils/global.h"

namespace dang::utils {

namespace detail {

struct AlwaysFalseHelper {};

} // namespace detail

template <typename...>
struct always_false : std::false_type {};

template <>
struct always_false<detail::AlwaysFalseHelper> : std::true_type {};

template <typename... TArgs>
inline constexpr auto always_false_v = always_false<TArgs...>::value;

template <auto v>
struct constant : std::integral_constant<decltype(v), v> {};

template <typename... TFunctions>
struct Overloaded : TFunctions... {
    using TFunctions::operator()...;
};

template <typename... TFunctions>
Overloaded(TFunctions...) -> Overloaded<TFunctions...>;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_equal_to_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_equal_to_comparable<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() == std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_equal_to_comparable_v = is_equal_to_comparable<TLeft, TRight>::value;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_not_equal_to_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_not_equal_to_comparable<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() != std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_not_equal_to_comparable_v = is_not_equal_to_comparable<TLeft, TRight>::value;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_less_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_less_comparable<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() < std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_less_comparable_v = is_less_comparable<TLeft, TRight>::value;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_less_equal_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_less_equal_comparable<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() <= std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_less_equal_comparable_v = is_less_equal_comparable<TLeft, TRight>::value;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_greater_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_greater_comparable<TLeft, TRight, std::void_t<decltype(std::declval<TLeft>() > std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_greater_comparable_v = is_greater_comparable<TLeft, TRight>::value;

template <typename TLeft, typename TRight = TLeft, typename = void>
struct is_greater_equal_comparable : std::false_type {};

template <typename TLeft, typename TRight>
struct is_greater_equal_comparable<TLeft,
                                   TRight,
                                   std::void_t<decltype(std::declval<TLeft>() >= std::declval<TRight>())>>
    : std::true_type {};

template <typename TLeft, typename TRight = TLeft>
inline constexpr auto is_greater_equal_comparable_v = is_greater_equal_comparable<TLeft, TRight>::value;

} // namespace dang::utils
