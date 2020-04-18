#pragma once

#include "utils.h"

#include "Convert.h"
#include "Types.h"

namespace dang::lua
{

class StackPos;
class AutoStackPos;

class StackIterator;

class VarStackPos;
class AutoVarStackPos;

using Arg = StackPos;
using VarArg = VarStackPos;

using Ret = StackPos;
using MultRet = VarStackPos;

template <typename T>
class TableAccessWrapper;

class PairsWrapper;
class IPairsWrapper;

/// <summary>Wraps a position on the Lua stack.</summary>
class StackPos {
public:
    friend StackIterator;

    /// <summary>Initializes a stack position without state and invalid 0 position.</summary>
    StackPos() = default;

    /// <summary>Wraps the current top element of the stack for the given Lua state.</summary>
    StackPos(lua_State* state)
        : state_(state)
        , pos_(lua_gettop(state))
    {
    }

    /// <summary>Wraps the given stack position on a Lua state, which must be positive.</summary>
    StackPos(lua_State* state, int pos)
        : state_(state)
        , pos_(pos)
    {
        assert(pos > 0);
    }

    /// <summary>Turns the given stack position into an absolute one and returns a wrapper to it.</summary>
    static StackPos abs(lua_State* state, int pos)
    {
        return StackPos(state, lua_absindex(state, pos));
    }

    // --- info ---

    /// <summary>Returns the associated Lua state.</summary>
    lua_State* state() const
    {
        return state_;
    }

    /// <summary>Returns the one-based index of the stack position.</summary>
    int pos() const
    {
        return pos_;
    }

    /// <summary>Returns the type of the element.</summary>
    Type type() const
    {
        return static_cast<Type>(lua_type(state_, pos_));
    }

    /// <summary>Returns the name of the element's type.</summary>
    std::string_view typeName() const
    {
        return lua_typename(state_, lua_type(state_, pos_));
    }

    /// <summary>Returns, wether the element is a boolean.</summary>
    bool isBoolean() const
    {
        return lua_isboolean(state_, pos_);
    }

    /// <summary>Returns, wether the element is a C function.</summary>
    bool isCFunction() const
    {
        return lua_iscfunction(state_, pos_);
    }

    /// <summary>Returns, wether the element is a Lua or C function.</summary>
    bool isFunction() const
    {
        return lua_isfunction(state_, pos_);
    }

    /// <summary>Returns, wether the element is an integer.</summary>
    bool isInteger() const
    {
        return lua_isinteger(state_, pos_);
    }

    /// <summary>Returns, wether the element is a light userdata.</summary>
    bool isLightUserdata() const
    {
        return lua_islightuserdata(state_, pos_);
    }

    /// <summary>Returns, wether the element is nil.</summary>
    bool isNil() const
    {
        return lua_isnil(state_, pos_);
    }

    /// <summary>Returns, wether the element has an invalid index.</summary>
    bool isNone() const
    {
        return lua_isnone(state_, pos_);
    }

    /// <summary>Returns, wether the element is nil or has an invalid index.</summary>
    bool isNoneOrNil() const
    {
        return lua_isnoneornil(state_, pos_);
    }

    /// <summary>Returns, wether the element is a number or a string convertible to a number.</summary>
    bool isNumber() const
    {
        return lua_isnumber(state_, pos_);
    }

    /// <summary>Returns, wether the element is a string or a number (which is always convertible to a string).</summary>
    bool isString() const
    {
        return lua_isstring(state_, pos_);
    }

    /// <summary>Returns, wether the element is a table.</summary>
    bool isTable() const
    {
        return lua_istable(state_, pos_);
    }

    /// <summary>Returns, wether the element is a thread.</summary>
    bool isThread() const
    {
        return lua_isthread(state_, pos_);
    }

    /// <summary>Returns, wether the element is a userdata (either full or light).</summary>
    bool isUserdata() const
    {
        return lua_isuserdata(state_, pos_);
    }

    // --- push / pop ---

    /// <summary>Pushes a copy of the element onto the stack.</summary>
    void push() const
    {
        lua_pushvalue(state_, pos_);
    }

    /// <summary>Pushes a copy of the element onto the stack of the given thread with the same Lua state.</summary>
    void push(lua_State* state) const
    {
        push();
        lua_xmove(state_, state, 1);
    }

    /// <summary>Pushes a copy of the element onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamed() const
    {
        push();
        return StackPos(state_);
    }

    /// <summary>Pushes a copy of the element onto the stack of the given thread with the same Lua state and returns a wrapper to it.</summary>
    StackPos pushNamed(lua_State* state) const
    {
        push(state);
        return StackPos(state);
    }

    /// <summary>Pushes a managed copy of the element onto the stack and returns a wrapper to it.</summary>
    AutoStackPos copy() const;

    /// <summary>Pushes a managed copy of the element onto the stack of the given thread with the same Lua state and returns a wrapper to it.</summary>
    AutoStackPos copy(lua_State* state) const;

    /// <summary>Convenience function to have named pop function calls with a debug assertion, checking that it actually pops the top.</summary>
    void pop() const
    {
        assert(lua_gettop(state_) == pos_);
        lua_pop(state_, 1);
    }

    // --- conversion ---

    /// <summary>Tries to convert the element to the given type and returns std::nullopt on failure.</summary>
    template <typename T>
    std::optional<T> optional() const
    {
        return Convert<T>::at(state_, pos_);
    }

