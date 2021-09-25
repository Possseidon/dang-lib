#pragma once

template <typename... T>
using maybe_const = std::tuple<T..., const T...>;
