#pragma once

#include "utils.h"

#include "Convert.h"
#include "Stack.h"
#include "Wrap.h"

namespace dang::lua
{

/// <summary>Wraps a Lua state and some useful methods to interact with its stack.</summary>
/// <remarks>As the case for lua_State*, this class can be used as an optional first parameter to wrapped Lua functions.</remarks>
class State {
public:
    /// <summary>Allows for implicit conversion from an existing Lua state.</summary>
    State(lua_State* state)
        : state_(state)
    {
    }

    /// <summary>Returns the actual state pointer, which it is also implicitly convertible to.</summary>
    lua_State* state() const
    {
        return state_;
    }

    /// <summary>Allows for implicit conversion to the actual Lua state pointer.</summary>
    operator lua_State* () const
    {
        return state_;
    }

    /// <summary>Returns a wrapper to the current top element of the stack.</summary>
    StackPos top() const
    {
        return StackPos(state_);
    }

    /// <summary>Returns a wrapper to the whole current stack.</summary>
    VarStackPos stack() const
    {
        return VarStackPos(state_);
    }

    /// <summary>Pops one or as many values as specified from the stack.</summary>
    void pop(int count = 1) const
    {
        lua_pop(state_, count);
    }

    /// <summary>Sets the size of the Lua stack, filling new values with nil.</summary>
    void setTop(int new_top) const
    {
        lua_settop(state_, new_top);
    }

    /// <summary>Returns, wether the thread can yield.</summary>
    bool isYieldable() const
    {
        return lua_isyieldable(state_);
    }

    /// <summary>Pushes a wrapped version of the given template parameter function onto the stack.</summary>
    template <auto Func, typename = detail::SignatureInfo<decltype(Func)>>
    void pushFunction() const
    {
        lua_pushcfunction(state_, wrap<Func>);
    }

    /// <summary>Pushes a wrapped version of the given template parameter function onto the stack and returns a wrapper to it.</summary>
    template <auto Func, typename = detail::SignatureInfo<decltype(Func)>>
    StackPos pushNamedFunction() const
    {
        pushFunction<Func>();
        return StackPos(state_);
    }

    /// <summary>Pushes a wrapped version of the given template parameter function onto the stack and returns a managed wrapper to it.</summary>
    template <auto Func, typename = detail::SignatureInfo<decltype(Func)>>
    AutoStackPos function() const
    {
        pushFunction<Func>();
        return AutoStackPos(state_);
    }

    /// <summary>Pushes a wrapped closure of the given function onto the stack.</summary>
    template <typename TFunc>
    void pushFunction(TFunc func) const
    {
        dlua::pushFunction<TFunc>(state_, func);
    }

    /// <summary>Pushes a wrapped closure of the given function onto the stack and returns a wrapper to it.</summary>
    template <typename TFunc>
    StackPos pushNamedFunction(TFunc func) const
    {
        pushFunction<TFunc>(func);
        return StackPos(state_);
    }

    /// <summary>Pushes a wrapped closure of the given function onto the stack and returns a managed wrapper to it.</summary>
    template <typename TFunc>
    AutoStackPos function(TFunc func) const
    {
        pushFunction<TFunc>(func);
        return AutoStackPos(state_);
    }

    /// <summary>Pushes an in-place constructed object onto the stack and returns how many elements got pushed.</summary>
    template <typename T, typename... TArgs>
    int pushValue(TArgs&&... args) const
    {
        return Convert<T>::push(state_, std::forward<TArgs>(args)...);
    }

    /// <summary>Pushes an in-place constructed object onto the stack and returns a wrapper to it/them.</summary>
    template <typename T, typename... TArgs>
    auto pushNamedValue(TArgs&&... args) const
    {
        if constexpr (Convert<T>::PushCount == 1) {
            pushValue<T, TArgs...>(std::forward<TArgs>(args)...);
            return StackPos(state_);
        }
        else {
            int count = pushValue<T, TArgs...>(std::forward<TArgs>(args)...);
            return VarStackPos(state_, count);
        }
    }

    /// <summary>Pushes a move constructed object on the stack and returns how many elements got pushed.</summary>
    template <typename T>
    int pushValue(T&& value) const
    {
        return Convert<T>::push(state_, std::forward<T>(value));
    }

    /// <summary>Pushes a move constructed object on the stack and returns a wrapper to it/them.</summary>
    template <typename T>
    auto pushNamedValue(T&& value) const
    {
        if constexpr (Convert<T>::PushCount == 1) {
            pushValue(std::forward<T>(value));
            return StackPos(state_, -1);
        }
        else {
            int count = pushValue(std::forward<T>(value));
            return VarStackPos::top(state_, count);
        }
    }

    /// <summary>Pushes a move constructed object on the stack and returns a managed wrapper to it/them.</summary>
    template <typename T>
    auto value(T&& value) const
    {
        if constexpr (Convert<T>::PushCount == 1) {
            pushValue(std::forward<T>(value));
            return AutoStackPos(state_);
        }
        else {
            int count = pushValue(std::forward<T>(value));
            return AutoVarStackPos(state_, count);
        }
    }

    /// <summary>Pushes a move constructed object on the stack and returns a managed wrapper to it/them.</summary>
    template <typename T>
    auto operator()(T&& value) const
    {
        return State::value(std::forward<T>(value));
    }

    /// <summary>Pushes a reference to the given object onto the stack.</summary>
    template <typename T>
    void pushRef(T& value) const
    {
        Convert<T>::pushRef(state_, value);
    }

    /// <summary>Pushes a reference to the given object onto the stack and returns a wrapper to it.</summary>
    template <typename T>
    StackPos pushNamedRef(T& value) const
    {
        pushRef<T>(value);
        return StackPos(state_);
    }

    /// <summary>Pushes a reference to the given object onto the stack and returns a managed wrapper to it.</summary>
    template <typename T>
    AutoStackPos ref(T& value) const
    {
        pushRef<T>(value);
        return AutoStackPos(state_);
    }

    /// <summary>Pushes the global table on the stack.</summary>
    void pushGlobalTable() const
    {
        lua_pushglobaltable(state_);
    }

    /// <summary>Pushes the global table on the stack and returns a wrapper to it.</summary>
    StackPos pushNamedGlobalTable() const
    {
        pushGlobalTable();
        return StackPos(state_);
    }

    /// <summary>Pushes the global table on the stack and returns a managed wrapper to it.</summary>
    AutoStackPos globalTable() const
    {
        pushGlobalTable();
        return AutoStackPos(state_);
    }

private:
    lua_State* state_;
};

/// <summary>Owns a Lua state and closes it, once it goes out of scope.</summary>
class OwnedState : public State {
public:
    /// <summary>Initializes a new Lua state using the default allocation function and opens all default libraries, unless specified not to.</summary>
    OwnedState(bool open_libs = true)
        : State(luaL_newstate())
    {
        if (open_libs)
            luaL_openlibs(*this);
    }

    /// <summary>Closes the Lua state using lua_close.</summary>
    ~OwnedState()
    {
        lua_close(*this);
    }

    OwnedState(const OwnedState&) = delete;
    OwnedState(OwnedState&&) = delete;
    OwnedState& operator=(const OwnedState&) = delete;
    OwnedState& operator=(OwnedState&&) = delete;
};

namespace detail
{

// Allows for usage of dlua::State as first parameter to wrapped functions

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(*)(State, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<State, TArgs...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { state, Convert<TArgs>::check(state, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<State, TArgs...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(State, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TClass*, State, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(state, 1), state, Convert<TArgs>::check(state, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, State, std::remove_reference_t<TArgs>...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

}

}