    /// <summary>Tries to convert the element to the given type and raises a generic error on failure.</summary>
    template <typename T>
    T as() const
    {
        if (auto result = optional<T>())
            return *result;
        throw luaL_error(state_, "invalid type");
    }

    /// <summary>Treats the element as a function argument and tries to convert the element to the given type, raising an argument error on failure.</summary>
    template <typename T>
    T check() const
    {
        return Convert<T>::check(state_, pos_);
    }

    /// <summary>Tries to convert the element into a reference to the given type and raises an error on failure.</summary>
    template <typename T>
    T& reference() const
    {
        static_assert(std::is_same_v<std::invoke_result_t<decltype(Convert<T>::check), lua_State*, int>, T&>, "value type does not allow references");

        if (auto result = optional<T>())
            return *result;
        throw luaL_error(pos_.state(), "invalid type");
    }

    // --- call ---

    // TODO: pcall
    // TODO: callk/pcallk

    /// <summary>Calls the element with the given parameters and pushes the given amount of results on the stack, potentially filling with nil values.</summary>
    /// <remarks>Use callPushAll for variadic results, which calls callPush with LUA_MULTRET and returns the actual result count.</remarks>
    template <typename... TArgs>
    void callPush(int result_count, TArgs&&... args) const
    {
        push();
        int arg_count = Convert<std::tuple<TArgs...>>::push(state_, std::tuple<TArgs...>{ std::forward<TArgs>(args)... });
        lua_call(state_, arg_count, result_count);
    }

    /// <summary>Calls the element with the given parameters and pushes all results on the stack, returning the result count.</summary>
    template <typename... TArgs>
    int callPushAll(TArgs&&... args) const
    {
        int old_top = lua_gettop(state_);
        callPush(LUA_MULTRET, std::forward<TArgs>(args)...);
        return lua_gettop(state_) - old_top;
    }

    /// <summary>Calls the element with the given parameters and pushes a single result onto the stack, returning a wrapper to it.</summary>
    template <typename... TArgs>
    StackPos callPushNamedRet(TArgs&&... args) const;
    /// <summary>Calls the element with the given parameters and pushes all results onto the stack, returning a wrapper to them.</summary>
    template <typename... TArgs>
    VarStackPos callPushNamedMultRet(TArgs&&... args) const;

    /// <summary>Calls the element with the given parameters, returning the template specified result, which defaults to void.</summary>
    template <typename TRet = void, typename... TArgs>
    TRet call(TArgs&&... args) const
    {
        int result_count;
        if constexpr (Convert<TRet>::PushCount) {
            callPush(*Convert<TRet>::PushCount, std::forward<TArgs>(args)...);
            result_count = *Convert<TRet>::PushCount;
        }
        else {
            result_count = callPushAll(std::forward<TArgs>(args)...);
        }

        if constexpr (Convert<TRet>::PushCount != 0) {
            if (auto result = Convert<TRet>::at(state_, -result_count)) {
                lua_pop(state_, result_count);
                return *result;
            }
            throw luaL_error(state_, "bad function result");
        }
    }

