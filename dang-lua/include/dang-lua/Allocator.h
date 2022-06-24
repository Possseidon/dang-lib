#pragma once

#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Wraps allocation function and optional userdata, which Lua passes to this function.
struct Allocator {
    constexpr Allocator(lua_Alloc function, void* userdata = nullptr)
        : function(function)
        , userdata(userdata)
    {}

    lua_Alloc function;
    void* userdata;
};

} // namespace dang::lua
