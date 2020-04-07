#include "pch.h"
#include "Reference.h"

namespace dang::lua
{

Reference::Reference(lua_State* state, int pos)
    : state_(state)
    , ref_((lua_pushvalue(state, pos), luaL_ref(state, LUA_REGISTRYINDEX)))
{
}

Reference::~Reference()
{
    luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
}

Reference::Reference(Reference&& other) noexcept
    : state_(other.state_)
    , ref_(other.ref_)
{
    other.ref_ = LUA_NOREF;
}

Reference& Reference::operator=(Reference&& other) noexcept
{
    state_ = other.state_;
    ref_ = other.ref_;
    other.ref_ = LUA_NOREF;
    return *this;
}

lua_State* Reference::state() const
{
    return state_;
}

StackPos Reference::push() const
{
    if (ref_ != LUA_REFNIL)
        lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
    else
        lua_pushnil(state_);
    return StackPos(state_, -1);
}

StackPos Reference::push(lua_State* L) const
{
    push();
    lua_xmove(state_, L, 1);
    return StackPos(L, -1);
}

}
