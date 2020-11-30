#pragma once

#include "utils.h"

#include "Convert.h"
#include "Types.h"

namespace dang::lua {

// Forward declaration technically doesn't seem to be necessary, but it prevents IntelliSense from spamming errors (and it doesn't hurt)
class State;

/// <summary>The amount of stack slots auxiliary library functions are allowed to use before they call checkstack themselves.</summary>
/// <remarks>This information can only be found in the documentation, there does not seem to be any constant for this defined.</remarks>
constexpr int auxiliary_required_pushable = 4;

// --- Reference ---

/// <summary>Wraps a reference to a Lua value, that lives in the registry table.</summary>
/// <remarks>Uses luaL_ref and luaL_unref from the auxiliary library.</remarks>
class Reference {
public:
    friend class State;
    friend struct Convert<Reference>;

    /// <summary>Default constructs a reference with LUA_NOREF and a nullptr state.</summary>
    Reference() = default;

    /// <summary>Removes the reference from the registry table.</summary>
    ~Reference()
    {
        // Quote: If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing.
        // This means, state can be nullptr without causing problems.
        // The Lua 5.4 source code proves this, as it currently checks for ref >= 0 before doing anything.
        luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
    }

    Reference(const Reference& other)
        : Reference((other.push(), other.state_))
    {}

    Reference(Reference&& other) noexcept
        : Reference()
    {
        swap(other);
    }

    Reference& operator=(Reference other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(Reference& other) noexcept
    {
        using std::swap;
        swap(state_, other.state_);
        swap(ref_, other.ref_);
    }

    friend void swap(Reference& lhs, Reference& rhs) noexcept { lhs.swap(rhs); }

private:
    /// <summary>Turns the top of the stack into a reference, popping the value in the process.</summary>
    explicit Reference(lua_State* state)
        : state_(state)
        , ref_(luaL_ref(state, LUA_REGISTRYINDEX))
    {}

    /// <summary>Pushes the referenced value on the stack.</summary>
    void push() const { lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_); }

    lua_State* state_ = nullptr;
    int ref_ = LUA_NOREF;
};

// UniqueReference not necessary. Reference is very cheap to move on its own.

/// <summary>Allows for easy sharing of the same reference.</summary>
using SharedReference = std::shared_ptr<Reference>;
using WeakReference = std::weak_ptr<Reference>;

template <typename T>
struct IsIndex : std::false_type {};

template <typename T>
struct IsIndices : std::false_type {};

template <typename T>
struct IsIndexRange : std::false_type {};

template <typename T>
struct IsStackIndex : std::false_type {};

template <typename T>
struct IsStackIndexResult : std::false_type {};

template <typename T>
struct IsStackIndices : std::false_type {};

template <typename T>
struct IsStackIndicesResult : std::false_type {};

template <typename T>
struct IsStackIndexRange : std::false_type {};

template <typename T>
struct IsStackIndexRangeResult : std::false_type {};

template <typename T>
struct IsPseudoIndex : std::false_type {};

template <typename T>
struct IsUpvalueIndex : std::false_type {};

template <typename T>
using IsAnyIndex = std::disjunction<IsIndex<T>, IsIndices<T>, IsIndexRange<T>>;

template <typename T>
using IsAnyStackIndex = std::disjunction<IsStackIndex<T>, IsStackIndices<T>, IsStackIndexRange<T>>;

template <typename T>
using IsAnyStackIndexResult =
    std::disjunction<IsStackIndexResult<T>, IsStackIndicesResult<T>, IsStackIndexRangeResult<T>>;

template <typename T>
using IsAnyMovedStackIndexResult =
    std::conjunction<IsAnyStackIndexResult<std::remove_reference_t<T>>, std::is_rvalue_reference<T>>;

template <typename T>
using IsFixedSizeStackIndex = std::disjunction<IsStackIndex<T>, IsStackIndices<T>>;

namespace detail {

// --- Signature Information ---

/// <summary>Meant to be used as a base class to provide information about the signature of functions.</summary>
template <typename TRet, typename... TArgs>
struct SignatureInfoBase {
    template <typename TArg>
    using FixedArgType = decltype(Convert<TArg>::check(std::declval<State&>(), 1));
    using Return = TRet;
    using Arguments = std::tuple<FixedArgType<TArgs>...>;

