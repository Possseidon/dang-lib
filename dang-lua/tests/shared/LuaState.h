#pragma once

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "lua.hpp"

/// @brief A very basic wrapper for owned Lua states, meant for use in testing.
class LuaState {
public:
    LuaState() = default;
    ~LuaState() { lua_close(state_); }

    LuaState(const LuaState&) = delete;
    LuaState(LuaState&&) = delete;
    LuaState& operator=(const LuaState&) = delete;
    LuaState& operator=(LuaState&&) = delete;

    /// @brief Provides easy access to the underlying Lua state.
    auto operator*() { return state_; }

    /// @brief Returns the error message, that was thrown by the given callback.
    template <typename TFunc>
    auto shouldThrow(TFunc func)
    {
        constexpr auto lua_func = +[](lua_State* state) {
            (*static_cast<TFunc*>(lua_touserdata(state, lua_upvalueindex(1))))();
            return 0;
        };
        lua_pushlightuserdata(state_, &func);
        lua_pushcclosure(state_, lua_func, 1);
        if (lua_pcall(state_, 0, 0, 0) == LUA_OK)
            return std::string();
        auto msg = dang::lua::Convert<std::string>::at(state_, -1).value_or(std::string());
        lua_pop(state_, 1);
        return msg;
    }

private:
    lua_State* state_ = luaL_newstate();
};
