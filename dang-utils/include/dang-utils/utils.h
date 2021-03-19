#pragma once

#include <type_traits>

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

// TODO: C++20 replace with std::popcount
template <typename T>
constexpr int popcount(T value)
{
    static_assert(std::is_unsigned_v<T>);
    static_assert(CHAR_BIT == 8);
    constexpr std::size_t bits = sizeof(T) * CHAR_BIT;
    // Modified version of an algorithm taken from:
    // https://en.wikipedia.org/wiki/Hamming_weight
    constexpr auto m1 = static_cast<T>(0x5555555555555555);
    constexpr auto m2 = static_cast<T>(0x3333333333333333);
    constexpr auto m4 = static_cast<T>(0x0f0f0f0f0f0f0f0f);
    constexpr auto h01 = static_cast<T>(0x0101010101010101);
    value -= (value >> 1) & m1;
    value = (value & m2) + ((value >> 2) & m2);
    value = (value + (value >> 4)) & m4;
    return static_cast<int>(static_cast<T>(value * h01) >> (bits - 8));
}

// TODO: C++20 replace with std::countl_zero
template <typename T>
constexpr int countl_zero(T value)
{
    static_assert(std::is_unsigned_v<T>);
    int count = 0;
    while (value) {
        value = static_cast<T>(value >> 1);
        count++;
    }
    return sizeof(T) * CHAR_BIT - count;
}

// TODO: C++20 replace with std::countr_zero
template <typename T>
constexpr int countr_zero(T value)
{
    static_assert(std::is_unsigned_v<T>);
    int count = 0;
    while (value) {
        value = static_cast<T>(value << 1);
        count++;
    }
    return sizeof(T) * CHAR_BIT - count;
}

} // namespace dang::utils
