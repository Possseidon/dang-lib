#pragma once

#include "Convert.h"
#include "Stack.h"

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

/// <summary>Wraps the in the template supplied function into a lua function.</summary>
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
constexpr luaL_Reg wrap(const char* name)
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

/// <summary>Increases the lifetime of any given stack position using the registry table.</summary>
class Reference {
public:
    /// <summary>Increases the lifetime of the given stack position by putting it in the registry table.</summary>
    Reference(lua_State* state, int pos)
        : state_(state)
        , ref_((lua_pushvalue(state, pos), luaL_ref(state, LUA_REGISTRYINDEX)))
    {
    }

    /// <summary>Removes the reference from the registry.</summary>
    ~Reference()
    {
        if (ref_ != LUA_NOREF && ref_ != LUA_REFNIL)
            luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
    }

    Reference(const Reference&) = delete;

    Reference(Reference&& other) noexcept
        : state_(other.state_)
        , ref_(other.ref_)
    {
        other.ref_ = LUA_NOREF;
    }

    Reference& operator=(const Reference&) = delete;

    Reference& operator=(Reference&& other) noexcept
    {
        state_ = other.state_;
        ref_ = other.ref_;
        other.ref_ = LUA_NOREF;
    }

    /// <summary>The associated lua state for the reference.</summary>
    lua_State* state()
    {
        return state_;
    };

    /// <summary>Pushes the referenced value on the stack again.</summary>
    int push()
    {
        if (ref_ != LUA_REFNIL)
            return lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
        lua_pushnil(state_);
        return LUA_TNIL;
    }

    /// <summary>Pushes the referenced value on the stack of the given lua state.</summary>
    int push(lua_State* L)
    {
        int result = push();
        if (L != state_)
            lua_xmove(state_, L, 1);
        return result;
    }

private:
    lua_State* state_;
    int ref_;
};

/// <summary>Wraps any lua function by creating a reference to it.</summary>
/// <remarks>Unlike references, this type is copyable, as it uses a shared_ptr for the reference.</remarks>
class Function {
public:
    /// <summary>Creates a new reference to function at the given stack position.</summary>
    Function(lua_State* state, int pos)
        : ref_(std::make_shared<Reference>(state, pos))
    {
    }

    /// <summary>Pushes the referenced function on the stack again.</summary>
    void push() const
    {
        ref_->push();
    }

    /// <summary>Pushes the referenced function on the stack of the given lua state.</summary>
    void push(lua_State* L) const
    {
        ref_->push(L);
    }

    /// <summary>Mimics a full call to the lua function with the given parameters and optional templated return value.</summary>
    template <typename TRet = MultRet, typename... TArgs>
    auto call(TArgs&&... args) const
    {
        int old_top = lua_gettop(ref_->state());
        push();
        int arg_count = Convert<std::tuple<TArgs...>>::push(ref_->state(), std::tuple<TArgs...>{ std::forward<TArgs>(args)... });
        lua_call(ref_->state(), arg_count, LUA_MULTRET);
        int result_count = lua_gettop(ref_->state()) - old_top;

        if constexpr (!std::is_void_v<TRet>) {
            auto result = Convert<TRet>::at(ref_->state(), -result_count);
            if (!result)
                luaL_error(ref_->state(), "bad function result (%s expected)", typeid(TRet).name());
            if constexpr (!std::is_same_v<Ret, TRet> && !std::is_same_v<MultRet, TRet>)
                lua_pop(ref_->state(), result_count);
            return *result;
        }
        else
            lua_pop(ref_->state(), result_count);
    }

    /// <summary>Mimics a full call to the lua function with the given parameters and a std::tuple of the given templated return values.</summary>
    template <typename... TResult, typename... TArgs>
    auto callMultRet(TArgs&&... args) const
    {
        return call<std::tuple<TResult...>>(std::forward<TArgs>(args)...);
    }

    /// <summary>Mimics a full call to the lua function, returning all values using a wrapper class, that basically wraps the lua stack.</summary>
    template <typename... TArgs>
    MultRet operator()(TArgs&&... args)
    {
        return call(std::forward<TArgs>(args)...);
    }

private:
    std::shared_ptr<Reference> ref_;
};

/// <summary>Wraps a lua function and has an overloaded call operator for the given return type.</summary>
template <typename TRet>
class FunctionRet : public Function {
public:
    using Function::Function;

    /// <summary>Mimics a full call to the lua function with the given parameters and return type.</summary>
    template <typename... TArgs>
    TRet operator()(TArgs&&... args)
    {
        return call<TRet>(std::forward<TArgs>(args)...);
    }
};

/// <summary>Wraps a lua function and has an overloaded call operator for a tuple out of the given return type.</summary>
template <typename... TRets>
class FunctionMultRet : public Function {
public:
    using Function::Function;

    /// <summary>Mimics a full call to the lua function with the given parameters and return types as std::tuple.</summary>
    template <typename... TArgs>
    auto operator()(TArgs&&... args)
    {
        return callMultRet<TRets...>(std::forward<TArgs>(args)...);
    }
};

namespace detail
{

template <typename T>
struct FunctionConverter {
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
        return 1;
    }
};

}

/// <summary>Allows for conversion between lua functions and the Function wrapper class.</summary>
template <>
struct Convert<Function> : detail::FunctionConverter<Function> {};

/// <summary>Allows for conversion between lua functions and the Function wrapper class.</summary>
template <typename TRet>
struct Convert<FunctionRet<TRet>> : detail::FunctionConverter<FunctionRet<TRet>> {};

/// <summary>Allows for conversion between lua functions and the Function wrapper class.</summary>
template <typename... TRets>
struct Convert<FunctionMultRet<TRets...>> : detail::FunctionConverter<FunctionMultRet<TRets...>> {};

}
