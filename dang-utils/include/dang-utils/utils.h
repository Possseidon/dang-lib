#pragma once

#include "dang-utils/global.h"

namespace dang::utils {

template <typename... TFunctions>
struct Overloaded : TFunctions... {
    using TFunctions::operator()...;
};

template <typename... TFunctions>
Overloaded(TFunctions...) -> Overloaded<TFunctions...>;

} // namespace dang::utils
