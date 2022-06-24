#pragma once

#include <optional>
#include <type_traits>

#include "dang-lua/Reference.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

template <typename TReference>
struct Convert<TReference, std::enable_if_t<std::is_same_v<std::remove_cv_t<TReference>, Reference>>> {
    // --- Check ---

    static constexpr bool can_check = false;

    // --- Push ---

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return "reference"; }

    static void push([[maybe_unused]] lua_State* state, const Reference& reference)
    {
        assert(reference.state() == state);
        reference.push();
    }
};

} // namespace dang::lua
