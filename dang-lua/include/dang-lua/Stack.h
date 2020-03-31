#pragma once

#include "Convert.h"
#include "Types.h"

namespace dang::lua
{

/// <summary>Wraps a position on the lua stack.</summary>
class StackPos {
public:
    friend class StackIterator;

    /// <summary>Wraps the given stack position.</summary>
    StackPos(lua_State* state, int pos);

    /// <summary>Returns the associated lua state.</summary>
    lua_State* state() const;
    /// <summary>Returns the stack position.</summary>
    int pos() const;

    /// <summary>Pushes a copy of the value on the stack.</summary>
    void push() const;
    /// <summary>Pushes a copy of the value on the stack of the given lua state.</summary>
    void push(lua_State* L) const;

    /// <summary>Returns the type of the stack position.</summary>
    Type type() const;

    /// <summary>Tries to convert the value to the given type and return std::nullopt on failure.</summary>
    template <typename T>
    std::optional<T> get() const
    {
        return Convert<T>::at(state(), pos());
    }

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

private:
    lua_State* state_;
    int pos_;
};

/// <summary>Wraps an argument of a lua function as a stack position.</summary>
class Arg : public StackPos {
public:
    using StackPos::StackPos;
};

/// <summary>Wraps a single return value of a lua function.</summary>
class Ret : public StackPos {
public:
    using StackPos::StackPos;

    /// <summary>Pushes the given value on the lua stack and returns a value, which can be returned from the function.</summary>
    template <typename TRet>
    static Ret push(lua_State* L, TRet&& value)
    {
        int pos = lua_gettop(L) + 1;
        Convert<TRet>::push(L, std::forward<TRet>(value));
        return Ret(L, pos);
    }
};

/// <summary>Enables iteration over variadic arguments of a lua function.</summary>
class StackIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = StackPos;
    using difference_type = int;
    using pointer = StackPos*;
    using reference = StackPos&;

    inline StackIterator() = default;
    inline explicit StackIterator(StackPos arg);

    inline reference operator*();
    inline pointer operator->();

    inline friend bool operator==(StackIterator lhs, StackIterator rhs);
    inline friend bool operator!=(StackIterator lhs, StackIterator rhs);
    inline friend bool operator<(StackIterator lhs, StackIterator rhs);
    inline friend bool operator<=(StackIterator lhs, StackIterator rhs);
    inline friend bool operator>(StackIterator lhs, StackIterator rhs);
    inline friend bool operator>=(StackIterator lhs, StackIterator rhs);

    inline StackIterator& operator++();
    inline StackIterator operator++(int);
    inline StackIterator& operator--();
    inline StackIterator operator--(int);

    inline StackIterator& operator+=(difference_type offset);
    inline StackIterator operator+(difference_type offset) const;
    inline StackIterator& operator-=(difference_type offset);
    inline StackIterator operator-(difference_type offset) const;

    inline value_type operator[](difference_type offset) const;

private:
    StackPos arg_;
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

    /// <summary>Returns the associated lua state.</summary>
    lua_State* state() const;
    /// <summary>Returns the first stack position.</summary>
    int pos() const;

    /// <summary>Copies all values onto the stack.</summary>
    int pushAll() const;
    /// <summary>Copies all values onto the stack of the given lua state.</summary>
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
    StackIterator begin() const;
    /// <summary>Returns a stack iterator, one after the last element.</summary>
    StackIterator end() const;

    /// <summary>Tries to convert all stack positions starting at the optional zero-based offset to a tuple of the specified types and raises and error on failure.</summary>
    template <typename... T>
    std::tuple<T...> check(int offset = 0)
    {
        return Convert<std::tuple<T...>>::check(state(), pos() + offset);
    }

    /// <summary>Tries to convert all stack positions starting at the optional zero-based offset to a tuple of the specified types and raises and error on failure.</summary>
    template <typename... T>
    operator std::tuple<T...>()
    {
        return check<T...>();
    };

private:
    lua_State* state_;
    int pos_;
    int count_;
};

/// <summary>Wraps variadic arguments to a lua function and mimics a container.</summary>
class VarArg : public VarStackPos {
public:
    using VarStackPos::VarStackPos;
};

/// <summary>Wraps muiltiple consecutive return values of a lua function and mimics a container.</summary>
class MultRet : public VarStackPos {
public:
    using VarStackPos::VarStackPos;

    /// <summary>Pushes the given value on the lua stack and returns a value, which can be returned from the function.</summary>
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
        return 1;
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
struct Convert<Ret> : Convert<StackPos> {
    /// <summary>Wraps the given argument stack position.</summary>
    static std::optional<Ret> at(lua_State* L, int pos)
    {
        return Ret(L, pos);
    }
};

template <>
struct Convert<VarStackPos> {
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
        return arg.pushAll(L);
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

}
