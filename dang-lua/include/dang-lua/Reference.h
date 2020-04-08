#pragma once

#include "Stack.h"

namespace dang::lua
{

/// <summary>Increases the lifetime of any given stack position using the registry table.</summary>
class Reference {
public:
    /// <summary>Increases the lifetime of the given stack position by putting it in the registry table.</summary>
    Reference(lua_State* state, int pos);
    /// <summary>Removes the reference from the registry.</summary>
    ~Reference();

    Reference(const Reference&) = delete;
    Reference(Reference&& other) noexcept;
    Reference& operator=(const Reference&) = delete;
    Reference& operator=(Reference&& other) noexcept;

    /// <summary>The associated Lua state for the reference.</summary>
    lua_State* state() const;

    /// <summary>Pushes the referenced value on the stack again, returning a wrapper to the pushed value.</summary>
    StackPos push() const;
    /// <summary>Pushes the referenced value on the stack of the given Lua state, returning a wrapper to the pushed value.</summary>
    StackPos push(lua_State* L) const;

    /// <summary>Calls the referenced value with the given parameters and returns the result.</summary>
    template <typename TRet = void, typename... TArgs>
    TRet call(TArgs&&... args) const
    {
        if (std::is_void_v<TRet>) {
            push().call<TRet>(std::forward<TArgs>(args)...);
            lua_pop(state_, 1);
        }
        else {
            auto result = push().call<TRet>(std::forward<TArgs>(args)...);
            lua_pop(state_, 1);
            return result;
        }
    }

    /// <summary>Calls the referenced value with the given parameters and returns the results as a std::tuple.</summary>
    template <typename... TResult, typename... TArgs>
    auto callMultRet(TArgs&&... args) const
    {
        return call<std::tuple<TResult...>>(std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the referenced value, discarding any return values.</summary>
    template <typename... TArgs>
    void operator()(TArgs&&... args) const
    {
        return call(std::forward<TArgs>(args)...);
    }

private:
    lua_State* state_;
    int ref_;
};

using SharedReference = std::shared_ptr<Reference>;

}
