#pragma once

#include "Convert.h"
#include "Types.h"

namespace dang::lua
{

template <typename T>
class StackIterator;

template <typename T>
class TableWrapper;

class StackPos;
class Arg;
class Ret;

class VarStackPos;
class VarArg;
class MultRet;

/// <summary>Wraps a position on the Lua stack.</summary>
class StackPos {
public:
    friend StackIterator<StackPos>;
    friend StackIterator<Arg>;
    friend StackIterator<Ret>;

    /// <summary>Wraps the given stack position or the last element if omitted.</summary>
    StackPos(lua_State* state, int pos = -1);

    /// <summary>Returns the associated Lua state.</summary>
    lua_State* state() const;
    /// <summary>Returns the stack position.</summary>
    int pos() const;

    /// <summary>Pushes a copy of the value on the stack.</summary>
    StackPos push() const;
    /// <summary>Pushes a copy of the value on the stack of the given Lua state.</summary>
    StackPos push(lua_State* L) const;

    /// <summary>Convenience function to have named pop function calls with debug assert, that it actually pops the top.</summary>
    void pop() const;

    /// <summary>Returns the type of the stack position.</summary>
    Type type() const;

    /// <summary>Tries to convert the value to the given type and return std::nullopt on failure.</summary>
    template <typename T>
    std::optional<T> value() const
    {
        return Convert<T>::at(state_, pos_);
    }

    /// <summary>Tries to convert the stack position and raises and error on failure.</summary>
    template <typename T>
    T as() const
    {
        if (auto result = value<T>())
            return *result;
        throw luaL_error(state_, "invalid type");
    }

    /// <summary>Calls the stack position with the given parameters and returns the result.</summary>
    template <typename TRet = void, typename... TArgs>
    TRet call(TArgs&&... args)
    {
        using ConvertArgs = Convert<std::tuple<TArgs...>>;
        using ConvertRet = Convert<TRet>;

        [[maybe_unused]] int old_pos;
        if constexpr (!ConvertRet::PushCount)
            old_pos = lua_gettop(state_);

        push();

        int arg_count = ConvertArgs::push(state_, std::tuple<TArgs...>{ std::forward<TArgs>(args)... });
        int result_count;
        if constexpr (ConvertRet::PushCount) {
            lua_call(state_, arg_count, *ConvertRet::PushCount);
            result_count = *ConvertRet::PushCount;
        }
        else {
            lua_call(state_, arg_count, LUA_MULTRET);
            result_count = lua_gettop(state_) - old_pos;
        }

        if constexpr (ConvertRet::PushCount != 0) {
            auto result = ConvertRet::at(state_, -result_count);
            if (!result)
                throw luaL_error(state_, "bad function result (%s expected)", ClassName<TRet>);
            lua_pop(state_, result_count);
            return *result;
        }
    }