    /// <summary>Calls the element with the given parameters and returns the results as a std::tuple.</summary>
    template <typename... TRets, typename... TArgs>
    std::tuple<TRets...> callMultRet(TArgs&&... args) const
    {
        return call<std::tuple<TRets...>>(std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with the given parameters and returns a managed wrapper of all results.</summary>
    template <typename... TArgs>
    AutoVarStackPos operator()(TArgs&&... args) const;

    // --- arithmetic operations ---

    /// <summary>Pushes the result of a binary arithmetic operation with another element onto the stack.</summary>
    void pushArith(ArithOp operation, StackPos other) const
    {
        assert(operation != ArithOp::UnaryMinus && operation != ArithOp::BinaryNot);
        push();
        other.push(state_);
        lua_arith(state_, static_cast<int>(operation));
    }

    /// <summary>Pushes the result of a binary arithmetic operation with another element onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedArith(ArithOp operation, StackPos other) const
    {
        pushArith(operation, other);
        return StackPos(state_);
    }

    /// <summary>Pushes the result of a binary arithmetic operation with another element onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos arith(ArithOp operation, StackPos other) const;

    /// <summary>Adds two elements, respecting the __add metamethod.</summary>
    friend AutoStackPos operator+(StackPos lhs, StackPos rhs);
    /// <summary>Subtracts an element from another, respecting the __sub metamethod.</summary>
    friend AutoStackPos operator-(StackPos lhs, StackPos rhs);
    /// <summary>Multiplies two elements, respecting the __mul metamethod.</summary>
    friend AutoStackPos operator*(StackPos lhs, StackPos rhs);
    /// <summary>Divides an element from another, respecting the __div metamethod.</summary>
    friend AutoStackPos operator/(StackPos lhs, StackPos rhs);
    /// <summary>Integer-divides an element from another, respecting the __idiv metamethod.</summary>
    AutoStackPos idiv(StackPos other);
    /// <summary>Calculates the modulus between two elements, respecting the __mod metamethod.</summary>
    friend AutoStackPos operator%(StackPos lhs, StackPos rhs);
    /// <summary>Takes the power of an element to another, respecting the __pow metamethod.</summary>
    friend AutoStackPos operator^(StackPos lhs, StackPos rhs);

    /// <summary>Calculates the binary and between two elements, respecting the __band metamethod.</summary>
    friend AutoStackPos operator&(StackPos lhs, StackPos rhs);
    /// <summary>Calculates the binary or between two elements, respecting the __bor metamethod.</summary>
    friend AutoStackPos operator|(StackPos lhs, StackPos rhs);
    /// <summary>Calculates the binary xor between two elements, respecting the __bxor metamethod.</summary>
    AutoStackPos bxor(StackPos other);

    /// <summary>Left shifts an element by another, respecting the __shl metamethod.</summary>
    friend AutoStackPos operator<<(StackPos lhs, StackPos rhs);
    /// <summary>Right shifts an element by another, respecting the __shr metamethod.</summary>
    friend AutoStackPos operator>>(StackPos lhs, StackPos rhs);

    /// <summary>Pushes the result of a unary arithmetic operation onto the stack.</summary>
    void pushArith(ArithOp operation) const
    {
        assert(operation == ArithOp::UnaryMinus || operation == ArithOp::BinaryNot);
        push();
        lua_arith(state_, static_cast<int>(operation));
    }

    /// <summary>Pushes the result of a unary arithmetic operation onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedArith(ArithOp operation) const
    {
        pushArith(operation);
        return StackPos(state_);
    }

    /// <summary>Pushes the result of a unary arithmetic operation onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos arith(ArithOp operation) const;

    /// <summary>Performs a unary minus on the element, respecting the __unm metamethod.</summary>
    AutoStackPos operator-() const;
    /// <summary>Performs a binary not on the element, respecting the __bnot metamethod.</summary>
    AutoStackPos operator~() const;

    // --- compare operations ---

    /// <summary>Compares two elements with the given operation, respecting metamethods.</summary>
    bool compare(CompareOp operation, StackPos other)
    {
        assert(state_ == other.state_);
        return lua_compare(state_, pos_, other.pos_, static_cast<int>(operation)) != 0;
    }

    /// <summary>Tests two elements for equality, respecting the __eq metamethod.</summary>
    friend bool operator==(StackPos lhs, StackPos rhs)
    {
        return lhs.compare(CompareOp::Equal, rhs);
    }

    /// <summary>Tests two elements for inequality, respecting the __eq metamethod.</summary>
    friend bool operator!=(StackPos lhs, StackPos rhs)
    {
        return !(lhs == rhs);
    }

    /// <summary>Tests, if an element is less than another element, respecting the __lt metamethod.</summary>
    friend bool operator<(StackPos lhs, StackPos rhs)
    {
        return lhs.compare(CompareOp::LessThan, rhs);
    }

    /// <summary>Tests, if an element is less than or equal to another element, respecting the __le metamethod.</summary>
    friend bool operator<=(StackPos lhs, StackPos rhs)
    {
        return lhs.compare(CompareOp::LessEqual, rhs);
    }

    /// <summary>Tests, if an element is greater than another element, respecting the __lt metamethod.</summary>
    /// <remarks>This simply swaps the operands and uses __lt, as in Lua.</remarks>
    friend bool operator>(StackPos lhs, StackPos rhs)
    {
        return rhs.compare(CompareOp::LessThan, lhs);
    }

    /// <summary>Tests, if an element is greater than or equal to another element, respecting the __le metamethod.</summary>
    /// <remarks>This simply swaps the operands and uses __le, as in Lua.</remarks>
    friend bool operator>=(StackPos lhs, StackPos rhs)
    {
        return rhs.compare(CompareOp::LessEqual, lhs);
    }

    /// <summary>Tests two elements for raw equality.</summary>
    bool rawEqual(StackPos other) const
    {
        assert(state_ == other.state_);
        return lua_rawequal(state_, pos_, other.pos_);
    }

    // --- other operations ---

    /// <summary>Performs a len operation on the element, pushing it onto the stack.</summary>
    void pushLen() const
    {
        lua_len(state_, pos_);
    }

    /// <summary>Performs a len operation on the element, pushing it onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedLen() const
    {
        pushLen();
        return StackPos(state_);
    }

    /// <summary>Performs a len operation on the element, pushing it onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos len() const;

    /// <summary>Performs a rawlen on the element and returns the length.</summary>
    std::size_t rawLen() const
    {
        return lua_rawlen(state_, pos_);
    }

    /// <summary>If the element has a metatable, pushes it onto the stack, otherwise returns false and pushes nothing.</summary>
    bool pushMetatable() const
    {
        return lua_getmetatable(state_, pos_);
    }

    /// <summary>Pushes the metatable of the element or nil onto the stack.</summary>
    void pushMetatableOrNil() const
    {
        if (!pushMetatable())
            lua_pushnil(state_);
    }

    /// <summary>Pushes the metatable of the element or nil onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedMetatable() const
    {
        pushMetatableOrNil();
        return StackPos(state_);
    }

    /// <summary>Pushes the metatable of the element or nil onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos metatable() const;

    /// <summary>Pushes the result of a concatenation operation with any number of other elements onto the stack.</summary>
    template <typename... TArgs>
    void pushConcat(TArgs... args) const
    {
        push();
        (Convert<TArgs>::push(std::forward<TArgs>(args)), ...);
        lua_concat(state_, 1 + sizeof...(TArgs));
    }

    /// <summary>Pushes the result of a concatenation operation with any number of other elements onto the stack and returns a wrapper to it.</summary>
    template <typename... TArgs>
    StackPos pushNamedConcat(TArgs... args) const
    {
        pushConcat<TArgs...>(std::forward<TArgs>(args)...);
        return StackPos(state_);
    }

    /// <summary>Pushes the result of a concatenation operation with any number of other elements onto the stack and returns a managed wrapper to it.</summary>
    template <typename... TArgs>
    AutoStackPos concat(TArgs... args) const;

    // --- formatting ---

    /// <summary>Converts the element into a string, respecting the __tostring metamethod.</summary>
    std::string toString() const
    {
        std::size_t length;
        const char* string = luaL_tolstring(state_, pos_, &length);
        std::string result(string, string + length);
        lua_pop(state_, 1);
        return result;
    }

    /// <summary>Performs a tostring on the stack position, which is printed to the stream.</summary>
    friend std::ostream& operator<<(std::ostream& ostream, StackPos pos)
    {
        return ostream << pos.toString();
    }

    // --- table access --

    /// <summary>Returns a wrapper, which can be used for table access.</summary>
    template <typename T>
    TableAccessWrapper<T> operator[](T&& key) const
    {
        return TableAccessWrapper<T>(*this, std::forward<T>(key));
    }

    /// <summary>Indexes the element using the top element of the stack, replacing it with the value.</summary>
    void pushGetTable() const
    {
        lua_gettable(state_, pos_);
    }

    /// <summary>Indexes the element using the top element of the stack, replacing it with the value and returns a wrapper to it.</summary>
    StackPos pushNamedGetTable() const
    {
        pushGetTable();
        return StackPos(state_);
    }

    /// <summary>Indexes the element using the top element of the stack, replacing it with the value and returns a managed wrapper to it.</summary>
    AutoStackPos getTable() const;

    /// <summary>Indexes the element using the given element, pushing it onto the stack.</summary>
    void pushGetTable(StackPos key) const
    {
        key.push();
        pushGetTable();
    }

    /// <summary>Indexes the element using the given element, pushing it onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedGetTable(StackPos key) const
    {
        pushGetTable(key);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using the given element, pushing it onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos getTable(StackPos key) const;

    /// <summary>Indexes the element using rawget and the top element of the stack, replacing it with the value.</summary>
    void pushRawGet() const
    {
        lua_rawget(state_, pos_);
    }

    /// <summary>Indexes the element using rawget and the top element of the stack, replacing it with the value and returns a wrapper to it.</summary>
    StackPos pushNamedRawGet() const
    {
        pushRawGet();
        return StackPos(state_);
    }

    /// <summary>Indexes the element using rawget and the top element of the stack, replacing it with the value and returns a managed wrapper to it.</summary>
    AutoStackPos rawGet() const;

    /// <summary>Indexes the element using rawget and the given element, pushing it onto the stack.</summary>
    void pushRawGet(StackPos key) const
    {
        key.push(state_);
        pushRawGet();
    }

    /// <summary>Indexes the element using rawget and the given element, pushing it onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedRawGet(StackPos key) const
    {
        pushRawGet(key);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using rawget and the given element, pushing it onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos rawGet(StackPos key) const;

    /// <summary>Indexes the element using given integer index, pushing the value onto the stack.</summary>
    void pushGetIndex(lua_Integer index) const
    {
        lua_geti(state_, pos_, index);
    }

    /// <summary>Indexes the element using given integer index, pushing the value onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedGetIndex(lua_Integer index) const
    {
        pushGetIndex(index);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using given integer index, pushing the value onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos getIndex(lua_Integer index) const;

    /// <summary>Indexes the element using rawgeti with the given integer index, pushing the value onto the stack.</summary>
    void pushRawGetIndex(lua_Integer index) const
    {
        lua_rawgeti(state_, pos_, index);
    }

    /// <summary>Indexes the element using rawgeti with the given integer index, pushing the value onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedRawGetIndex(lua_Integer index) const
    {
        pushRawGetIndex(index);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using rawgeti with the given integer index, pushing the value onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos rawGetIndex(lua_Integer index) const;

    /// <summary>Indexes the element using given field name, pushing the value onto the stack.</summary>
    void pushGetField(const char* field) const
    {
        lua_getfield(state_, pos_, field);
    }

    /// <summary>Indexes the element using given field name, pushing the value onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedGetField(const char* field) const
    {
        pushGetField(field);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using given field name, pushing the value onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos getField(const char* field) const;

    /// <summary>Indexes the element using given userdata pointer, pushing the value onto the stack.</summary>
    void pushRawGetPointer(const void* userdata) const
    {
        lua_rawgetp(state_, pos_, userdata);
    }

    /// <summary>Indexes the element using given userdata pointer, pushing the value onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedRawGetPointer(const void* userdata) const
    {
        pushRawGetPointer(userdata);
        return StackPos(state_);
    }

    /// <summary>Indexes the element using given userdata pointer, pushing the value onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos rawGetPointer(const void* userdata) const;

    /// <summary>Performs a table set on the element, using the top two elements as key and value, which are popped.</summary>
    void setTable() const
    {
        lua_settable(state_, pos_);
    }

    /// <summary>Performs a table set on the element, using the given key and value.</summary>
    void setTable(StackPos key, StackPos value) const
    {
        key.push(state_);
        value.push(state_);
        setTable();
    }

    /// <summary>Performs a raw table set on the element, using the top two elements as key and value, which are popped.</summary>
    void rawSet() const
    {
        lua_rawset(state_, pos_);
    }

    /// <summary>Performs a raw table set on the element, using the given key and value.</summary>
    void rawSet(StackPos key, StackPos value) const
    {
        key.push(state_);
        value.push(state_);
        rawSet();
    }

    /// <summary>Performs a table set on the element, using the given integer index and top element as value, which is popped.</summary>
    void setIndex(lua_Integer index) const
    {
        lua_seti(state_, pos_, index);
    }

    /// <summary>Performs a table set on the element, using the given integer index and value.</summary>
    void setIndex(lua_Integer index, StackPos value) const
    {
        value.push(state_);
        setIndex(index);
    }

    /// <summary>Performs a raw table set on the element, using the given integer index and top element as value, which is popped.</summary>
    void rawSetIndex(lua_Integer index) const
    {
        lua_rawseti(state_, pos_, index);
    }

    /// <summary>Performs a raw table set on the element, using the given integer index and value.</summary>
    void rawSetIndex(lua_Integer index, StackPos value) const
    {
        value.push(state_);
        rawSetIndex(index);
    }

    /// <summary>Performs a table set on the element, using the given field name and top element as value, which is popped.</summary>
    void setField(const char* field) const
    {
        lua_setfield(state_, pos_, field);
    }

    /// <summary>Performs a table set on the element, using the given field name and value.</summary>
    void setField(const char* field, StackPos value) const
    {
        value.push(state_);
        setField(field);
    }

    /// <summary>Performs a raw table set on the element, using the given userdata pointer and top element as value, which is popped.</summary>
    void rawSetPointer(const void* userdata) const
    {
        lua_rawsetp(state_, pos_, userdata);
    }

    /// <summary>Performs a raw table set on the element, using the given userdata pointer and value.</summary>
    void rawSetPointer(const void* userdata, StackPos value) const
    {
        value.push(state_);
        rawSetPointer(userdata);
    }

    // --- iteration ---

    // TODO: Respect __pairs and __ipairs metamethods

    /// <summary>Allows for range based for loops, which work in a similar fashion to a Lua pairs for loop.</summary>
    PairsWrapper pairs() const;
    /// <summary>Allows for range based for loops, which work in a similar fashion to a Lua ipairs for loop.</summary>
    IPairsWrapper ipairs() const;

    // --- other ---

    /// <summary>Allows for access from wrappers like e.g. TableAccessWrapper.</summary>
    StackPos* operator->()
    {
        return this;
    }

private:
    lua_State* state_ = nullptr;
    int pos_ = 0;
};

/// <summary>An initially top stack position, which automatically pops itself at the end.</summary>
class AutoStackPos : public StackPos {
public:
    /// <summary>Turns the top stack position into an automatic stack position, which is popped automatically.</summary>
    explicit AutoStackPos(lua_State* state)
        : StackPos(state)
    {
    }

    /// <summary>Automatically pops the stack position, if it hasn't been popped yet.</summary>
    ~AutoStackPos()
    {
        StackPos::pop();
    }

    AutoStackPos(const AutoStackPos&) = delete;
    AutoStackPos(AutoStackPos&&) = delete;
    AutoStackPos& operator=(const AutoStackPos&) = delete;
    AutoStackPos& operator=(AutoStackPos&&) = delete;

    void pop() = delete;
};

/// <summary>Enables iteration over variadic stack positions.</summary>
class StackIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = StackPos;
    using difference_type = int;
    using pointer = StackPos*;
    using reference = StackPos&;

    StackIterator() = default;

    explicit StackIterator(StackPos arg)
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

    difference_type operator-(StackIterator other) const
    {
        return arg_.pos_ - other.arg_.pos_;
    }

    value_type operator[](difference_type offset) const
    {
        return StackPos(arg_.state_, arg_.pos_ + offset);
    }

private:
    StackPos arg_;
};

/// <summary>Wraps muiltiple consecutive elements on the Lua stack and mimics a container.</summary>
class VarStackPos {
public:
    VarStackPos() = default;

    /// <summary>Wraps all elements on the stack.</summary>
    VarStackPos(lua_State* state)
        : state_(state)
        , pos_(1)
        , count_(lua_gettop(state))
    {
    }

    /// <summary>Wraps multiple consecutive elements in the given area.</summary>
    VarStackPos(lua_State* state, int pos, int count)
        : state_(state)
        , pos_(pos)
        , count_(count)
    {
    }

    /// <summary>Wraps the top elements of the stack, starting at the given element.</summary>
    static VarStackPos from(lua_State* state, int pos)
    {
        return VarStackPos(state, pos, lua_gettop(state) - pos + 1);
    }

    /// <summary>Wraps the given number of top elements on the stack.</summary>
    static VarStackPos top(lua_State* state, int count)
    {
        return VarStackPos(state, lua_gettop(state) - count + 1, count);
    }

    /// <summary>Returns the associated Lua state.</summary>
    lua_State* state() const
    {
        return state_;
    }

    /// <summary>Returns the position of the first element.</summary>
    int pos() const
    {
        return pos_;
    }

    /// <summary>Returns the count of the wrapped elements.</summary>
    int count() const
    {
        return count_;
    }

    /// <summary>Pushes a copy of all wrapped elements onto the stack and returns the count.</summary>
    int pushValues() const
    {
        for (auto arg : *this)
            arg.push();
        return count_;
    }

    /// <summary>Copies all values onto the stack of the given thread and returns the count.</summary>
    int pushValues(lua_State* state) const
    {
        for (auto arg : *this)
            arg.push(state);
        return count_;
    }

    /// <summary>Pushes a copy of all wrapped elements onto the stack and returns a wrapper to them.</summary>
    VarStackPos pushNamedValues() const
    {
        int count = pushValues();
        return VarStackPos::top(state_, count);
    }

    /// <summary>Copies all values onto the stack of the given thread and returns a wrapper to them.</summary>
    VarStackPos pushNamedValues(lua_State* state) const
    {
        int count = pushValues(state);
        return VarStackPos::top(state, count);
    }

    /// <summary>Pushes a copy of all wrapped elements onto the stack and returns a managed wrapper to them.</summary>
    AutoVarStackPos copyValues() const;

    /// <summary>Copies all values onto the stack of the given thread and returns a managed wrapper to them.</summary>
    AutoVarStackPos copyValues(lua_State* state) const;

    /// <summary>Convenience function to have named pop function calls with a debug assertion, checking that it actually pops the top.</summary>
    void popValues() const
    {
        assert(pos_ - 1 + count_ == lua_gettop(state_));
        lua_settop(state_, pos_ - 1);
    }

    /// <summary>Returns, wether there are no wrapped stack positions.</summary>
    bool empty() const
    {
        return count_ == 0;
    }

    /// <summary>Returns the count of the wrapped stack positions.</summary>
    int size() const
    {
        return count_;
    }

    /// <summary>Returns the count of the wrapped stack positions.</summary>
    int max_size() const
    {
        return count_;
    }

    /// <summary>Returns the the wrapped stack positions at the given one-based index.</summary>
    StackPos operator[](int pos) const
    {
        return StackPos(state_, pos_ + pos - 1);
    }

    /// <summary>Returns a substack, starting at the given one-based index.</summary>
    VarStackPos substack(int from) const
    {
        return VarStackPos(state_, pos_ + from - 1, count_ - from + 1);
    }

    /// <summary>Returns a stack iterator for the first element.</summary>
    StackIterator begin() const
    {
        return StackIterator(StackPos(state_, pos_));
    }

    /// <summary>Returns a stack iterator, one after the last element.</summary>
    StackIterator end() const
    {
        return StackIterator(StackPos(state_, pos_ + count_));
    }

private:
    lua_State* state_ = nullptr;
    int pos_ = 0;
    int count_ = 0;
};

/// <summary>Initially top stack elements, which automatically pop themselves at the end.</summary>
class AutoVarStackPos : public VarStackPos {
public:
    /// <summary>Turns count top stack elements into automatic elements, which are popped automatically.</summary>
    AutoVarStackPos(lua_State* state, int count)
        : VarStackPos(VarStackPos::top(state, count))
    {
    }

    /// <summary>Automatically pops the elements, using a debug assert, that it actually pops the top.</summary>
    ~AutoVarStackPos()
    {
        VarStackPos::popValues();
    }

    AutoVarStackPos(const AutoVarStackPos&) = delete;
    AutoVarStackPos(AutoVarStackPos&&) = delete;
    AutoVarStackPos& operator=(const AutoVarStackPos&) = delete;
    AutoVarStackPos& operator=(AutoVarStackPos&&) = delete;

    void popValues() = delete;
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
    static constexpr bool isValid(lua_State*, int)
    {
        return true;
    }

    /// <summary>Wraps the element and never returns std::nullopt.</summary>
    static std::optional<StackPos> at(lua_State* state, int pos)
    {
        return StackPos(state, pos);
    }

    /// <summary>Wraps the argument and never raises an error.</summary>
    static StackPos check(lua_State* state, int arg)
    {
        return StackPos(state, arg);
    }

    /// <summary>Pushes a copy of the given argument stack position on the stack.</summary>
    static int push(lua_State* state, StackPos arg)
    {
        arg.push(state);
        return *PushCount;
    }
};

template <>
struct Convert<AutoStackPos> : Convert<StackPos> {};

template <>
struct Convert<VarStackPos> {
    static constexpr std::optional<int> PushCount = std::nullopt;

    /// <summary>Always returns true.</summary>
    static constexpr bool isExact(lua_State*, int)
    {
        return true;
    }

    /// <summary>Always returns true.</summary>
    static constexpr bool isValid(lua_State*, int)
    {
        return true;
    }

    /// <summary>Wraps the elements starting at pos and never returns std::nullopt.</summary>
    static std::optional<VarStackPos> at(lua_State* state, int pos)
    {
        return VarStackPos::from(state, pos);
    }

    /// <summary>Wraps the arguments starting at arg and never raises an error.</summary>
    static VarStackPos check(lua_State* state, int arg)
    {
        return VarStackPos::from(state, arg);
    }

    /// <summary>Pushes a copy of each argument on the stack.</summary>
    static int push(lua_State* state, VarStackPos arg)
    {
        return arg.pushValues(state);
    }
};

template <>
struct Convert<AutoVarStackPos> : Convert<VarStackPos> {};

/// <summary>Wraps the entry of a stack element using a key of any type.</summary>
template <typename TKey>
class TableAccessWrapper {
public:
    static_assert(Convert<TKey>::PushCount == 1, "table[key] only allows one key");

    /// <summary>Creates a wrapper for the given element and key.</summary>
    TableAccessWrapper(StackPos pos, TKey key)
        : pos_(pos)
        , key_(key)
    {
    }

    /// <summary>Pushes the result of a table access onto the stack.</summary>
    void pushGet() const
    {
        if constexpr (std::is_convertible_v<TKey, lua_Integer>) {
            pos_.pushGetIndex(key_);
        }
        else if constexpr (std::is_convertible_v<TKey, const char*>) {
            pos_.pushGetField(key_);
        }
        else if constexpr (std::is_same_v<std::remove_cv_t<TKey>, std::string>) {
            pos_.pushGetField(key_.c_str());
        }
        else {
            Convert<TKey>::push(pos_.state(), key_);
            pos_.pushGetTable();
        }
    }

    /// <summary>Pushes the result of a table access onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedGet() const
    {
        pushGet();
        return StackPos(pos_.state());
    }

    /// <summary>Pushes the result of a table access onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos get() const
    {
        pushGet();
        return AutoStackPos(pos_.state());
    }

    /// <summary>Pushes the result of a raw table access onto the stack.</summary>
    void pushRawGet() const
    {
        if constexpr (std::is_convertible_v<TKey, lua_Integer>) {
            pos_.pushRawGetIndex(key_);
        }
        else if constexpr (std::is_same_v<std::remove_cv_t<TKey>, void*>) {
            pos_.pushRawGetPointer(key_);
        }
        else {
            Convert<TKey>::push(pos_.state(), key_);
            pos_.pushRawGet();
        }
    }

    /// <summary>Pushes the result of a raw table access onto the stack and returns a wrapper to it.</summary>
    StackPos pushNamedRawGet() const
    {
        pushRawGet();
        return StackPos(pos_.state());
    }

    /// <summary>Pushes the result of a raw table access onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos rawGet() const
    {
        pushRawGet();
        return AutoStackPos(pos_.state());
    }

    /// <summary>Performs a table set with the given value.</summary>
    template <typename TValue>
    void set(TValue&& value)
    {
        static_assert(Convert<TValue>::PushCount == 1, "table access only allows one value");
        if constexpr (std::is_convertible_v<TKey, lua_Integer>) {
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.setIndex(key_);
        }
        else if constexpr (std::is_convertible_v<TKey, const char*>) {
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.setField(key_);
        }
        else if constexpr (std::is_same_v<std::remove_cv_t<TKey>, std::string>) {
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.setField(key_.c_str());
        }
        else {
            Convert<TKey>::push(pos_.state(), key_);
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.setTable();
        }
    }

    /// <summary>Performs a raw table set with the given value.</summary>
    template <typename TValue>
    void rawSet(TValue&& value)
    {
        static_assert(Convert<TValue>::PushCount == 1, "table access only allows one value");
        if constexpr (std::is_convertible_v<TKey, lua_Integer>) {
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.rawSetIndex(key_);
        }
        else if constexpr (std::is_same_v<std::remove_cv_t<TKey>, void*>) {
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.rawSetPointer(key_);
        }
        else {
            Convert<TKey>::push(pos_.state(), key_);
            Convert<TValue>::push(pos_.state(), std::forward<TValue>(value));
            pos_.rawSet();
        }
    }

    /// <summary>Pushes the result of a table access onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos operator*() const
    {
        return get();
    }

    /// <summary>Pushes the result of a table access onto the stack and returns a managed wrapper to it.</summary>
    AutoStackPos operator->() const
    {
        return get();
    }

    /// <summary>Performs a table set with the given value.</summary>
    template <typename TValue>
    TableAccessWrapper& operator=(TValue&& value)
    {
        set<TValue>(std::forward<TValue>(value));
        return *this;
    }

private:
    StackPos pos_;
    TKey key_;
};

class PairsIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<StackPos, StackPos>;
    using difference_type = lua_Integer;
    using pointer = value_type*;
    using reference = value_type&;

    /// <summary>Initializes an end iterator.</summary>
    PairsIterator() = default;

    /// <summary>Initializes a begin iterator for the given element.</summary>
    PairsIterator(StackPos iterable)
        : iterable_(iterable)
        , pair_({ {}, {} })
    {
        lua_pushnil(iterable.state());
        pair_->first = StackPos(iterable.state());
        next();
        pair_->second = StackPos(iterable.state());
    }

    const value_type& operator*() const
    {
        return *pair_;
    }

    const value_type* operator->() const
    {
        return &*pair_;
    }

    friend bool operator==(const PairsIterator& lhs, const PairsIterator& rhs)
    {
        return lhs.pair_ == rhs.pair_;
    }

    friend bool operator!=(const PairsIterator& lhs, const PairsIterator& rhs)
    {
        return lhs.pair_ != rhs.pair_;
    }

    PairsIterator& operator++()
    {
        pair_->second.pop();
        next();
        return *this;
    }

    void operator++(int)
    {
        ++(*this);
    }

private:
    void next()
    {
        if (!lua_next(iterable_.state(), iterable_.pos()))
            pair_ = std::nullopt;
    }

    StackPos iterable_;
    std::optional<std::pair<StackPos, StackPos>> pair_;
};

class IPairsIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<lua_Integer, StackPos>;
    using difference_type = lua_Integer;
    using pointer = value_type*;
    using reference = value_type&;

    /// <summary>Initializes an end iterator.</summary>
    IPairsIterator() = default;

    /// <summary>Initializes a begin iterator for the given element.</summary>
    IPairsIterator(StackPos iterable)
        : iterable_(iterable)
        , ipair_(1, {})
    {
        next();
        ipair_.second = StackPos(iterable_.state());
    }

    const value_type& operator*() const
    {
        return ipair_;
    }

    const value_type* operator->() const
    {
        return &ipair_;
    }

    friend bool operator==(const IPairsIterator& lhs, const IPairsIterator& rhs)
    {
        return lhs.ipair_.first == rhs.ipair_.first;
    }

    friend bool operator!=(const IPairsIterator& lhs, const IPairsIterator& rhs)
    {
        return lhs.ipair_.first != rhs.ipair_.first;
    }

    IPairsIterator& operator++()
    {
        ipair_.second.pop();
        ipair_.first++;
        next();
        return *this;
    }

    void operator++(int)
    {
        ++(*this);
    }

private:
    void next()
    {
        if (lua_geti(iterable_.state(), iterable_.pos(), ipair_.first) == LUA_TNIL) {
            ipair_.first = 0;
            lua_pop(iterable_.state(), 1);
        }
    }

    StackPos iterable_;
    std::pair<lua_Integer, StackPos> ipair_;
};

/// <summary>Wraps the iteration process using pairs.</summary>
class PairsWrapper {
public:
    PairsWrapper(StackPos iterable)
        : iterable_(iterable)
    {
    }

    PairsIterator begin()
    {
        return { iterable_ };
    }

    PairsIterator end()
    {
        return {};
    }

private:
    StackPos iterable_;
};

/// <summary>Wraps the iteration process using ipairs.</summary>
class IPairsWrapper {
public:
    IPairsWrapper(StackPos iterable)
        : iterable_(iterable)
    {
    }

    IPairsIterator begin()
    {
        return { iterable_ };
    }

    IPairsIterator end()
    {
        return {};
    }

private:
    StackPos iterable_;
};

inline AutoStackPos StackPos::copy() const
{
    push();
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::copy(lua_State* state) const
{
    push(state);
    return AutoStackPos(state);
}

template<typename... TArgs>
inline StackPos StackPos::callPushNamedRet(TArgs&&... args) const
{
    callPush(1, std::forward<TArgs>(args)...);
    return StackPos(state_);
}

template<typename... TArgs>
inline VarStackPos StackPos::callPushNamedMultRet(TArgs&&... args) const
{
    int result_count = callPushAll(std::forward<TArgs>(args)...);
    return VarStackPos(state_, result_count);
}

template<typename... TArgs>
inline AutoVarStackPos StackPos::operator()(TArgs&&... args) const
{
    int result_count = callPushAll(std::forward<TArgs>(args)...);
    return AutoVarStackPos(state_, result_count);
}

inline AutoStackPos StackPos::len() const
{
    pushLen();
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::metatable() const
{
    pushMetatableOrNil();
    return AutoStackPos(state_);
}

template<typename... TArgs>
inline AutoStackPos StackPos::concat(TArgs... args) const
{
    pushConcat(std::forward<TArgs>(args)...);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::arith(ArithOp operation, StackPos other) const
{
    pushArith(operation, other);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::arith(ArithOp operation) const
{
    pushArith(operation);
    return AutoStackPos(state_);
}

inline AutoStackPos operator+(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Add, rhs);
}

inline AutoStackPos operator-(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Sub, rhs);
}

inline AutoStackPos operator*(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Mul, rhs);
}

inline AutoStackPos operator/(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Div, rhs);
}

inline AutoStackPos StackPos::idiv(StackPos other)
{
    return arith(ArithOp::IDiv, other);
}

inline AutoStackPos StackPos::bxor(StackPos other)
{
    return arith(ArithOp::BinaryXOr, other);
}

inline AutoStackPos operator%(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Mod, rhs);
}

inline AutoStackPos operator&(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::BinaryAnd, rhs);
}

inline AutoStackPos operator|(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::BinaryOr, rhs);
}

inline AutoStackPos operator^(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Pow, rhs);
}

inline AutoStackPos operator<<(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::LeftShift, rhs);
}

