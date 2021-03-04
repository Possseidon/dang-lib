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

} // namespace dang::utils
