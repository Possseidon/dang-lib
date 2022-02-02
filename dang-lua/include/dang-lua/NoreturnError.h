#pragma once

#include "dang-lua/global.h"

// Lua error functions with the [[noreturn]] attribute.

// clang gives a warnings, which can safely be disabled.
// gcc's warning cannot be disabled, so std::abort is used here.

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#endif

[[noreturn]] inline void noreturn_lua_error(lua_State* state)
{
    lua_error(state);
#ifdef __GNUC__
    std::abort();
#endif
}

[[noreturn]] inline void noreturn_luaL_error(lua_State* state, const char* message)
{
    lua_pushstring(state, message);
    lua_error(state);
#ifdef __GNUC__
    std::abort();
#endif
}

[[noreturn]] inline void noreturn_luaL_typeerror(lua_State* state, int arg, const char* type_name)
{
    luaL_typeerror(state, arg, type_name);
#ifdef __GNUC__
    std::abort();
#endif
}

[[noreturn]] inline void noreturn_luaL_argerror(lua_State* state, int arg, const char* extra_message)
{
    luaL_argerror(state, arg, extra_message);
#ifdef __GNUC__
    std::abort();
#endif
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