inline AutoStackPos operator>>(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::RightShift, rhs);
}

inline AutoStackPos StackPos::operator-() const
{
    return arith(ArithOp::UnaryMinus);
}

inline AutoStackPos StackPos::operator~() const
{
    return arith(ArithOp::BinaryNot);
}

inline AutoStackPos StackPos::getTable() const
{
    pushGetTable();
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::getTable(StackPos key) const
{
    pushGetTable(key);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::rawGet() const
{
    pushRawGet();
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::rawGet(StackPos key) const
{
    pushRawGet(key);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::getIndex(lua_Integer index) const
{
    pushGetIndex(index);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::rawGetIndex(lua_Integer index) const
{
    pushRawGetIndex(index);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::getField(const char* field) const
{
    pushGetField(field);
    return AutoStackPos(state_);
}

inline AutoStackPos StackPos::rawGetPointer(const void* userdata) const
{
    pushRawGetPointer(userdata);
    return AutoStackPos(state_);
}

inline PairsWrapper StackPos::pairs() const
{
    return PairsWrapper(*this);
}

inline IPairsWrapper StackPos::ipairs() const
{
    return IPairsWrapper(*this);
}

inline AutoVarStackPos VarStackPos::copyValues() const
{
    pushValues();
    return AutoVarStackPos(state_, count_);
}

inline AutoVarStackPos VarStackPos::copyValues(lua_State* state) const
{
    pushValues(state);
    return AutoVarStackPos(state, count_);
}

}
