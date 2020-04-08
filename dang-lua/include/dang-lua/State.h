#pragma once

#include "Convert.h"
#include "Stack.h"
#include "Wrap.h"

namespace dang::lua
{

/// <summary>Wraps a Lua state and some useful methods to interact with the lua stack.</summary>
class State {
public:
    /// <summary>Allows for implicit conversion from an existing Lua state.</summary>
    State(lua_State* state);

    /// <summary>Allows for implicit conversion to the actual Lua state pointer.</summary>
    operator lua_State* () const;

    /// <summary>Pushes a wrapped version of the given template parameter function onto the stack and returns a wrapper to it.</summary>
    template <auto Func, typename = detail::SignatureInfo<decltype(Func)>>
    StackPos push()
    {
        lua_pushcfunction(state_, wrap<Func>);
        return StackPos(state_, -1);
    }

    /// <summary>Pushes a wrapped closure of the given function onto the stack and returns a wrapper to it.</summary>
    template <typename TFunc>
    StackPos pushFunction(TFunc func)
    {
        dlua::pushFunction<TFunc>(state_, func);
        return StackPos(state_, -1);
    }

    /// <summary>Pushes an in-place constructed object onto the stack and returns a wrapper to it/them.</summary>
    template <typename T, typename... TArgs>
    auto push(TArgs&&... args)
    {
        if constexpr (Convert<T>::PushCount == 1) {
            Convert<T>::push(state_, std::forward<TArgs>(args)...);
            return StackPos(state_, -1);
        }
        else {
            int old_top = lua_gettop(state_);
            Convert<T>::push(state_, std::forward<TArgs>(args)...);
            return VarStackPos(state_, old_top + 1);
        }
    }

    /// <summary>Pushes a move constructed object on the stack and returns a wrapper to it.</summary>
    template <typename T>
    StackPos push(T&& value)
    {
        if constexpr (Convert<T>::PushCount == 1) {
            Convert<T>::push(state_, std::forward<T>(value));
            return StackPos(state_, -1);
        }
        else {
            int old_top = lua_gettop(state_);
            Convert<T>::push(state_, std::forward<T>(value));
            return StackPos(state_, old_top + 1);
        }

    }

    /// <summary>Pushes a reference to the given object onto the stack and returns a wrapper to it.</summary>
    template <typename T>
    StackPos pushRef(T& value)
    {
        Convert<T>::pushRef(state_, value);
        return StackPos(state_, -1);
    }

    /// <summary>Pushes the global table on the stack and returns a wrapper to it and returns a wrapper to it.</summary>
    StackPos pushGlobal()
    {
        lua_pushglobaltable(state_);
        return StackPos(state_, -1);
    }

    /// <summary>Pops one or as many values as specified from the stack.</summary>
    void pop(int count = 1) const;

    /// <summary>Returns the top element of the stack.</summary>
    StackPos top() const;

    /// <summary>Sets the size of the Lua stack, filling new values with nil.</summary>
    void setTop(int new_top) const;

    /// <summary>Wraps the given one-based stack position.</summary>
    StackPos operator[](int pos) const;

    /// <summary>Used as in combination with a return statement to return any value.</summary>
    template <typename TRet>
    Ret ret(TRet&& result)
    {
        return Ret::push(state_, std::forward<TRet>(result));
    }

    /// <summary>Used as in combination with a return statement to return any values.</summary>
    template <typename... TRets>
    MultRet multret(TRets&&... results)
    {
        return MultRet::push(state_, std::forward<TRets>(results)...);
    }

private:
    lua_State* state_;
};

/// <summary>Owns a Lua state and closes it, once it goes out of scope.</summary>
class OwnedState : public State {
public:
    OwnedState(bool open_libs = true);
    ~OwnedState();

    OwnedState(const OwnedState&) = delete;
    OwnedState(OwnedState&&) = delete;
    OwnedState& operator=(const OwnedState&) = delete;
    OwnedState& operator=(OwnedState&&) = delete;
};

namespace detail
{

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(__cdecl*)(State, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<State, TArgs...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { L, Convert<TArgs>::check(L, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<State, TArgs...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(__cdecl TClass::*)(State, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TClass*, State, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(L, 1), L, Convert<TArgs>::check(L, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, State, std::remove_reference_t<TArgs>...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
    }
};

}

}
