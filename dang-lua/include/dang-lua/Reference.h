#pragma once

#include "utils.h"

#include "Stack.h"

namespace dang::lua
{

/// <summary>Increases the lifetime of any given stack element using the registry table.</summary>
class Reference {
public:
    /// <summary>Increases the lifetime of the given stack element by putting it in the registry table.</summary>
    Reference(lua_State* state, int pos)
        : state_(state)
        , ref_((lua_pushvalue(state, pos), luaL_ref(state, LUA_REGISTRYINDEX)))
    {
    }

    /// <summary>Increases the lifetime of the given stack element by putting it in the registry table.</summary>
    Reference(StackPos pos)
        : Reference(pos.state(), pos.pos())
    {
    }

    /// <summary>Removes the reference from the registry.</summary>
    ~Reference()
    {
        luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
    }

    Reference(const Reference&) = delete;

    Reference(Reference&& other) noexcept
        : state_(other.state_)
        , ref_(std::exchange(other.ref_, LUA_NOREF))
    {
    }

    Reference& operator=(const Reference&) = delete;

    Reference& operator=(Reference&& other) noexcept
    {
        state_ = other.state_;
        ref_ = std::exchange(other.ref_, LUA_NOREF);
        return *this;
    }

    /// <summary>Turns the top stack element into a reference and pops the original value.</summary>
    static Reference take(lua_State* state)
    {
        return Reference(state);
    }

    /// <summary>The associated Lua state for the reference.</summary>
    lua_State* state() const
    {
        return state_;
    }

    /// <summary>Pushes the referenced value on the stack again.</summary>
    void pushValue() const
    {
        if (ref_ != LUA_REFNIL)
            lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
        else
            lua_pushnil(state_);
    }

    /// <summary>Pushes the referenced value on the stack again and returns a wrapper to it.</summary>
    StackPos pushNamedValue() const
    {
        pushValue();
        return StackPos(state_);
    }

    /// <summary>Pushes the referenced value on the stack again and returns a managed wrapper to it.</summary>
    AutoStackPos value() const
    {
        pushValue();
        return AutoStackPos(state_);
    }

    /// <summary>Pushes the referenced value on the stack again and returns a managed wrapper to it.</summary>
    AutoStackPos operator*() const
    {
        return value();
    }

    /// <summary>Pushes the referenced value on the stack again and returns a managed wrapper to it.</summary>
    AutoStackPos operator->() const
    {
        return value();
    }

private:
    /// <summary>Turns the top of the stack into a reference and pops the original value.</summary>
    explicit Reference(lua_State* state)
        : state_(state)
        , ref_(luaL_ref(state, LUA_REGISTRYINDEX))
    {
    }

    lua_State* state_;
    int ref_;
};

using SharedReference = std::shared_ptr<Reference>;

}