    /// <summary>Calls the stack position with the given parameters and returns the results as a std::tuple.</summary>
    template <typename... TRets, typename... TArgs>
    std::tuple<TRets...> callMultRet(TArgs&&... args)
    {
        return call<std::tuple<TRets...>>(std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the stack position with the given parameters, discarding any return values.</summary>
    template <typename... TArgs>
    void operator()(TArgs&&... args)
    {
        call(std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the stack position with the given parameters and pushes the single result on the stack, returning a wrapper to it.</summary>
    template <typename... TArgs>
    Ret callPushRet(TArgs&&... args);
    /// <summary>Calls the stack position with the given parameters and pushes the results on the stack, returning a wrapper to them.</summary>
    template <typename... TArgs>
    MultRet callPushMultRet(TArgs&&... args);

    /// <summary>Returns a wrapper, which can be used for table access.</summary>
    template <typename T>
    TableWrapper<T> operator[](T key)
    {
        return TableWrapper<T>(*this, key);
    }

private:
    lua_State* state_;
    int pos_;
};

/// <summary>Wraps an argument of a Lua function as a stack position.</summary>
class Arg : public StackPos {
public:
    using StackPos::StackPos;

    /// <summary>Tries to convert the value to the given type and raises and error on failure.</summary>
    template <typename T>
    T check() const
    {
        return Convert<T>::check(state(), pos());
    }

    /// <summary>Tries to convert the value to the given type and raises and error on failure.</summary>
    template <typename T>
    operator T() const
    {
        return check<T>();
    }
};

/// <summary>Wraps a single return value of a Lua function.</summary>
class Ret : public StackPos {
public:
    using StackPos::StackPos;
};

/// <summary>Enables iteration over variadic arguments of a Lua function.</summary>
template <typename T>
class StackIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = int;
    using pointer = T*;
    using reference = T&;

    StackIterator() = default;
    explicit StackIterator(T arg)
        : arg_(arg)
    {
    }

    reference operator*()
    {
        return arg_;
    }

    pointer operator->()
    {
        return &arg_;
    }

    friend bool operator==(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() == rhs.arg_.pos();
    }

    friend bool operator!=(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() != rhs.arg_.pos();
    }

    friend bool operator<(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() < rhs.arg_.pos();
    }

    friend bool operator<=(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() <= rhs.arg_.pos();
    }

    friend bool operator>(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() > rhs.arg_.pos();
    }

    friend bool operator>=(StackIterator lhs, StackIterator rhs)
    {
        return lhs.arg_.pos() >= rhs.arg_.pos();
    }

    StackIterator& operator++()
    {
        arg_.pos_++;
        return *this;
    }

    StackIterator operator++(int)
    {
        auto old = *this;
        ++(*this);
        return old;
    }
    StackIterator& operator--()
    {
        arg_.pos_--;
        return *this;
    }

    StackIterator operator--(int)
    {
        auto old = *this;
        --(*this);
        return old;
    }

    StackIterator& operator+=(difference_type offset)
    {
        arg_.pos_ += offset;
        return *this;
    }

    StackIterator operator+(difference_type offset) const
    {
        return StackIterator(StackPos(arg_.state_, arg_.pos_ + offset));
    }

    StackIterator& operator-=(difference_type offset)
    {
        arg_.pos_ -= offset;
        return *this;
    }

    StackIterator operator-(difference_type offset) const
    {
        return StackIterator(StackPos(arg_.state_, arg_.pos_ - offset));
    }

    value_type operator[](difference_type offset) const
    {
        return StackPos(arg_.state_, arg_.pos_ + offset);
    }

private:
    T arg_;
};

/// <summary>Wraps muiltiple consecutive stack positions and mimics a container.</summary>
class VarStackPos {
public:
    /// <summary>Wraps multiple stack positions, starting at the given position.</summary>
    VarStackPos(lua_State* state, int pos)
        : state_(state)
        , pos_(lua_absindex(state, pos))
        , count_(std::max(0, lua_gettop(state) - pos_ + 1))
    {
    }

    /// <summary>Wraps a given number of stack positions, starting at the given position.</summary>
    VarStackPos(lua_State* state, int pos, int count)
        : state_(state)
        , pos_(lua_absindex(state, pos))
        , count_(count)
    {
    }

    /// <summary>Returns the associated Lua state.</summary>
    lua_State* state() const;
    /// <summary>Returns the first stack position.</summary>
    int pos() const;

    /// <summary>Copies all values onto the stack.</summary>
    int pushAll() const;
    /// <summary>Copies all values onto the stack of the given Lua state.</summary>
    int pushAll(lua_State* L) const;

    /// <summary>Returns, wether there are no wrapped stack positions.</summary>
    bool empty() const;
    /// <summary>Returns the count of the wrapped stack positions.</summary>
    int size() const;
    /// <summary>Returns the count of the wrapped stack positions.</summary>
    int max_size() const;

    /// <summary>Returns the the wrapped stack positions at the given one-based index.</summary>
    StackPos operator[](int pos) const;
    /// <summary>Returns a substack, starting at the given one-based index.</summary>
    VarStackPos substack(int from) const;

    /// <summary>Returns a stack iterator for the first element.</summary>
    StackIterator<StackPos> begin() const;
    /// <summary>Returns a stack iterator, one after the last element.</summary>
    StackIterator<StackPos> end() const;

protected:
    /// <summary>Returns the the wrapped stack positions at the given one-based index.</summary>
    template <typename T>
    T indexHelper(int pos) const
    {
        return T(state_, pos_ + pos - 1);
    }

    /// <summary>Returns a substack, starting at the given one-based index.</summary>
    template <typename T>
    T substackHelper(int from) const
    {
        return T(state_, pos_ + from - 1, std::max(0, count_ - from + 1));
    }

    /// <summary>Returns a stack iterator for the first element.</summary>
    template <typename T>
    StackIterator<T> beginHelper() const
    {
        return StackIterator<T>(T(state_, pos_));
    }

    /// <summary>Returns a stack iterator, one after the last element.</summary>
    template <typename T>
    StackIterator<T> endHelper() const
    {
        return StackIterator<T>(T(state_, pos_ + count_));
    }

private:
    lua_State* state_;
    int pos_;
    int count_;
};

/// <summary>Wraps variadic arguments to a Lua function and mimics a container.</summary>
class VarArg : public VarStackPos {
public:
    using VarStackPos::VarStackPos;

    /// <summary>Returns the the wrapped stack positions at the given one-based index.</summary>
    Arg operator[](int pos) const;
    /// <summary>Returns a substack, starting at the given one-based index.</summary>
    VarArg substack(int from) const;

    /// <summary>Returns a stack iterator for the first element.</summary>
    StackIterator<Arg> begin() const;
    /// <summary>Returns a stack iterator, one after the last element.</summary>
    StackIterator<Arg> end() const;

    /// <summary>Tries to convert all stack positions starting at the optional zero-based offset to a tuple of the specified types and raises and error on failure.</summary>
    template <typename... T>
    std::tuple<T...> check(int offset = 0) const
    {
        return Convert<std::tuple<T...>>::check(state(), pos() + offset);
    }

    /// <summary>Tries to convert all stack positions to a tuple of the specified types and raises and error on failure.</summary>
    template <typename... T>
    operator std::tuple<T...>() const
    {
        return check<T...>();
    };
};

/// <summary>Wraps muiltiple consecutive return values of a Lua function and mimics a container.</summary>
class MultRet : public VarStackPos {
public:
    using VarStackPos::VarStackPos;

    /// <summary>Returns the the wrapped stack positions at the given one-based index.</summary>
    Ret operator[](int pos) const;
    /// <summary>Returns a substack, starting at the given one-based index.</summary>
    MultRet substack(int from) const;

    /// <summary>Returns a stack iterator for the first element.</summary>
    StackIterator<Ret> begin() const;
    /// <summary>Returns a stack iterator, one after the last element.</summary>
    StackIterator<Ret> end() const;

    /// <summary>Pushes the given value on the Lua stack and returns a value, which can be returned from the function.</summary>
    template <typename... TRets>
    static MultRet push(lua_State* L, TRets&&... value)
    {
        int pos = lua_gettop(L) + 1;
        (Convert<TRets>::push(L, std::forward<TRets>(value)), ...);
        return MultRet(L, pos);
    }
};

template <>
struct Convert<StackPos> {
    static constexpr std::optional<int> PushCount = 1;

    /// <summary>Always returns true.</summary>
    static constexpr bool isExact(lua_State*, int)
    {
        return true;
    }

    /// <summary>Always returns true.</summary>
    static constexpr bool isValid(lua_State* L, int pos)
    {
        return isExact(L, pos);
    }

    /// <summary>Pushes a copy of the given argument stack position on the stack.</summary>
    static int push(lua_State* L, const StackPos& arg)
    {
        arg.push(L);
        return *PushCount;
    }
};

template <>
struct Convert<Arg> : Convert<StackPos> {
    /// <summary>Wraps the given argument stack position.</summary>
    static Arg check(lua_State* L, int arg)
    {
        return Arg(L, arg);
    }
};

template <>
struct Convert<VarStackPos> {
    static constexpr std::optional<int> PushCount = std::nullopt;

    /// <summary>Always returns true.</summary>
    static constexpr bool isExact(lua_State*, int)
    {
        return true;
    }

    /// <summary>Always returns true.</summary>
    static constexpr bool isValid(lua_State* L, int pos)
    {
        return isExact(L, pos);
    }

    /// <summary>Pushes a copy of each argument on the stack.</summary>
    static int push(lua_State* L, const VarStackPos& arg)
    {
        arg.pushAll(L);
        return arg.size();
    }
};

template <>
struct Convert<VarArg> : Convert<VarStackPos> {
    /// <summary>Wraps the given argument stack position.</summary>
    static VarArg check(lua_State* L, int arg)
    {
        return VarArg(L, arg);
    }
};

template <>
struct Convert<MultRet> : Convert<VarStackPos> {
    /// <summary>Wraps the given argument stack position.</summary>
    static std::optional<MultRet> at(lua_State* L, int pos)
    {
        return MultRet(L, pos);
    }
};

/// <summary>Wraps the entry of a table on the stack using a key of any type.</summary>
template <typename TKey>
class TableWrapper {
public:
    static_assert(Convert<TKey>::PushCount == 1, "table[key] only allows one key");

    TableWrapper(StackPos pos, TKey key)
        : pos_(pos)
        , key_(key)
    {
    }

    /// <summary>Pushes the value of the table with the key of that wrapper onto the stack.</summary>
    StackPos push() const
    {
        Convert<TKey>::push(pos_.state(), key_);
        lua_gettable(pos_.state(), pos_.pos());
        return StackPos(pos_.state());
    }

    /// <summary>Pushes the value of the table with the key of that wrapper onto the given stack.</summary>
    StackPos push(lua_State* L)
    {
        push();
        lua_xmove(pos_.state(), L, 1);
        return StackPos(L);
    }

    /// <summary>Assigns the given value to the table at the key of the wrapper.</summary>
    template <typename TValue>
    TableWrapper& operator=(TValue&& value)
    {
        static_assert(Convert<TValue>::PushCount == 1, "table[key] = value only allows one value");

        Convert<TKey>::push(pos_.state(), key_);
        Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
        lua_settable(pos_.state(), pos_.pos());
        return *this;
    }

    /// <summary>Treats value of the table at the key of the wrapper as a tempalted type and throws an error on failure.</summary>
    template <typename TValue>
    TValue as() const
    {
        static_assert(Convert<TValue>::PushCount == 1, "value = table[key] only allows one value");

        push();
        auto result = Convert<TValue>::at(pos_.state(), -1);
        if constexpr (!std::is_base_of_v<StackPos, TValue> && !std::is_base_of_v<VarStackPos, TValue>)
            lua_pop(pos_.state(), 1);
        if (result)
            return *result;
        throw luaL_error(pos_.state(), "table value has incorrect type");
    }

    /// <summary>Treats value of the table at the key of the wrapper as a tempalted type and throws an error on failure.</summary>
    template <typename TValue>
    operator TValue() const
    {
        return as<TValue>();
    }

    /// <summary>Treats value of the table at the key of the wrapper as a reference to the tempalted type and throws an error on failure.</summary>
    template <typename TValue>
    TValue& ref() const
    {
        static_assert(std::is_same_v<std::invoke_result_t<decltype(Convert<TValue>::check), lua_State*, int>, TValue&>, "value type does not allow references");
        static_assert(Convert<TValue>::PushCount == 1, "value = table[key] only allows one value");

        push();
        auto result = Convert<TValue>::at(pos_.state(), -1);
        if constexpr (!std::is_base_of_v<StackPos, TValue> && !std::is_base_of_v<VarStackPos, TValue>)
            lua_pop(pos_.state(), 1);
        if (result)
            return *result;
        throw luaL_error(pos_.state(), "table value has incorrect type");
    }

    /// <summary>Calls the value of the table at the key of the wrapper with the given parameters and returns the result.</summary>
    template <typename TRet = MultRet, typename... TArgs>
    auto call(TArgs&&... args) const
    {
        auto value = push();
        auto result = value.call<TRet>(std::forward<TArgs>(args)...);
        lua_pop(pos_.state(), 1);
        return result;
    }

    /// <summary>Calls the value of the table at the key of the wrapper with the given parameters and returns the results as a std::tuple.</summary>
    template <typename... TRets, typename... TArgs>
    auto callMultRet(TArgs&&... args) const
    {
        return call<std::tuple<TRets...>>(std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the value of the table at the key of the wrapper with the given parameters, discarding any return values.</summary>
    template <typename... TArgs>
    MultRet operator()(TArgs&&... args) const
    {
        return call(std::forward<TArgs>(args)...);
    }

private:
    StackPos pos_;
    TKey key_;
};

/// <summary>Allows for conversion between any table wrapper type and Lua.</summary>
template <typename TKey>
struct Convert<TableWrapper<TKey>> {
    static constexpr std::optional<int> PushCount = 1;

    static int push(lua_State* L, TableWrapper<TKey> wrapper)
    {
        wrapper.push(L);
        return *PushCount;
    }
};

template<typename ...TArgs>
inline Ret StackPos::callPushRet(TArgs&& ...args)
{
    using ConvertArgs = Convert<std::tuple<TArgs...>>;
    push();
    int arg_count = ConvertArgs::push(state_, std::tuple<TArgs...>{ std::forward<TArgs>(args)... });
    lua_call(state_, arg_count, 1);
    return Ret(state_);
}

template<typename ...TArgs>
inline MultRet StackPos::callPushMultRet(TArgs&& ...args)
{
    using ConvertArgs = Convert<std::tuple<TArgs...>>;
    push();
    int ret_pos = lua_gettop(state_);
    int arg_count = ConvertArgs::push(state_, std::tuple<TArgs...>{ std::forward<TArgs>(args)... });
    lua_call(state_, arg_count, LUA_MULTRET);
    return MultRet(state_, ret_pos);
}

}
