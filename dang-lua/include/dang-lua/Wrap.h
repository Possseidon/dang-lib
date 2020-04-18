#pragma once

#include "utils.h"

#include "Convert.h"
#include "Stack.h"
#include "Reference.h"

namespace dang::lua
{

namespace detail
{

template <typename TFunc>
struct SignatureInfo;

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(*)(TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TArgs...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { Convert<TArgs>::check(state, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<TArgs...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(*)(lua_State*, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<lua_State*, TArgs...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { state, Convert<TArgs>::check(state, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<lua_State*, TArgs...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(&)(TArgs...)> : SignatureInfo<TRet(*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<std::function<TRet(TArgs...)>> : SignatureInfo<TRet(*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TClass*, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(state, 1), Convert<TArgs>::check(state, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, std::remove_reference_t<TArgs>...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(lua_State*, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TClass*, lua_State*, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(state, 1), state, Convert<TArgs>::check(state, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, lua_State*, std::remove_reference_t<TArgs>...> convertArguments(lua_State* state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) const> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) noexcept> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) const noexcept> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

/// <summary>A function wrapper, that expects a std::function of the templated type in the first upvalue slot of the called closure.</summary>
template <typename Func>
int wrappedFunction(lua_State* state)
{
    using Info = detail::SignatureInfo<Func>;
    Func func = Convert<Func>::check(state, lua_upvalueindex(1));
    if constexpr (std::is_void_v<Info::Return>) {
        std::apply(func, Info::convertArguments(state));
        return 0;
    }
    else {
        using Result = decltype(std::apply(func, Info::convertArguments(state)));
        return Convert<Result>::push(state, std::apply(func, Info::convertArguments(state)));
    }
}

}

/// <summary>Wraps the template supplied function into a Lua function in an almost cost-free way.</summary>
template <auto Func>
int wrap(lua_State* state)
{
    using Info = detail::SignatureInfo<decltype(Func)>;
    if constexpr (std::is_void_v<Info::Return>) {
        std::apply(Func, Info::convertArguments(state));
        return 0;
    }
    else {
        using Result = decltype(std::apply(Func, Info::convertArguments(state)));
        return Convert<Result>::push(state, std::apply(Func, Info::convertArguments(state)));
    }
}

/// <summary>Turns the function-like object into a std::function and pushes a wrapped closure onto the stack.</summary>
template <typename TFunc>
void pushFunction(lua_State* state, TFunc&& func)
{
    auto wrapped_function = std::function(std::forward<TFunc>(func));
    Convert<decltype(std::function(func))>::push(state, wrapped_function);
    lua_pushcclosure(state, detail::wrappedFunction<decltype(wrapped_function)>, 1);
}

/// <summary>Returns a luaL_Reg with the wrapped template supplied function and given name.</summary>
template <auto Func>
constexpr luaL_Reg reg(const char* name)
{
    return { name, dlua::wrap<Func> };
}

}
