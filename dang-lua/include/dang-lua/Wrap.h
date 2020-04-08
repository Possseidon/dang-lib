#pragma once

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
    static std::tuple<TArgs...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { Convert<TArgs>::check(L, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<TArgs...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(*)(lua_State*, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<lua_State*, TArgs...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { L, Convert<TArgs>::check(L, static_cast<int>(Indices + 1))... };
    }

    static std::tuple<lua_State*, TArgs...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
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
    static std::tuple<TClass*, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(L, 1), Convert<TArgs>::check(L, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, std::remove_reference_t<TArgs>...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(lua_State*, TArgs...)> {
    using Return = TRet;
    using Arguments = std::tuple<TArgs...>;

    template <std::size_t... Indices>
    static std::tuple<TClass*, lua_State*, std::remove_reference_t<TArgs>...> convertArgumentsHelper(lua_State* L, std::index_sequence<Indices...>)
    {
        return { &Convert<TClass>::check(L, 1), L, Convert<TArgs>::check(L, static_cast<int>(Indices + 2))... };
    }

    static std::tuple<TClass*, lua_State*, std::remove_reference_t<TArgs>...> convertArguments(lua_State* L)
    {
        return convertArgumentsHelper(L, std::index_sequence_for<TArgs...>{});
    }
};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) const> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) noexcept> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TClass::*)(TArgs...) const noexcept> : SignatureInfo<TRet(TClass::*)(TArgs...)> {};

}

/// <summary>Wraps the in the template supplied function into a Lua function.</summary>
template <auto Func>
int wrap(lua_State* L)
{
    using Info = detail::SignatureInfo<decltype(Func)>;
    if constexpr (std::is_void_v<Info::Return>) {
        std::apply(Func, Info::convertArguments(L));
        return 0;
    }
    else {
        using Result = decltype(std::apply(Func, Info::convertArguments(L)));
        return Convert<Result>::push(L, std::apply(Func, Info::convertArguments(L)));
    }
}

/// <summary>Returns a luaL_Reg with the wrapped template supplied function and given name.</summary>
template <auto Func>
constexpr luaL_Reg reg(const char* name)
{
    return { name, dlua::wrap<Func> };
}

namespace detail
{

template <typename Func>
int wrappedFunction(lua_State* L)
{
    using Info = detail::SignatureInfo<Func>;
    Func func = Convert<Func>::check(L, lua_upvalueindex(1));
    if constexpr (std::is_void_v<Info::Return>) {
        std::apply(func, Info::convertArguments(L));
        return 0;
    }
    else {
        using Result = decltype(std::apply(func, Info::convertArguments(L)));
        return Convert<Result>::push(L, std::apply(func, Info::convertArguments(L)));
    }
}

}

/// <summary>Pushes a wrapped closure of the given function onto the stack.</summary>
template <typename TFunc>
void pushFunction(lua_State* L, TFunc any_func)
{
    Convert<decltype(std::function(any_func))>::push(L, std::function(any_func));
    lua_pushcclosure(L, detail::wrappedFunction<decltype(std::function(any_func))>, 1);
}

class Function : public Reference {
    using Reference::Reference;
};

/// <summary>Wraps a Lua function and has an overloaded call operator for the given return type.</summary>
template <typename TRet>
class FunctionRet : public Reference {
public:
    using Reference::Reference;

    /// <summary>Mimics a full call to the Lua function with the given parameters and return type.</summary>
    template <typename... TArgs>
    TRet operator()(TArgs&&... args) const
    {
        return call<TRet>(std::forward<TArgs>(args)...);
    }
};

/// <summary>Wraps a Lua function and has an overloaded call operator for a tuple out of the given return type.</summary>
template <typename... TRets>
class FunctionMultRet : public Reference {
public:
    using Reference::Reference;

    /// <summary>Mimics a full call to the Lua function with the given parameters and return types as std::tuple.</summary>
    template <typename... TArgs>
    auto operator()(TArgs&&... args) const
    {
        return callMultRet<TRets...>(std::forward<TArgs>(args)...);
    }
};

namespace detail
{

template <typename T>
struct FunctionConverter {
    static constexpr std::optional<int> PushCount = 1;

    /// <summary>Returns, wether the given stack position is a function.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return lua_isfunction(L, pos);
    }

    /// <summary>Returns, wether the given stack position is a function.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return isExact(L, pos);
    }

    /// <summary>Returns the function at the given stack position or std::nullopt, if it is not a function.</summary>
    static std::optional<T> at(lua_State* L, int pos)
    {
        if (lua_isfunction(L, pos))
            return T(L, pos);
        return std::nullopt;
    }

    /// <summary>Returns the function at the given argument stack position or raises and error, if it is not a function.</summary>
    static T check(lua_State* L, int arg)
    {
        luaL_checktype(L, arg, LUA_TFUNCTION);
        return T(L, arg);
    }

    /// <summary>Pushes the given function onto the stack.</summary>
    static int push(lua_State* L, const T& function)
    {
        function.push(L);
        return *PushCount;
    }
};

}

/// <summary>Allows for conversion between Lua functions and the Function wrapper class.</summary>
template <>
struct Convert<Function> : detail::FunctionConverter<Function> {};

/// <summary>Allows for conversion between Lua functions and the Function wrapper class.</summary>
template <typename TRet>
struct Convert<FunctionRet<TRet>> : detail::FunctionConverter<FunctionRet<TRet>> {};

/// <summary>Allows for conversion between Lua functions and the Function wrapper class.</summary>
template <typename... TRets>
struct Convert<FunctionMultRet<TRets...>> : detail::FunctionConverter<FunctionMultRet<TRets...>> {};

}
