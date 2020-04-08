#include "pch.h"
#include "State.h"

namespace dang::lua
{

State::State(lua_State* state)
    : state_(state)
{
}

State::operator lua_State* () const
{
    return state_;
}

void State::pop(int count) const
{
    lua_pop(state_, count);
}

StackPos State::top() const
{
    return StackPos(state_, -1);
}

void State::setTop(int new_top) const
{
    lua_settop(state_, new_top);
}

StackPos State::operator[](int pos) const
{
    return StackPos(state_, pos);
}

OwnedState::OwnedState(bool open_libs)
    : State(luaL_newstate())
{
    if (open_libs)
        luaL_openlibs(*this);
}

OwnedState::~OwnedState()
{
    lua_close(*this);
}

}
