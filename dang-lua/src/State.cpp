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
