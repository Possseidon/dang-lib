#pragma once

template <typename... T>
using maybe_cv = std::tuple<T..., const T..., volatile T..., const volatile T...>;