    static constexpr bool AnyFixedSizeStackArgs = (IsFixedSizeStackIndex<TArgs>::value || ...);
    static constexpr bool AnyStateArgs = ((std::is_same_v<TArgs, State&> || IsAnyIndex<TArgs>::value) || ...);

protected:
    /// <summary>
    /// <para>Calculates the index of the next argument from the given index list of all previous arguments.</para>
    /// <para>An empty index sequence {} will give "1", an index sequence {0} will give "1" plus the size of the first argument, etc...</para>
    /// </summary>
    template <std::size_t... Indices>
    static constexpr int indexOffset(std::index_sequence<Indices...>)
    {
        static_assert((Convert<std::tuple_element_t<Indices, Arguments>>::PushCount && ...),
                      "Only the last function argument can be variadic.");
        return (1 + ... + *Convert<std::tuple_element_t<Indices, Arguments>>::PushCount);
    }
};

/// <summary>Provides information about a function signature and the means to convert arguments of a Lua C-Function into actual C++ values.</summary>
/// <remarks>Argument conversion relies on the Convert template.</remarks>
template <typename TFunc>
struct SignatureInfo;

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (*)(TArgs...)> : SignatureInfoBase<TRet, TArgs...> {
    /// <summary>Uses the Convert template to check all the arguments on the stack and returns a tuple representing these arguments.</summary>
    static auto convertArguments(State& state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }

    /// <summary>Uses the Convert template to check all the arguments on the stack and returns a tuple representing these arguments.</summary>
    static auto convertArgumentsRaw(lua_State* state)
    {
        return convertArgumentsRawHelper(state, std::index_sequence_for<TArgs...>{});
    }

private:
    using Base = SignatureInfoBase<TRet, TArgs...>;

    /// <summary>Helper function to convert all arguments, as "indexOffset" relies on an index sequence itself.</summary>
    template <std::size_t... Indices>
    static typename Base::Arguments convertArgumentsHelper(State& state, std::index_sequence<Indices...>)
    {
        return {Convert<TArgs>::check(state, Base::indexOffset(std::make_index_sequence<Indices>{}))...};
    }

    /// <summary>Helper function to convert all arguments, as "indexOffset" relies on an index sequence itself.</summary>
    template <std::size_t... Indices>
    static typename Base::Arguments convertArgumentsRawHelper(lua_State* state, std::index_sequence<Indices...>)
    {
        return {Convert<TArgs>::check(state, Base::indexOffset(std::make_index_sequence<Indices>{}))...};
    }
};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (*)(TArgs...) noexcept> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TArgs...)> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(TArgs...) noexcept> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (&)(TArgs...)> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (&)(TArgs...) noexcept> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(&&)(TArgs...)> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet(&&)(TArgs...) noexcept> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<std::function<TRet(TArgs...)>> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet (TClass::*)(TArgs...)> : SignatureInfo<TRet (*)(TClass&, TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet (TClass::*)(TArgs...) const> : SignatureInfo<TRet (TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet (TClass::*)(TArgs...) noexcept> : SignatureInfo<TRet (TClass::*)(TArgs...)> {};

template <typename TClass, typename TRet, typename... TArgs>
struct SignatureInfo<TRet (TClass::*)(TArgs...) const noexcept> : SignatureInfo<TRet (TClass::*)(TArgs...)> {};

// --- Index Iterator ---

/// <summary>Provides iteration functionality for the Lua stack and upvalues.</summary>
/// <remarks>Basically just wraps "Index" (which is cheap to copy anyways) with iterator functionality.</remarks>
template <typename TIndex>
class IndexIterator {
public:
    using difference_type = int;
    using value_type = TIndex;
    using pointer = TIndex*;
    using reference = TIndex&;
    using iterator_category = std::random_access_iterator_tag;

    IndexIterator() = default;

    explicit IndexIterator(TIndex index)
        : index_(index)
    {}

    TIndex* operator->() { return &index_; }

    const TIndex* operator->() const { return &index_; }

    TIndex& operator*() { return index_; }

    const TIndex& operator*() const { return index_; }

    friend bool operator==(IndexIterator lhs, IndexIterator rhs) { return lhs->index() == rhs->index(); }

    friend bool operator!=(IndexIterator lhs, IndexIterator rhs) { return lhs->index() != rhs->index(); }

    friend bool operator<(IndexIterator lhs, IndexIterator rhs) { return (lhs - rhs) < 0; }

    friend bool operator<=(IndexIterator lhs, IndexIterator rhs) { return (lhs - rhs) <= 0; }

    friend bool operator>(IndexIterator lhs, IndexIterator rhs) { return (lhs - rhs) > 0; }

    friend bool operator>=(IndexIterator lhs, IndexIterator rhs) { return (lhs - rhs) >= 0; }

    IndexIterator& operator++() { return *this += 1; }

    IndexIterator operator++(int) { return std::exchange(*this, *this + 1); }

    IndexIterator& operator--() { return *this -= 1; }

    IndexIterator operator--(int) { return std::exchange(*this, *this - 1); }

    IndexIterator operator+(int offset) { return IndexIterator(index_.offset(offset)); }

    IndexIterator& operator+=(int offset) { return *this = *this + offset; }

    IndexIterator operator-(int offset) { return IndexIterator(index_.offset(-offset)); }

    IndexIterator& operator-=(int offset) { return *this = *this - offset; }

    friend int operator-(IndexIterator lhs, IndexIterator rhs) { return rhs->diff(*lhs); }

private:
    TIndex index_;
};

// --- Index Implementations ---

/// <summary>Tag to signify that the given indices should be used as is.</summary>
struct DirectInit {};

/// <summary>Serves as a base for all index wrappers.</summary>
template <typename TState>
class IndexImplBase {
public:
    /// <summary>Returns the associated Lua state.</summary>
    TState& state() const { return *state_; }

protected:
    /// <summary>Default constructible for the sake of simplifying the iterator wrapper.</summary>
    IndexImplBase() = default;

    /// <summary>Associates the given Lua state with the index.</summary>
    explicit IndexImplBase(TState& state)
        : state_(&state)
    {}

    /// <summary>Applies an offset to the given index.</summary>
    /// <remarks>Note: Stack indices grow positive, while upvalues grow negative.</remarks>
    static int applyOffset(int index, int offset)
    {
        assert(index < LUA_REGISTRYINDEX || index >= 1);
        return index >= 0 ? index + offset : index - offset;
    }

    /// <summary>The difference between two indices.</summary>
    /// <remarks>See applyOffset for why this is necessary.</remarks>
    static int indexDiff(int first, int last)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
        return first >= 0 ? last - first : first - last;
    }

private:
    TState* state_ = nullptr;
};

/// <summary>The general implementation for the various types of Lua stack index positions.</summary>
template <typename TState, typename TIndex>
class IndexImpl : public IndexImplBase<TState> {
public:
    /// <summary>Default constructible for sake of simplifying iterator implementation.</summary>
    /// <remarks>Index defaults to 0, which is not a valid index. (Indices start at 1)</remarks>
    IndexImpl() = default;

    /// <summary>Directly initializes to the given index.</summary>
    IndexImpl(DirectInit, TState& state, int index)
        : IndexImplBase<TState>(state)
        , index_(index)
    {
        assert(index <= LUA_REGISTRYINDEX || index >= 1);
    }

    // --- Index ---

    /// <summary>The stored index.</summary>
    int index() const { return index_; }

    /// <summary>Same as index().</summary>
    /// <remarks>Merely for compatibility with StackIndices and StackIndexRange.</remarks>
    int first() const { return index(); }

    /// <summary>Same as index().</summary>
    /// <remarks>Merely for compatibility with StackIndices and StackIndexRange.</remarks>
    int last() const { return index(); }

    /// <summary>Returns 1.</summary>
    /// <remarks>Merely for compatibility with StackIndices and StackIndexRange.</remarks>
    constexpr int size() const { return 1; }

    /// <summary>Returns another index, which is offset by a given amount.</summary>
    /// <remarks>See: applyOffset</remarks>
    auto offset(int offset) const { return TIndex(DirectInit{}, this->state(), this->applyOffset(index_, offset)); }

    /// <summary>Returns the difference to another index.</summary>
    /// <remarks>See: indexDiff</remarks>
    int diff(TIndex other) const { return this->indexDiff(index_, other.index_); }

    // --- Index Properties ---

    /// <summary>Whether the index is a position on the Lua stack.</summary>
    bool isStack() const { return this->state().isStack(index()); }

    /// <summary>Whether the index is a pseudo index (upvalue or registry).</summary>
    bool isPseudo() const { return this->state().isPseudo(index()); }

    /// <summary>Whether the index is an upvalue.</summary>
    bool isUpvalue() const { return this->state().isUpvalue(index()); }

    /// <summary>Whether the index is the index for the registry table.</summary>
    bool isRegistry() const { return this->state().isRegistry(index()); }

    /// <summary>Wether the index (plus an optional offset) is at the bottom of the stack.</summary>
    bool isBottom(int offset = 0) const { return this->state().isIndexBottom(index(), offset); }

    /// <summary>Wether the index (plus an optional offset) is at the top of the stack.</summary>
    bool isTop(int offset = 0) const { return this->state().isIndexTop(index(), offset); }

    /// <summary>Returns the (zero-based) offset from the bottom of the stack.</summary>
    int offsetFromBottom() const { return this->state().indexOffsetFromBottom(index()); }

    /// <summary>Returns the (zero-based) offset from the top of the stack.</summary>
    int offsetFromTop() const { return this->state().indexOffsetFromTop(index()); }

    // --- Stack Queries ---

    /// <summary>Returns the type of the element.</summary>
    Type type() const { return this->state().type(index()); }

    /// <summary>Returns the name of the element's type.</summary>
    std::string_view typeName() const { return this->state().typeName(index()); }

    /// <summary>Whether the index is not valid.</summary>
    bool isNone() const { return this->state().isNone(index()); }

    /// <summary>Whether the element is nil.</summary>
    bool isNil() const { return this->state().isNil(index()); }

    /// <summary>Whether the index is not valid or the element is nil.</summary>
    bool isNoneOrNil() const { return this->state().isNoneOrNil(index()); }

    /// <summary>Whether the element is a boolean.</summary>
    bool isBoolean() const { return this->state().isBoolean(index()); }

    /// <summary>Whether the element is light userdata.</summary>
    bool isLightUserdata() const { return this->state().isLightUserdata(index()); }

    /// <summary>Whether the element is a number or a string convertible to a number.</summary>
    bool isNumber() const { return this->state().isNumber(index()); }

    /// <summary>Whether the element is an integer or a string convertible to an integer.</summary>
    bool isInteger() const { return this->state().isInteger(index()); }

    /// <summary>Whether the element is a string or a number (which is always convertible to a string).</summary>
    bool isString() const { return this->state().isString(index()); }

    /// <summary>Whether the element is a table.</summary>
    bool isTable() const { return this->state().isTable(index()); }

    /// <summary>Whether the element is a function (either C or Lua).</summary>
    bool isFunction() const { return this->state().isFunction(index()); }

    /// <summary>Whether the element is a C function.</summary>
    bool isCFunction() const { return this->state().isCFunction(index()); }

    /// <summary>Whether the element is full or light userdata.</summary>
    bool isUserdata() const { return this->state().isUserdata(index()); }

    /// <summary>Whether the element is a thread.</summary>
    bool isThread() const { return this->state().isThread(index()); }

    // --- Conversion ---

    /// <summary>Uses the Convert template to convert the element.</summary>
    /// <remarks>Returns an optional, which is std::nullopt if the conversion failed.</remarks>
    template <typename T>
    auto to()
    {
        return this->state().template to<T>(index());
    }

    /// <summary>Treats the element as a function argument and uses the Convert template to convert it.</summary>
    /// <remarks>Raises a Lua (argument) error if the conversion failed.</remarks>
    template <typename T>
    decltype(auto) check()
    {
        return this->state().template check<T>(index());
    }

    /// <summary>Follows the same semantics as Lua, returning "true" for anything but "false" and "nil".</summary>
    explicit operator bool() { return check<bool>(); }

    // --- Error ---

    [[noreturn]] void error()
    {
        // always move, as it doesn't return anyway
        this->state().error(std::move(*this));
    }

    [[noreturn]] void argError(const char* extra_message) { this->state().argError(index(), extra_message); }

    [[noreturn]] void argError(const std::string& extra_message) { this->state().argError(index(), extra_message); }

    [[noreturn]] void typeError(const char* type_name) { this->state().typeError(index(), type_name); }

    [[noreturn]] void typeError(const std::string& type_name) { this->state().typeError(index(), type_name); }

    // --- Calling ---

    /// <summary>Calls the element with an arbitrary number of arguments, returning a fixed number of results.</summary>
    /// <remarks>Also supports LUA_MULTRET.</remarks>
    template <int Results = 0, typename... TArgs>
    auto call(TArgs&&... args) const
    {
        return this->state().template call<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning a specified number of results.</summary>
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) const
    {
        return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET internally.</summary>
    template <typename... TArgs>
    auto operator()(TArgs&&... args) const
    {
        return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <int Results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) const
    {
        return this->state().template pcall<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) const
    {
        return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) const
    {
        return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    // --- Operations ---

    /// <summary>Replaces the element with another value.</summary>
    /// <remarks>This is what one might expect from the assignment operator, yet the assignment operator only changes the index itself, not its element.</remarks>
    template <typename T>
    auto& replace(T&& value) const
    {
        if (isTop()) {
            this->state().pop();
            this->state().push(std::forward<T>(value));
        }
        else {
            this->state().replace(index(), std::forward<T>(value));
        }
        return *this;
    }

    /// <summary>Performs integer division with another value. (// in Lua)</summary>
    template <typename T>
    auto idiv(T&& other) const
    {
        return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// <summary>Raises the element to the power of another value. (^ in Lua)</summary>
    template <typename T>
    auto pow(T&& other) const
    {
        return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// <summary>Performs binary xor with another value. (~ in Lua)</summary>
    template <typename T>
    auto bxor(T&& other) const
    {
        return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    /// <summary>Pushes the length of the element on the stack.</summary>
    /// <remarks>This can invoke the __len meta-method and therefore doesn't necessarily return an integer.</remarks>
    auto pushLength() const { return this->state().pushLength(index()); }

    /// <summary>Returns the length of the element.</summary>
    /// <remarks>This can invoke the __len meta-method and raises an error if that doesn't return an integer.</remarks>
    auto length() const { return this->state().length(index()); }

    /// <summary>Returns the raw length of the value, which does not invoke meta-method.</summary>
    auto rawLength() const { return this->state().rawLength(index()); }

    // --- Table Access ---

    /// <summary>Queries the element like a table with the given key, returning both the type and the pushed value.</summary>
    /// <remarks>Can invoke the __index meta-method.</remarks>
    template <typename TKey>
    auto getTableWithType(TKey&& key)
    {
        return this->state().getTableWithType(*this, std::forward<TKey>(key));
    }

    /// <summary>Queries the element like a table with the given key, returning the pushed value.</summary>
    /// <remarks>Can invoke the __index meta-method.</remarks>
    template <typename TKey>
    auto getTable(TKey&& key)
    {
        return this->state().getTable(*this, std::forward<TKey>(key));
    }

    /// <summary>Convenience for table access, however it can only be used to query and not to set.</summary>
    template <typename TKey>
    auto operator[](TKey&& key)
    {
        return this->state().getTable(*this, std::forward<TKey>(key));
    }

    /// <summary>Sets a key of the element like a table to the given value.</summary>
    /// <remarks>Can invoke the __newindex meta-method.</remarks>
    template <typename TKey, typename TValue>
    void setTable(TKey&& key, TValue&& value)
    {
        this->state().setTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// <summary>Similar to getTableWithType, but does not invoke meta-methods.</summary>
    template <typename TKey>
    auto rawGetTableWithType(TKey&& key)
    {
        return this->state().rawGetTableWithType(*this, std::forward<TKey>(key));
    }

    /// <summary>Similar to getTable, but does not invoke meta-methods.</summary>
    template <typename TKey>
    auto rawGetTable(TKey&& key)
    {
        return this->state().rawGetTable(*this, std::forward<TKey>(key));
    }

    /// <summary>Similar to setTable, but does not invoke meta-methods.</summary>
    template <typename TKey, typename TValue>
    void rawSetTable(TKey&& key, TValue&& value)
    {
        this->state().rawSetTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// <summary>Returns the metatable of the element or std::nullopt if it doesn't have one.</summary>
    auto getMetatable() { return this->state().getMetatable(index()); }

    /// <summary>Sets the metatable of the element to the specified table; or nil to remove it.</summary>
    template <typename TMetatable>
    auto setMetatable(TMetatable&& metatable)
    {
        this->state().setMetatable(index(), std::forward<TMetatable>(metatable));
    }

    /// <summary>Returns the next key-value-pair of the table or nothing, if the table has been exhausted.</summary>
    template <typename TKey>
    auto next(TKey&& key)
    {
        return this->state().next(index(), std::forward<TKey>(key));
    }

    // --- Formatting ---

    /// <summary>Converts the element to a string in a reasonable format using luaL_tolstring.</summary>
    std::string format() const { return this->state().format(index()); }

    /// <summary>Prints the element to the stream using the format function.</summary>
    friend std::ostream& operator<<(std::ostream& stream, const IndexImpl& index) { return stream << index.format(); }

    // --- Reference ---

    /// <summary>Stores the element as a reference in the registry table and returns a wrapper.</summary>
    auto ref() { return this->state().ref(*this); }

private:
    int index_ = 0;
};

/// <summary>The basic implementation for a compile-time fixed size range of indices.</summary>
template <typename TState, int Count>
class IndicesImpl : public IndexImplBase<TState> {
public:
    static_assert(Count >= 0, "Indices must have a non-negative count.");

    /// <summary>Default constructible to stay consistent with Index.</summary>
    IndicesImpl() = default;

    /// <summary>Directly initializes the starting index with the given value.</summary>
    IndicesImpl(DirectInit, TState& state, int first)
        : IndexImplBase<TState>(state)
        , first_(first)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
    }

    /// <summary>Returns the first index of the range.</summary>
    int first() const { return first_; }

    /// <summary>Returns the size of the range.</summary>
    constexpr int size() const { return Count; }

private:
    int first_ = 0;
};

/// <summary>The basic implementation for a range of indices.</summary>
template <typename TState>
class IndexRangeImpl : public IndexImplBase<TState> {
public:
    /// <summary>Default constructible to stay consistent with Index.</summary>
    IndexRangeImpl() = default;

    /// <summary>Directly initializes the starting index with the given values.</summary>
    IndexRangeImpl(DirectInit, TState& state, int first, int count)
        : IndexImplBase<TState>(state)
        , first_(first)
        , count_(count)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
        assert(count >= 0);
    }

    /// <summary>Returns the first index of the range.</summary>
    int first() const { return first_; }

    /// <summary>Returns the size of the range.</summary>
    int size() const { return count_; }

private:
    int first_ = 0;
    int count_ = 0;
};

/// <summary>The general implementation for a range of indices.</summary>
/// <remarks>If Size is -1, uses IndexRangeImpl as base class, otherwise uses IndicesImpl.</remarks>
template <typename TState, typename TIndex, int Size>
class MultiIndexImpl : public std::conditional_t<Size != -1, IndicesImpl<TState, Size>, IndexRangeImpl<TState>> {
public:
    // Using base constructor
    using std::conditional_t<Size != -1, IndicesImpl<TState, Size>, IndexRangeImpl<TState>>::conditional_t;

    /// <summary>Returns the last index of the range.</summary>
    int last() const { return this->applyOffset(this->first(), this->size() - 1); }

    /// <summary>Returns a single index at the given zero-based (!) offset.</summary>
    auto operator[](int offset) const
    {
        return TIndex(DirectInit{}, this->state(), this->applyOffset(this->first(), offset));
    }

    /// <summary>Returns an iterator to the first element.</summary>
    auto begin() { return IndexIterator<TIndex>((*this)[0]); }

    /// <summary>Returns an iterator to one after the last element.</summary>
    auto end() { return IndexIterator<TIndex>((*this)[this->size()]); }

    // --- Index Properties ---

    /// <summary>Whether the range starts at the bottom of the stack.</summary>
    bool isBottom(int offset = 0) const { return this->state().isIndexBottom(this->first(), offset); }

    /// <summary>Whether the range ends at the top of the stack.</summary>
    bool isTop(int offset = 0) const { return this->state().isIndexTop(this->last(), offset); }

    // --- Formatting ---

    /// <summary>Prints all indices to the stream, separated by comma (and space).</summary>
    friend std::ostream& operator<<(std::ostream& stream, const MultiIndexImpl& indices)
    {
        int size = indices.size();
        if (size == 0)
            return stream;
        stream << indices[0];
        for (int i = 1; i < size; i++)
            stream << ", " << indices[i];
        return stream;
    }
};

// --- Index Wrappers ---

/// <summary>Results can be consumed by other operations if they are also rvalues.</summary>
enum class StackIndexType { Reference, Result };

/// <summary>Wraps any index on the Lua stack.</summary>
template <typename TState, StackIndexType Type>
class StackIndex : public IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>> {
public:
    using IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>>::IndexImpl;

    /// <summary>Takes any stack index.</summary>
    StackIndex(TState& state, int index)
        : IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>>(
              DirectInit{}, state, state.absStackIndex(index))
    {
        assert(index > LUA_REGISTRYINDEX);
    }

    /// <summary>Returns a result index, for which rvalues can be consumed by some functions.</summary>
    auto asResult() { return StackIndex<TState, StackIndexType::Result>(DirectInit{}, this->state(), this->index()); }

    /// <summary>Pops the value if it lies at the top of the stack.</summary>
    void popIfTop()
    {
        if (this->isTop())
            this->state().pop();
    }

    // --- Calling ---

    /// <summary>Calls the element with an arbitrary number of arguments, returning a fixed number of results.</summary>
    /// <remarks>Also supports LUA_MULTRET.</remarks>
    template <int Results = 0, typename... TArgs>
    auto call(TArgs&&... args) const&
    {
        return this->state().template call<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning a fixed number of results.</summary>
    /// <remarks>Also supports LUA_MULTRET.</remarks>
    template <int Results = 0, typename... TArgs>
    auto call(TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().template call<Results>(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().template call<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning a specified number of results.</summary>
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) const&
    {
        return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning a specified number of results.</summary>
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().callReturning(results, std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET internally.</summary>
    template <typename... TArgs>
    auto operator()(TArgs&&... args) const&
    {
        return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET internally.</summary>
    template <typename... TArgs>
    auto operator()(TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().callMultRet(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <int Results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) const&
    {
        return this->state().template pcall<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <int Results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().template pcall<Results>(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().template pcall<Results>(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) const&
    {
        return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().pcallReturning(results, std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) const&
    {
        return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be the error itself).</summary>
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().pcallMultRet(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    // --- Operations ---

    /// <summary>Performs integer division with another value. (// in Lua)</summary>
    template <typename T>
    auto idiv(T&& other) const&
    {
        return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// <summary>Performs integer division with another value. (// in Lua)</summary>
    template <typename T>
    auto idiv(T&& other) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().template arith<ArithOp::IDiv>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// <summary>Raises the element to the power of another value. (^ in Lua)</summary>
    template <typename T>
    auto pow(T&& other) const&
    {
        return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// <summary>Raises the element to the power of another value. (^ in Lua)</summary>
    template <typename T>
    auto pow(T&& other) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().template arith<ArithOp::Pow>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// <summary>Performs binary xor with another value. (~ in Lua)</summary>
    template <typename T>
    auto bxor(T&& other) const&
    {
        return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    /// <summary>Performs binary xor with another value. (~ in Lua)</summary>
    template <typename T>
    auto bxor(T&& other) &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().template arith<ArithOp::BinaryXOr>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    /// <summary>Returns the length of the element using luaL_len.</summary>
    /// <remarks>This can invoke the __len meta-method and raises an error if that doesn't return a lua_Integer.</remarks>
    auto length() const& { return this->state().length(this->index()); }

    /// <summary>Returns the length of the element using luaL_len.</summary>
    /// <remarks>This can invoke the __len meta-method and raises an error if that doesn't return a lua_Integer.</remarks>
    auto length() &&
    {
        auto result = this->state().length(this->index());
        if constexpr (Type == StackIndexType::Result)
            popIfTop();
        return result;
    }

    /// <summary>Returns the raw length of the value, which does not invoke meta-methods.</summary>
    auto rawLength() const& { return this->state().rawLength(this->index()); }

    /// <summary>Returns the raw length of the value, which does not invoke meta-methods.</summary>
    auto rawLength() &&
    {
        auto result = this->state().rawLength(this->index());
        if constexpr (Type == StackIndexType::Result)
            popIfTop();
        return result;
    }

    // --- Table Access ---

    /// <summary>Sets a key of the element like a table to the given value.</summary>
    /// <remarks>Can invoke the __newindex meta-method.</remarks>
    template <typename TKey, typename TValue>
    void setTable(TKey&& key, TValue&& value) const&
    {
        this->state().setTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// <summary>Sets a key of the element like a table to the given value.</summary>
    /// <remarks>Can invoke the __newindex meta-method.</remarks>
    template <typename TKey, typename TValue>
    void setTable(TKey&& key, TValue&& value) &&
    {
        this->state().setTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
        // lua_settable doesn't actually pop the table, only the key and value
        if constexpr (Type == StackIndexType::Result)
            popIfTop();
    }

    /// <summary>Similar to setTable, but does not invoke meta-methods.</summary>
    template <typename TKey, typename TValue>
    void rawSetTable(TKey&& key, TValue&& value) const&
    {
        this->state().rawSetTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// <summary>Similar to setTable, but does not invoke meta-methods.</summary>
    template <typename TKey, typename TValue>
    void rawSetTable(TKey&& key, TValue&& value) &&
    {
        this->state().rawSetTable(*this, std::forward<TKey>(key), std::forward<TValue>(value));
        // lua_rawset doesn't actually pop the table, only the key and value
        if constexpr (Type == StackIndexType::Result)
            popIfTop();
    }

    // --- Reference ---

    /// <summary>Stores the element as a reference in the registry table and returns a wrapper.</summary>
    auto ref() & { return this->state().ref(*this); }

    /// <summary>Stores the element as a reference in the registry table and returns a wrapper.</summary>
    auto ref() &&
    {
        if constexpr (Type == StackIndexType::Result)
            return this->state().ref(std::move(*this));
        else
            return this->state().ref(*this);
    }

    // --- Formatting ---

    /// <summary>Prints the element to the stream using the format function.</summary>
    friend std::ostream& operator<<(std::ostream& stream, StackIndex&& index)
    {
        stream << index;
        if constexpr (Type == StackIndexType::Result)
            index.popIfTop();
        return stream;
    }
};

/// <summary>Wraps the registry pseudo index.</summary>
template <typename TState>
class RegistryIndex : public IndexImpl<TState, RegistryIndex<TState>> {
public:
    using IndexImpl<TState, RegistryIndex<TState>>::IndexImpl;

    /// <summary>Implicitly assumes the registry index.</summary>
    RegistryIndex(TState& state)
        : IndexImpl<TState, RegistryIndex<TState>>(DirectInit{}, state, LUA_REGISTRYINDEX)
    {}
};

/// <summary>Wraps an upvalue index.</summary>
template <typename TState>
class UpvalueIndex : public IndexImpl<TState, UpvalueIndex<TState>> {
public:
    using IndexImpl<TState, UpvalueIndex<TState>>::IndexImpl;

    /// <summary>Converts the 1-based index into an upvalue-index.</summary>
    UpvalueIndex(TState& state, int index)
        : IndexImpl<TState, UpvalueIndex<TState>>(DirectInit{}, state, lua_upvalueindex(index))
    {
        assert(index >= 1);
    }
};

// --- Indices Wrappers ---

/// <summary>Wraps a compile-time fixed size range of stack indices.</summary>
template <typename TState, int Count, StackIndexType Type>
class StackIndices : public MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, Count> {
public:
    using MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, Count>::MultiIndexImpl;

    /// <summary>Takes any stack index to start the range at.</summary>
    StackIndices(TState& state, int first)
        : MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, Count>(
              DirectInit{}, state, state.absStackIndex(first))
    {
        assert(first > LUA_REGISTRYINDEX);
    }

    /// <summary>Returns result indices, for which rvalues can be consumed by some functions.</summary>
    auto asResults()
    {
        return StackIndices<TState, Count, StackIndexType::Result>(DirectInit{}, this->state(), this->first());
    }

    // --- Formatting ---

    /// <summary>Prints all indices to the stream, separated by comma (and space).</summary>
    friend std::ostream& operator<<(std::ostream& stream, StackIndices&& indices)
    {
        static_assert(Type == StackIndexType::Result);
        stream << indices;
        if (indices.isTop())
            indices.state().pop(indices.size());
        return stream;
    }
};

/// <summary>Wraps a compile-time fixed size range of upvalues.</summary>
template <typename TState, int Count>
class UpvalueIndices : public MultiIndexImpl<TState, UpvalueIndex<TState>, Count> {
public:
    using MultiIndexImpl<TState, UpvalueIndex<TState>, Count>::MultiIndexImpl;

    /// <summary>Converts the 1-based index into an upvalue index, to start the range at.</summary>
    UpvalueIndices(TState& state, int first)
        : MultiIndexImpl<TState, UpvalueIndex<TState>, Count>(DirectInit{}, state, lua_upvalueindex(first))
    {
        assert(first >= 1);
    }
};

// --- Index Range Wrappers ---

/// <summary>Wraps a range stack indices.</summary>
template <typename TState, StackIndexType Type>
class StackIndexRange : public MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1> {
public:
    using MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1>::MultiIndexImpl;

    /// <summary>Takes any stack index to start the range at.</summary>
    StackIndexRange(TState& state, int first, int count)
        : MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1>(
              DirectInit{}, state, state.absStackIndex(first), count)
    {
        assert(first > LUA_REGISTRYINDEX);
    }

    /// <summary>Returns result indices, for which rvalues can be consumed by some functions.</summary>
    auto asResults()
    {
        return StackIndexRange<TState, StackIndexType::Result>(
            DirectInit{}, this->state(), this->first(), this->size());
    }

    // --- Formatting ---

    /// <summary>Prints all indices to the stream, separated by comma (and space).</summary>
    friend std::ostream& operator<<(std::ostream& stream, StackIndexRange&& index_range)
    {
        static_assert(Type == StackIndexType::Result);
        stream << index_range;
        if (index_range.isTop())
            index_range.state().pop(index_range.size());
        return stream;
    }
};

/// <summary>Wraps a range of upvalues.</summary>
template <typename TState>
class UpvalueIndexRange : public MultiIndexImpl<TState, UpvalueIndex<TState>, -1> {
public:
    using MultiIndexImpl<TState, UpvalueIndex<TState>, -1>::MultiIndexImpl;

    /// <summary>Converts the 1-based index into an upvalue index, to start the range at.</summary>
    UpvalueIndexRange(TState& state, int first, int count)
        : MultiIndexImpl<TState, UpvalueIndex<TState>, -1>(DirectInit{}, state, lua_upvalueindex(first), count)
    {
        assert(first >= 1);
    }
};

} // namespace detail

using StackIndex = detail::StackIndex<State, detail::StackIndexType::Reference>;
using ConstStackIndex = detail::StackIndex<const State, detail::StackIndexType::Reference>;

using StackIndexResult = detail::StackIndex<State, detail::StackIndexType::Result>;
using ConstStackIndexResult = detail::StackIndex<const State, detail::StackIndexType::Result>;

using RegistryIndex = detail::RegistryIndex<State>;
using ConstRegistryIndex = detail::RegistryIndex<const State>;

using UpvalueIndex = detail::UpvalueIndex<State>;
using ConstUpvalueIndex = detail::UpvalueIndex<const State>;

template <int Size>
using StackIndices = detail::StackIndices<State, Size, detail::StackIndexType::Reference>;
template <int Size>
using ConstStackIndices = detail::StackIndices<const State, Size, detail::StackIndexType::Reference>;

template <int Size>
using StackIndicesResult = detail::StackIndices<State, Size, detail::StackIndexType::Result>;
template <int Size>
using ConstStackIndicesResult = detail::StackIndices<const State, Size, detail::StackIndexType::Result>;

template <int Size>
using UpvalueIndices = detail::UpvalueIndices<State, Size>;
template <int Size>
using ConstUpvalueIndices = detail::UpvalueIndices<const State, Size>;

using StackIndexRange = detail::StackIndexRange<State, detail::StackIndexType::Reference>;
using ConstStackIndexRange = detail::StackIndexRange<const State, detail::StackIndexType::Reference>;

using StackIndexRangeResult = detail::StackIndexRange<State, detail::StackIndexType::Result>;
using ConstStackIndexRangeResult = detail::StackIndexRange<const State, detail::StackIndexType::Result>;

using UpvalueIndexRange = detail::UpvalueIndexRange<State>;
using ConstUpvalueIndexRange = detail::UpvalueIndexRange<const State>;

template <typename TState, detail::StackIndexType Type>
struct IsIndex<detail::StackIndex<TState, Type>> : std::true_type {};
template <typename TState>
struct IsIndex<detail::RegistryIndex<TState>> : std::true_type {};
template <typename TState>
struct IsIndex<detail::UpvalueIndex<TState>> : std::true_type {};

template <typename TState, int Count, detail::StackIndexType Type>
struct IsIndices<detail::StackIndices<TState, Count, Type>> : std::true_type {};
template <typename TState, int Count>
struct IsIndices<detail::UpvalueIndices<TState, Count>> : std::true_type {};

template <typename TState, detail::StackIndexType Type>
struct IsIndexRange<detail::StackIndexRange<TState, Type>> : std::true_type {};
template <typename TState>
struct IsIndexRange<detail::UpvalueIndexRange<TState>> : std::true_type {};

template <typename TState, detail::StackIndexType Type>
struct IsStackIndex<detail::StackIndex<TState, Type>> : std::true_type {};

template <typename TState>
struct IsStackIndexResult<detail::StackIndex<TState, detail::StackIndexType::Result>> : std::true_type {};

template <typename TState, int Count, detail::StackIndexType Type>
struct IsStackIndices<detail::StackIndices<TState, Count, Type>> : std::true_type {};

template <typename TState, int Count>
struct IsStackIndicesResult<detail::StackIndices<TState, Count, detail::StackIndexType::Result>> : std::true_type {};

template <typename TState, detail::StackIndexType Type>
struct IsStackIndexRange<detail::StackIndexRange<TState, Type>> : std::true_type {};

template <typename TState>
struct IsStackIndexRangeResult<detail::StackIndexRange<TState, detail::StackIndexType::Result>> : std::true_type {};

template <typename TState>
struct IsPseudoIndex<detail::RegistryIndex<TState>> : std::true_type {};
template <typename TState>
struct IsPseudoIndex<detail::UpvalueIndex<TState>> : std::true_type {};

template <typename TState>
struct IsUpvalueIndex<detail::UpvalueIndex<TState>> : std::true_type {};

// --- State ---

/// <summary>Wraps the template supplied function into a Lua function in an almost cost-free way.</summary>
template <auto Func>
inline int wrap(lua_State* state);

/// <summary>Returns a luaL_Reg with the wrapped template supplied function and given name.</summary>
template <auto Func>
inline constexpr luaL_Reg reg(const char* name);

/// <summary>Wraps a Lua state or thread.</summary>
class State {
public:
    friend class OwnedState;
    friend class ScopedStack;

    /// <summary>Mainly used to construct from a C function parameter.</summary>
    State(lua_State* state)
        : state_(state)
    {}

    State(const State&) = delete;
    State(State&&) = default;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = default;

    void swap(State& other)
    {
        using std::swap;
        swap(state_, other.state_);
        swap(top_, other.top_);
#ifndef NDEBUG
        swap(pushable_, other.pushable_);
#endif
    }

    friend void swap(State& lhs, State& rhs) { lhs.swap(rhs); }

    // --- State Conversion ---

    /// <summary>Returns the wrapped Lua state.</summary>
    lua_State* state() const { return state_; }

    /// <summary>Returns the wrapped Lua state.</summary>
    operator lua_State*() const { return state_; }

    // --- State Properties ---

    /// <summary>Returns the version number of this Lua state.</summary>
    auto version() const { return lua_version(state_); }

    /// <summary>Checks whether the code making the call and the Lua library being called are using the same version of Lua and the same numeric types.</summary>
    void checkVersion() const
    {
        assertPushableAuxiliary();
        luaL_checkversion(state_);
    }

    /// <summary>Returns the current status of the thread.</summary>
    auto status() const { return static_cast<Status>(lua_status(state_)); }

    /// <summary>Whether the thread can yield.</summary>
    bool isYieldable() const { return lua_isyieldable(state_); }

    /// <summary>Replaces the panic function with the given function, returning the old one.</summary>
    auto replacePanicFunction(lua_CFunction panic_function) { return lua_atpanic(state_, panic_function); }

    // TODO: Somehow make this more typesafe.
    /// <summary>Returns a reference to a pointer that can be used freely.</summary>
    void*& extraspace() { return *static_cast<void**>(lua_getextraspace(state_)); }

    // --- Allocator ---

    /// <summary>Returns the current allocation function and associated userdata pointer.</summary>
    auto getAllocator() const
    {
        void* userdata;
        auto allocator = lua_getallocf(state_, &userdata);
        return std::tuple{allocator, userdata};
    }

    /// <summary>Sets a new allocation function and an optionally associated userdata pointer.</summary>
    void setAllocator(lua_Alloc allocator, void* userdata = nullptr) { lua_setallocf(state_, allocator, userdata); }

    // --- Garbage Collector ---

    /// <summary>Performs a full garbage-collection cycle.</summary>
    void gcCollect() { gc(GCOption::Collect); }

    /// <summary>Stops the garbage collector.</summary>
    void gcStop() { gc(GCOption::Stop); }

    /// <summary>Restarts the garbage collector.</summary>
    void gcRestart() { gc(GCOption::Restart); }

    /// <summary>Returns the current amount of memory (in Kbytes) in use by Lua.</summary>
    int gcCount() const { return gc(GCOption::Count); }

    /// <summary>Returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.</summary>
    int gcCountBytes() const { return gc(GCOption::CountBytes); }

    /// <summary>Performs an incremental step of garbage collection, corresponding to the allocation of stepsize Kbytes.</summary>
    void gcStep(int stepsize) { gc(GCOption::Step, stepsize); }

    /// <summary>Whether the collector is running (i.e., not stopped).</summary>
    bool gcIsRunning() const { return gc(GCOption::IsRunning); }

    /// <summary>Changes the collector to incremental mode with the given parameters and returns the previous mode.</summary>
    auto gcIncremental(int pause, int stepmul, int stepsize)
    {
        return static_cast<GCOption>(gc(GCOption::Incremental, pause, stepmul, stepsize));
    }

    /// <summary>Changes the collector to generational mode with the given parameters and returns the previous mode.</summary>
    auto gcGenerational(int minormul, int majormul)
    {
        return static_cast<GCOption>(gc(GCOption::Generational, minormul, majormul));
    }

    // --- Index Wrapping ---

    /// <summary>Returns a wrapper to a stack index.</summary>
    StackIndex stackIndex(int index) { return {*this, index}; }

    /// <summary>Returns a wrapper to a stack index.</summary>
    ConstStackIndex stackIndex(int index) const { return {*this, index}; }

    /// <summary>Returns a wrapper to the registry pseudo index.</summary>
    RegistryIndex registry() { return {*this}; }

    /// <summary>Returns a wrapper to the registry pseudo index.</summary>
    ConstRegistryIndex registry() const { return {*this}; }

    /// <summary>Converts a 1-based index into an upvalue index and returns a wrapper to it.</summary>
    UpvalueIndex upvalue(int index) { return {*this, index}; }

    /// <summary>Converts a 1-based index into an upvalue index and returns a wrapper to it.</summary>
    ConstUpvalueIndex upvalue(int index) const { return {*this, index}; }

    // --- Indices Wrapping ---

    /// <summary>Returns a wrapper to a compile-time fixed range of stack indices.</summary>
    template <int Count>
    StackIndices<Count> stackIndices(int first)
    {
        return {*this, first};
    }

    /// <summary>Returns a wrapper to a compile-time fixed range of stack indices.</summary>
    template <int Count>
    ConstStackIndices<Count> stackIndices(int first) const
    {
        return {*this, first};
    }

    /// <summary>Returns a wrapper to a compile-time fixed range of upvalues.</summary>
    template <int Count>
    UpvalueIndices<Count> upvalueIndices(int first)
    {
        return {*this, first};
    }

    /// <summary>Returns a wrapper to a compile-time fixed range of upvalues.</summary>
    template <int Count>
    ConstUpvalueIndices<Count> upvalueIndices(int first) const
    {
        return {*this, first};
    }

    // --- Index Range Wrapping ---

    /// <summary>Returns a wrapper to a range of stack indices.</summary>
    StackIndexRange stackIndexRange(int first, int count) { return {*this, first, count}; }

    /// <summary>Returns a wrapper to a range of stack indices.</summary>
    ConstStackIndexRange stackIndexRange(int first, int count) const { return {*this, first, count}; }

    /// <summary>Returns a wrapper to a range of upvalues.</summary>
    UpvalueIndexRange upvalueIndexRange(int first, int count) { return {*this, first, count}; }

    /// <summary>Returns a wrapper to a range of upvalues.</summary>
    ConstUpvalueIndexRange upvalueIndexRange(int first, int count) const { return {*this, first, count}; }

    // --- Top and Bottom Wrapping ---

    /// <summary>Returns a wrapper to the first stack index.</summary>
    auto bottom() { return stackIndex(1); }

    /// <summary>Returns a wrapper to the first stack index.</summary>
    auto bottom() const { return stackIndex(1); }

    /// <summary>Returns a wrapper to a compile-time fixed size range of the first stack indices.</summary>
    template <int Count>
    auto bottom()
    {
        return stackIndices<Count>(1);
    }

    /// <summary>Returns a wrapper to a compile-time fixed size range of the first stack indices.</summary>
    template <int Count>
    auto bottom() const
    {
        return stackIndices<Count>(1);
    }

    /// <summary>Returns a wrapper to a range of the first stack indices.</summary>
    auto bottom(int count) { return stackIndexRange(1, count); }

    /// <summary>Returns a wrapper to a range of the first stack indices.</summary>
    auto bottom(int count) const { return stackIndexRange(1, count); }

    /// <summary>Returns a wrapper to the last stack index.</summary>
    auto top() { return stackIndex(size()); }

    /// <summary>Returns a wrapper to the last stack index.</summary>
    auto top() const { return stackIndex(size()); }

    /// <summary>Returns a wrapper to a compile-time fixed size range of the last stack indices.</summary>
    template <int Count>
    auto top()
    {
        return stackIndices<Count>(size() - Count + 1);
    }

    /// <summary>Returns a wrapper to a compile-time fixed size range of the last stack indices.</summary>
    template <int Count>
    auto top() const
    {
        return stackIndices<Count>(size() - Count + 1);
    }

    /// <summary>Returns a wrapper to a range of the last stack indices.</summary>
    auto top(int count) { return stackIndexRange(size() - count + 1, count); }

    /// <summary>Returns a wrapper to a range of the last stack indices.</summary>
    auto top(int count) const { return stackIndexRange(size() - count + 1, count); }

    // --- Index Properties ---

    /// <summary>Whether the given index is a stack index.</summary>
    static constexpr bool isStack(int index) { return index > LUA_REGISTRYINDEX; }

    /// <summary>Whether the given index is a pseudo index.</summary>
    static constexpr bool isPseudo(int index) { return index <= LUA_REGISTRYINDEX; }

    /// <summary>Whether the given index is an upvalue index.</summary>
    static constexpr bool isUpvalue(int index) { return index < LUA_REGISTRYINDEX; }

    /// <summary>Whether the given index is the registry index.</summary>
    static constexpr bool isRegistry(int index) { return index == LUA_REGISTRYINDEX; }

    /// <summary>Whether the given index is the first stack index or "offset" above it.</summary>
    bool isIndexBottom(int index, int offset = 0) const
    {
        assert(offset >= 0);
        return index == 1 + offset;
    }

    /// <summary>Whether the given index is the last stack index or "offset" below it.</summary>
    bool isIndexTop(int index, int offset = 0) const
    {
        assert(offset >= 0);
        return index == size() - offset;
    }

    /// <summary>Returns how many positions the given stack index is above the first index.</summary>
    int indexOffsetFromBottom(int index) const { return index - 1; }

    /// <summary>Returns how many positions the given stack index is below the last index.</summary>
    int indexOffsetFromTop(int index) const { return size() - index; }

    /// <summary>Turns negative stack indices into positive ones, leaving pseudo indices as is.</summary>
    int absIndex(int index) const { return index > LUA_REGISTRYINDEX && index < 0 ? size() + index + 1 : index; }

    /// <summary>Turns negative stack indices into positive ones, but does not working with pseudo indices.</summary>
    int absStackIndex(int index) const { return index < 0 ? size() + index + 1 : index; }

    // --- Stack Queries ---

    /// <summary>The (cached) size of the stack, which is also the top index, as indices start at 1.</summary>
    int size() const { return top_; }

    /// <summary>Returns the type of the element at the given index.</summary>
    Type type(int index) const { return static_cast<Type>(lua_type(state_, index)); }

    /// <summary>Returns the type name of the element at the given index.</summary>
    std::string_view typeName(int index) const { return luaL_typename(state_, index); }

    /// <summary>Whether the index is not valid.</summary>
    bool isNone(int index) const
    {
        assertAcceptable(index);
        return lua_isnone(state_, index);
    }

    /// <summary>Whether the element at the given index is nil.</summary>
    bool isNil(int index) const
    {
        assertAcceptable(index);
        return lua_isnil(state_, index);
    }

    /// <summary>Whether the index is not valid or its element is nil.</summary>
    bool isNoneOrNil(int index) const
    {
        assertAcceptable(index);
        return lua_isnoneornil(state_, index);
    }

    /// <summary>Whether the element at the given index is a boolean.</summary>
    bool isBoolean(int index) const
    {
        assertAcceptable(index);
        return lua_isboolean(state_, index);
    }

    /// <summary>Whether the element at the given index is light userdata.</summary>
    bool isLightUserdata(int index) const
    {
        assertAcceptable(index);
        return lua_islightuserdata(state_, index);
    }

    /// <summary>Whether the element at the given index is a number or a string convertible to a number.</summary>
    bool isNumber(int index) const
    {
        assertAcceptable(index);
        return lua_isnumber(state_, index);
    }

    /// <summary>Whether the element at the given index is an integer or a string convertible to an integer.</summary>
    bool isInteger(int index) const
    {
        assertAcceptable(index);
        return lua_isinteger(state_, index);
    }

    /// <summary>Whether the element at the given index is a string or a number (which is always convertible to a string).</summary>
    bool isString(int index) const
    {
        assertAcceptable(index);
        return lua_isstring(state_, index);
    }

    /// <summary>Whether the element at the given index is a table.</summary>
    bool isTable(int index) const
    {
        assertAcceptable(index);
        return lua_istable(state_, index);
    }

    /// <summary>Whether the element at the given index is a function (either C or Lua).</summary>
    bool isFunction(int index) const
    {
        assertAcceptable(index);
        return lua_isfunction(state_, index);
    }

    /// <summary>Whether the element at the given index is a C function.</summary>
    bool isCFunction(int index) const
    {
        assertAcceptable(index);
        return lua_iscfunction(state_, index);
    }

    /// <summary>Whether the element at the given index is full or light userdata.</summary>
    bool isUserdata(int index) const
    {
        assertAcceptable(index);
        return lua_isuserdata(state_, index);
    }

    /// <summary>Whether the element at the given index is a thread.</summary>
    bool isThread(int index) const
    {
        assertAcceptable(index);
        return lua_isthread(state_, index);
    }

    // --- Conversion ---

    // TODO: These are not const because of automatic number to string conversion...
    //       Possibly add const overloads that don't to this conversion.

    /// <summary>Uses the Convert template to convert the element at the given index.</summary>
    /// <remarks>Returns an optional, which is std::nullopt if the conversion failed.</remarks>
    template <typename T>
    auto to(int index)
    {
        return Convert<T>::at(state_, index);
    }

    /// <summary>Treats the element at the given index as a function argument and uses the Convert template to convert it.</summary>
    /// <remarks>Raises a Lua (argument) error if the conversion failed.</remarks>
    template <typename T>
    decltype(auto) check(int index)
    {
        return Convert<T>::check(state_, index);
    }

    // --- Stack Maintenance ---

    /// <summary>Asserts, whether the given positive index is currently acceptable without checking the stack.</summary>
    void assertAcceptable([[maybe_unused]] int index) const { assert(index >= 1 && index - size() <= pushable_); }

    /// <summary>Ensures, that the given positive index is going to be acceptable and returns false if it can't.</summary>
    bool checkAcceptable(int index) const
    {
        assert(index >= 1);
        return checkPushable(index - size());
    }

    /// <summary>Ensures, that the given positive index is going to be acceptable and raises a Lua error if it can't.</summary>
    void ensureAcceptable(int index, const char* error_message = nullptr) const
    {
        assert(index >= 1);
        ensurePushable(index - size(), error_message);
    }

    /// <summary>Asserts, whether it is possible to push a given number of elements without checking the stack.</summary>
    void assertPushable([[maybe_unused]] int count = 1) const { assert(count <= pushable_); }

    /// <summary>Asserts, whether it is possible to call an auxiliary function with the current stack.</summary>
    void assertPushableAuxiliary() const { assertPushable(auxiliary_required_pushable); }

    /// <summary>Trys to ensure, that it is possible to push a given number of values, returning false if it can't.</summary>
    bool checkPushable(int count = 1) const
    {
        if (!lua_checkstack(state_, count))
            return false;
#ifndef NDEBUG
        pushable_ = std::max(pushable_, count);
#endif
        return true;
    }

    /// <summary>Trys to ensures, that an auxiliary library function can be called, retunring false if it can't.</summary>
    bool checkPushableAuxiliary() const { return checkPushable(auxiliary_required_pushable); }

    /// <summary>Ensures, that it is possible to push a given number of values, raising a Lua error if it can't.</summary>
    void ensurePushable(int count = 1, const char* error_message = nullptr) const
    {
        luaL_checkstack(state_, count, error_message);
#ifndef NDEBUG
        pushable_ = std::max(pushable_, count);
#endif
    }

    /// <summary>Ensures, that an auxiliary library function can be called, returning false if it can't.</summary>
    void ensurePushableAuxiliary() const { ensurePushable(auxiliary_required_pushable); }

    // --- Push and Pop ---

    /// <summary>Pushes nil values up to the given positive index.</summary>
    void padWithNil(int index)
    {
        assert(index >= 1);
        int current_top = size();
        if (index <= current_top)
            return;
        lua_settop(state_, index);
        notifyPush(index - current_top);
    }

    /// <summary>Pops a given number of elements from the stack.</summary>
    void pop(int count = 1)
    {
        lua_pop(state_, count);
        notifyPush(-count);
    }

    /// <summary>Uses the Convert template to push all given values on the stack and returns a wrapper to them.</summary>
    /// <remarks>Automatically ignores all rvalue stack indices if possible.</remarks>
    template <typename... TValues>
    auto push(TValues&&... values)
    {
        if constexpr (CombinedPushCount<TValues...>) {
            constexpr int push_count = *CombinedPushCount<TValues...>;
            pushHelper(std::forward<TValues>(values)...);
            if constexpr (push_count == 1)
                return top().asResult();
            else
                return top<push_count>().asResults();
        }
        else {
            int push_count = combinedPushCount(values...);
            pushHelper(std::forward<TValues>(values)...);
            return top(push_count).asResults();
        }
    }

    /// <summary>Pushes each tuple element on the stack.</summary>
    template <typename... TValues>
    auto push(const std::tuple<TValues...>& tuple)
    {
        return pushTupleValues(tuple, std::index_sequence_for<TValues...>{});
    }

    /// <summary>Pushes each tuple element on the stack.</summary>
    template <typename... TValues>
    auto push(std::tuple<TValues...>&& tuple)
    {
        return pushTupleValues(std::move(tuple), std::index_sequence_for<TValues...>{});
    }

    /// <summary>Pushes nil on the stack.</summary>
    auto pushNil() { return push(nullptr); }

    /// <summary>Pushes a newly created table with optional hints for the size of its array and record parts.</summary>
    auto pushTable(int array_hint = 0, int record_hint = 0)
    {
        lua_createtable(state_, array_hint, record_hint);
        notifyPush();
        return top().asResult();
    }

    /// <summary>Pushes a newly created thread on the stack.</summary>
    auto pushThread()
    {
        State thread = lua_newthread(state_);
        notifyPush();
        return std::tuple{top().asResult(), std::move(thread)};
    }

    /// <summary>Pushes an in-place constructed object with the given parameters.</summary>
    template <typename T, typename... TArgs>
    auto pushNew(TArgs&&... args)
    {
        if constexpr (Convert<T>::PushCount) {
            constexpr int push_count = *Convert<T>::PushCount;
            assertPushable(push_count);
            Convert<T>::push(state_, std::forward<TArgs>(args)...);
            notifyPush(push_count);
            if constexpr (push_count == 1)
                return top().asResult();
            else
                return top<push_count>().asResults();
        }
        else {
            T object(std::forward<TArgs>(args)...);
            int push_count = Convert<T>::getPushCount(object);
            assertPushable(push_count);
            Convert<T>::push(state_, std::move(object));
            notifyPush(push_count);
            return top(push_count).asResults();
        }
    }

    /// <summary>Pushes a function or closure with an arbitrary number of upvalues on the stack.</summary>
    template <typename... TUpvalues>
    auto pushFunction(lua_CFunction function, TUpvalues&&... upvalues)
    {
        auto upvalue_count = push(std::forward<TUpvalues>(upvalues)...).size();
        lua_pushcclosure(state_, function, upvalue_count);
        notifyPush(1 - upvalue_count);
        return top().asResult();
    }

    /// <summary>Pushes a template specified function or closure with an arbitrary number of upvalues on the stack.</summary>
    template <auto Function, typename... TUpvalues>
    auto pushFunction(TUpvalues&&... upvalues)
    {
        return pushFunction(wrap<Function>, std::forward<TUpvalues>(upvalues)...);
    }

    /// <summary>Turns the function-like object into a std::function and pushes a wrapped closure onto the stack.</summary>
    /// <remarks>The first upvalue is used to store the function object itself.</remarks>
    template <typename TFunc, typename... TUpvalues>
    auto pushFunction(TFunc&& func, TUpvalues&&... upvalues)
    {
        std::function wrapped_func{std::forward<TFunc>(func)};
        return pushFunction(
            wrappedFunction<decltype(wrapped_func)>, std::move(wrapped_func), std::forward<TUpvalues>(upvalues)...);
    }

    /// <summary>Pushes the global table on the stack.</summary>
    auto pushGlobalTable()
    {
        lua_pushglobaltable(state_);
        return top().asResult();
    }

    /// <summary>Same as push, recommended to use for temporaries in expressions.</summary>
    template <typename... TValues>
    auto operator()(TValues&&... values)
    {
        return push(std::forward<TValues>(values)...);
    }

    /// <summary>Replaces the given positive index with a single value.</summary>
    template <typename TIndex, typename TValue>
    void replace(TIndex&& index, TValue&& value)
    {
        static_assert(Convert<TValue>::PushCount == 1, "Supplied value must take up a single stack position.");
        static_assert(IsIndex<std::decay_t<TIndex>>::value, "Supplied index must be an index.");

        if constexpr (IsIndex<std::decay_t<TValue>>::value) {
            assertPushable();
            lua_copy(state_, value.index(), index.index());
            if constexpr (IsAnyMovedStackIndexResult<TValue&&>)
                if (value.isTop())
                    pop();
        }
        else {
            assertPushable();
            Convert<TValue>::push(state_, std::forward<TValue>(value));
            lua_replace(state_, index.index());
        }
    }

    // --- Error ---

    template <typename TMessage>
    [[noreturn]] void error(TMessage&& message)
    {
        static_assert(Convert<TMessage>::PushCount == 1, "Supplied message must take up a single stack position.");
        push(std::forward<TMessage>(message));
        // technically lua_error pops the message, but since it doesn't return this is not really visible to users
        detail::noreturn_lua_error(state_);
    }

    [[noreturn]] void argError(int arg, const char* extra_message)
    {
        detail::noreturn_luaL_argerror(state_, arg, extra_message);
    }

    [[noreturn]] void argError(int arg, const std::string& extra_message) { argError(arg, extra_message.c_str()); }

    [[noreturn]] void typeError(int arg, const char* type_name)
    {
        detail::noreturn_luaL_typeerror(state_, arg, type_name);
    }

    [[noreturn]] void typeError(int arg, const std::string& type_name) { typeError(arg, type_name.c_str()); }

    // --- Calling ---

    /// <summary>Calls the given function with the supplied arguments and returns a template specified number of results.</summary>
    template <int Results = 0, typename TFunc, typename... TArgs>
    auto call(TFunc&& func, TArgs&&... args)
    {
        static_assert(Convert<TFunc>::PushCount == 1, "Supplied function must take up a single stack position.");

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        if constexpr (Results == LUA_MULTRET) {
            auto first_result_index = size() - 1 - arg_count;
            lua_call(state_, arg_count, Results);
            auto results = lua_gettop(state_) - first_result_index;
            notifyPush(results - 1 - arg_count);
            return top(results).asResults();
        }
        else {
            int diff = Results - arg_count - 1;
            assertPushable(diff);
            lua_call(state_, arg_count, Results);
            notifyPush(diff);
            if constexpr (Results == 1)
                return top().asResult();
            else
                return top<Results>().asResults();
        }
    }

    /// <summary>Calls the given function with the supplied arguments and returns all results.</summary>
    template <typename TFunc, typename... TArgs>
    auto callMultRet(TFunc&& func, TArgs&&... args)
    {
        return call<LUA_MULTRET>(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the given function with the supplied arguments and returns a specified number of results.</summary>
    template <typename TFunc, typename... TArgs>
    auto callReturning(int results, TFunc&& func, TArgs&&... args)
    {
        static_assert(Convert<TFunc>::PushCount == 1, "Supplied function must take up a single stack position.");
        assert(results != LUA_MULTRET); // TODO: Support LUA_MULTRET

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        assertPushable(results - 1 - arg_count);
        lua_call(state_, arg_count, results);
        notifyPush(results - 1 - arg_count);
        return top(results).asResults();
    }

    /// <summary>Calls the given function with the supplied arguments in protected mode and returns the status and a template specified number of results.</summary>
    template <int Results = 0, typename TFunc, typename... TArgs>
    auto pcall(TFunc&& func, TArgs&&... args)
    {
        static_assert(Convert<TFunc>::PushCount == 1, "Supplied function must take up a single stack position.");

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        if constexpr (Results == LUA_MULTRET) {
            auto first_result_index = size() - 1 - arg_count;
            auto status = static_cast<Status>(lua_pcall(state_, arg_count, Results, 0)); // TODO: Message Handler
            auto results = status == Status::Ok ? lua_gettop(state_) - first_result_index : 1;
            notifyPush(results - 1 - arg_count);
            return std::tuple{status, top(results).asResults()};
        }
        else {
            assertPushable(Results - 1 - arg_count);
            auto status = static_cast<Status>(lua_pcall(state_, arg_count, Results, 0)); // TODO: Message Handler
            auto results = status == Status::Ok ? Results : 1;
            notifyPush(results - 1 - arg_count);
            return std::tuple{status, top(results).asResults()};
        }
    }

    /// <summary>Calls the given function with the supplied arguments in protected mode and returns the status and all results.</summary>
    template <typename TFunc, typename... TArgs>
    auto pcallMultRet(TFunc&& func, TArgs&&... args)
    {
        return pcall<LUA_MULTRET>(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
    }

    /// <summary>Calls the given function with the supplied arguments in protected mode and returns the status and a specfied number of results.</summary>
    template <typename TFunc, typename... TArgs>
    auto pcallReturning(int results, TFunc&& func, TArgs&&... args)
    {
        static_assert(Convert<TFunc>::PushCount == 1, "Supplied function must take up a single stack position.");
        assert(results != LUA_MULTRET); // TODO: Support LUA_MULTRET

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        assertPushable(results - 1 - arg_count);
        auto status = static_cast<Status>(lua_pcall(state_, arg_count, results, 0)); // TODO: Message Handler
        if (status != Status::Ok)
            results = 1;
        notifyPush(results - 1 - arg_count);
        return std::tuple{status, top(results).asResults()};
    }

    // --- Compiling ---

    /// <summary>Compiles the given code and returns a the compilation status and depending on this either the function or error message.</summary>
    auto load(const char* buffer, std::size_t size, const char* name = nullptr, LoadMode mode = LoadMode::Default)
    {
        // TODO: C++20 replace buffer and size with std::span
        assertPushableAuxiliary();
        int status = luaL_loadbufferx(state_, buffer, size, name, loadModeNames[static_cast<std::size_t>(mode)]);
        notifyPush();
        return std::tuple{static_cast<Status>(status), top().asResult()};
    }

    /// <summary>Compiles the given code and returns a the compilation status and depending on this either the function or error message.</summary>
    auto load(std::string_view buffer, const char* name = nullptr, LoadMode mode = LoadMode::Default)
    {
        return load(buffer.data(), buffer.size(), name, mode);
    }

    /// <summary>Compiles the given code and returns a the compilation status and depending on this either the function or error message.</summary>
    auto load(const char* buffer, std::size_t size, const std::string& name, LoadMode mode = LoadMode::Default)
    {
        return load(buffer, size, name.c_str(), mode);
    }

    /// <summary>Compiles the given code and returns a the compilation status and depending on this either the function or error message.</summary>
    auto load(std::string_view buffer, const std::string& name, LoadMode mode = LoadMode::Default)
    {
        return load(buffer.data(), buffer.size(), name.c_str(), mode);
    }

    // --- Operations ---

    /// <summary>Performs a unary operation on the given operand.</summary>
    /// <remarks>Asserts, that the given operation is actually unary.</remarks>
    template <typename T>
    auto unary(ArithOp operation, T&& operand)
    {
        assert(operation == ArithOp::UnaryMinus || operation == ArithOp::BinaryNot);
        auto pushed_operand = push(std::forward<T>(operand));
        static_assert(std::is_same_v<decltype(pushed_operand), StackIndex>,
                      "Unary operations require exactly one operand.");
        lua_arith(state_, static_cast<int>(operation));
        return top().asResult();
    }

    /// <summary>Performs a binary operation on the two given operands.</summary>
    /// <remarks>Asserts, that the given operation is not unary.</remarks>
    template <typename TLeft, typename TRight>
    auto binary(ArithOp operation, TLeft&& lhs, TRight&& rhs)
    {
        assert(operation != ArithOp::UnaryMinus && operation != ArithOp::BinaryNot);
        auto pushed_operands = push(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
        static_assert(std::is_same_v<decltype(pushed_operands), StackIndices<2>>,
                      "Binary operations require exactly two operands.");
        lua_arith(state_, static_cast<int>(operation));
        notifyPush(-1);
        return top().asResult();
    }

    /// <summary>Performs a compile-time known operation on the operand(s).</summary>
    template <ArithOp Operation, typename... TArgs>
    auto arith(TArgs&&... args)
    {
        constexpr bool unary_op = Operation == ArithOp::UnaryMinus || Operation == ArithOp::BinaryNot;
        auto pushed_args = push(std::forward<TArgs>(args)...);
        constexpr bool unary_arg =
            Convert<decltype(pushed_args)>::PushCount && *Convert<decltype(pushed_args)>::PushCount == 1;
        constexpr bool binary_args =
            Convert<decltype(pushed_args)>::PushCount && *Convert<decltype(pushed_args)>::PushCount == 2;
        static_assert(unary_op && unary_arg || binary_args, "Argument count does not match the operation type.");
        lua_arith(state_, static_cast<int>(Operation));
        if constexpr (binary_args)
            notifyPush(-1);
        return top().asResult();
    }

    /// <summary>Compares the two operands with a given operation.</summary>
    template <typename TLeft, typename TRight>
    bool compare(CompareOp operation, TLeft&& lhs, TRight&& rhs) const
    {
        static_assert(Convert<TLeft>::PushCount == 1, "Left operand must take up a single stack position.");
        static_assert(Convert<TRight>::PushCount == 1, "Right operand must take up a single stack position.");

        constexpr bool left_is_index = IsIndex<std::decay_t<TLeft>>::value;
        constexpr bool right_is_index = IsIndex<std::decay_t<TRight>>::value;
        if constexpr (left_is_index) {
            if constexpr (right_is_index)
                return lua_compare(state_, lhs.index(), rhs.index(), static_cast<int>(operation)) != 0;
            else {
                assertPushable();
                Convert<TRight>::push(state_, std::forward<TRight>(rhs));
                bool result = lua_compare(state_, lhs.index(), -1, static_cast<int>(operation)) != 0;
                lua_pop(state_, 1);
                return result;
            }
        }
        else {
            if constexpr (right_is_index) {
                assertPushable();
                Convert<TLeft>::push(state_, std::forward<TLeft>(lhs));
                bool result = lua_compare(state_, -1, rhs.index(), static_cast<int>(operation)) != 0;
                lua_pop(state_, 1);
                return result;
            }
            else {
                assertPushable(2);
                Convert<TLeft>::push(state_, std::forward<TLeft>(lhs));
                Convert<TRight>::push(state_, std::forward<TRight>(rhs));
                bool result = lua_compare(state_, -2, -1, static_cast<int>(operation)) != 0;
                lua_pop(state_, 2);
                return result;
            }
        }
    }

    /// <summary>Concatenates all given values.</summary>
    /// <remarks>Returns an empty string if no arguments are given and the value itself if only one value is provided.</remarks>
    template <typename... TValues>
    auto concat(TValues&&... values)
    {
        auto pushed_values = push(std::forward<TValues>(values)...);
        lua_concat(state_, pushed_values.size());
        notifyPush(1 - pushed_values.size());
        return top().asResult();
    }

    /// <summary>Pushes the length of the element at the given index on the stack.</summary>
    /// <remarks>This can invoke the __len meta-method and therefore doesn't necessarily return an integer.</remarks>
    auto pushLength(int index)
    {
        lua_len(state_, index);
        notifyPush();
        return top().asResult();
    }

    /// <summary>Returns the length of the element at the given index.</summary>
    /// <remarks>This can invoke the __len meta-method and raises an error if that doesn't return an integer.</remarks>
    auto length(int index) const
    {
        assertPushableAuxiliary();
        return luaL_len(state_, index);
    }

    /// <summary>Returns the raw length of the value, which does not invoke meta-method.</summary>
    auto rawLength(int index) const { return lua_rawlen(state_, index); }

    // --- Table Access ---

    /// <summary>Queries the table with the given key, returning both the type and the pushed value.</summary>
    /// <remarks>Can invoke the __index meta-method.</remarks>
    template <typename TTable, typename TKey>
    auto getTableWithType(TTable& table, TKey&& key)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        if constexpr (std::is_integral_v<std::decay_t<TKey>> && !std::is_same_v<std::decay_t<TKey>, bool>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            Type type = static_cast<Type>(lua_geti(state_, table.index(), lua_Integer{key}));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            Type type = static_cast<Type>(lua_getfield(state_, table.index(), key));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, std::string>) {
            return getTable(table, key.c_str());
        }
        else {
            push(std::forward<TKey>(key));
            Type type = static_cast<Type>(lua_gettable(state_, table.index()));
            // remove key, add value
            // -1, +1
            // notifyPush(0);
            return std::tuple{type, top().asResult()};
        }
    }

    /// <summary>Queries the table with the given key, returning the pushed value.</summary>
    /// <remarks>Can invoke the __index meta-method.</remarks>
    template <typename TTable, typename TKey>
    auto getTable(TTable& table, TKey&& key)
    {
        auto [type, index] = getTableWithType(table, std::forward<TKey>(key));
        return index;
    }

    /// <summary>Sets a key of the table to the given value.</summary>
    /// <remarks>Can invoke the __newindex meta-method.</remarks>
    template <typename TTable, typename TKey, typename TValue>
    void setTable(TTable& table, TKey&& key, TValue&& value)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        static_assert(Convert<TValue>::PushCount == 1, "Supplied value must take up a single stack position.");
        if constexpr (std::is_integral_v<std::decay_t<TKey>> && !std::is_same_v<std::decay_t<TKey>, bool>) {
            push(std::forward<TValue>(value));
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            lua_seti(state_, table.index(), lua_Integer{key});
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            push(std::forward<TValue>(value));
            lua_setfield(state_, table.index(), key);
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, std::string>) {
            setTable(table, key.c_str(), std::forward<TValue>(value));
        }
        else {
            push(std::forward<TKey>(key), std::forward<TValue>(value));
            lua_settable(state_, table.index());
            // remove key and value, push nothing
            // -2, +0
            notifyPush(-2);
        }
    }

    /// <summary>Similar to getTableWithType, but does not invoke meta-methods.</summary>
    template <typename TTable, typename TKey>
    auto rawGetTableWithType(TTable& table, TKey&& key)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        if constexpr (std::is_integral_v<std::decay_t<TKey>> && !std::is_same_v<std::decay_t<TKey>, bool>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            Type type = static_cast<Type>(lua_rawgeti(state_, table.index(), lua_Integer{key}));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_pointer_v<TKey>) {
            assertPushable();
            Type type = static_cast<Type>(lua_rawgetp(state_, table.index(), key));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else {
            push(std::forward<TKey>(key));
            Type type = static_cast<Type>(lua_rawget(state_, table.index()));
            // remove key, add value
            // -1, +1
            // notifyPush(0);
            return std::tuple{type, top().asResult()};
        }
    }

    /// <summary>Similar to getTable, but does not invoke meta-methods.</summary>
    template <typename TTable, typename TKey>
    auto rawGetTable(TTable& table, TKey&& key)
    {
        auto [type, index] = rawGetTableWithType(table, std::forward<TKey>(key));
        return index;
    }

    /// <summary>Similar to setTable, but does not invoke meta-methods.</summary>
    template <typename TTable, typename TKey, typename TValue>
    void rawSetTable(TTable& table, TKey&& key, TValue&& value)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        static_assert(Convert<TValue>::PushCount == 1, "Supplied value must take up a single stack position.");
        if constexpr (std::is_integral_v<std::decay_t<TKey>> && !std::is_same_v<std::decay_t<TKey>, bool>) {
            push(std::forward<TValue>(value));
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            lua_rawseti(state_, table.index(), lua_Integer{key});
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else if constexpr (std::is_pointer_v<TKey>) {
            push(std::forward<TValue>(value));
            lua_rawsetp(state_, table.index(), key);
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else {
            push(std::forward<TKey>(key), std::forward<TValue>(value));
            lua_rawset(state_, table.index());
            // remove key and value, push nothing
            // -2, +0
            notifyPush(-2);
        }
    }

    template <typename TKey>
    auto getGlobalWithType(TKey&& key)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            Type type = static_cast<Type>(lua_getglobal(state_, key));
            notifyPush(1);
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, std::string>) {
            return getGlobalWithType(key.c_str());
        }
        else {
            return pushGlobalTable().getTableWithType(std::forward<TKey>(key));
        }
    }

    template <typename TKey>
    auto getGlobal(TKey&& key)
    {
        auto [type, result] = getGlobalWithType(std::forward<TKey>(key));
        return result;
    }

    /// <summary>Sets a new value in the global table.</summary>
    template <typename TKey, typename TValue>
    void setGlobal(TKey&& key, TValue&& value)
    {
        static_assert(Convert<TKey>::PushCount == 1, "Supplied key must take up a single stack position.");
        static_assert(Convert<TValue>::PushCount == 1, "Supplied value must take up a single stack position.");
        if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            push(std::forward<TValue>(value));
            lua_setglobal(state_, key);
            notifyPush(-1);
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, std::string>) {
            return setGlobal(key.c_str(), std::forward<TValue>(value));
        }
        else {
            return pushGlobalTable().setTable(std::forward<TKey>(key), std::forward<TValue>(value));
        }
    }

    /// <summary>Returns the metatable of the element at the given index or std::nullopt if it doesn't have one.</summary>
    std::optional<StackIndexResult> getMetatable(int index)
    {
        if (lua_getmetatable(state_, index)) {
            notifyPush(1);
            return top().asResult();
        }
        return std::nullopt;
    }

    /// <summary>Sets the metatable of the element at the given index to the specified table; or nil to remove it.</summary>
    template <typename TMetatable>
    void setMetatable(int index, TMetatable&& metatable)
    {
        push(std::forward<TMetatable>(metatable));
        lua_setmetatable(state_, index);
        notifyPush(-1);
    }

    /// <summary>Returns the next key-value-pair of the table or nothing, if the table has been exhausted.</summary>
    template <typename TKey>
    auto next(int table_index, TKey&& key)
    {
        push(std::forward<TKey>(key));
        if (lua_next(state_, table_index)) {
            notifyPush(1);
            return top(2).asResults();
        }
        notifyPush(-1);
        return top(0).asResults();
    }

    // --- Formatting ---

    /// <summary>Converts the element at the given index to a string in a reasonable format using luaL_tolstring.</summary>
    std::string format(int index) const
    {
        std::size_t length;
        const char* string = luaL_tolstring(state_, index, &length);
        std::string result(string, string + length);
        lua_pop(state_, 1);
        return result;
    }

    /*
    /// <summary>This version can be used by the debugger, as it returns references to up to 1024 formatted strings per thread.</summary>
    const std::string& formatDebug(int index) const
    {
        thread_local std::size_t current = 0;
        thread_local std::vector<std::string> results(1024);
        auto& result = results[current] = format(index) + " (" + std::string(typeName(index)) + ")";
        current++;
        current %= results.size();
        return result;
    }
    */

    // --- Libraries ---

    /// <summary>Opens a single standard library.</summary>
    void openLib(StandardLibrary library)
    {
        assertPushableAuxiliary();
        int index = static_cast<int>(library);
        luaL_requiref(state_, libraryNames[index], libraryFunctions[index], 1);
        lua_pop(state_, 1);
    }

    /// <summary>Pushes a single standard library on the stack and returns a wrapper to it.</summary>
    auto pushLib(StandardLibrary library)
    {
        assertPushableAuxiliary();
        int index = static_cast<int>(library);
        luaL_requiref(state_, libraryNames[index], libraryFunctions[index], 1);
        notifyPush();
        return top().asResult();
    }

    /// <summary>Opens all Lua standard libraries.</summary>
    void openLibs()
    {
        assertPushableAuxiliary();
        luaL_openlibs(state_);
    }

    /// <summary>Opens a library with the given name, using the specified function and optionally makes it global.</summary>
    void require(const char* name, lua_CFunction open_function, bool global = false)
    {
        assertPushableAuxiliary();
        luaL_requiref(state_, name, open_function, global);
        lua_pop(state_, 1);
    }

    template <typename T>
    void require(bool global = false)
    {
        require(ClassName<T>, wrap<&ClassInfo<T>::require>, global);
    }

    /// <summary>Pushes a library with the given name, using the specified function on the stack, returns a wrapper to it and optionally makes it global.</summary>
    auto pushRequire(const char* name, lua_CFunction open_function, bool global = false)
    {
        assertPushableAuxiliary();
        luaL_requiref(state_, name, open_function, global);
        notifyPush();
        return top().asResult();
    }

    template <typename T>
    auto pushRequire(bool global = false)
    {
        return pushRequire(ClassName<T>, wrap<&ClassInfo<T>::require>, global);
    }

    /// <summary>Registers a function with a given name in the global table.</summary>
    template <typename TFunc, typename... TUpvalues>
    void registerGlobal(const char* name, TFunc&& func, TUpvalues&&... upvalues)
    {
        pushFunction(std::forward<TFunc>(func), std::forward<TUpvalues>(upvalues)...);
        lua_setglobal(state_, name);
        notifyPush(-1);
    }

    /// <summary>Registers a function with a given name in the global table.</summary>
    template <typename TFunc, typename... TUpvalues>
    void registerGlobal(const std::string& name, TFunc&& func, TUpvalues&&... upvalues)
    {
        registerGlobal(name.c_str(), std::forward<TFunc>(func), std::forward<TUpvalues>(upvalues)...);
    }

    /// <summary>Registers a template specified function with a given name in the global table.</summary>
    template <auto Func, typename... TUpvalues>
    void registerGlobal(const char* name, TUpvalues&&... upvalues)
    {
        pushFunction<Func>(std::forward<TUpvalues>(upvalues)...);
        lua_setglobal(state_, name);
        notifyPush(-1);
    }

    /// <summary>Registers a template specified function with a given name in the global table.</summary>
    template <auto Func, typename... TUpvalues>
    void registerGlobal(const std::string& name, TUpvalues&&... upvalues)
    {
        registerGlobal<Func>(name.c_str(), std::forward<TUpvalues>(upvalues)...);
    }

    // --- Function Wrapping ---

    /// <summary>Updates the internally stored top value to allow "wrap" to pad parameters with nil.</summary>
    void maxFuncArg(int index)
    {
        if (top_ < index)
            top_ = index;
    }

    // --- Reference ---

    /// <summary>Stores the value as a reference in the registry table and returns a wrapper.</summary>
    template <typename T>
    Reference ref(T&& value)
    {
        static_assert(Convert<T>::PushCount == 1, "Supplied value must take up a single stack position.");
        push(std::forward<T>(value));
        notifyPush(-1);
        return Reference(state());
    }

private:
    /// <summary>Helper function for lua_gc which is const, since some options are, in fact, const.</summary>
    template <typename... TArgs>
    int gc(GCOption option, TArgs&&... args) const
    {
        return lua_gc(state_, static_cast<int>(option), std::forward<TArgs>(args)...);
    }

    /// <summary>After some values got pushed (or popped) this function has to be called.</summary>
    void notifyPush(int count = 1)
    {
        assertPushable(count);
        top_ += count;
#ifndef NDEBUG
        pushable_ -= count;
#endif
    }

    /// <summary>Uses the Convert template to push all given values on the stack.</summary>
    /// <remarks>Automatically ignores all rvalue stack indices if possible.</remarks>
    template <typename TFirst, typename... TRest>
    void pushHelper(TFirst&& first, TRest&&... rest)
    {
        if constexpr (IsAnyMovedStackIndexResult<TFirst&&>::value) {
            int skipped = 1;
            int top_offset = indexOffsetFromTop(first.last());
            if (top_offset > 0) {
                countSkipped(skipped, top_offset, std::forward<TRest>(rest)...);
            }
            if (top_offset == 0) {
                pushWithSkip(skipped, std::forward<TFirst>(first), std::forward<TRest>(rest)...);
                return;
            }
        }
        pushValue(std::forward<TFirst>(first));
        (pushValue(std::forward<TRest>(rest)), ...);
    }

    /// <summary>Uses the Convert template to push all given values on the stack.</summary>
    /// <remarks>Automatically ignores all rvalue stack indices if possible.</remarks>
    void pushHelper()
    {
        // push nothing
    }

    /// <summary>Pushes all elements of the tuple in order.</summary>
    template <typename... TValues, std::size_t... Indices>
    auto pushTupleValues(const std::tuple<TValues...>& tuple, std::index_sequence<Indices...>)
    {
        return push(std::get<Indices>(tuple)...);
    }

    /// <summary>Pushes all elements of the tuple in order.</summary>
    template <typename... TValues, std::size_t... Indices>
    auto pushTupleValues(std::tuple<TValues...>&& tuple, std::index_sequence<Indices...>)
    {
        return push(std::move(std::get<Indices>(tuple))...);
    }

    /// <summary>Counts how many elements can be skipped because they are rvalue stack indices at the correct position.</summary>
    template <typename TFirst, typename... TRest>
    void countSkipped(int& skipped, int& top_offset, [[maybe_unused]] TFirst&& first, [[maybe_unused]] TRest&&... rest)
    {
        if constexpr (IsAnyMovedStackIndexResult<TFirst&&>::value) {
            skipped++;
            top_offset -= first.size();
            if (top_offset <= 0 || top_offset != indexOffsetFromTop(first.last()))
                return;
            countSkipped(skipped, top_offset, std::forward<TRest>(rest)...);
        }
    }

    /// <summary>Counts how many elements can be skipped because they are rvalue stack indices at the correct position.</summary>
    void countSkipped(int&, int&)
    {
        // nothing
    }

    /// <summary>Skips a given amount of arguments and pushes the rest.</summary>
    template <typename... TArgs>
    void pushWithSkip(std::size_t skip, TArgs&&... args)
    {
        pushWithSkipHelper(skip, std::index_sequence_for<TArgs...>(), std::forward<TArgs>(args)...);
    }

    /// <summary>Actual implementation of pushWithSkip using an index sequence.</summary>
    template <std::size_t... Indices, typename... TArgs>
    void pushWithSkipHelper(std::size_t skip, std::index_sequence<Indices...>, TArgs&&... args)
    {
        ((Indices >= skip ? pushValue(std::forward<TArgs>(args)) : (void)0), ...);
    }

    /// <summary>Simply pushes a single value on the stack.</summary>
    template <typename T>
    void pushValue(T&& value)
    {
        if constexpr (Convert<T>::PushCount) {
            constexpr int push_count = *Convert<T>::PushCount;
            assertPushable(push_count);
            Convert<T>::push(state_, std::forward<T>(value));
            notifyPush(push_count);
        }
        else {
            int push_count = Convert<T>::getPushCount(value);
            assertPushable(push_count);
            Convert<T>::push(state_, std::forward<T>(value));
            notifyPush(push_count);
        }
    }

    /// <summary>A function wrapper, that expects a std::function of the templated type in the first upvalue slot of the called closure.</summary>
    template <typename Func>
    static int wrappedFunction(lua_State* state)
    {
        using Info = detail::SignatureInfo<Func>;
        // TODO: Is "check" appropriate here?
        Func func = Convert<Func>::check(state, lua_upvalueindex(1));

        // TODO: Code duplication with wrap
        State lua(state);
        auto old_top = lua.size();
        auto&& args = Info::convertArguments(lua);
        if (old_top != lua.size()) {
            assert(lua.size() > old_top);
            lua.ensurePushable(lua.size() - old_top);
            lua_settop(state, lua.size());
        }

        if constexpr (std::is_void_v<Info::Return>) {
            std::apply(func, std::move(args));
            return 0;
        }
        else {
            return lua.push(std::apply(func, std::move(args))).size();
        }
    }

    lua_State* state_;
    int top_ = lua_gettop(state_);
#ifndef NDEBUG
    mutable int pushable_ = LUA_MINSTACK;
#endif
};

template <auto Func>
inline int wrap(lua_State* state)
{
    using Info = detail::SignatureInfo<decltype(Func)>;

    if constexpr (Info::AnyStateArgs) {
        State lua(state);
        auto old_top = lua.size();
        auto&& args = Info::convertArguments(lua);

        // convertArguments calls maxFuncArg for StackIndex and StackIndices, which updates the internal size
        if constexpr (Info::AnyFixedSizeStackArgs) {
            if (old_top != lua.size()) {
                // It should only increase
                assert(lua.size() > old_top);
                // Fill the rest with nil
                lua.ensurePushable(lua.size() - old_top);
                lua_settop(state, lua.size());
            }
        }

        if constexpr (std::is_void_v<typename Info::Return>) {
            std::apply(Func, std::move(args));
            return 0;
        }
        else {
            return lua.push(std::apply(Func, std::move(args))).size();
        }
    }
    else {
        auto&& args = Info::convertArgumentsRaw(state);
        if constexpr (std::is_void_v<typename Info::Return>) {
            std::apply(Func, std::move(args));
            return 0;
        }
        else {
            auto&& result = std::apply(Func, std::move(args));
            using TResult = decltype(result);
            if constexpr (Convert<TResult>::PushCount) {
                constexpr auto push_count = *Convert<TResult>::PushCount;
                if constexpr (push_count > LUA_MINSTACK)
                    luaL_checkstack(state, push_count);
                Convert<TResult>::push(state, std::move(result));
                return push_count;
            }
            else {
                auto push_count = Convert<TResult>::pushCount(result);
                if (push_count > LUA_MINSTACK)
                    luaL_checkstack(state, push_count);
                Convert<TResult>::push(state, std::move(result));
                return push_count;
            }
        }
    }
}

template <auto Func>
inline constexpr luaL_Reg reg(const char* name)
{
    return {name, wrap<Func>};
}

/// <summary>A Lua state wrapper, which owns the state and therefore closes it when it goes out of scope.</summary>
class OwnedState : public State {
public:
    /// <summary>Creates a new Lua state, by default opening the standard libraries and using the auxiliary allocation and panic function.</summary>
    /// <remarks>A custom allocation function can be supplied and userdata is used solely as extra parameter to that function.</remarks>
    OwnedState(bool open_libs = true, lua_Alloc allocator = nullptr, void* userdata = nullptr)
        : State(allocator ? lua_newstate(allocator, userdata) : luaL_newstate())
    {
        if (open_libs)
            openLibs();
    }

    /// <summary>Closes the Lua state if it is not already closed.</summary>
    ~OwnedState() { close(); }

    OwnedState(const OwnedState&) = delete;

    OwnedState(OwnedState&& other) noexcept
        : State(std::exchange(other.state_, nullptr))
    {
        top_ = other.top_;
#ifndef NDEBUG
        pushable_ = other.pushable_;
#endif
    }

    OwnedState& operator=(const OwnedState&) = delete;

    OwnedState& operator=(OwnedState&& other) noexcept
    {
        if (this == &other)
            return *this;
        close();
        state_ = std::exchange(other.state_, nullptr);
        top_ = other.top_;
#ifndef NDEBUG
        pushable_ = other.pushable_;
#endif
        return *this;
    }

    /// <summary>Whether the state as been closed manually.</summary>
    bool closed() { return state_ == nullptr; }

    /// <summary>Closes the Lua state manually, can be called multiple times.</summary>
    void close()
    {
        if (closed())
            return;
        lua_close(state_);
        state_ = nullptr;
    }
};

/// <summary>Used to automatically pop any pushed elements at the end of the scope.</summary>
class ScopedStack {
public:
    /// <summary>Constructs a scoped stack for the given stack.</summary>
    ScopedStack(State& state)
        : state_(state)
        , initially_pushed_(state.top_)
    {}

    /// <summary>Resets the top to the initial one, effectively popping all leftover elements.</summary>
    ~ScopedStack()
    {
        // accidentally popped values cannot be recreated and would simply be filled with nil
        // therefore this sanity check
        assert(state_.top_ >= initially_pushed_);
        state_.pop(state_.top_ - initially_pushed_);
    }

private:
    State& state_;
    int initially_pushed_;
};

template <>
struct Convert<Reference> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = false;

    static void push([[maybe_unused]] lua_State* state, const Reference& reference)
    {
        assert(reference.state_ == state);
        reference.push();
    }
};

template <>
struct Convert<State&> {
    static constexpr std::optional<int> PushCount = 0;
    static constexpr bool AllowNesting = true;

    static bool isExact(State&, int) { return true; }

    static constexpr bool isValid(State&, int) { return true; }

    static State& check(State& state, int) { return state; }
};

using Arg = dlua::StackIndexResult;
template <std::size_t Size>
using Args = StackIndicesResult<Size>;
using VarArgs = StackIndexRangeResult;

namespace detail {

struct ConvertIndexBase {
    static constexpr bool isExact(lua_State*, int) { return true; }

    static constexpr bool isValid(lua_State*, int) { return true; }
};

struct ConvertIndex : ConvertIndexBase {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = false;

    template <typename TIndex>
    static void push(lua_State* state, TIndex index)
    {
        assert(index.state().state() == state);
        lua_pushvalue(state, index.index());
    }
};

struct ConvertStackIndex : ConvertIndex {
    static std::optional<dang::lua::StackIndex> at(State& state, int pos)
    {
        state.maxFuncArg(pos);
        return state.stackIndex(pos);
    }

    static dang::lua::StackIndex check(State& state, int arg)
    {
        state.maxFuncArg(arg);
        return state.stackIndex(arg);
    }
};

struct ConvertStackIndexResult : ConvertIndex {
    static std::optional<dang::lua::StackIndexResult> at(State& state, int pos)
    {
        state.maxFuncArg(pos);
        return state.stackIndex(pos).asResult();
    }

    static dang::lua::StackIndexResult check(State& state, int arg)
    {
        state.maxFuncArg(arg);
        return state.stackIndex(arg).asResult();
    }
};

template <int Count>
struct ConvertIndices : ConvertIndexBase {
    static constexpr std::optional<int> PushCount = Count;
    static constexpr bool AllowNesting = false;

    template <typename TIndices>
    static void push(lua_State* state, TIndices indices)
    {
        assert(indices.state().state() == state);
        pushHelper(state, indices, std::make_index_sequence<Count>{});
    }

private:
    template <typename TIndices, std::size_t... Indices>
    static void pushHelper(lua_State* state, TIndices indices, std::index_sequence<Indices...>)
    {
        (lua_pushvalue(state, indices[Indices].index()), ...);
    }
};

template <int Count>
struct ConvertStackIndices : ConvertIndices<Count> {
    static std::optional<dang::lua::StackIndices<Count>> at(State& state, int pos)
    {
        state.maxFuncArg(pos + Count - 1);
        return state.stackIndices<Count>(pos);
    }

    static dang::lua::StackIndices<Count> check(State& state, int arg)
    {
        state.maxFuncArg(arg + Count - 1);
        return state.stackIndices<Count>(arg);
    }
};

template <int Count>
struct ConvertStackIndicesResult : ConvertIndices<Count> {
    static std::optional<dang::lua::StackIndicesResult<Count>> at(State& state, int pos)
    {
        state.maxFuncArg(pos + Count - 1);
        return state.stackIndices<Count>(pos).asResults();
    }

    static dang::lua::StackIndicesResult<Count> check(State& state, int arg)
    {
        state.maxFuncArg(arg + Count - 1);
        return state.stackIndices<Count>(arg).asResults();
    }
};

struct ConvertIndexRange : ConvertIndexBase {
    static constexpr std::optional<int> PushCount = std::nullopt;
    static constexpr bool AllowNesting = false;

    template <typename TIndexRange>
    static void push(lua_State* state, TIndexRange index_range)
    {
        assert(index_range.state().state() == state);
        for (int i = 0; i < index_range.size(); i++)
            lua_pushvalue(state, index_range[i].index());
    }

    template <typename TIndexRange>
    static int getPushCount(TIndexRange index_range)
    {
        return index_range.size();
    }
};

struct ConvertStackIndexRange : ConvertIndexRange {
    static std::optional<dang::lua::StackIndexRange> at(State& state, int pos)
    {
        return state.stackIndexRange(pos, state.size() - pos + 1);
    }

    static dang::lua::StackIndexRange check(State& state, int arg)
    {
        return state.stackIndexRange(arg, state.size() - arg + 1);
    }
};

struct ConvertStackIndexRangeResult : ConvertIndexRange {
    static std::optional<dang::lua::StackIndexRangeResult> at(State& state, int pos)
    {
        return state.stackIndexRange(pos, state.size() - pos + 1).asResults();
    }

    static dang::lua::StackIndexRangeResult check(State& state, int arg)
    {
        return state.stackIndexRange(arg, state.size() - arg + 1).asResults();
    }
};

} // namespace detail

template <typename TState, typename TIndex>
struct Convert<detail::IndexImpl<TState, TIndex>> : detail::ConvertIndex {};
template <typename TState>
struct Convert<detail::StackIndex<TState, detail::StackIndexType::Reference>> : detail::ConvertStackIndex {};
template <typename TState>
struct Convert<detail::StackIndex<TState, detail::StackIndexType::Result>> : detail::ConvertStackIndexResult {};
template <typename TState>
struct Convert<detail::RegistryIndex<TState>> : detail::ConvertIndex {};
template <typename TState>
struct Convert<detail::UpvalueIndex<TState>> : detail::ConvertIndex {};

template <typename TState, typename TIndex, int Count>
struct Convert<detail::MultiIndexImpl<TState, TIndex, Count>> : detail::ConvertIndices<Count> {};

template <typename TState, int Count>
struct Convert<detail::StackIndices<TState, Count, detail::StackIndexType::Reference>>
    : detail::ConvertStackIndices<Count> {};
template <typename TState, int Count>
struct Convert<detail::StackIndices<TState, Count, detail::StackIndexType::Result>>
    : detail::ConvertStackIndicesResult<Count> {};
template <typename TState, int Count>
struct Convert<detail::UpvalueIndices<TState, Count>> : detail::ConvertIndices<Count> {};

template <typename TState>
struct Convert<detail::StackIndexRange<TState, detail::StackIndexType::Reference>> : detail::ConvertStackIndexRange {};
template <typename TState>
struct Convert<detail::StackIndexRange<TState, detail::StackIndexType::Result>>
    : detail::ConvertStackIndexRangeResult {};
template <typename TState>
struct Convert<detail::UpvalueIndexRange<TState>> : detail::ConvertIndexRange {};

// --- Utility ---

/// <summary>Function object that checks Lua arguments for a certain type.</summary>
template <typename T>
struct ArgCheck {
    template <typename TArg>
    auto operator()(TArg&& arg) const
    {
        return std::forward<TArg>(arg).template check<T>();
    }
};

/// <summary>Function object that converts Lua values into an optional of the given type.</summary>
template <typename T>
struct ValueTo {
    template <typename TIndex>
    auto operator()(TIndex&& index) const
    {
        return std::forward<TIndex>(index).template to<T>();
    }
};

namespace detail {

/// <summary>Requires either of the two arguments to be an index and returns the associated state.</summary>
template <typename TLeft, typename TRight>
inline auto& stateOf(TLeft& lhs, TRight& rhs)
{
    if constexpr (IsIndex<std::decay_t<TLeft>>::value)
        return lhs.state();
    else
        return rhs.state();
}

/// <summary>Whether any of the type parameters is an index.</summary>
template <typename... TArgs>
using AnyIndex = std::disjunction<IsIndex<std::decay_t<TArgs>>...>;

/// <summary>Enable-if wrapper to check for any index and results in StackIndexResult.</summary>
template <typename... TArgs>
using EnableIfAnyIndex = std::enable_if_t<AnyIndex<TArgs...>::value, dlua::StackIndexResult>;

} // namespace detail

template <typename TLeft, typename TRight>
inline auto operator+(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Add>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator-(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Sub>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator*(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Mul>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator%(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Mod>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

// Would be ambiguous with binary xor
/*
template <typename TLeft, typename TRight>
inline auto operator^(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Pow>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator/(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::Div>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

// Integer division works different in C++
/*
template <typename TLeft, typename TRight>
inline auto operator//(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::IDiv>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator&(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::BinaryAnd>(std::forward<TLeft>(lhs),
                                                                        std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator|(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::BinaryOr>(std::forward<TLeft>(lhs),
                                                                       std::forward<TRight>(rhs));
}

// Would be ambiguous with pow
/*
template <typename TLeft, typename TRight>
inline auto operator^(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::BinaryXOr>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator<<(TLeft&& lhs, TRight&& rhs)
    -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value && !std::is_same_v<std::decay_t<TLeft>, std::ostream>,
                        StackIndexResult>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::LeftShift>(std::forward<TLeft>(lhs),
                                                                        std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator>>(TLeft&& lhs, TRight&& rhs) -> detail::EnableIfAnyIndex<TLeft, TRight>
{
    return detail::stateOf(lhs, rhs).template arith<ArithOp::RightShift>(std::forward<TLeft>(lhs),
                                                                         std::forward<TRight>(rhs));
}

template <typename T>
inline auto operator-(T&& operand) -> detail::EnableIfAnyIndex<T>
{
    return operand.stack().template arith<ArithOp::UnaryMinus>(std::forward<T>(operand));
}

template <typename T>
inline auto operator~(T&& operand) -> detail::EnableIfAnyIndex<T>
{
    return operand.stack().template arith<ArithOp::BinaryNot>(std::forward<T>(operand));
}

template <typename TLeft, typename TRight>
inline auto operator==(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return detail::stateOf(left, right)
        .compare(CompareOp::Equal, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator!=(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return !detail::stateOf(left, right)
                .compare(CompareOp::Equal, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator<(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return detail::stateOf(left, right)
        .compare(CompareOp::LessThan, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator<=(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return detail::stateOf(left, right)
        .compare(CompareOp::LessEqual, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator>(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return detail::stateOf(left, right)
        .compare(CompareOp::LessThan, std::forward<TRight>(right), std::forward<TLeft>(left));
}

template <typename TLeft, typename TRight>
inline auto operator>=(TLeft&& left, TRight&& right) -> std::enable_if_t<detail::AnyIndex<TLeft, TRight>::value, bool>
{
    return detail::stateOf(left, right)
        .compare(CompareOp::LessEqual, std::forward<TRight>(right), std::forward<TLeft>(left));
}

/*
#ifdef _DEBUG
namespace detail::force_include
{

// forcefully include these functions for debugger visualization
extern const auto State_formatDebug = &dlua::State::formatDebug;

}
#endif
*/

} // namespace dang::lua
