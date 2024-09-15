#pragma once

#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

template <typename TLuaState>
struct Convert<TLuaState, std::enable_if_t<std::is_same_v<std::remove_cv_t<TLuaState>, lua_State*>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 0;

    static lua_State* check(lua_State* state, int) { return state; }

    // --- Push ---

    static constexpr bool can_push = false;
};

} // namespace dang::lua
