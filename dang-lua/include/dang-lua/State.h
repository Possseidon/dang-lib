#pragma once

// TODO: #include <std>

#include "dang-lua/Allocator.h"
#include "dang-lua/Convert.h"
#include "dang-lua/NoreturnError.h"
#include "dang-lua/Reference.h"
#include "dang-lua/Types.h"
#include "dang-lua/global.h"
#include "dang-utils/utils.h"

#include "tl/expected.hpp"

// TODO: More explicit about which index parameters must be positive

namespace dang::lua {

namespace detail {

class StateBase;

} // namespace detail

class StateRef;
class Thread;
class State;

/// @brief The amount of stack slots auxiliary library functions can use before they call lua_checkstack themselves.
/// @remark There is no constant for this defined and this value can only be found in the documentation.
constexpr int auxiliary_required_pushable = 4;

// --- Type Traits

namespace detail {

template <typename>
struct is_index_helper : std::false_type {};

template <typename>
struct is_indices_helper : std::false_type {};

template <typename>
struct is_index_range_helper : std::false_type {};

template <typename>
struct is_stack_index_helper : std::false_type {};

template <typename>
struct is_stack_index_result_helper : std::false_type {};

template <typename>
struct is_stack_indices_helper : std::false_type {};

template <typename>
struct is_stack_indices_result_helper : std::false_type {};

template <typename>
struct is_stack_index_range_helper : std::false_type {};

template <typename>
struct is_stack_index_range_result_helper : std::false_type {};

template <typename>
struct is_registry_index_helper : std::false_type {};

template <typename>
struct is_pseudo_index_helper : std::false_type {};

template <typename>
struct is_upvalue_index_helper : std::false_type {};

} // namespace detail

template <typename T>
struct is_index : detail::is_index_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_index_v = is_index<T>::value;

template <typename T>
struct is_indices : detail::is_indices_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_indices_v = is_indices<T>::value;

template <typename T>
struct is_index_range : detail::is_index_range_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_index_range_v = is_index_range<T>::value;

template <typename T>
struct is_stack_index : detail::is_stack_index_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_index_v = is_stack_index<T>::value;

template <typename T>
struct is_stack_index_result : detail::is_stack_index_result_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_index_result_v = is_stack_index_result<T>::value;

template <typename T>
struct is_stack_indices : detail::is_stack_indices_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_indices_v = is_stack_indices<T>::value;

template <typename T>
struct is_stack_indices_result : detail::is_stack_indices_result_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_indices_result_v = is_stack_indices_result<T>::value;

template <typename T>
struct is_stack_index_range : detail::is_stack_index_range_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_index_range_v = is_stack_index_range<T>::value;

template <typename T>
struct is_stack_index_range_result : detail::is_stack_index_range_result_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_stack_index_range_result_v = is_stack_index_range_result<T>::value;

template <typename T>
struct is_registry_index : detail::is_registry_index_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_registry_index_v = is_registry_index<T>::value;

template <typename T>
struct is_pseudo_index : detail::is_pseudo_index_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_pseudo_index_v = is_pseudo_index<T>::value;

template <typename T>
struct is_upvalue_index : detail::is_upvalue_index_helper<std::remove_cv_t<T>> {};

template <typename T>
inline constexpr auto is_upvalue_index_v = is_upvalue_index<T>::value;

template <typename T>
struct is_any_index : std::disjunction<is_index<T>, is_indices<T>, is_index_range<T>> {};

template <typename T>
inline constexpr auto is_any_index_v = is_any_index<T>::value;

template <typename T>
struct is_any_stack_index : std::disjunction<is_stack_index<T>, is_stack_indices<T>, is_stack_index_range<T>> {};

template <typename T>
inline constexpr auto is_any_stack_index_v = is_any_stack_index<T>::value;

template <typename T>
struct is_any_stack_index_result
    : std::disjunction<is_stack_index_result<T>, is_stack_indices_result<T>, is_stack_index_range_result<T>> {};

template <typename T>
inline constexpr auto is_any_stack_index_result_v = is_any_stack_index_result<T>::value;

template <typename T>
struct is_any_moved_stack_index_result
    : std::conjunction<is_any_stack_index_result<std::remove_reference_t<T>>, std::is_rvalue_reference<T>> {};

template <typename T>
inline constexpr auto is_any_moved_stack_index_result_v = is_any_moved_stack_index_result<T>::value;

template <typename T>
struct is_fixed_size_stack_index : std::disjunction<is_stack_index<T>, is_stack_indices<T>> {};

template <typename T>
inline constexpr auto is_fixed_size_stack_index_v = is_fixed_size_stack_index<T>::value;

namespace detail {

// --- Signature Information ---

/// @brief Meant to be used as a base class to provide information about the signature of functions.
template <typename TRet, typename... TArgs>
struct SignatureInfoBase {
private:
    /// @brief Converts the given type to the type, which is actually returned by Convert::check.
    template <typename TArg>
    using FixedArgType = decltype(Convert<std::remove_reference_t<TArg>>::check(std::declval<StateRef&>(), 1));

public:
    using Signature = TRet(TArgs...);
    using Return = TRet;
    using Arguments = std::tuple<FixedArgType<TArgs>...>;

    static constexpr bool any_fixed_size_stack_args = (is_fixed_size_stack_index_v<TArgs> || ...);
    static constexpr bool any_state_args = ((std::is_same_v<TArgs, StateRef&> || is_any_index_v<TArgs>) || ...);

protected:
    /// @brief Calculates the index of the next argument from the given index list of all previous arguments.
    /// @remark An empty index sequence {} will give "1", an index sequence {0} will give "1" plus the size of the first
    /// argument, etc...
    template <std::size_t... v_indices>
    static constexpr int indexOffset(std::index_sequence<v_indices...>)
    {
        using ArgsTuple = std::tuple<std::remove_reference_t<TArgs>...>;
        static_assert((Convert<std::tuple_element_t<v_indices, ArgsTuple>>::check_count && ...),
                      "Only the last function argument can be variadic.");
        return (1 + ... + *Convert<std::tuple_element_t<v_indices, ArgsTuple>>::check_count);
    }
};

/// @brief Provides information about a function signature and the means to convert arguments of a Lua C function into
/// actual C++ values.
/// @remark Argument conversion relies on the Convert template.
template <typename>
struct SignatureInfo;

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (*)(TArgs...)> : SignatureInfoBase<TRet, TArgs...> {
    /// @brief Uses the Convert template to check all the arguments on the stack and returns a tuple representing these
    /// arguments.
    static auto convertArguments(StateRef& state)
    {
        return convertArgumentsHelper(state, std::index_sequence_for<TArgs...>{});
    }

    /// @brief Uses the Convert template to check all the arguments on the stack and returns a tuple representing these
    /// arguments.
    static auto convertArgumentsRaw(lua_State* state)
    {
        return convertArgumentsRawHelper(state, std::index_sequence_for<TArgs...>{});
    }

private:
    using Base = SignatureInfoBase<TRet, TArgs...>;

    /// @brief Helper function to convert all arguments, as "indexOffset" relies on an index sequence itself.
    template <std::size_t... v_indices>
    static typename Base::Arguments convertArgumentsHelper([[maybe_unused]] StateRef& state,
                                                           std::index_sequence<v_indices...>)
    {
        return {Convert<std::remove_reference_t<TArgs>>::check(
            state, Base::indexOffset(std::make_index_sequence<v_indices>{}))...};
    }

    /// @brief Helper function to convert all arguments, as "indexOffset" relies on an index sequence itself.
    template <std::size_t... v_indices>
    static typename Base::Arguments convertArgumentsRawHelper([[maybe_unused]] lua_State* state,
                                                              std::index_sequence<v_indices...>)
    {
        return {Convert<std::remove_reference_t<TArgs>>::check(
            state, Base::indexOffset(std::make_index_sequence<v_indices>{}))...};
    }
};

// TODO: Fix this SignatureInfo mess with SFINAE or something...

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
struct SignatureInfo<TRet (&&)(TArgs...)> : SignatureInfo<TRet (*)(TArgs...)> {};

template <typename TRet, typename... TArgs>
struct SignatureInfo<TRet (&&)(TArgs...) noexcept> : SignatureInfo<TRet (*)(TArgs...)> {};

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

template <typename TClass, typename TRet>
struct SignatureInfo<TRet TClass::*> : SignatureInfo<TRet (TClass::*)()> {};

// --- Index Iterator ---

/// @brief Provides iteration functionality for the Lua stack and upvalues.
/// @remark Basically just wraps "Index" (which is cheap to copy anyways) with iterator functionality.
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

/// @brief Tag to signify that the given indices should be used as is.
struct DirectInit {};

/// @brief Serves as a base for all index wrappers.
template <typename TState>
class IndexImplBase {
public:
    /// @brief Returns the associated Lua state.
    TState& state() const { return *state_; }

protected:
    /// @brief Default constructible for the sake of simplifying the iterator wrapper.
    IndexImplBase() = default;

    /// @brief Associates the given Lua state with the index.
    explicit IndexImplBase(TState& state)
        : state_(&state)
    {}

    /// @brief Applies an offset to the given index.
    /// @remark Note: Stack indices grow positive, while upvalues grow negative.
    static int applyOffset(int index, int offset)
    {
        assert(index < LUA_REGISTRYINDEX || index >= 1);
        return index >= 0 ? index + offset : index - offset;
    }

    /// @brief The difference between two indices.
    /// @remark See applyOffset for why this is necessary.
    static int indexDiff(int first, int last)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
        return first >= 0 ? last - first : first - last;
    }

private:
    TState* state_ = nullptr;
};

/// @brief The general implementation for the various types of Lua stack index positions.
template <typename TState, typename TIndex>
class IndexImpl : public IndexImplBase<TState> {
public:
    /// @brief Default constructible for sake of simplifying iterator implementation.
    /// @remark Index defaults to 0, which is not a valid index. (Indices start at 1)
    IndexImpl() = default;

    /// @brief Directly initializes to the given index.
    IndexImpl(DirectInit, TState& state, int index)
        : IndexImplBase<TState>(state)
        , index_(index)
    {
        assert(index <= LUA_REGISTRYINDEX || index >= 1);
    }

    // --- Index ---

    /// @brief The stored index.
    int index() const { return index_; }

    /// @brief Same as index().
    /// @remark Merely for compatibility with StackIndices and StackIndexRange.
    int first() const { return index(); }

    /// @brief Same as index().
    /// @remark Merely for compatibility with StackIndices and StackIndexRange.
    int last() const { return index(); }

    /// @brief Returns 1.
    /// @remark Merely for compatibility with StackIndices and StackIndexRange.
    static constexpr int size() { return 1; }

    /// @brief Returns false.
    /// @remark Merely for compatibility with StackIndices and StackIndexRange.
    static constexpr bool empty() { return false; }

    /// @brief Returns another index, which is offset by a given amount.
    /// @remark See: applyOffset
    auto offset(int offset) const { return TIndex(DirectInit{}, this->state(), this->applyOffset(index_, offset)); }

    /// @brief Returns the difference to another index.
    /// @remark See: indexDiff
    int diff(TIndex other) const { return this->indexDiff(index_, other.index_); }

    // --- Index Properties ---

    /// @brief Whether the index is a position on the Lua stack.
    bool isStack() const { return this->state().isStack(index()); }

    /// @brief Whether the index is a pseudo index (upvalue or registry).
    bool isPseudo() const { return this->state().isPseudo(index()); }

    /// @brief Whether the index is the index for the registry table.
    bool isRegistry() const { return this->state().isRegistry(index()); }

    /// @brief Whether the index is an upvalue.
    bool isUpvalue() const { return this->state().isUpvalue(index()); }

    /// @brief Whether the index (plus an optional offset) is at the bottom of the stack.
    bool isBottom(int offset = 0) const { return this->state().isIndexBottom(index(), offset); }

    /// @brief Whether the index (plus an optional offset) is at the top of the stack.
    bool isTop(int offset = 0) const { return this->state().isIndexTop(index(), offset); }

    /// @brief Returns the (zero-based) offset from the bottom of the stack.
    int offsetFromBottom() const { return this->state().indexOffsetFromBottom(index()); }

    /// @brief Returns the (zero-based) offset from the top of the stack.
    int offsetFromTop() const { return this->state().indexOffsetFromTop(index()); }

    // --- Stack Queries ---

    /// @brief Returns the type of the element.
    Type type() const { return this->state().type(index()); }

    /// @brief Returns the name of the element's type.
    std::string_view typeName() const { return this->state().typeName(index()); }

    /// @brief Whether the index is not valid.
    bool isNone() const { return this->state().isNone(index()); }

    /// @brief Whether the element is nil.
    bool isNil() const { return this->state().isNil(index()); }

    /// @brief Whether the index is not valid or the element is nil.
    bool isNoneOrNil() const { return this->state().isNoneOrNil(index()); }

    /// @brief Whether the element is a boolean.
    bool isBoolean() const { return this->state().isBoolean(index()); }

    /// @brief Whether the element is light userdata.
    bool isLightUserdata() const { return this->state().isLightUserdata(index()); }

    /// @brief Whether the element is a number or a string convertible to a number.
    bool isNumber() const { return this->state().isNumber(index()); }

    /// @brief Whether the element is an integer or a string convertible to an integer.
    bool isInteger() const { return this->state().isInteger(index()); }

    /// @brief Whether the element is a string or a number (which is always convertible to a string).
    bool isString() const { return this->state().isString(index()); }

    /// @brief Whether the element is a table.
    bool isTable() const { return this->state().isTable(index()); }

    /// @brief Whether the element is a function (either C or Lua).
    bool isFunction() const { return this->state().isFunction(index()); }

    /// @brief Whether the element is a C function.
    bool isCFunction() const { return this->state().isCFunction(index()); }

    /// @brief Whether the element is full or light userdata.
    bool isUserdata() const { return this->state().isUserdata(index()); }

    /// @brief Whether the element is a thread.
    bool isThread() const { return this->state().isThread(index()); }

    // --- Conversion ---

    /// @brief Uses the Convert template to convert the element.
    /// @remark Returns an optional, which is std::nullopt if the conversion failed.
    template <typename T>
    auto to()
    {
        return this->state().template to<T>(index());
    }

    /// @brief Treats the element as a function argument and uses the Convert template to convert it.
    /// @remark Raises a Lua (argument) error if the conversion failed.
    template <typename T>
    decltype(auto) check()
    {
        return this->state().template check<T>(index());
    }

    /// @brief Follows the same semantics as Lua, returning "true" for anything but "false" and "nil".
    explicit operator bool() const { return this->state().template check<bool>(index()); }

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

    /// @brief Calls the element with an arbitrary number of arguments, returning a fixed number of results.
    /// @remark Also supports LUA_MULTRET.
    template <int v_results = 0, typename... TArgs>
    auto call(TArgs&&... args) const
    {
        return this->state().template call<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning a specified number of results.
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) const
    {
        return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET
    /// internally.
    template <typename... TArgs>
    auto operator()(TArgs&&... args) const
    {
        return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <int v_results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) const
    {
        return this->state().template pcall<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) const
    {
        return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) const
    {
        return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Returns a metafield including its type, if it exists.
    auto getMetafieldWithType(const char* field) const { return this->state().getMetafieldWithType(index(), field); }

    /// @brief Returns a metafield, if it exists.
    auto getMetafield(const char* field) const { return this->state().getMetafield(index(), field); }

    /// @brief If a metafield with the given name exists, calls it and returns its result.
    auto callMeta(const char* field) const { return this->state().callMeta(index(), field); }

    // --- Operations ---

    /// @brief Replaces the element with another value.
    /// @remark This is what one might expect from the assignment operator, yet the assignment operator only changes the
    /// index itself, not its element.
    template <typename T>
    auto& replace(T&& value) const
    {
        this->state().replace(index(), std::forward<T>(value));
        return *this;
    }

    /// @brief Removes the value from the stack, moving everything after, to fill the gap.
    /// @remark Invalidates any greater indices!
    void remove() { this->state().remove(index()); }

    /// @brief Performs integer division with another value. (// in Lua)
    template <typename T>
    auto idiv(T&& other) const
    {
        return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// @brief Raises the element to the power of another value. (^ in Lua)
    template <typename T>
    auto pow(T&& other) const
    {
        return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// @brief Performs binary xor with another value. (~ in Lua)
    template <typename T>
    auto bxor(T&& other) const
    {
        return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    /// @brief Pushes the length of the element on the stack.
    /// @remark This can invoke the __len metamethod and therefore doesn't necessarily return an integer.
    auto pushLength() const { return this->state().pushLength(index()); }

    /// @brief Returns the length of the element.
    /// @remark This can invoke the __len metamethod and raises an error if that doesn't return an integer.
    auto length() const { return this->state().length(index()); }

    /// @brief Returns the raw length of the value, which does not invoke metamethod.
    auto rawLength() const { return this->state().rawLength(index()); }

    // --- Table Access ---

    /// @brief Queries the element like a table with the given key, returning both the type and the pushed value.
    /// @remark Can invoke the __index metamethod.
    template <typename TKey>
    auto getTableWithType(TKey&& key)
    {
        return this->state().getTableWithType(index(), std::forward<TKey>(key));
    }

    /// @brief Queries the element like a table with the given key, returning the pushed value.
    /// @remark Can invoke the __index metamethod.
    template <typename TKey>
    auto getTable(TKey&& key)
    {
        return this->state().getTable(index(), std::forward<TKey>(key));
    }

    /// @brief Convenience for table access, however it can only be used to query and not to set.
    template <typename TKey>
    auto operator[](TKey&& key)
    {
        return this->state().getTable(index(), std::forward<TKey>(key));
    }

    /// @brief Sets a key of the element like a table to the given value.
    /// @remark Can invoke the __newindex metamethod.
    template <typename TKey, typename TValue>
    void setTable(TKey&& key, TValue&& value)
    {
        this->state().setTable(index(), std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// @brief Similar to getTableWithType, but does not invoke metamethods.
    template <typename TKey>
    auto rawGetTableWithType(TKey&& key)
    {
        return this->state().rawGetTableWithType(index(), std::forward<TKey>(key));
    }

    /// @brief Similar to getTable, but does not invoke metamethods.
    template <typename TKey>
    auto rawGetTable(TKey&& key)
    {
        return this->state().rawGetTable(index(), std::forward<TKey>(key));
    }

    /// @brief Similar to setTable, but does not invoke metamethods.
    template <typename TKey, typename TValue>
    void rawSetTable(TKey&& key, TValue&& value)
    {
        this->state().rawSetTable(index(), std::forward<TKey>(key), std::forward<TValue>(value));
    }

    /// @brief Returns the metatable of the element or std::nullopt if it doesn't have one.
    auto getMetatable() { return this->state().getMetatable(index()); }

    /// @brief Sets the metatable of the element to the specified table; or nil to remove it.
    template <typename TMetatable>
    auto setMetatable(TMetatable&& metatable)
    {
        this->state().setMetatable(index(), std::forward<TMetatable>(metatable));
    }

    /// @brief Sets all functions in the given container of luaL_Reg on the table.
    template <typename TFuncs>
    void setFuncs(const TFuncs& funcs)
    {
        this->state().setFuncs(index(), funcs);
    }

    // --- Iteration ---

    /// @brief Returns the next key-value-pair of the table or nothing, if the table has been exhausted.
    template <typename TKey>
    auto next(TKey&& key) const
    {
        return this->state().next(index(), std::forward<TKey>(key));
    }

    /// @copydoc dang::lua::detail::StateBase::pairs(int)
    auto pairs() const { return this->state().pairs(index()); }
    /// @copydoc dang::lua::detail::StateBase::keys(int)
    auto keys() const { return this->state().keys(index()); }
    /// @copydoc dang::lua::detail::StateBase::values(int)
    auto values() const { return this->state().values(index()); }

    /// @copydoc dang::lua::detail::StateBase::pairsRaw(int)
    auto pairsRaw() const { return this->state().pairsRaw(index()); }
    /// @copydoc dang::lua::detail::StateBase::keysRaw(int)
    auto keysRaw() const { return this->state().keysRaw(index()); }
    /// @copydoc dang::lua::detail::StateBase::valuesRaw(int)
    auto valuesRaw() const { return this->state().valuesRaw(index()); }

    /// @copydoc dang::lua::detail::StateBase::ipairs(int)
    auto ipairs() const { return this->state().ipairs(index()); }
    /// @copydoc dang::lua::detail::StateBase::ikeys(int)
    auto ikeys() const { return this->state().ikeys(index()); }
    /// @copydoc dang::lua::detail::StateBase::ivalues(int)
    auto ivalues() const { return this->state().ivalues(index()); }

    /// @copydoc dang::lua::detail::StateBase::ipairsLen(int)
    auto ipairsLen() const { return this->state().ipairsLen(index()); }
    /// @copydoc dang::lua::detail::StateBase::ikeysLen(int)
    auto ikeysLen() const { return this->state().ikeysLen(index()); }
    /// @copydoc dang::lua::detail::StateBase::ivaluesLen(int)
    auto ivaluesLen() const { return this->state().ivaluesLen(index()); }

    /// @copydoc dang::lua::detail::StateBase::ipairsRaw(int)
    auto ipairsRaw() const { return this->state().ipairsRaw(index()); }
    /// @copydoc dang::lua::detail::StateBase::ikeysRaw(int)
    auto ikeysRaw() const { return this->state().ikeysRaw(index()); }
    /// @copydoc dang::lua::detail::StateBase::ivaluesRaw(int)
    auto ivaluesRaw() const { return this->state().ivaluesRaw(index()); }

    /// @copydoc dang::lua::detail::StateBase::iterate(int,int)
    template <int v_value_offset = 0>
    auto iterate()
    {
        return this->state().template iterate<v_value_offset>(index(), 1);
    }

    /// @copydoc dang::lua::detail::StateBase::iterateMultiple(int,int)
    template <int v_value_count, int v_value_offset = 0>
    auto iterateMultiple()
    {
        return this->state().template iterateMultiple<v_value_count, v_value_offset>(index(), 1);
    }

    /// @copydoc dang::lua::detail::StateBase::iteratePair(int,int)
    template <int v_value_offset = 0>
    auto iteratePair()
    {
        return this->state().template iteratePair<v_value_offset>(index(), 1);
    }

    /// @copydoc dang::lua::detail::StateBase::iterateVarying(int,int)
    template <int v_value_offset = 0>
    auto iterateVarying()
    {
        return this->state().template iterateVarying<v_value_offset>(index(), 1);
    }

    // --- Formatting ---

    /// @brief Converts the element to a string in a reasonable format using luaL_tolstring.
    std::string format() const { return this->state().format(index()); }

    /// @brief Prints the element to the stream using the format function.
    friend std::ostream& operator<<(std::ostream& stream, const IndexImpl& index) { return stream << index.format(); }

    // --- Reference ---

    /// @brief Stores the element as a reference in the registry table and returns a wrapper.
    auto ref() { return this->state().ref(*this); }

    // --- To Close ---

    /// @brief Marks the value as to-be-closed.
    /// @see https://www.lua.org/manual/5.4/manual.html#lua_toclose
    void toClose() { this->state().toClose(index_); }

    // --- Debug ---

    /// @brief Returns debug information about the function.
    template <typename... TTypes>
    auto getFunctionInfo()
    {
        return this->state().template getFunctionInfo<TTypes...>(*this);
    }

    /// @brief Returns debug and line information about the function.
    template <typename... TTypes>
    auto getFunctionInfoWithLines()
    {
        return this->state().template getFunctionInfoWithLines<TTypes...>(*this);
    }

    /// @brief Returns all debug information about the function.
    auto getFullFunctionInfo() { return this->state().getFullFunctionInfo(*this); }

    /// @brief Returns all debug and line information about the function.
    auto getFullFunctionInfoWithLines() { return this->state().getFullFunctionInfoWithLines(*this); }

private:
    int index_ = 0;
};

/// @brief The basic implementation for a compile-time fixed size range of indices.
template <typename TState, int v_count>
class IndicesImpl : public IndexImplBase<TState> {
public:
    static_assert(v_count >= 0, "Indices must have a non-negative count.");

    /// @brief Default constructible to stay consistent with Index.
    IndicesImpl() = default;

    /// @brief Directly initializes the starting index with the given value.
    IndicesImpl(DirectInit, TState& state, int first)
        : IndexImplBase<TState>(state)
        , first_(first)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
    }

    /// @brief Returns the first index of the range.
    int first() const { return first_; }

    /// @brief Returns the size of the range.
    static constexpr int size() { return v_count; }

    /// @brief Whether the size is zero.
    static constexpr bool empty() { return size() == 0; }

private:
    int first_ = 0;
};

/// @brief The basic implementation for a range of indices.
template <typename TState>
class IndexRangeImpl : public IndexImplBase<TState> {
public:
    /// @brief Default constructible to stay consistent with Index.
    IndexRangeImpl() = default;

    /// @brief Directly initializes the starting index with the given values.
    IndexRangeImpl(DirectInit, TState& state, int first, int count)
        : IndexImplBase<TState>(state)
        , first_(first)
        , count_(count)
    {
        assert(first < LUA_REGISTRYINDEX || first >= 1);
        assert(count >= 0);
    }

    /// @brief Returns the first index of the range.
    int first() const { return first_; }

    /// @brief Returns the size of the range.
    int size() const { return count_; }

    /// @brief Whether the size is zero.
    bool empty() const { return size() == 0; }

private:
    int first_ = 0;
    int count_ = 0;
};

/// @brief The general implementation for a range of indices.
/// @remark If v_size is -1, uses IndexRangeImpl as base class, otherwise uses IndicesImpl.
template <typename TState, typename TIndex, int v_size>
class MultiIndexImpl : public std::conditional_t<v_size != -1, IndicesImpl<TState, v_size>, IndexRangeImpl<TState>> {
public:
    // Using base constructor
    using std::conditional_t<v_size != -1, IndicesImpl<TState, v_size>, IndexRangeImpl<TState>>::conditional_t;

    /// @brief Returns the last index of the range.
    int last() const { return this->applyOffset(this->first(), this->size() - 1); }

    /// @brief Returns a single index at the given zero-based (!) offset.
    auto operator[](int offset) const
    {
        return TIndex(DirectInit{}, this->state(), this->applyOffset(this->first(), offset));
    }

    /// @brief Returns an iterator to the first element.
    auto begin() const { return IndexIterator<TIndex>((*this)[0]); }

    /// @brief Returns an iterator to one after the last element.
    auto end() const { return IndexIterator<TIndex>((*this)[this->size()]); }

    // --- Index Properties ---

    /// @brief Whether the range starts at the bottom of the stack.
    bool isBottom(int offset = 0) const { return this->state().isIndexBottom(this->first(), offset); }

    /// @brief Whether the range ends at the top of the stack.
    bool isTop(int offset = 0) const { return this->state().isIndexTop(this->last(), offset); }

    // --- Formatting ---

    /// @brief Prints all indices to the stream, separated by comma (and space).
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

/// @brief Results can be consumed by other operations if they are also rvalues.
enum class StackIndexType { Reference, Result };

/// @brief Wraps any index on the Lua stack.
template <typename TState, StackIndexType v_type>
class StackIndex : public IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>> {
public:
    using IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>>::IndexImpl;

    /// @brief Takes any stack index.
    StackIndex(TState& state, int index)
        : IndexImpl<TState, StackIndex<TState, StackIndexType::Reference>>(
              DirectInit{}, state, state.absStackIndex(index))
    {
        assert(index > LUA_REGISTRYINDEX);
    }

    /// @brief Returns a result index, for which rvalues can be consumed by some functions.
    auto asResult() { return StackIndex<TState, StackIndexType::Result>(DirectInit{}, this->state(), this->index()); }

    /// @brief Assumes, that the value lies at the top of the stack and pops it.
    void pop()
    {
        assert(this->isTop());
        this->state().pop();
    }

    /// @brief Pops the value if it lies at the top of the stack.
    void popIfTop()
    {
        if (this->isTop())
            this->state().pop();
    }

    // --- Calling ---

    /// @brief Calls the element with an arbitrary number of arguments, returning a fixed number of results.
    /// @remark Also supports LUA_MULTRET.
    template <int v_results = 0, typename... TArgs>
    auto call(TArgs&&... args) const&
    {
        return this->state().template call<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning a fixed number of results.
    /// @remark Also supports LUA_MULTRET.
    template <int v_results = 0, typename... TArgs>
    auto call(TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().template call<v_results>(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().template call<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning a specified number of results.
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) const&
    {
        return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning a specified number of results.
    template <typename... TArgs>
    auto callReturning(int results, TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().callReturning(results, std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().callReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET
    /// internally.
    template <typename... TArgs>
    auto operator()(TArgs&&... args) const&
    {
        return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element with an arbitrary number of arguments, returning all results using LUA_MULTRET
    /// internally.
    template <typename... TArgs>
    auto operator()(TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().callMultRet(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().callMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <int v_results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) const&
    {
        return this->state().template pcall<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <int v_results = 0, typename... TArgs>
    auto pcall(TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().template pcall<v_results>(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().template pcall<v_results>(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) const&
    {
        return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallReturning(int results, TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().pcallReturning(results, std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().pcallReturning(results, *this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) const&
    {
        return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    /// @brief Calls the element in protected mode, returning a tuple of Status and the actual result (which can also be
    /// the error itself).
    template <typename... TArgs>
    auto pcallMultRet(TArgs&&... args) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().pcallMultRet(std::move(*this), std::forward<TArgs>(args)...);
        else
            return this->state().pcallMultRet(*this, std::forward<TArgs>(args)...);
    }

    // --- Operations ---

    /// @brief Performs integer division with another value. (// in Lua)
    template <typename T>
    auto idiv(T&& other) const&
    {
        return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// @brief Performs integer division with another value. (// in Lua)
    template <typename T>
    auto idiv(T&& other) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().template arith<ArithOp::IDiv>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::IDiv>(*this, std::forward<T>(other));
    }

    /// @brief Raises the element to the power of another value. (^ in Lua)
    template <typename T>
    auto pow(T&& other) const&
    {
        return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// @brief Raises the element to the power of another value. (^ in Lua)
    template <typename T>
    auto pow(T&& other) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().template arith<ArithOp::Pow>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::Pow>(*this, std::forward<T>(other));
    }

    /// @brief Performs binary xor with another value. (~ in Lua)
    template <typename T>
    auto bxor(T&& other) const&
    {
        return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    /// @brief Performs binary xor with another value. (~ in Lua)
    template <typename T>
    auto bxor(T&& other) &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().template arith<ArithOp::BinaryXOr>(std::move(*this), std::forward<T>(other));
        else
            return this->state().template arith<ArithOp::BinaryXOr>(*this, std::forward<T>(other));
    }

    // --- Reference ---

    /// @brief Stores the element as a reference in the registry table and returns a wrapper.
    auto ref() & { return this->state().ref(*this); }

    /// @brief Stores the element as a reference in the registry table and returns a wrapper.
    auto ref() &&
    {
        if constexpr (v_type == StackIndexType::Result)
            return this->state().ref(std::move(*this));
        else
            return this->state().ref(*this);
    }

    // --- Debug ---

    /// @brief Returns debug information about the function.
    template <typename... TTypes>
    auto getFunctionInfo() &
    {
        return this->state().template getFunctionInfo<TTypes...>(*this);
    }

    /// @brief Returns debug information about the function.
    template <typename... TTypes>
    auto getFunctionInfo() &&
    {
        return this->state().template getFunctionInfo<TTypes...>(std::move(*this));
    }

    /// @brief Returns debug and line information about the function.
    template <typename... TTypes>
    auto getFunctionInfoWithLines() &
    {
        return this->state().template getFunctionInfoWithLines<TTypes...>(*this);
    }

    /// @brief Returns debug and line information about the function.
    template <typename... TTypes>
    auto getFunctionInfoWithLines() &&
    {
        return this->state().template getFunctionInfoWithLines<TTypes...>(std::move(*this));
    }

    /// @brief Returns all debug information about the function.
    auto getFullFunctionInfo() & { return this->state().getFullFunctionInfo(*this); }

    /// @brief Returns all debug information about the function.
    auto getFullFunctionInfo() && { return this->state().getFullFunctionInfo(std::move(*this)); }

    /// @brief Returns all debug and line information about the function.
    auto getFullFunctionInfoWithLines() & { return this->state().getFullFunctionInfoWithLines(*this); }

    /// @brief Returns all debug and line information about the function.
    auto getFullFunctionInfoWithLines() && { return this->state().getFullFunctionInfoWithLines(std::move(*this)); }
};

/// @brief Wraps the registry pseudo index.
template <typename TState>
class RegistryIndex : public IndexImpl<TState, RegistryIndex<TState>> {
public:
    using IndexImpl<TState, RegistryIndex<TState>>::IndexImpl;

    /// @brief Implicitly assumes the registry index.
    RegistryIndex(TState& state)
        : IndexImpl<TState, RegistryIndex<TState>>(DirectInit{}, state, LUA_REGISTRYINDEX)
    {}

    /// @brief Returns the registry index.
    static constexpr int index() { return LUA_REGISTRYINDEX; }

    /// @brief Returns the registry index.
    static constexpr int first() { return LUA_REGISTRYINDEX; }

    /// @brief Returns the registry index.
    static constexpr int last() { return LUA_REGISTRYINDEX; }
};

/// @brief Wraps an upvalue index.
template <typename TState>
class UpvalueIndex : public IndexImpl<TState, UpvalueIndex<TState>> {
public:
    using IndexImpl<TState, UpvalueIndex<TState>>::IndexImpl;

    /// @brief Converts the 1-based index into an upvalue-index.
    UpvalueIndex(TState& state, int index)
        : IndexImpl<TState, UpvalueIndex<TState>>(DirectInit{}, state, lua_upvalueindex(index))
    {
        assert(index >= 1);
    }
};

// --- Indices Wrappers ---

/// @brief Wraps a compile-time fixed size range of stack indices.
template <typename TState, int v_count, StackIndexType v_type>
class StackIndices : public MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, v_count> {
public:
    using MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, v_count>::MultiIndexImpl;

    /// @brief Takes any stack index to start the range at.
    StackIndices(TState& state, int first)
        : MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, v_count>(
              DirectInit{}, state, state.absStackIndex(first))
    {
        assert(first > LUA_REGISTRYINDEX);
    }

    /// @brief Returns result indices, for which rvalues can be consumed by some functions.
    auto asResult()
    {
        return StackIndices<TState, v_count, StackIndexType::Result>(DirectInit{}, this->state(), this->first());
    }

    /// @brief For structured binding support.
    template <std::size_t v_index>
    auto get() const
    {
        return (*this)[v_index].asResult();
    }
};

/// @brief Wraps a compile-time fixed size range of upvalues.
template <typename TState, int v_count>
class UpvalueIndices : public MultiIndexImpl<TState, UpvalueIndex<TState>, v_count> {
public:
    using MultiIndexImpl<TState, UpvalueIndex<TState>, v_count>::MultiIndexImpl;

    /// @brief Converts the 1-based index into an upvalue index, to start the range at.
    UpvalueIndices(TState& state, int first)
        : MultiIndexImpl<TState, UpvalueIndex<TState>, v_count>(DirectInit{}, state, lua_upvalueindex(first))
    {
        assert(first >= 1);
    }

    /// @brief For structured binding support.
    template <std::size_t v_index>
    auto get() const
    {
        return (*this)[v_index];
    }
};

// --- Index Range Wrappers ---

/// @brief Wraps a range stack indices.
template <typename TState, StackIndexType v_type>
class StackIndexRange : public MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1> {
public:
    using MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1>::MultiIndexImpl;

    /// @brief Takes any stack index to start the range at.
    StackIndexRange(TState& state, int first, int count)
        : MultiIndexImpl<TState, StackIndex<TState, StackIndexType::Reference>, -1>(
              DirectInit{}, state, state.absStackIndex(first), count)
    {
        assert(first > LUA_REGISTRYINDEX);
    }

    /// @brief Allows for implicit conversion from StackIndex.
    StackIndexRange(StackIndex<TState, v_type> index)
        : StackIndexRange(index.state(), index.index(), 1)
    {}

    /// @brief Allows for implicit conversion from any StackIndices.
    template <int v_count>
    StackIndexRange(StackIndices<TState, v_count, v_type> indices)
        : StackIndexRange(indices.state(), indices.first(), v_count)
    {}

    /// @brief Returns result indices, for which rvalues can be consumed by some functions.
    auto asResult()
    {
        return StackIndexRange<TState, StackIndexType::Result>(
            DirectInit{}, this->state(), this->first(), this->size());
    }
};

/// @brief Wraps a range of upvalues.
template <typename TState>
class UpvalueIndexRange : public MultiIndexImpl<TState, UpvalueIndex<TState>, -1> {
public:
    using MultiIndexImpl<TState, UpvalueIndex<TState>, -1>::MultiIndexImpl;

    /// @brief Converts the 1-based index into an upvalue index, to start the range at.
    UpvalueIndexRange(TState& state, int first, int count)
        : MultiIndexImpl<TState, UpvalueIndex<TState>, -1>(DirectInit{}, state, lua_upvalueindex(first), count)
    {
        assert(first >= 1);
    }

    /// @brief Allows for implicit conversion from UpvalueIndex.
    UpvalueIndexRange(UpvalueIndex<TState> index)
        : UpvalueIndexRange(index.state(), index.index(), 1)
    {}

    /// @brief Allows for implicit conversion from any UpvalueIndices.
    template <int v_count>
    UpvalueIndexRange(UpvalueIndices<TState, v_count> indices)
        : UpvalueIndexRange(indices.state(), indices.first(), v_count)
    {}
};

} // namespace detail

using StackIndex = detail::StackIndex<detail::StateBase, detail::StackIndexType::Reference>;
using ConstStackIndex = detail::StackIndex<const detail::StateBase, detail::StackIndexType::Reference>;

using StackIndexResult = detail::StackIndex<detail::StateBase, detail::StackIndexType::Result>;
using ConstStackIndexResult = detail::StackIndex<const detail::StateBase, detail::StackIndexType::Result>;

using RegistryIndex = detail::RegistryIndex<detail::StateBase>;
using ConstRegistryIndex = detail::RegistryIndex<const detail::StateBase>;

using UpvalueIndex = detail::UpvalueIndex<detail::StateBase>;
using ConstUpvalueIndex = detail::UpvalueIndex<const detail::StateBase>;

template <int v_size>
using StackIndices = detail::StackIndices<detail::StateBase, v_size, detail::StackIndexType::Reference>;
template <int v_size>
using ConstStackIndices = detail::StackIndices<const detail::StateBase, v_size, detail::StackIndexType::Reference>;

template <int v_size>
using StackIndicesResult = detail::StackIndices<detail::StateBase, v_size, detail::StackIndexType::Result>;
template <int v_size>
using ConstStackIndicesResult = detail::StackIndices<const detail::StateBase, v_size, detail::StackIndexType::Result>;

template <int v_size>
using UpvalueIndices = detail::UpvalueIndices<detail::StateBase, v_size>;
template <int v_size>
using ConstUpvalueIndices = detail::UpvalueIndices<const detail::StateBase, v_size>;

using StackIndexRange = detail::StackIndexRange<detail::StateBase, detail::StackIndexType::Reference>;
using ConstStackIndexRange = detail::StackIndexRange<const detail::StateBase, detail::StackIndexType::Reference>;

using StackIndexRangeResult = detail::StackIndexRange<detail::StateBase, detail::StackIndexType::Result>;
using ConstStackIndexRangeResult = detail::StackIndexRange<const detail::StateBase, detail::StackIndexType::Result>;

using UpvalueIndexRange = detail::UpvalueIndexRange<detail::StateBase>;
using ConstUpvalueIndexRange = detail::UpvalueIndexRange<const detail::StateBase>;

namespace detail {

template <typename TState, StackIndexType v_type>
struct is_index_helper<StackIndex<TState, v_type>> : std::true_type {};
template <typename TState>
struct is_index_helper<RegistryIndex<TState>> : std::true_type {};
template <typename TState>
struct is_index_helper<UpvalueIndex<TState>> : std::true_type {};

template <typename TState, int v_count, StackIndexType v_type>
struct is_indices_helper<StackIndices<TState, v_count, v_type>> : std::true_type {};
template <typename TState, int v_count>
struct is_indices_helper<UpvalueIndices<TState, v_count>> : std::true_type {};

template <typename TState, StackIndexType v_type>
struct is_index_range_helper<StackIndexRange<TState, v_type>> : std::true_type {};
template <typename TState>
struct is_index_range_helper<UpvalueIndexRange<TState>> : std::true_type {};

template <typename TState, StackIndexType v_type>
struct is_stack_index_helper<StackIndex<TState, v_type>> : std::true_type {};

template <typename TState>
struct is_stack_index_result_helper<StackIndex<TState, StackIndexType::Result>> : std::true_type {};

template <typename TState, int v_count, StackIndexType v_type>
struct is_stack_indices_helper<StackIndices<TState, v_count, v_type>> : std::true_type {};

template <typename TState, int v_count>
struct is_stack_indices_result_helper<StackIndices<TState, v_count, StackIndexType::Result>> : std::true_type {};

template <typename TState, StackIndexType v_type>
struct is_stack_index_range_helper<StackIndexRange<TState, v_type>> : std::true_type {};

template <typename TState>
struct is_stack_index_range_result_helper<StackIndexRange<TState, StackIndexType::Result>> : std::true_type {};

template <typename TState>
struct is_pseudo_index_helper<RegistryIndex<TState>> : std::true_type {};
template <typename TState>
struct is_pseudo_index_helper<UpvalueIndex<TState>> : std::true_type {};

template <typename TState>
struct is_registry_index_helper<RegistryIndex<TState>> : std::true_type {};

template <typename TState>
struct is_upvalue_index_helper<UpvalueIndex<TState>> : std::true_type {};

} // namespace detail

using Arg = StackIndexResult;

template <int v_size>
using Args = StackIndicesResult<v_size>;

using VarArgs = StackIndexRangeResult;

template <int v_count>
using CallResult = std::conditional_t<
    v_count == 0,
    void,
    std::conditional_t<v_count == LUA_MULTRET, VarArgs, std::conditional_t<v_count == 1, Arg, Args<v_count>>>>;

// --- State ---

/// @brief Wraps the template supplied function into a Lua function in an almost cost-free way.
/// @remark Unlike wrap, this does not catch exceptions, which can lead to unexpected results.
template <auto v_func, typename TCovariantClass = void>
int wrapUnsafe(lua_State* state);

/// @brief Wraps the template supplied function into a Lua function in an almost cost-free way.
/// @remark Forwards exceptions of any type as Lua errors.
template <auto v_func, typename TCovariantClass = void>
int wrap(lua_State* state);

/// @brief Wraps the template supplied function into a Lua function in an almost cost-free way.
/// @remark If an exception is thrown the function returns the `fail` value plus the exception message.
template <auto v_func, typename TCovariantClass = void>
int wrapReturnException(lua_State* state);

/// @brief Returns a luaL_Reg with the wrapped template supplied function and given name.
/// @remark Unlike reg, this does not catch exceptions, which can lead to unexpected results.
template <auto v_func, typename TCovariantClass = void>
constexpr luaL_Reg regUnsafe(const char* name);

/// @brief Returns a luaL_Reg with the wrapped template supplied function and given name.
/// @remark Forwards exceptions of any type as Lua errors.
template <auto v_func, typename TCovariantClass = void>
constexpr luaL_Reg reg(const char* name);

/// @brief Returns a luaL_Reg with the wrapped template supplied function and given name.
/// @remark If an exception is thrown the function returns the `fail` value plus the exception message.
template <auto v_func, typename TCovariantClass = void>
constexpr luaL_Reg regReturnException(const char* name);

/// @brief Wraps setting the given field of the type as a Lua function.
/// @remark Unlike wrapSet, this does not catch exceptions, which can lead to unexpected results.
template <auto v_field, typename TCovariantClass = void>
int wrapSetUnsafe(lua_State* state);

/// @brief Wraps setting the given field of the type as a Lua function.
/// @remark Forwards exceptions of any type as Lua errors.
template <auto v_field, typename TCovariantClass = void>
int wrapSet(lua_State* state);

/// @brief Returns a luaL_Reg that sets the given field.
/// @remark Unlike regSet, this does not catch exceptions, which can lead to unexpected results.
template <auto v_field, typename TCovariantClass = void>
constexpr luaL_Reg regSetUnsafe(const char* name);

/// @brief Returns a luaL_Reg that sets the given field.
/// @remark Forwards exceptions of any type as Lua errors.
template <auto v_field, typename TCovariantClass = void>
constexpr luaL_Reg regSet(const char* name);

/// @brief Utility for creating properties with wrap and wrapSet for the given field and optional covariant class.
/// @remark Unlike field, this does not catch exceptions, which can lead to unexpected results.
template <auto v_field, typename TCovariantClass = void>
constexpr Property fieldUnsafe(const char* name);

/// @brief Utility for creating properties with wrap and wrapSet for the given field and optional covariant class.
/// @remark Forwards exceptions of any type as Lua errors.
template <auto v_field, typename TCovariantClass = void>
constexpr Property field(const char* name);

struct Error {
    Status status;
    StackIndexResult message;
};

template <typename T>
using Expected = tl::expected<T, Error>;

struct LoadInfo {
    // TODO: Name by pointer is quite questionable.

    template <typename TBuffer>
    LoadInfo(TBuffer&& buffer, const char* name = nullptr, LoadMode mode = LoadMode::Default)
        : buffer(std::forward<TBuffer>(buffer))
        , name(name)
        , mode(mode)
    {}

    template <typename TBuffer>
    LoadInfo(TBuffer&& buffer, const std::string& name, LoadMode mode = LoadMode::Default)
        : buffer(std::forward<TBuffer>(buffer))
        , name(name.c_str())
        , mode(mode)
    {}

    std::string_view buffer;
    const char* name;
    LoadMode mode;
};

struct DebugInfoHook {
    DebugInfoHook(const lua_Debug& ar)
        : event(static_cast<Hook>(ar.event))
    {}

    Hook event;
};

struct DebugInfoName {
    DebugInfoName(const lua_Debug& ar)
        : name(ar.name ? std::optional<std::string>(std::in_place, ar.name) : std::nullopt)
        , name_what(ar.namewhat)
    {}

    std::optional<std::string> name;
    std::string name_what; // TODO: Replace with an enum.
};

struct DebugInfoSource {
    DebugInfoSource(const lua_Debug& ar)
        : what(ar.what)
        , source(ar.source, ar.source + ar.srclen)
        , line_defined(ar.linedefined)
        , last_line_defined(ar.lastlinedefined)
        , short_src(ar.short_src)
    {}

    std::string what;
    std::string source;
    int line_defined;
    int last_line_defined;
    std::string short_src;
};

struct DebugInfoLine {
    DebugInfoLine(const lua_Debug& ar)
        : current_line(ar.currentline)
    {}

    int current_line;
};

struct DebugInfoTailCall {
    DebugInfoTailCall(const lua_Debug& ar)
        : is_tail_call(ar.istailcall)
    {}

    bool is_tail_call;
};

struct DebugInfoUpvalues {
    DebugInfoUpvalues(const lua_Debug& ar)
        : upvalue_count(ar.nups)
        , parameter_count(ar.nparams)
        , is_var_arg(ar.isvararg)
    {}

    int upvalue_count;
    int parameter_count;
    bool is_var_arg;
};

struct DebugInfoTransfer {
    DebugInfoTransfer(const lua_Debug& ar)
        : first_transferred(ar.ftransfer)
        , transferred_count(ar.ntransfer)
    {}

    int first_transferred;
    int transferred_count;
};

template <typename... TTypes>
struct DebugInfo : TTypes... {
    DebugInfo(const lua_Debug& ar)
        : TTypes(ar)...
    {}
};

template <DebugInfoType>
struct debug_info_type {};

template <>
struct debug_info_type<DebugInfoType::Line> {
    using type = DebugInfoLine;
};

template <>
struct debug_info_type<DebugInfoType::Name> {
    using type = DebugInfoName;
};

template <>
struct debug_info_type<DebugInfoType::Source> {
    using type = DebugInfoSource;
};

template <>
struct debug_info_type<DebugInfoType::TailCall> {
    using type = DebugInfoTailCall;
};

template <>
struct debug_info_type<DebugInfoType::Upvalues> {
    using type = DebugInfoUpvalues;
};

template <DebugInfoType v_type>
using debug_info_type_t = typename debug_info_type<v_type>::type;

template <typename>
struct debug_info_enum {};

template <>
struct debug_info_enum<DebugInfoLine> : dutils::constant<DebugInfoType::Line> {};

template <>
struct debug_info_enum<DebugInfoName> : dutils::constant<DebugInfoType::Name> {};

template <>
struct debug_info_enum<DebugInfoSource> : dutils::constant<DebugInfoType::Source> {};

template <>
struct debug_info_enum<DebugInfoTailCall> : dutils::constant<DebugInfoType::TailCall> {};

template <>
struct debug_info_enum<DebugInfoUpvalues> : dutils::constant<DebugInfoType::Upvalues> {};

template <typename TDebugInfo>
inline constexpr auto debug_info_enum_v = debug_info_enum<TDebugInfo>::value;

template <typename TValue>
class next_iterator;

class next_pair_iterator;

class next_key_iterator;

class next_value_iterator;

template <typename TValue, bool v_raw>
class index_length_iterator;

template <bool v_raw>
class index_length_pair_iterator;

template <bool v_raw>
class index_length_key_iterator;

template <bool v_raw>
class index_length_value_iterator;

template <typename TValue, bool v_raw>
class index_while_iterator;

template <bool v_raw>
class index_while_pair_iterator;

template <bool v_raw>
class index_while_key_iterator;

template <bool v_raw>
class index_while_value_iterator;

template <typename TValue, int v_value_count, int v_value_offset>
class generator_iterator;

template <int v_value_offset>
class generator_index_iterator;

template <int v_value_count, int v_value_offset>
class generator_indices_iterator;

template <int v_value_offset>
class generator_index_range_iterator;

template <typename... TIterators>
class iterator_variant;

class IterationWrapperBase;

template <typename TIterator>
class IterationWrapper;

template <template <bool> typename TIterator, bool v_raw>
class IndexLengthIterationWrapper;

template <typename TIterator>
class GeneratorIterationWrapper;

template <typename... TIterationWrappers>
class IterationWrapperVariant;

using PairsIterationWrapper = IterationWrapperVariant<GeneratorIterationWrapper<generator_indices_iterator<2, 0>>,
                                                      IterationWrapper<next_pair_iterator>>;
using KeysIterationWrapper = IterationWrapperVariant<GeneratorIterationWrapper<generator_index_iterator<0>>,
                                                     IterationWrapper<next_key_iterator>>;
using ValuesIterationWrapper = IterationWrapperVariant<GeneratorIterationWrapper<generator_index_iterator<1>>,
                                                       IterationWrapper<next_value_iterator>>;

using PairsRawIterationWrapper = IterationWrapper<next_pair_iterator>;
using KeysRawIterationWrapper = IterationWrapper<next_key_iterator>;
using ValuesRawIterationWrapper = IterationWrapper<next_value_iterator>;

using IPairsIterationWrapper = IterationWrapper<index_while_pair_iterator<false>>;
using IKeysIterationWrapper = IterationWrapper<index_while_key_iterator<false>>;
using IValuesIterationWrapper = IterationWrapper<index_while_value_iterator<false>>;

using IPairsLenIterationWrapper = IndexLengthIterationWrapper<index_length_pair_iterator, false>;
using IKeysLenIterationWrapper = IndexLengthIterationWrapper<index_length_key_iterator, false>;
using IValuesLenIterationWrapper = IndexLengthIterationWrapper<index_length_value_iterator, false>;

using IPairsRawIterationWrapper = IndexLengthIterationWrapper<index_length_pair_iterator, true>;
using IKeysRawIterationWrapper = IndexLengthIterationWrapper<index_length_key_iterator, true>;
using IValuesRawIterationWrapper = IndexLengthIterationWrapper<index_length_value_iterator, true>;

template <int v_value_offset>
using IterateWrapper = GeneratorIterationWrapper<generator_index_iterator<v_value_offset>>;

template <int v_value_count, int v_value_offset>
using IterateMultipleWrapper = GeneratorIterationWrapper<generator_indices_iterator<v_value_count, v_value_offset>>;

template <int v_value_offset>
using IterateVaryingWrapper = GeneratorIterationWrapper<generator_index_range_iterator<v_value_offset>>;

namespace detail {

/// @brief Wraps a Lua state or thread.
class StateBase {
public:
    friend class dang::lua::StateRef;
    friend class dang::lua::Thread;
    friend class dang::lua::State;

    /// @brief Returns the wrapped Lua state.
    /// @remark The returned state is const to prevent direct API calls on it.
    const lua_State* state() { return state_; }

    // --- State Properties ---

    /// @brief Returns the version number of this Lua state.
    auto version() const { return lua_version(state_); }

    /// @brief Checks whether the code making the call and the Lua library being called are using the same version of
    /// Lua and the same numeric types.
    void checkVersion() const
    {
        assertPushableAuxiliary();
        luaL_checkversion(state_);
    }

    /// @brief Returns the current status of the thread.
    auto status() const { return static_cast<Status>(lua_status(state_)); }

    /// @brief Whether the thread can yield.
    bool isYieldable() const { return lua_isyieldable(state_); }

    /// @brief Replaces the panic function with the given function, returning the old one.
    auto replacePanicFunction(lua_CFunction panic_function) { return lua_atpanic(state_, panic_function); }

    // TODO: Somehow make this more type safe.
    /// @brief Returns a reference to a pointer that can be used freely.
    void*& extraspace() { return *static_cast<void**>(lua_getextraspace(state_)); }

    // --- Allocator ---

    /// @brief Returns the current allocation function and associated userdata pointer.
    auto getAllocator() const
    {
        void* userdata;
        auto allocator = lua_getallocf(state_, &userdata);
        return Allocator{allocator, userdata};
    }

    /// @brief Sets a new allocation function and an optionally associated userdata pointer.
    void setAllocator(Allocator allocator) { lua_setallocf(state_, allocator.function, allocator.userdata); }

    // --- Garbage Collector ---

    /// @brief Performs a full garbage-collection cycle.
    void gcCollect() { gc(GCOption::Collect); }

    /// @brief Stops the garbage collector.
    void gcStop() { gc(GCOption::Stop); }

    /// @brief Restarts the garbage collector.
    void gcRestart() { gc(GCOption::Restart); }

    /// @brief Returns the current amount of memory (in KiB) in use by Lua.
    int gcCount() const { return gc(GCOption::Count); }

    /// @brief Returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.
    int gcCountBytes() const { return gc(GCOption::CountBytes); }

    /// @brief Performs an incremental step of garbage collection, corresponding to the allocation of step size KiB.
    void gcStep(int stepsize) { gc(GCOption::Step, stepsize); }

    /// @brief Whether the collector is running (i.e., not stopped).
    bool gcIsRunning() const { return gc(GCOption::IsRunning); }

    /// @brief Changes the collector to incremental mode with the given parameters and returns the previous mode.
    /// @remark A zero parameter means to not change that value.
    auto gcIncremental(int pause, int stepmul, int stepsize)
    {
        return static_cast<GCOption>(gc(GCOption::Incremental, pause, stepmul, stepsize));
    }

    /// @brief Changes the collector to generational mode with the given parameters and returns the previous mode.
    /// @remark A zero parameter means to not change that value.
    auto gcGenerational(int minormul, int majormul)
    {
        return static_cast<GCOption>(gc(GCOption::Generational, minormul, majormul));
    }

    // --- Index Wrapping ---

    /// @brief Returns a wrapper to a stack index.
    dang::lua::StackIndex stackIndex(int index) { return {*this, index}; }

    /// @brief Returns a wrapper to a stack index.
    dang::lua::ConstStackIndex stackIndex(int index) const { return {*this, index}; }

    /// @brief Returns a wrapper to the registry pseudo index.
    dang::lua::RegistryIndex registry() { return {*this}; }

    /// @brief Returns a wrapper to the registry pseudo index.
    dang::lua::ConstRegistryIndex registry() const { return {*this}; }

    /// @brief Converts a 1-based index into an upvalue index and returns a wrapper to it.
    dang::lua::UpvalueIndex upvalue(int index) { return {*this, index}; }

    /// @brief Converts a 1-based index into an upvalue index and returns a wrapper to it.
    dang::lua::ConstUpvalueIndex upvalue(int index) const { return {*this, index}; }

    // --- Indices Wrapping ---

    /// @brief Returns a wrapper to a compile-time fixed range of stack indices.
    template <int v_count>
    dang::lua::StackIndices<v_count> stackIndices(int first)
    {
        return {*this, first};
    }

    /// @brief Returns a wrapper to a compile-time fixed range of stack indices.
    template <int v_count>
    dang::lua::ConstStackIndices<v_count> stackIndices(int first) const
    {
        return {*this, first};
    }

    /// @brief Returns a wrapper to a compile-time fixed range of upvalues.
    template <int v_count>
    dang::lua::UpvalueIndices<v_count> upvalueIndices(int first)
    {
        return {*this, first};
    }

    /// @brief Returns a wrapper to a compile-time fixed range of upvalues.
    template <int v_count>
    dang::lua::ConstUpvalueIndices<v_count> upvalueIndices(int first) const
    {
        return {*this, first};
    }

    // --- Index Range Wrapping ---

    /// @brief Returns a wrapper to a range of stack indices.
    dang::lua::StackIndexRange stackIndexRange(int first, int count) { return {*this, first, count}; }

    /// @brief Returns a wrapper to a range of stack indices.
    dang::lua::ConstStackIndexRange stackIndexRange(int first, int count) const { return {*this, first, count}; }

    /// @brief Returns a wrapper to a range of upvalues.
    dang::lua::UpvalueIndexRange upvalueIndexRange(int first, int count) { return {*this, first, count}; }

    /// @brief Returns a wrapper to a range of upvalues.
    dang::lua::ConstUpvalueIndexRange upvalueIndexRange(int first, int count) const { return {*this, first, count}; }

    // --- Top and Bottom Wrapping ---

    /// @brief Returns a wrapper to the first stack index.
    auto bottom() { return stackIndex(1); }

    /// @brief Returns a wrapper to the first stack index.
    auto bottom() const { return stackIndex(1); }

    /// @brief Returns a wrapper to a compile-time fixed size range of the first stack indices.
    template <int v_count>
    auto bottom()
    {
        return stackIndices<v_count>(1);
    }

    /// @brief Returns a wrapper to a compile-time fixed size range of the first stack indices.
    template <int v_count>
    auto bottom() const
    {
        return stackIndices<v_count>(1);
    }

    /// @brief Returns a wrapper to a range of the first stack indices.
    auto bottom(int count) { return stackIndexRange(1, count); }

    /// @brief Returns a wrapper to a range of the first stack indices.
    auto bottom(int count) const { return stackIndexRange(1, count); }

    /// @brief Returns a wrapper to the last stack index.
    auto top() { return stackIndex(size()); }

    /// @brief Returns a wrapper to the last stack index.
    auto top() const { return stackIndex(size()); }

    /// @brief Returns a wrapper to a compile-time fixed size range of the last stack indices.
    template <int v_count>
    auto top()
    {
        return stackIndices<v_count>(size() - v_count + 1);
    }

    /// @brief Returns a wrapper to a compile-time fixed size range of the last stack indices.
    template <int v_count>
    auto top() const
    {
        return stackIndices<v_count>(size() - v_count + 1);
    }

    /// @brief Returns a wrapper to a range of the last stack indices.
    auto top(int count) { return stackIndexRange(size() - count + 1, count); }

    /// @brief Returns a wrapper to a range of the last stack indices.
    auto top(int count) const { return stackIndexRange(size() - count + 1, count); }

    // --- Index Properties ---

    /// @brief Whether the given index is a stack index.
    static constexpr bool isStack(int index) { return index > LUA_REGISTRYINDEX; }

    /// @brief Whether the given index is a positive stack index.
    static constexpr bool isPositiveStack(int index) { return index > 0; }

    /// @brief Whether the given index is a negative stack index.
    static constexpr bool isNegativeStack(int index) { return index > LUA_REGISTRYINDEX && index < 0; }

    /// @brief Whether the given index is a pseudo index.
    static constexpr bool isPseudo(int index) { return index <= LUA_REGISTRYINDEX; }

    /// @brief Whether the given index is the registry index.
    static constexpr bool isRegistry(int index) { return index == LUA_REGISTRYINDEX; }

    /// @brief Whether the given index is an upvalue index.
    static constexpr bool isUpvalue(int index) { return index < LUA_REGISTRYINDEX; }

    /// @brief Whether the given index is the first stack index or "offset" above it.
    static constexpr bool isIndexBottom(int index, int offset = 0)
    {
        assert(offset >= 0);
        return index == 1 + offset;
    }

    /// @brief Whether the given index is the last stack index or "offset" below it.
    bool isIndexTop(int index, int offset = 0) const
    {
        assert(offset >= 0);
        return index == size() - offset;
    }

    /// @brief Returns how many positions the given stack index is above the first index.
    static constexpr int indexOffsetFromBottom(int index) { return index - 1; }

    /// @brief Returns how many positions the given stack index is below the last index.
    int indexOffsetFromTop(int index) const { return size() - index; }

    /// @brief Turns negative stack indices into positive ones, leaving pseudo indices as is.
    int absIndex(int index) const { return index > LUA_REGISTRYINDEX && index < 0 ? size() + index + 1 : index; }

    /// @brief Turns negative stack indices into positive ones, but does not working with pseudo indices.
    int absStackIndex(int index) const { return index < 0 ? size() + index + 1 : index; }

    // --- Stack Queries ---

    /// @brief The (cached) size of the stack, which is also the top index, as indices start at 1.
    int size() const { return top_; }

    /// @brief Whether the stack is empty.
    bool empty() const { return size() == 0; }

    /// @brief Returns the type of the element at the given index.
    Type type(int index) const { return static_cast<Type>(lua_type(state_, index)); }

    /// @brief Returns the type name of the element at the given index.
    std::string_view typeName(int index) const { return luaL_typename(state_, index); }

    /// @brief Whether the index is not valid.
    bool isNone(int index) const
    {
        assertAcceptable(index);
        return lua_isnone(state_, index);
    }

    /// @brief Whether the element at the given index is nil.
    bool isNil(int index) const
    {
        assertAcceptable(index);
        return lua_isnil(state_, index);
    }

    /// @brief Whether the index is not valid or its element is nil.
    bool isNoneOrNil(int index) const
    {
        assertAcceptable(index);
        return lua_isnoneornil(state_, index);
    }

    /// @brief Whether the element at the given index is a boolean.
    bool isBoolean(int index) const
    {
        assertAcceptable(index);
        return lua_isboolean(state_, index);
    }

    /// @brief Whether the element at the given index is light userdata.
    bool isLightUserdata(int index) const
    {
        assertAcceptable(index);
        return lua_islightuserdata(state_, index);
    }

    /// @brief Whether the element at the given index is a number or a string convertible to a number.
    bool isNumber(int index) const
    {
        assertAcceptable(index);
        return lua_isnumber(state_, index);
    }

    /// @brief Whether the element at the given index is an actual integer.
    bool isInteger(int index) const
    {
        assertAcceptable(index);
        return lua_isinteger(state_, index);
    }

    /// @brief Whether the element at the given index is a string or a number (which is always convertible to a string).
    bool isString(int index) const
    {
        assertAcceptable(index);
        return lua_isstring(state_, index);
    }

    /// @brief Whether the element at the given index is a table.
    bool isTable(int index) const
    {
        assertAcceptable(index);
        return lua_istable(state_, index);
    }

    /// @brief Whether the element at the given index is a function (either C or Lua).
    bool isFunction(int index) const
    {
        assertAcceptable(index);
        return lua_isfunction(state_, index);
    }

    /// @brief Whether the element at the given index is a C function.
    bool isCFunction(int index) const
    {
        assertAcceptable(index);
        return lua_iscfunction(state_, index);
    }

    /// @brief Whether the element at the given index is full or light userdata.
    bool isUserdata(int index) const
    {
        assertAcceptable(index);
        return lua_isuserdata(state_, index);
    }

    /// @brief Whether the element at the given index is a thread.
    bool isThread(int index) const
    {
        assertAcceptable(index);
        return lua_isthread(state_, index);
    }

    // --- Conversion ---

    // TODO: These are not const because of automatic number to string conversion...
    //       Possibly add const overloads that don't to this conversion.

    /// @brief Uses the Convert template to convert the element at the given index.
    /// @remark Returns an optional, which is std::nullopt if the conversion failed.
    template <typename T>
    auto to(int index)
    {
        return Convert<T>::at(state_, index);
    }

    /// @brief Treats the element at the given index as a function argument and uses the Convert template to convert it.
    /// @remark Raises a Lua (argument) error if the conversion failed.
    template <typename T>
    decltype(auto) check(int index)
    {
        return Convert<T>::check(state_, index);
    }

    // --- Stack Maintenance ---

    /// @brief Asserts, whether the given index is currently acceptable without checking the stack.
    void assertAcceptable([[maybe_unused]] int index) const { assert(index != 0 && index - size() <= pushable_); }

    /// @brief Asserts, whether it is possible to push a given number of elements without checking the stack.
    void assertPushable([[maybe_unused]] int count = 1) const { assert(count <= pushable_); }

    /// @brief Asserts, whether it is possible to call an auxiliary function with the current stack.
    void assertPushableAuxiliary() const { assertPushable(auxiliary_required_pushable); }

    /// @brief Ensures, that the given index is going to be acceptable and returns false if it can't.
    bool checkAcceptable(int index) const
    {
        assert(index != 0);
        return index < 0 || checkPushable(index - size());
    }

    /// @brief Tries to ensure, that it is possible to push a given number of values, returning false if it can't.
    bool checkPushable(int count = 1) const
    {
        if (!lua_checkstack(state_, count))
            return false;
#ifndef NDEBUG
        pushable_ = std::max(pushable_, count);
#endif
        return true;
    }

    /// @brief Tries to ensures, that an auxiliary library function can be called, returning false if it can't.
    bool checkPushableAuxiliary() const { return checkPushable(auxiliary_required_pushable); }

    /// @brief Ensures, that the given index is going to be acceptable and raises a Lua error if it can't.
    void ensureAcceptable(int index, const char* error_message = nullptr) const
    {
        assert(index != 0);
        if (index > 0)
            ensurePushable(index - size(), error_message);
    }

    /// @brief Ensures, that it is possible to push a given number of values, raising a Lua error if it can't.
    void ensurePushable(int count = 1, const char* error_message = nullptr) const
    {
        luaL_checkstack(state_, count, error_message);
#ifndef NDEBUG
        pushable_ = std::max(pushable_, count);
#endif
    }

    /// @brief Ensures, that an auxiliary library function can be called, returning false if it can't.
    void ensurePushableAuxiliary() const { ensurePushable(auxiliary_required_pushable); }

private:
    // --- Push and Pop Tuple Helper ---

    template <std::size_t v_index, typename TFirst, typename... TRest>
    static constexpr std::optional<std::size_t> findFirstTupleHelper()
    {
        if constexpr (is_tuple_v<TFirst>)
            return v_index;
        return findFirstTupleHelper<v_index + 1, TRest...>();
    }

    template <std::size_t>
    static constexpr std::optional<std::size_t> findFirstTupleHelper()
    {
        return std::nullopt;
    }

    template <typename... TValues>
    static constexpr auto findFirstTuple()
    {
        return findFirstTupleHelper<0, TValues...>();
    }

    template <typename... TBefore>
    struct PushUnpackTupleSkip {
        StateBase& lua;

        template <typename TTuple, typename... TAfter, std::size_t... v_indices>
        auto push(std::index_sequence<v_indices...>, TBefore... before, TTuple&& tuple, TAfter&&... after)
        {
            return lua.push(std::forward<TBefore>(before)...,
                            std::get<v_indices>(std::forward<TTuple>(tuple))...,
                            std::forward<TAfter>(after)...);
        }

        template <std::size_t v_skip, typename TFirst, typename... TRest>
        auto pushSkip(TBefore... before, TFirst&& first, TRest&&... rest)
        {
            if constexpr (v_skip == 0)
                return push(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TFirst>>>(),
                            std::forward<TBefore>(before)...,
                            std::forward<TFirst>(first),
                            std::forward<TRest>(rest)...);
            else
                return PushUnpackTupleSkip<TBefore..., TFirst>{lua}.template pushSkip<v_skip - 1>(
                    std::forward<TBefore>(before)..., std::forward<TFirst>(first), std::forward<TRest>(rest)...);
        }
    };

public:
    // --- Push and Pop ---

    /// @brief Uses the Convert template to push all given values on the stack and returns a wrapper to them.
    /// @remark Automatically ignores all rvalue stack indices if possible.
    template <typename... TValues>
    auto push(TValues&&... values)
    {
        constexpr auto first_tuple_index = findFirstTuple<std::remove_reference_t<TValues>...>();
        if constexpr (first_tuple_index) {
            return PushUnpackTupleSkip<>{*this}.pushSkip<*first_tuple_index>(std::forward<TValues>(values)...);
        }
        else {
            constexpr auto constexpr_push_count = combined_push_count<std::remove_reference_t<TValues>...>;
            if constexpr (constexpr_push_count) {
                constexpr int push_count = *constexpr_push_count;
                pushHelper(std::forward<TValues>(values)...);
                if constexpr (push_count == 1)
                    return top().asResult();
                else
                    return top<push_count>().asResult();
            }
            else {
                int push_count = combinedPushCount(values...);
                pushHelper(std::forward<TValues>(values)...);
                return top(push_count).asResult();
            }
        }
    }

    /// @brief Same as push, recommended to use for temporaries in expressions.
    template <typename... TValues>
    auto operator()(TValues&&... values)
    {
        return push(std::forward<TValues>(values)...);
    }

    /// @brief Pushes nil on the stack.
    auto pushNil() { return push(nil); }

    /// @brief Pushes nil values up to the given positive index.
    void padWithNil(int index)
    {
        // Can also pad "nothing".
        assert(index >= 0);
        int current_top = size();
        if (index <= current_top)
            return;
        lua_settop(state_, index);
        notifyPush(index - current_top);
    }

    /// @brief Pushes the `fail` value on the stack.
    auto pushFail() { return push(fail); }

    /// @brief Pushes a newly created empty table with optional hints for the size of its array and record parts.
    auto pushEmptyTable(int array_hint = 0, int record_hint = 0)
    {
        lua_createtable(state_, array_hint, record_hint);
        notifyPush();
        return top().asResult();
    }

    /// @brief Pushes a newly created table containing all elements in the given range and optionally adds an "n" field
    /// with the count.
    /// @remark The "n" field is necessary if the table may contain nil values.
    template <typename TIter>
    auto pushArrayTable(TIter first, TIter last, bool with_n = false)
    {
        auto array_hint = getTableHint(first, last);
        auto record_hint = with_n ? 1 : 0;
        auto result = pushEmptyTable(array_hint, record_hint);
        lua_Integer n = 0;
        std::for_each(first, last, [&](const auto& item) { result.setTable(++n, item); });
        if (with_n)
            result.setTable("n", n);
        return result;
    }

    /// @brief Pushes a newly created table containing all elements in the given initializer list.
    template <typename T>
    auto pushArrayTable(std::initializer_list<T> collection, bool with_n = false)
    {
        return pushArrayTable(begin(collection), end(collection), with_n);
    }

    /// @brief Pushes a newly created table containing all elements in the given collection.
    template <typename T>
    auto pushArrayTable(const T& collection, bool with_n = false)
    {
        using std::begin, std::end;
        return pushArrayTable(begin(collection), end(collection), with_n);
    }

    /// @brief Pushes a newly created table containing keys for all elements in the given range.
    /// @remark By default, each entry's value is set to `true`, but another value can be provided.
    template <typename TIter, typename TValue = bool>
    auto pushSetTable(TIter first, TIter last, const TValue& value = true)
    {
        auto result = pushEmptyTable(0, getTableHint(first, last));
        std::for_each(first, last, [&](const auto& item) { result.setTable(item, value); });
        return result;
    }

    /// @brief Pushes a newly created table containing keys for all elements in the given initializer list.
    /// @remark By default, each entry's value is set to `true`, but another value can be provided.
    template <typename T, typename TValue = bool>
    auto pushSetTable(std::initializer_list<T> collection, const TValue& value = true)
    {
        return pushSetTable(begin(collection), end(collection), value);
    }

    /// @brief Pushes a newly created table containing keys for all elements in the given collection.
    /// @remark By default, each entry's value is set to `true`, but another value can be provided.
    template <typename T, typename TValue = bool>
    auto pushSetTable(const T& collection, const TValue& value = true)
    {
        using std::begin, std::end;
        return pushSetTable(begin(collection), end(collection), value);
    }

    /// @brief Pushes a newly created table containing all key-value pairs in the given range.
    /// @remark Works with collections of luaL_Reg.
    template <typename TIter>
    auto pushMapTable(TIter first, TIter last)
    {
        auto result = pushEmptyTable(0, getTableHint(first, last));
        std::for_each(first, last, [&](const auto& pair) {
            const auto& [key, value] = pair;
            result.setTable(key, value);
        });
        return result;
    }

    /// @brief Pushes a newly created table containing all key-value pairs in the given initializer list.
    template <typename TPair>
    auto pushMapTable(std::initializer_list<TPair> collection)
    {
        return pushMapTable(begin(collection), end(collection));
    }

    /// @brief Pushes a newly created table containing all key-value pairs in the given collection.
    /// @remark Works with collections of luaL_Reg.
    template <typename T>
    auto pushMapTable(const T& collection)
    {
        using std::begin, std::end;
        return pushMapTable(begin(collection), end(collection));
    }

    /// @brief Pushes a newly created thread on the stack.
    auto pushThread()
    {
        // TODO: Fix this once Thread is properly implemented.
        // Thread thread = lua_newthread(state_);
        // notifyPush();
        // return std::tuple{std::move(thread), top().asResult()};
    }

    /// @brief Pushes an in-place constructed object with the given parameters.
    template <typename T, typename... TArgs>
    auto pushNew(TArgs&&... args)
    {
        constexpr auto constexpr_push_count = Convert<T>::push_count;
        if constexpr (constexpr_push_count) {
            constexpr auto push_count = *constexpr_push_count;
            assertPushable(push_count);
            Convert<T>::push(state_, std::forward<TArgs>(args)...);
            notifyPush(push_count);
            if constexpr (push_count == 1)
                return top().asResult();
            else
                return top<push_count>().asResult();
        }
        else {
            T object(std::forward<TArgs>(args)...);
            int push_count = Convert<T>::getPushCount(object);
            assertPushable(push_count);
            Convert<T>::push(state_, std::move(object));
            notifyPush(push_count);
            return top(push_count).asResult();
        }
    }

    /// @brief Pushes a function or closure with an arbitrary number of upvalues on the stack.
    template <typename... TUpvalues>
    auto pushFunction(lua_CFunction func, TUpvalues&&... upvalues)
    {
        auto upvalue_count = push(std::forward<TUpvalues>(upvalues)...).size();
        lua_pushcclosure(state_, func, upvalue_count);
        notifyPush(1 - upvalue_count);
        return top().asResult();
    }

    /// @brief Pushes the global table on the stack.
    auto pushGlobalTable()
    {
        lua_pushglobaltable(state_);
        notifyPush();
        return top().asResult();
    }

    /// @brief Pops a given number of elements from the stack.
    void pop(int count = 1)
    {
        lua_pop(state_, count);
        notifyPush(-count);
    }

    /// @brief Replaces the given positive index with a single value.
    template <typename TValue>
    void replace(int index, TValue&& value)
    {
        using ConvertValue = Convert<std::remove_reference_t<TValue>>;

        static_assert(ConvertValue::push_count == 1, "Supplied value must take up a single stack position.");

        assert(isPositiveStack(index));

        if constexpr (is_index_v<std::remove_reference_t<TValue>>) {
            if (value.index() == index)
                return;
        }

        if (isIndexTop(index)) {
            lua_pop(state_, 1);
            ConvertValue::push(state_, std::forward<TValue>(value));
            return;
        }

        if constexpr (is_index_v<std::remove_reference_t<TValue>>) {
            lua_copy(state_, value.index(), index);
            if constexpr (is_any_moved_stack_index_result_v<TValue&&>) {
                if (value.isTop())
                    pop();
            }
        }
        else {
            assertPushable();
            ConvertValue::push(state_, std::forward<TValue>(value));
            lua_replace(state_, index);
        }
    }

    /// @brief Removes a value from the stack, moving everything after, to fill the gap.
    /// @remark Invalidates any greater indices!
    void remove(int index)
    {
        lua_remove(state_, index);
        notifyPush(-1);
    }

    // --- Error ---

    template <typename TMessage>
    [[noreturn]] void error(TMessage&& message)
    {
        using ConvertMessage = Convert<std::remove_reference_t<TMessage>>;

        static_assert(ConvertMessage::push_count == 1, "Supplied message must take up a single stack position.");

        push(std::forward<TMessage>(message));
        // technically lua_error pops the message, but since it doesn't return this is not really visible to users
        noreturn_lua_error(state_);
    }

    [[noreturn]] void argError(int arg, const char* extra_message)
    {
        noreturn_luaL_argerror(state_, arg, extra_message);
    }

    [[noreturn]] void argError(int arg, const std::string& extra_message) { argError(arg, extra_message.c_str()); }

    [[noreturn]] void typeError(int arg, const char* type_name) { noreturn_luaL_typeerror(state_, arg, type_name); }

    [[noreturn]] void typeError(int arg, const std::string& type_name) { typeError(arg, type_name.c_str()); }

    // --- Compiling ---

    /// @brief Compiles the given code and returns a the compilation status and depending on this either the function or
    /// error message.
    Expected<Arg> load(const LoadInfo& info)
    {
        assertPushableAuxiliary();
        auto status = static_cast<Status>(
            luaL_loadbufferx(state_, info.buffer.data(), info.buffer.size(), info.name, load_mode_names[info.mode]));
        notifyPush();
        if (status != Status::Ok)
            return tl::unexpected(Error{status, top().asResult()});
        return top().asResult();
    }

    // --- Calling ---

    /// @brief Calls the given function with the supplied arguments and returns a template specified number of results.
    template <int v_results = 0, typename TFunc, typename... TArgs>
    auto call(TFunc&& func, TArgs&&... args) -> CallResult<v_results>
    {
        using ConvertFunc = Convert<std::remove_reference_t<TFunc>>;

        static_assert(ConvertFunc::push_count == 1, "Supplied function must take up a single stack position.");

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        if constexpr (v_results == LUA_MULTRET) {
            auto first_result_index = size() - 1 - arg_count;
            lua_call(state_, arg_count, v_results);
            auto results = lua_gettop(state_) - first_result_index;
            notifyPush(results - 1 - arg_count);
            return top(results).asResult();
        }
        else {
            int diff = v_results - arg_count - 1;
            assertPushable(diff);
            lua_call(state_, arg_count, v_results);
            notifyPush(diff);
            if constexpr (v_results == 0) {
                return;
            }
            else if constexpr (v_results == 1) {
                return top().asResult();
            }
            else {
                return top<v_results>().asResult();
            }
        }
    }

    /// @brief Calls the given function with the supplied arguments and returns all results.
    template <typename TFunc, typename... TArgs>
    auto callMultRet(TFunc&& func, TArgs&&... args)
    {
        return call<LUA_MULTRET>(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
    }

    /// @brief Calls the given function with the supplied arguments and returns a specified number of results.
    template <typename TFunc, typename... TArgs>
    auto callReturning(int results, TFunc&& func, TArgs&&... args)
    {
        using ConvertFunc = Convert<std::remove_reference_t<TFunc>>;

        static_assert(ConvertFunc::push_count == 1, "Supplied function must take up a single stack position.");

        assert(results != LUA_MULTRET); // TODO: Support LUA_MULTRET

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        assertPushable(results - 1 - arg_count);
        lua_call(state_, arg_count, results);
        notifyPush(results - 1 - arg_count);
        return top(results).asResult();
    }

    /// @brief Calls the given function with the supplied arguments in protected mode and returns the status and a
    /// template specified number of results.
    template <int v_results = 0, typename TFunc, typename... TArgs>
    auto pcall(TFunc&& func, TArgs&&... args) -> Expected<CallResult<v_results>>
    {
        using ConvertFunc = Convert<std::remove_reference_t<TFunc>>;

        static_assert(ConvertFunc::push_count == 1, "Supplied function must take up a single stack position.");

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        if constexpr (v_results == LUA_MULTRET) {
            auto first_result_index = size() - 1 - arg_count;
            auto status = static_cast<Status>(lua_pcall(state_, arg_count, v_results, 0)); // TODO: Message Handler
            auto results = status == Status::Ok ? lua_gettop(state_) - first_result_index : 1;
            notifyPush(results - 1 - arg_count);
            if (status != Status::Ok)
                return tl::unexpected(Error{status, top().asResult()});
            return top(results).asResult();
        }
        else {
            assertPushable(v_results - 1 - arg_count);
            auto status = static_cast<Status>(lua_pcall(state_, arg_count, v_results, 0)); // TODO: Message Handler
            auto results = status == Status::Ok ? v_results : 1;
            notifyPush(results - 1 - arg_count);
            if constexpr (v_results == 0) {
                if (status != Status::Ok)
                    return tl::unexpected(Error{status, top().asResult()});
                return {};
            }
            else if constexpr (v_results == 1) {
                if (status != Status::Ok)
                    return tl::unexpected(Error{status, top().asResult()});
                return top().asResult();
            }
            else {
                if (status != Status::Ok)
                    return tl::unexpected(Error{status, top().asResult()});
                return top<v_results>().asResult();
            }
        }
    }

    /// @brief Calls the given function with the supplied arguments in protected mode and returns the status and all
    /// results.
    template <typename TFunc, typename... TArgs>
    auto pcallMultRet(TFunc&& func, TArgs&&... args)
    {
        return pcall<LUA_MULTRET>(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
    }

    /// @brief Calls the given function with the supplied arguments in protected mode and returns the status and a
    /// specified number of results.
    template <typename TFunc, typename... TArgs>
    auto pcallReturning(int results, TFunc&& func, TArgs&&... args) -> Expected<VarArgs>
    {
        using ConvertFunc = Convert<std::remove_reference_t<TFunc>>;

        static_assert(ConvertFunc::push_count == 1, "Supplied function must take up a single stack position.");

        assert(results != LUA_MULTRET); // TODO: Support LUA_MULTRET

        int arg_count = push(std::forward<TFunc>(func), std::forward<TArgs>(args)...).size() - 1;

        assertPushable(results - 1 - arg_count);
        auto status = static_cast<Status>(lua_pcall(state_, arg_count, results, 0)); // TODO: Message Handler
        if (status != Status::Ok)
            results = 1;
        notifyPush(results - 1 - arg_count);
        if (status != Status::Ok)
            return tl::unexpected(Error{status, top().asResult()});
        return Expected<VarArgs>{top(results).asResult()};
    }

    /// @brief Returns a metafield including its type, if it exists.
    std::optional<std::tuple<Type, StackIndexResult>> getMetafieldWithType(int index, const char* field)
    {
        assertPushableAuxiliary();
        auto type = static_cast<Type>(luaL_getmetafield(state_, index, field));
        if (type == Type::Nil)
            return std::nullopt;
        notifyPush(1);
        return std::tuple{type, top().asResult()};
    }

    /// @brief Returns a metafield, if it exists.
    std::optional<StackIndexResult> getMetafield(int index, const char* field)
    {
        if (auto result = getMetafieldWithType(index, field)) {
            auto [type, value] = *result;
            return value;
        }
        return std::nullopt;
    }

    /// @brief If a metafield with the given name exists, calls it and returns its result.
    std::optional<StackIndexResult> callMeta(int index, const char* field)
    {
        if (luaL_callmeta(state_, index, field)) {
            notifyPush();
            return top().asResult();
        }
        return std::nullopt;
    }

    // --- Compiling and Calling ---

    template <int v_results = 0, typename... TArgs>
    auto callString(const LoadInfo& info, TArgs&&... args)
    {
        return load(info).map(
            [&](Arg function) { return std::move(function).call<v_results>(std::forward<TArgs>(args)...); });
    }

    template <typename... TArgs>
    auto callStringMultRet(const LoadInfo& info, TArgs&&... args)
    {
        return callString<LUA_MULTRET>(info, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    auto callStringReturning(int results, const LoadInfo& info, TArgs&&... args)
    {
        return load(info).map(
            [&](Arg function) { return std::move(function).callReturning(results, std::forward<TArgs>(args)...); });
    }

    template <int v_results = 0, typename... TArgs>
    auto pcallString(const LoadInfo& info, TArgs&&... args)
    {
        return load(info).and_then(
            [&](Arg function) { return std::move(function).pcall<v_results>(std::forward<TArgs>(args)...); });
    }

    template <typename... TArgs>
    auto pcallStringMultRet(const LoadInfo& info, TArgs&&... args)
    {
        return pcallString<LUA_MULTRET>(info, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    auto pcallStringReturning(int results, const LoadInfo& info, TArgs&&... args)
    {
        return load(info).and_then(
            [&](Arg function) { return std::move(function).pcallReturning(results, std::forward<TArgs>(args)...); });
    }

    // --- Operations ---

    /// @brief Performs a unary operation on the given operand.
    /// @remark Asserts, that the given operation is actually unary.
    template <typename T>
    auto unary(ArithOp operation, T&& operand)
    {
        assert(operation == ArithOp::UnaryMinus || operation == ArithOp::BinaryNot);
        auto pushed_operand = push(std::forward<T>(operand));
        static_assert(std::is_same_v<decltype(pushed_operand), dang::lua::StackIndex>,
                      "Unary operations require exactly one operand.");
        lua_arith(state_, static_cast<int>(operation));
        return top().asResult();
    }

    /// @brief Performs a binary operation on the two given operands.
    /// @remark Asserts, that the given operation is not unary.
    template <typename TLeft, typename TRight>
    auto binary(ArithOp operation, TLeft&& lhs, TRight&& rhs)
    {
        assert(operation != ArithOp::UnaryMinus && operation != ArithOp::BinaryNot);
        auto pushed_operands = push(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
        static_assert(std::is_same_v<decltype(pushed_operands), dang::lua::StackIndices<2>>,
                      "Binary operations require exactly two operands.");
        lua_arith(state_, static_cast<int>(operation));
        notifyPush(-1);
        return top().asResult();
    }

    /// @brief Performs a compile-time known operation on the operand(s).
    template <ArithOp v_operation, typename... TArgs>
    auto arith(TArgs&&... args)
    {
        constexpr bool unary_op = v_operation == ArithOp::UnaryMinus || v_operation == ArithOp::BinaryNot;
        constexpr bool binary_op = !unary_op;
        auto pushed_args = push(std::forward<TArgs>(args)...);
        constexpr auto push_count = pushed_args.size();
        constexpr bool unary_arg = push_count == 1;
        constexpr bool binary_args = push_count == 2;
        static_assert(unary_op && unary_arg || binary_op && binary_args,
                      "Argument count does not match the operation type.");
        lua_arith(state_, static_cast<int>(v_operation));
        if constexpr (binary_op)
            notifyPush(-1);
        return top().asResult();
    }

    /// @brief Compares the two operands with a given operation.
    template <typename TLeft, typename TRight>
    bool compare(CompareOp operation, TLeft&& lhs, TRight&& rhs) const
    {
        // Even though lua_compare never actually consumes its arguments, this wrapper will pop temporarily converted
        // values automatically. The reason for this is to keep the overloaded operators simple and intuitive.

        using ConvertLeft = Convert<std::remove_reference_t<TLeft>>;
        using ConvertRight = Convert<std::remove_reference_t<TRight>>;

        static_assert(ConvertLeft::push_count == 1, "Left operand must take up a single stack position.");
        static_assert(ConvertRight::push_count == 1, "Right operand must take up a single stack position.");

        constexpr bool left_is_index = is_index_v<std::remove_reference_t<TLeft>>;
        constexpr bool right_is_index = is_index_v<std::remove_reference_t<TRight>>;
        if constexpr (left_is_index) {
            if constexpr (right_is_index)
                return lua_compare(state_, lhs.index(), rhs.index(), static_cast<int>(operation)) != 0;
            else {
                // Make sure it doesn't overwrite a potential none value and compares that to itself.
                // Note, that comparing anything with none is false (even none itself).
                if (lhs.isNone())
                    return false;
                assertPushable();
                ConvertRight::push(state_, std::forward<TRight>(rhs));
                bool result = lua_compare(state_, lhs.index(), -1, static_cast<int>(operation)) != 0;
                lua_pop(state_, 1);
                return result;
            }
        }
        else {
            if constexpr (right_is_index) {
                // See above.
                if (rhs.isNone())
                    return false;
                assertPushable();
                ConvertLeft::push(state_, std::forward<TLeft>(lhs));
                bool result = lua_compare(state_, -1, rhs.index(), static_cast<int>(operation)) != 0;
                lua_pop(state_, 1);
                return result;
            }
            else {
                assertPushable(2);
                ConvertLeft::push(state_, std::forward<TLeft>(lhs));
                ConvertRight::push(state_, std::forward<TRight>(rhs));
                bool result = lua_compare(state_, -2, -1, static_cast<int>(operation)) != 0;
                lua_pop(state_, 2);
                return result;
            }
        }
    }

    /// @brief Concatenates all given values.
    /// @remark Returns an empty string if no arguments are given and the value itself if only one value is provided.
    template <typename... TValues>
    auto concat(TValues&&... values)
    {
        auto pushed_values = push(std::forward<TValues>(values)...);
        lua_concat(state_, pushed_values.size());
        notifyPush(1 - pushed_values.size());
        return top().asResult();
    }

    /// @brief Pushes the length of the element at the given index on the stack.
    /// @remark This can invoke the __len metamethod and therefore doesn't necessarily return an integer.
    auto pushLength(int index)
    {
        lua_len(state_, index);
        notifyPush();
        return top().asResult();
    }

    /// @brief Returns the length of the element at the given index.
    /// @remark This can invoke the __len metamethod and raises an error if that doesn't return an integer.
    auto length(int index) const
    {
        assertPushableAuxiliary();
        return luaL_len(state_, index);
    }

    /// @brief Returns the raw length of the value, which does not invoke a metamethod.
    auto rawLength(int index) const { return lua_rawlen(state_, index); }

    // --- Table Access ---

    /// @brief Queries the table with the given key, returning both the type and the pushed value.
    /// @remark Can invoke the __index metamethod.
    template <typename TKey>
    auto getTableWithType(int table_index, TKey&& key)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");

        if constexpr (std::is_integral_v<std::remove_reference_t<TKey>> &&
                      !std::is_same_v<dutils::remove_cvref_t<TKey>, bool>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            auto type = static_cast<Type>(lua_geti(state_, table_index, lua_Integer{key}));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            auto type = static_cast<Type>(lua_getfield(state_, table_index, key));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else {
            push(std::forward<TKey>(key));
            if (isNegativeStack(table_index))
                table_index--;

            auto type = static_cast<Type>(lua_gettable(state_, table_index));
            // remove key, add value
            // -1, +1
            // notifyPush(0);
            return std::tuple{type, top().asResult()};
        }
    }

    /// @brief Queries the table with the given key, returning the pushed value.
    /// @remark Can invoke the __index metamethod.
    template <typename TKey>
    auto getTable(int table_index, TKey&& key)
    {
        auto [type, index] = getTableWithType(table_index, std::forward<TKey>(key));
        return index;
    }

    /// @brief Sets a key of the table to the given value.
    /// @remark Can invoke the __newindex metamethod.
    template <typename TKey, typename TValue>
    void setTable(int table_index, TKey&& key, TValue&& value)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;
        using ConvertValue = Convert<std::remove_reference_t<TValue>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");
        static_assert(ConvertValue::push_count == 1, "Supplied value must take up a single stack position.");

        if constexpr (std::is_integral_v<std::remove_reference_t<TKey>> &&
                      !std::is_same_v<dutils::remove_cvref_t<TKey>, bool>) {
            push(std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index--;

            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            lua_seti(state_, table_index, lua_Integer{key});
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            push(std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index--;

            lua_setfield(state_, table_index, key);
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else {
            push(std::forward<TKey>(key), std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index -= 2;

            lua_settable(state_, table_index);
            // remove key and value, push nothing
            // -2, +0
            notifyPush(-2);
        }
    }

    /// @brief Similar to getTableWithType, but does not invoke metamethods.
    template <typename TKey>
    auto rawGetTableWithType(int table_index, TKey&& key)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");

        if constexpr (std::is_integral_v<std::remove_reference_t<TKey>> &&
                      !std::is_same_v<dutils::remove_cvref_t<TKey>, bool>) {
            assertPushable();
            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            auto type = static_cast<Type>(lua_rawgeti(state_, table_index, lua_Integer{key}));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else if constexpr (std::is_same_v<TKey, void*> || std::is_same_v<TKey, const void*>) {
            assertPushable();
            auto type = static_cast<Type>(lua_rawgetp(state_, table_index, key));
            // remove nothing, add value
            // -0, +1
            notifyPush();
            return std::tuple{type, top().asResult()};
        }
        else {
            push(std::forward<TKey>(key));
            if (isNegativeStack(table_index))
                table_index--;

            auto type = static_cast<Type>(lua_rawget(state_, table_index));
            // remove key, add value
            // -1, +1
            // notifyPush(0);
            return std::tuple{type, top().asResult()};
        }
    }

    /// @brief Similar to getTable, but does not invoke metamethods.
    template <typename TKey>
    auto rawGetTable(int table_index, TKey&& key)
    {
        auto [type, index] = rawGetTableWithType(table_index, std::forward<TKey>(key));
        return index;
    }

    /// @brief Similar to setTable, but does not invoke metamethods.
    template <typename TKey, typename TValue>
    void rawSetTable(int table_index, TKey&& key, TValue&& value)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;
        using ConvertValue = Convert<std::remove_reference_t<TValue>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");
        static_assert(ConvertValue::push_count == 1, "Supplied value must take up a single stack position.");

        if constexpr (std::is_integral_v<std::remove_reference_t<TKey>> &&
                      !std::is_same_v<dutils::remove_cvref_t<TKey>, bool>) {
            push(std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index--;

            // lua_Integer{ key } disallows narrowing conversions, which is perfect
            lua_rawseti(state_, table_index, lua_Integer{key});
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else if constexpr (std::is_same_v<TKey, void*> || std::is_same_v<TKey, const void*>) {
            push(std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index--;

            lua_rawsetp(state_, table_index, key);
            // remove value, push nothing
            // -1, +0
            notifyPush(-1);
        }
        else {
            push(std::forward<TKey>(key), std::forward<TValue>(value));
            if (isNegativeStack(table_index))
                table_index -= 2;

            lua_rawset(state_, table_index);
            // remove key and value, push nothing
            // -2, +0
            notifyPush(-2);
        }
    }

    template <typename TKey>
    auto getGlobalWithType(TKey&& key)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");

        if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            auto type = static_cast<Type>(lua_getglobal(state_, key));
            notifyPush(1);
            return std::tuple{type, top().asResult()};
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

    /// @brief Sets a new value in the global table.
    template <typename TKey, typename TValue>
    void setGlobal(TKey&& key, TValue&& value)
    {
        using ConvertKey = Convert<std::remove_reference_t<TKey>>;
        using ConvertValue = Convert<std::remove_reference_t<TValue>>;

        static_assert(ConvertKey::push_count == 1, "Supplied key must take up a single stack position.");
        static_assert(ConvertValue::push_count == 1, "Supplied value must take up a single stack position.");

        if constexpr (std::is_same_v<std::decay_t<TKey>, const char*>) {
            push(std::forward<TValue>(value));
            lua_setglobal(state_, key);
            notifyPush(-1);
        }
        else {
            return pushGlobalTable().setTable(std::forward<TKey>(key), std::forward<TValue>(value));
        }
    }

    /// @brief Returns the metatable of the element at the given index or std::nullopt if it doesn't have one.
    std::optional<StackIndexResult> getMetatable(int index)
    {
        if (lua_getmetatable(state_, index)) {
            notifyPush(1);
            return top().asResult();
        }
        return std::nullopt;
    }

    /// @brief Sets the metatable of the element at the given index to the specified table; or nil to remove it.
    template <typename TMetatable>
    void setMetatable(int index, TMetatable&& metatable)
    {
        push(std::forward<TMetatable>(metatable));
        lua_setmetatable(state_, index);
        notifyPush(-1);
    }

    /// @brief Sets all functions in the given container of luaL_Reg on the table.
    template <typename TFuncs>
    void setFuncs(int table_index, const TFuncs& funcs)
    {
        for (const auto& func : funcs)
            setTable(table_index, func.name, func.func);
    }

    // --- Iteration ---

    /// @brief Returns the next key-value-pair of the table or nothing, if the table has been exhausted.
    template <typename TKey>
    std::optional<StackIndicesResult<2>> next(int table_index, TKey&& key)
    {
        push(std::forward<TKey>(key));
        if (lua_next(state_, table_index)) {
            notifyPush(1);
            return top<2>().asResult();
        }
        notifyPush(-1);
        return std::nullopt;
    }

    /// @brief Can be used to iterate over a table similar to how `pairs` in Lua works.
    /// ```lua
    /// for key, value in pairs(table) do
    ///   -- use key and value
    /// end
    /// ```
    /// @remark Like Lua `pairs` this respects the `__pairs` metamethod.
    PairsIterationWrapper pairs(int index);

    /// @copybrief dang::lua::detail::StateBase::pairs(int)
    /// ```lua
    /// for key in pairs(table) do
    ///   -- use key
    /// end
    /// ```
    /// @remark Like Lua `pairs` this respects the `__pairs` metamethod.
    KeysIterationWrapper keys(int index);

    /// @copybrief dang::lua::detail::StateBase::pairs(int)
    /// ```lua
    /// for _, value in pairs(table) do
    ///   -- use value
    /// end
    /// ```
    /// @remark Like Lua `pairs` this respects the `__pairs` metamethod.
    ValuesIterationWrapper values(int index);

    /// @brief Can be used to iterate over a table without invoking the `__pairs` metamethod.
    /// ```lua
    /// for key, value in next, table do
    ///   -- use key and value
    /// end
    /// ```
    PairsRawIterationWrapper pairsRaw(int index);

    /// @copybrief dang::lua::detail::StateBase::pairsRaw(int)
    /// ```lua
    /// for key in next, table do
    ///   -- use key
    /// end
    /// ```
    KeysRawIterationWrapper keysRaw(int index);

    /// @copybrief dang::lua::detail::StateBase::pairsRaw(int)
    /// ```lua
    /// for _, value in next, table do
    ///   -- use value
    /// end
    /// ```
    ValuesRawIterationWrapper valuesRaw(int index);

    /// @brief Can be used to iterate over a table similar to how `ipairs` in Lua works.
    /// ```lua
    /// for index, value in ipairs(table) do
    ///   -- use index and value
    /// end
    /// ```
    /// @remark Like Lua `ipairs` this iterates until a `nil` value is found, disregarding the actual length of the
    /// table.
    IPairsIterationWrapper ipairs(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairs(int)
    /// ```lua
    /// for index in ipairs(table) do
    ///   -- use index
    /// end
    /// ```
    /// @remark Like Lua `ipairs` this iterates until a `nil` value is found, disregarding the actual length of the
    /// table.
    IKeysIterationWrapper ikeys(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairs(int)
    /// ```lua
    /// for _, value in ipairs(table) do
    ///   -- use value
    /// end
    /// ```
    /// @remark Like Lua `ipairs` this iterates until a `nil` value is found, disregarding the actual length of the
    /// table.
    IValuesIterationWrapper ivalues(int index);

    /// @brief Iterates over a table by querying its length in advance.
    /// ```lua
    /// for index = 1, #table do
    ///   local value = table[index]
    ///   -- use index and value
    /// end
    /// ```
    IPairsLenIterationWrapper ipairsLen(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairsLen(int)
    /// ```lua
    /// for index = 1, #table do
    ///   -- use index
    /// end
    /// ```
    /// @remark This is optimized in a way, so that it doesn't push anything on the stack.
    IKeysLenIterationWrapper ikeysLen(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairsLen(int)
    /// ```lua
    /// for index = 1, #table do
    ///   local value = table[index]
    ///   -- use value
    /// end
    /// ```
    IValuesLenIterationWrapper ivaluesLen(int index);

    /// @brief Iterates over a table by querying its length in advance and without invoking any metamethods.
    /// ```lua
    /// for index = 1, rawlen(table) do
    ///   local value = rawget(table, index)
    ///   -- use index and value
    /// end
    /// ```
    IPairsRawIterationWrapper ipairsRaw(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairsRaw(int)
    /// ```lua
    /// for index = 1, rawlen(table) do
    ///   local value = rawget(table, index)
    ///   -- use value
    /// end
    /// ```
    /// @remark This is optimized in a way, so that it doesn't push anything on the stack.
    IKeysRawIterationWrapper ikeysRaw(int index);

    /// @copybrief dang::lua::detail::StateBase::ipairsRaw(int)
    /// ```lua
    /// for index = 1, rawlen(table) do
    ///   -- use index
    /// end
    /// ```
    IValuesRawIterationWrapper ivaluesRaw(int index);

    /// @brief Allows for iteration using generator functions, similar to how `for`-loops work in Lua.
    /// ```lua
    /// for value in generator[, state[, initial[, close]]] do
    ///   -- use value
    /// end
    /// ```
    /// - Calls `generator` until its first return value is `nil` or nothing is returned.
    /// - When `state` is provided, it is passed as first argument to every call of `generator`.
    /// - When `initial` is provided, it serves as the initial value for the control variable.
    /// - When `close` is provided, it is marked as to-be-closed automatically.
    template <int v_value_offset = 0>
    IterateWrapper<v_value_offset> iterate(int index, int input_count)
    {
        return {*this, index, input_count};
    }

    /// @copydoc dang::lua::detail::StateBase::iterate(int,int)
    /// @remark Multiple return values can be used instead of just one.
    template <int v_value_count, int v_value_offset = 0>
    IterateMultipleWrapper<v_value_count, v_value_offset> iterateMultiple(int index, int input_count)
    {
        return {*this, index, input_count};
    }

    /// @copydoc dang::lua::detail::StateBase::iterate(int,int)
    /// @remark A pair of two return values can be used instead of just one.
    template <int v_value_offset = 0>
    auto iteratePair(int index, int input_count)
    {
        return iterateMultiple<2, v_value_offset>(index, input_count);
    }

    /// @copydoc dang::lua::detail::StateBase::iterate(int,int)
    /// @remark A varying number of return values can be used instead of just one.
    template <int v_value_offset = 0>
    IterateVaryingWrapper<v_value_offset> iterateVarying(int index, int input_count)
    {
        return {*this, index, input_count};
    }

    // --- Formatting ---

    /// @brief Converts the element at the given index to a string in a reasonable format using luaL_tolstring.
    std::string format(int index) const
    {
        std::size_t length;
        checkPushableAuxiliary();
        const char* string = luaL_tolstring(state_, index, &length);
        std::string result(string, string + length);
        lua_pop(state_, 1);
        return result;
    }

    /*
    /// @brief This version can be used by the debugger, as it returns references to up to 1024 formatted strings per
    /// thread.
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

    /// @brief Opens a single standard library.
    void openLib(StandardLibrary library)
    {
        assertPushableAuxiliary();
        luaL_requiref(state_, library_names[library], library_functions[library], 1);
        lua_pop(state_, 1);
    }

    /// @brief Pushes a single standard library on the stack and returns a wrapper to it.
    auto pushLib(StandardLibrary library)
    {
        assertPushableAuxiliary();
        luaL_requiref(state_, library_names[library], library_functions[library], 1);
        notifyPush();
        return top().asResult();
    }

    /// @brief Opens all Lua standard libraries.
    void openLibs()
    {
        assertPushableAuxiliary();
        luaL_openlibs(state_);
    }

    /// @brief Opens a library with the given name, using the specified function and optionally makes it global.
    void require(const char* name, lua_CFunction open_function, bool global = false)
    {
        assertPushableAuxiliary();
        luaL_requiref(state_, name, open_function, global);
        lua_pop(state_, 1);
    }

    template <typename T>
    void require(bool global = false)
    {
        require(ClassInfo<T>::getCheckTypename().c_str(), wrap<&ClassInfo<T>::require>, global);
    }

    /// @brief Pushes a library with the given name, using the specified function on the stack, returns a wrapper to it
    /// and optionally makes it global.
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
        return pushRequire(ClassInfo<T>::getCheckTypename().c_str(), wrap<&ClassInfo<T>::require>, global);
    }

    /// @brief Registers a function with a given name in the global table.
    template <typename TFunc, typename... TUpvalues>
    void registerGlobal(const char* name, TFunc&& func, TUpvalues&&... upvalues)
    {
        pushFunction(std::forward<TFunc>(func), std::forward<TUpvalues>(upvalues)...);
        lua_setglobal(state_, name);
        notifyPush(-1);
    }

    /// @brief Registers a template specified function with a given name in the global table.
    template <auto v_func, typename... TUpvalues>
    void registerGlobal(const char* name, TUpvalues&&... upvalues)
    {
        pushFunction<v_func>(std::forward<TUpvalues>(upvalues)...);
        lua_setglobal(state_, name);
        notifyPush(-1);
    }

    // --- Function Wrapping ---

    // TODO: Make this private and friend Convert?

    /// @brief Updates the internally stored top value to allow "wrap" to pad parameters with nil.
    void maxFuncArg(int index)
    {
        if (top_ < index)
            top_ = index;
    }

    // --- Reference ---

    /// @brief Stores the value as a reference in the registry table and returns a wrapper.
    template <typename T>
    Reference ref(T&& value)
    {
        using ConvertT = Convert<std::remove_reference_t<T>>;

        static_assert(ConvertT::push_count == 1, "Supplied value must take up a single stack position.");

        push(std::forward<T>(value));
        notifyPush(-1);
        return Reference::consume(state_);
    }

    // --- To Close ---

    /// @brief Marks the value as to-be-closed.
    /// @see https://www.lua.org/manual/5.4/manual.html#lua_toclose
    void toClose(int index) { lua_toclose(state_, index); }

    // --- Debug ---

    /// @brief Returns the current debug hook function.
    auto getHook() const { return lua_gethook(state_); }

    /// @brief Returns the current debug hook count.
    auto getHookCount() const { return lua_gethookcount(state_); }

    /// @brief Returns the current debug hook mask.
    auto getHookMask() const { return Hooks::fromBits(lua_gethookmask(state_)); }

    /// @brief Returns debug information about a function in the current call stack.
    template <typename... TTypes>
    auto getStackInfo(int stack_level = 0) const
    {
        return getStackInfoHelper<TTypes...>(stack_level);
    }

    /// @brief Returns debug information about a function in the current call stack and the function itself.
    template <typename... TTypes>
    auto getStackInfoWithFunction(int stack_level = 0)
    {
        return std::tuple{getStackInfoHelper<TTypes...>(stack_level, true, false), top().asResult()};
    }

    /// @brief Returns debug and line information about a function in the current call stack.
    template <typename... TTypes>
    auto getStackInfoWithLines(int stack_level = 0)
    {
        return std::tuple{getStackInfoHelper<TTypes...>(stack_level, false, true), top().asResult()};
    }

    /// @brief Returns debug information about a function in the current call stack, the function itself and line
    /// information.
    template <typename... TTypes>
    auto getStackInfoWithFunctionAndLines(int stack_level = 0)
    {
        return std::tuple{getStackInfoHelper<TTypes...>(stack_level, true, true),
                          stackIndex(size() - 1).asResult(),
                          top().asResult()};
    }

    /// @brief Returns all debug information about a function in the current call stack.
    auto getFullStackInfo(int stack_level = 0) const
    {
        return getStackInfo<DebugInfoLine, DebugInfoName, DebugInfoSource, DebugInfoTailCall, DebugInfoUpvalues>(
            stack_level);
    }

    /// @brief Returns all debug information about a function in the current call stack and the function itself.
    auto getFullStackInfoWithFunction(int stack_level = 0)
    {
        return getStackInfoWithFunction<DebugInfoLine,
                                        DebugInfoName,
                                        DebugInfoSource,
                                        DebugInfoTailCall,
                                        DebugInfoUpvalues>(stack_level);
    }

    /// @brief Returns all debug and line information about a function in the current call stack.
    auto getFullStackInfoWithLines(int stack_level = 0)
    {
        return getStackInfoWithLines<DebugInfoLine,
                                     DebugInfoName,
                                     DebugInfoSource,
                                     DebugInfoTailCall,
                                     DebugInfoUpvalues>(stack_level);
    }

    /// @brief Returns all debug information about a function in the current call stack, the function itself and line
    /// information.
    auto getFullStackInfoWithFunctionAndLines(int stack_level = 0)
    {
        return getStackInfoWithFunctionAndLines<DebugInfoLine,
                                                DebugInfoName,
                                                DebugInfoSource,
                                                DebugInfoTailCall,
                                                DebugInfoUpvalues>(stack_level);
    }

    /// @brief Returns debug information about the given function.
    template <typename... TTypes, typename TFunction>
    auto getFunctionInfo(TFunction&& function)
    {
        return getFunctionInfoHelper<TTypes...>(std::forward<TFunction>(function), false);
    }

    /// @brief Returns debug and line information about the given function.
    template <typename... TTypes, typename TFunction>
    auto getFunctionInfoWithLines(TFunction&& function)
    {
        return std::tuple{getFunctionInfoHelper<TTypes...>(std::forward<TFunction>(function), true), top().asResult()};
    }

    /// @brief Returns all debug information about the given function.
    template <typename TFunction>
    auto getFullFunctionInfo(TFunction&& function)
    {
        return getFunctionInfo<DebugInfoSource, DebugInfoUpvalues>(std::forward<TFunction>(function));
    }

    /// @brief Returns all debug and line information about the given function.
    template <typename TFunction>
    auto getFullFunctionInfoWithLines(TFunction&& function)
    {
        return getFunctionInfoWithLines<DebugInfoSource, DebugInfoUpvalues>(std::forward<TFunction>(function));
    }

private:
    /// @brief Initializes state and stack size with the given values.
    StateBase(lua_State* state, int top)
        : state_(state)
        , top_(top)
    {}

    void swap(StateBase& other)
    {
        using std::swap;
        swap(state_, other.state_);
        swap(top_, other.top_);
#ifndef NDEBUG
        swap(pushable_, other.pushable_);
#endif
    }

    /// @brief Helper function for lua_gc which is const, since some options are, in fact, const.
    template <typename... TArgs>
    int gc(GCOption option, TArgs&&... args) const
    {
        return lua_gc(state_, static_cast<int>(option), std::forward<TArgs>(args)...);
    }

    /// @brief After some values got pushed (or popped) this function has to be called.
    void notifyPush(int count = 1)
    {
        if constexpr (false) {
            if (count < 0)
                std::clog << "pop " << -count << '\n';
            else if (count > 0) {
                std::clog << "push " << format(top_ + 1);
                for (int i = 2; i <= count; i++)
                    std::clog << ", " << format(top_ + i);
                std::clog << '\n';
            }
        }

        assertPushable(count);
        top_ += count;
#ifndef NDEBUG
        pushable_ -= count;
#endif
    }

    /// @brief Uses the Convert template to push all given values on the stack.
    /// @remark Automatically ignores all rvalue stack indices if possible.
    template <typename TFirst, typename... TRest>
    void pushHelper(TFirst&& first, TRest&&... rest)
    {
        if constexpr (is_any_moved_stack_index_result_v<TFirst&&>) {
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

    /// @brief Uses the Convert template to push all given values on the stack.
    /// @remark Automatically ignores all rvalue stack indices if possible.
    void pushHelper()
    {
        // push nothing
    }

    /// @brief Counts how many elements can be skipped because they are rvalue stack indices at the correct position.
    template <typename TFirst, typename... TRest>
    void countSkipped([[maybe_unused]] int& skipped,
                      [[maybe_unused]] int& top_offset,
                      [[maybe_unused]] TFirst&& first,
                      [[maybe_unused]] TRest&&... rest)
    {
        if constexpr (is_any_moved_stack_index_result_v<TFirst&&>) {
            skipped++;
            top_offset -= first.size();
            if (top_offset <= 0 || top_offset != indexOffsetFromTop(first.last()))
                return;
            countSkipped(skipped, top_offset, std::forward<TRest>(rest)...);
        }
    }

    /// @brief Counts how many elements can be skipped because they are rvalue stack indices at the correct position.
    void countSkipped(int&, int&)
    {
        // nothing
    }

    /// @brief Skips a given amount of arguments and pushes the rest.
    template <typename... TArgs>
    void pushWithSkip(std::size_t skip, TArgs&&... args)
    {
        pushWithSkipHelper(skip, std::index_sequence_for<TArgs...>(), std::forward<TArgs>(args)...);
    }

    /// @brief Actual implementation of pushWithSkip using an index sequence.
    template <std::size_t... v_indices, typename... TArgs>
    void pushWithSkipHelper(std::size_t skip, std::index_sequence<v_indices...>, TArgs&&... args)
    {
        ((v_indices >= skip ? pushValue(std::forward<TArgs>(args)) : (void)0), ...);
    }

    /// @brief Simply pushes a single value on the stack.
    template <typename T>
    void pushValue(T&& value)
    {
        using ConvertT = Convert<std::remove_reference_t<T>>;

        constexpr auto constexpr_push_count = ConvertT::push_count;
        if constexpr (constexpr_push_count) {
            constexpr auto push_count = *constexpr_push_count;
            assertPushable(push_count);
            ConvertT::push(state_, std::forward<T>(value));
            notifyPush(push_count);
        }
        else {
            int push_count = ConvertT::getPushCount(value);
            assertPushable(push_count);
            ConvertT::push(state_, std::forward<T>(value));
            notifyPush(push_count);
        }
    }

    /// @brief A function wrapper, that expects a std::function of the templated type in the first upvalue slot of the
    /// called closure.
    template <typename TFunc>
    static int wrappedFunction(lua_State* state);

    /// @brief Builds the "what" string, used in lua_getinfo
    /// @param stack_function Prefixes the string with a `>`.
    /// @param types Which informations should be included in the activation record.
    /// @param push_function Appends the string with a `f`.
    /// @param push_lines Appends the string with a `L`.
    static constexpr auto buildDebugInfoWhatString(bool stack_function,
                                                   DebugInfoTypes types,
                                                   bool push_function = false,
                                                   bool push_lines = false)
    {
        // 1 stack_function
        // 1 push_function
        // 1 push_lines
        // 1 null terminator
        // -> types + 4
        std::array<char, dutils::enum_count_v<DebugInfoType> + 4> result{};

        auto pos = &result[0];
        if (stack_function)
            *(pos++) = '>';
        for (auto type : types)
            *(pos++) = debug_info_type_chars[type];
        if (push_function)
            *(pos++) = 'f';
        if (push_lines)
            *(pos++) = 'L';

        return result;
    }

    /// @brief Returns debug information about a function in the current call stack.
    template <typename... TTypes>
    DebugInfo<TTypes...> getStackInfoHelper(int stack_level) const
    {
        auto what = buildDebugInfoWhatString(false, {debug_info_enum_v<TTypes>...});
        lua_Debug ar;
        lua_getstack(state_, stack_level, &ar);
        lua_getinfo(state_, what.data(), &ar);
        return DebugInfo<TTypes...>(ar);
    }

    /// @brief Returns debug information about a function in the current call stack.
    /// @param push_function Additionally push the function on the stack.
    /// @param push_lines Additionally push a set of line numbers for this function in form of a table.
    template <typename... TTypes>
    DebugInfo<TTypes...> getStackInfoHelper(int stack_level, bool push_function, bool push_lines)
    {
        auto what = buildDebugInfoWhatString(false, {debug_info_enum_v<TTypes>...}, push_function, push_lines);
        lua_Debug ar;
        lua_getstack(state_, stack_level, &ar);
        lua_getinfo(state_, what.data(), &ar);
        notifyPush(push_function + push_lines);
        return DebugInfo<TTypes...>(ar);
    }

    /// @brief Returns debug information about the given function.
    /// @param push_function Additionally push the function on the stack.
    /// @param push_lines Additionally push a set of line numbers for this function in form of a table.
    template <typename... TTypes, typename TFunction>
    DebugInfo<TTypes...> getFunctionInfoHelper(TFunction&& function, bool push_lines)
    {
        auto what = buildDebugInfoWhatString(true, {debug_info_enum_v<TTypes>...}, false, push_lines);
        lua_Debug ar;
        push(std::forward<TFunction>(function));
        lua_getinfo(state_, what.data(), &ar);
        notifyPush(push_lines - 1);
        return DebugInfo<TTypes...>(ar);
    }

    /// @brief Returns a suggested array hint to use when creating a table from a range.
    template <typename TIter>
    constexpr auto getTableHint([[maybe_unused]] TIter first, [[maybe_unused]] TIter last)
    {
        if constexpr (std::is_convertible_v<typename std::iterator_traits<TIter>::iterator_category,
                                            std::random_access_iterator_tag>) {
            constexpr auto max = std::numeric_limits<int>::max();
            auto size = std::distance(first, last);
            return size < max ? static_cast<int>(size) : max;
        }
        else
            return 0;
    }

    lua_State* state_;
    int top_;
#ifndef NDEBUG
    mutable int pushable_ = LUA_MINSTACK;
#endif
};

} // namespace detail

/// @brief Wraps a Lua state that got passed to a C function.
class StateRef : public detail::StateBase {
public:
    /// @brief Used exclusively to construct from a C function parameter.
    /// @remark It queries the stack size once and manually keeps track for better optimization.
    /// @remark It expects LUA_MINSTACK (20) pushable values and asserts that there is no overflow.
    StateRef(lua_State* state)
        : StateBase(state, lua_gettop(state))
    {}

    // Only moves are allowed to prevent different states from going out of sync.
    // Technically not even moves should be necessary with the specific use case of this class.

    StateRef(const StateRef&) = delete;
    StateRef(StateRef&&) = default;
    StateRef& operator=(const StateRef&) = delete;
    StateRef& operator=(StateRef&&) = default;

    void swap(StateRef& other) { StateBase::swap(other); }

    friend void swap(StateRef& lhs, StateRef& rhs) { lhs.swap(rhs); }

    // --- State Conversion ---

    /// @brief Returns the wrapped Lua state.
    /// @remark The returned state is const to prevent direct API calls on it.
    const lua_State* state() const& { return state_; }

    /// @brief Invalidates the State wrapper, extracting the wrapped Lua state.
    /// @remark Doesn't actually clear the state, but using both would invalidate the internal state of the wrapper.
    lua_State* state() && { return state_; }

private:
    template <typename, typename...>
    friend struct detail::SignatureInfoBase;

    template <typename>
    friend struct detail::SignatureInfo;

    /// @brief This implicit conversion simplifies signature info greatly.
    operator lua_State*() const { return state_; }
};

/// @brief Wraps a Lua thread.
class Thread : public detail::StateBase {
    // TODO: Implement Thread.
};

/// @brief A Lua state wrapper, which owns the state and therefore closes it when it goes out of scope.
class State : public detail::StateBase {
public:
    /// @brief Creates a new Lua state with the given allocator and optionally opens the standard libraries.
    /// @remark Prefer withLibs when standard libraries should be opened unconditionally.
    explicit State(std::optional<Allocator> allocator = std::nullopt, bool open_libs = false)
        : StateBase(allocator ? lua_newstate(allocator->function, allocator->userdata) : luaL_newstate(), 0)
    {
        if (open_libs)
            openLibs();
    }

    /// @brief Creates a new Lua state with the given allocator and opens the standard libraries.
    static State withLibs(std::optional<Allocator> allocator = std::nullopt) { return State(allocator, true); }

    /// @brief Closes the Lua state if it is not already closed.
    ~State() { close(); }

    State(const State&) = delete;

    State(State&& other) noexcept
        : StateBase(std::exchange(other.state_, nullptr), other.top_)
    {
#ifndef NDEBUG
        pushable_ = other.pushable_;
#endif
    }

    State& operator=(const State&) = delete;

    State& operator=(State&& other) noexcept
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

    void swap(State& other) { StateBase::swap(other); }

    friend void swap(State& lhs, State& rhs) { lhs.swap(rhs); }

    /// @brief Whether the state as been closed manually.
    bool closed() { return state_ == nullptr; }

    /// @brief Closes the Lua state manually, can be called multiple times.
    void close()
    {
        if (closed())
            return;
        lua_close(state_);
        state_ = nullptr;
    }
};

/// @brief Used to automatically pop any pushed elements at the end of the scope.
class ScopedStack {
public:
    /// @brief Constructs a scoped stack for the given stack.
    explicit ScopedStack(detail::StateBase& state, int offset = 0)
        : state_(state)
        , initially_pushed_(state.size() + offset)
    {}

    /// @brief Resets the top to the initial one, effectively popping all leftover elements.
    ~ScopedStack()
    {
        // accidentally popped values cannot be recreated and would simply be filled with nil
        // therefore this sanity check
        assert(state_.size() >= initially_pushed_);
        state_.pop(state_.size() - initially_pushed_);
    }

private:
    detail::StateBase& state_;
    int initially_pushed_;
};

template <typename TSignature>
struct FunctionUnsafe {
    FunctionUnsafe(std::function<TSignature> func)
        : func(std::move(func))
    {}

    std::function<TSignature> func;
};

template <typename TFunc>
auto functionUnsafe(TFunc&& func)
{
    return FunctionUnsafe(std::function(std::forward<TFunc>(func)));
}

template <typename TSignature>
struct FunctionReturnException {
    FunctionReturnException(std::function<TSignature> func)
        : func(std::move(func))
    {}

    std::function<TSignature> func;
};

template <typename TFunc>
auto functionReturnException(TFunc&& func)
{
    return FunctionReturnException(std::function(std::forward<TFunc>(func)));
}

template <typename TFunctionType, auto v_func>
inline int wrapUnsafeHelper(lua_State* state)
{
    using Info = detail::SignatureInfo<TFunctionType>;

    if constexpr (Info::any_state_args) {
        StateRef lua(state);
        auto old_top = lua.size();
        auto&& args = Info::convertArguments(lua);

        // convertArguments calls maxFuncArg for StackIndex and StackIndices, which updates the internal size
        if constexpr (Info::any_fixed_size_stack_args) {
            if (old_top != lua.size()) {
                // It should only increase
                assert(lua.size() > old_top);
                // Fill the rest with nil
                lua.ensurePushable(lua.size() - old_top);
                lua_settop(state, lua.size());
            }
        }

        if constexpr (std::is_void_v<typename Info::Return>) {
            std::apply(v_func, std::move(args));
            return 0;
        }
        else {
            return lua.push(std::apply(v_func, std::move(args))).size();
        }
    }
    else {
        auto&& args = Info::convertArgumentsRaw(state);
        if constexpr (std::is_void_v<typename Info::Return>) {
            std::apply(v_func, std::move(args));
            return 0;
        }
        else {
            auto&& result = std::apply(v_func, std::move(args));
            using ConvertResult = Convert<std::remove_reference_t<decltype(result)>>;
            if constexpr (ConvertResult::push_count) {
                constexpr auto push_count = *ConvertResult::push_count;
                if constexpr (push_count > LUA_MINSTACK)
                    luaL_checkstack(state, push_count, nullptr);
                ConvertResult::push(state, std::move(result));
                return push_count;
            }
            else {
                auto push_count = ConvertResult::getPushCount(result);
                if (push_count > LUA_MINSTACK)
                    luaL_checkstack(state, push_count, nullptr);
                ConvertResult::push(state, std::move(result));
                return push_count;
            }
        }
    }
}

template <auto v_func, typename TCovariantClass>
inline int wrapUnsafe(lua_State* state)
{
    if constexpr (std::is_same_v<TCovariantClass, void>)
        return wrapUnsafeHelper<decltype(v_func), v_func>(state);
    else
        return wrapUnsafeHelper<dutils::covariant_member_pointer_t<decltype(v_func), TCovariantClass>, v_func>(state);
}

template <auto v_func, typename TCovariantClass>
inline int wrap(lua_State* state)
{
    try {
        return wrapUnsafe<v_func, TCovariantClass>(state);
    }
    catch (const std::exception& e) {
        noreturn_luaL_error(state, e.what());
    }
}

template <auto v_func, typename TCovariantClass>
inline int wrapReturnException(lua_State* state)
{
    try {
        return wrapUnsafe<v_func, TCovariantClass>(state);
    }
    catch (const std::exception& e) {
        luaL_pushfail(state);
        lua_pushstring(state, e.what());
        return 2;
    }
}

template <auto v_func, typename TCovariantClass>
inline constexpr luaL_Reg regUnsafe(const char* name)
{
    return {name, wrapUnsafe<v_func, TCovariantClass>};
}

template <auto v_func, typename TCovariantClass>
inline constexpr luaL_Reg reg(const char* name)
{
    return {name, wrap<v_func, TCovariantClass>};
}

template <auto v_func, typename TCovariantClass>
inline constexpr luaL_Reg regReturnException(const char* name)
{
    return {name, wrapReturnException<v_func, TCovariantClass>};
}

template <auto v_field, typename TCovariantClass = void>
constexpr auto wrapSetFunc()
{
    using MemberPointerClass = dutils::member_pointer_class_t<decltype(v_field)>;
    using Class = std::conditional_t<std::is_same_v<TCovariantClass, void>, MemberPointerClass, TCovariantClass>;
    static_assert(std::is_convertible_v<Class*, MemberPointerClass*>, "Given class is not covariant.");
    using Value = dutils::member_pointer_type_t<decltype(v_field)>;
    return +[](Class& object, const Value& value) { object.*v_field = value; };
}

template <auto v_field, typename TCovariantClass>
inline int wrapSetUnsafe(lua_State* state)
{
    return wrapUnsafe<wrapSetFunc<v_field, TCovariantClass>()>(state);
}

template <auto v_field, typename TCovariantClass>
inline int wrapSet(lua_State* state)
{
    return wrap<wrapSetFunc<v_field, TCovariantClass>()>(state);
}

template <auto v_field, typename TCovariantClass>
inline constexpr luaL_Reg regSetUnsafe(const char* name)
{
    return {name, wrapSetUnsafe<v_field, TCovariantClass>};
}

template <auto v_field, typename TCovariantClass>
inline constexpr luaL_Reg regSet(const char* name)
{
    return {name, wrapSet<v_field, TCovariantClass>};
}

template <auto v_field, typename TCovariantClass>
inline constexpr Property fieldUnsafe(const char* name)
{
    return {name, wrapUnsafe<v_field, TCovariantClass>, wrapSetUnsafe<v_field, TCovariantClass>};
}

template <auto v_field, typename TCovariantClass>
inline constexpr Property field(const char* name)
{
    return {name, wrap<v_field, TCovariantClass>, wrapSet<v_field, TCovariantClass>};
}

// --- Iteration Wrappers ---

/// @brief Used as a non-polymorphic base class for table iterators.
template <typename TValue>
class table_iterator_base {
public:
    using difference_type = lua_Integer;
    using value_type = TValue;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::input_iterator_tag;

    table_iterator_base() = default;

    table_iterator_base(detail::StateBase& state, int table_index)
        : state_(&state)
        , table_index_(table_index)
    {}

    auto& state() const { return *state_; }

    auto table() const { return state().stackIndex(table_index_); }

private:
    detail::StateBase* state_ = nullptr;
    int table_index_;
};

/// @brief Uses lua_next to iterate over a table.
template <typename TValue>
class next_iterator : public table_iterator_base<TValue> {
public:
    friend class next_pair_iterator;
    friend class next_key_iterator;
    friend class next_value_iterator;

    next_iterator() = default;

    next_iterator(detail::StateBase& state, int table_index)
        : table_iterator_base<TValue>(state, table_index)
    {
        if (auto next = this->table().next(nil))
            key_index_ = next->first();
    }

    next_iterator& operator++()
    {
        assert(key_index_);
        auto diff = this->state().size() - *key_index_;
        assert(diff >= 0);
        if (diff > 0)
            this->state().pop(diff);
        if (!this->table().next(this->state().stackIndex(*key_index_).asResult()))
            key_index_.reset();
        return *this;
    }

    void operator++(int) { ++*this; }

    bool operator==(const next_iterator& other) const { return key_index_.has_value() == other.key_index_.has_value(); }

    bool operator!=(const next_iterator& other) const { return !(*this == other); }

private:
    std::optional<int> key_index_;
};

/// @brief Uses lua_next to iterate over all pairs of a table.
class next_pair_iterator : public next_iterator<StackIndicesResult<2>> {
public:
    using next_iterator<StackIndicesResult<2>>::next_iterator;

    auto operator*() const { return state().template stackIndices<2>(*key_index_).asResult(); }
};

/// @brief Uses lua_next to iterate over all keys of a table.
class next_key_iterator : public next_iterator<StackIndexResult> {
public:
    using next_iterator<StackIndexResult>::next_iterator;

    auto operator*() const { return state().stackIndex(*key_index_).asResult(); }
};

/// @brief Uses lua_next to iterate over all values of a table.
class next_value_iterator : public next_iterator<StackIndexResult> {
public:
    using next_iterator<StackIndexResult>::next_iterator;

    auto operator*() const { return state().stackIndex(*key_index_ + 1).asResult(); }
};

/// @brief Queries the table length once and iterates over the table.
template <typename TValue, bool v_raw>
class index_length_iterator : public table_iterator_base<TValue> {
public:
    friend class index_length_pair_iterator<v_raw>;
    friend class index_length_value_iterator<v_raw>;

    index_length_iterator() = default;

    index_length_iterator(detail::StateBase& state, int table_index, lua_Integer index, lua_Integer size)
        : table_iterator_base<TValue>(state, table_index)
        , top_(state.size())
        , index_(index)
        , size_(size)
    {
        doGetTable();
    }

    index_length_iterator& operator++()
    {
        assert(index_);
        auto diff = this->state().size() - top_;
        assert(diff >= 0);
        if (diff > 0)
            this->state().pop(diff);
        ++index_;
        doGetTable();
        return *this;
    }

    void operator++(int) { ++*this; }

    bool operator==(const index_length_iterator& other) const { return index_ == other.index_; }

    bool operator!=(const index_length_iterator& other) const { return !(*this == other); }

private:
    void doGetTable()
    {
        if (index_ > size_)
            return;
        if constexpr (v_raw)
            this->table().rawGetTable(index_);
        else
            this->table().getTable(index_);
    }

    int top_;
    lua_Integer index_;
    lua_Integer size_;
};

/// @brief Queries the table length once and iterates over all ipairs.
template <bool v_raw>
class index_length_pair_iterator : public index_length_iterator<std::pair<lua_Integer, StackIndexResult>, v_raw> {
public:
    using index_length_iterator<std::pair<lua_Integer, StackIndexResult>, v_raw>::index_length_iterator;

    auto operator*() const { return std::pair{this->index_, this->state().stackIndex(this->top_ + 1).asResult()}; }
};

/// @brief Queries the table length once and iterates over all ikeys.
template <bool v_raw>
class index_length_key_iterator {
public:
    using difference_type = lua_Integer;
    using value_type = lua_Integer;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::random_access_iterator_tag;

    index_length_key_iterator() = default;

    index_length_key_iterator([[maybe_unused]] detail::StateBase& state,
                              [[maybe_unused]] int table_index,
                              lua_Unsigned index,
                              [[maybe_unused]] bool is_end)
        : index_(index)
    {}

    index_length_key_iterator& operator++()
    {
        index_++;
        return *this;
    }

    index_length_key_iterator operator++(int)
    {
        auto old = *this;
        ++*this;
        return old;
    }

    index_length_key_iterator& operator--()
    {
        index_--;
        return *this;
    }

    index_length_key_iterator operator--(int)
    {
        auto old = *this;
        --*this;
        return old;
    }

    index_length_key_iterator& operator+=(lua_Integer count)
    {
        index_ += count;
        return *this;
    }

    index_length_key_iterator& operator-=(lua_Integer count)
    {
        index_ -= count;
        return *this;
    }

    value_type& operator[](lua_Integer offset) { return *(*this + offset); }

    bool operator==(const index_length_key_iterator& other) const { return index_ == other.index_; }
    bool operator!=(const index_length_key_iterator& other) const { return index_ != other.index_; }
    bool operator<(const index_length_key_iterator& other) const { return index_ < other.index_; }
    bool operator<=(const index_length_key_iterator& other) const { return index_ <= other.index_; }
    bool operator>(const index_length_key_iterator& other) const { return index_ > other.index_; }
    bool operator>=(const index_length_key_iterator& other) const { return index_ >= other.index_; }

    const value_type& operator*() const { return index_; }
    const value_type* operator->() const { return &index_; }

private:
    value_type index_;
};

/// @brief Queries the table length once and iterates over all ivalues.
template <bool v_raw>
class index_length_value_iterator : public index_length_iterator<StackIndexResult, v_raw> {
public:
    using index_length_iterator<StackIndexResult, v_raw>::index_length_iterator;

    auto operator*() const { return this->state().stackIndex(this->top_ + 1).asResult(); }
};

/// @brief Similar to ipairs, iterates over the table until nil is found.
template <typename TValue, bool v_raw>
class index_while_iterator : public table_iterator_base<TValue> {
public:
    friend class index_while_pair_iterator<v_raw>;
    friend class index_while_key_iterator<v_raw>;
    friend class index_while_value_iterator<v_raw>;

    index_while_iterator() = default;

    index_while_iterator(detail::StateBase& state, int table_index)
        : table_iterator_base<TValue>(state, table_index)
        , top_(state.size())
        , index_(1)
    {
        doGetTable();
    }

    index_while_iterator& operator++()
    {
        assert(index_);
        auto diff = this->state().size() - top_;
        assert(diff >= 0);
        if (diff > 0)
            this->state().pop(diff);
        ++*index_;
        doGetTable();
        return *this;
    }

    void operator++(int) { ++*this; }

    bool operator==(const index_while_iterator& other) const { return index_.has_value() == other.index_.has_value(); }

    bool operator!=(const index_while_iterator& other) const { return !(*this == other); }

private:
    void doGetTable()
    {
        auto [type, index] = [&] {
            if constexpr (v_raw)
                return this->table().rawGetTableWithType(*index_);
            else
                return this->table().getTableWithType(*index_);
        }();

        if (type == Type::Nil)
            index_ = std::nullopt;
    }

    int top_;
    std::optional<lua_Integer> index_;
};

/// @brief Similar to ipairs, iterates over all ipairs until nil is found.
template <bool v_raw>
class index_while_pair_iterator : public index_while_iterator<std::pair<lua_Integer, StackIndexResult>, v_raw> {
public:
    using index_while_iterator<std::pair<lua_Integer, StackIndexResult>, v_raw>::index_while_iterator;

    auto operator*() const { return std::pair{*this->index_, this->state().stackIndex(this->top_ + 1).asResult()}; }
};

/// @brief Similar to ipairs, iterates over all ikeys until nil is found.
template <bool v_raw>
class index_while_key_iterator : public index_while_iterator<lua_Integer, v_raw> {
public:
    using index_while_iterator<lua_Integer, v_raw>::index_while_iterator;

    auto operator*() const { return *this->index_; }
};

/// @brief Similar to ipairs, iterates over all ivalues until nil is found.
template <bool v_raw>
class index_while_value_iterator : public index_while_iterator<StackIndexResult, v_raw> {
public:
    using index_while_iterator<StackIndexResult, v_raw>::index_while_iterator;

    auto operator*() const { return this->state().stackIndex(this->top_ + 1).asResult(); }
};

/// @brief A non-polymorphic CRTP base class for generic iteration using a generator function.
template <typename TDerived, typename TValue, int v_value_count, int v_value_offset>
class generator_iterator_base {
public:
    friend class generator_index_iterator<v_value_offset>;
    friend class generator_indices_iterator<v_value_count - v_value_offset, v_value_offset>;
    friend class generator_index_range_iterator<v_value_offset>;

    using difference_type = lua_Integer;
    using value_type = TValue;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::input_iterator_tag;

    generator_iterator_base() = default;

    /// @brief Takes up to 4 values being: `func`, `state`, `initial` and `close`.
    generator_iterator_base(detail::StateBase& state, int base_index, int input_count)
        : state_(&state)
        , base_index_(base_index)
        , input_count_(input_count)
        , value_index_(input_count >= 3 ? std::optional{base_index + 2} : std::nullopt)
    {
        if (input_count >= 4)
            state.toClose(base_index + 3);
        generateNext();
    }

    TDerived& operator++()
    {
        assert(value_index_);
        auto diff = state_->size() - *value_index_;
        assert(diff >= 0);
        if (diff > 0)
            state_->pop(diff);
        generateNext();
        return static_cast<TDerived&>(*this);
    }

    void operator++(int) { ++*this; }

    bool operator==(const generator_iterator_base& other) const
    {
        return value_index_.has_value() == other.value_index_.has_value();
    }

    bool operator!=(const generator_iterator_base& other) const { return !(*this == other); }

private:
    void generateNext()
    {
        auto values = [&] {
            switch (input_count_) {
            case 0:
                return value_index_ ? state_->call<v_value_count>(nil, nil, state_->stackIndex(*value_index_))
                                    : state_->call<v_value_count>(nil, nil, nil);
            case 1:
                return value_index_ ? state_->call<v_value_count>(
                                          state_->stackIndex(base_index_), nil, state_->stackIndex(*value_index_))
                                    : state_->call<v_value_count>(state_->stackIndex(base_index_), nil, nil);
            default:
                return value_index_ ? state_->call<v_value_count>(state_->stackIndex(base_index_),
                                                                  state_->stackIndex(base_index_ + 1),
                                                                  state_->stackIndex(*value_index_))
                                    : state_->call<v_value_count>(
                                          state_->stackIndex(base_index_), state_->stackIndex(base_index_ + 1), nil);
            }
        }();

        if (values.empty() || state_->isNil(values.first()))
            value_index_ = std::nullopt;
        else {
            if (value_index_)
                state_->remove(*value_index_);
            else
                value_index_ = values.first();
            static_cast<TDerived&>(*this).setValueCount(values.size());
        }
    }

    detail::StateBase* state_;
    int base_index_;
    int input_count_;
    std::optional<int> value_index_;
};

/// @brief Uses a generator function to iterate over a fixed number of values.
template <typename TValue, int v_value_count, int v_value_offset>
class generator_iterator
    : public generator_iterator_base<generator_iterator<TValue, v_value_count, v_value_offset>,
                                     TValue,
                                     v_value_count,
                                     v_value_offset> {
public:
    using generator_iterator_base<generator_iterator<TValue, v_value_count, v_value_offset>,
                                  TValue,
                                  v_value_count,
                                  v_value_offset>::generator_iterator_base;

    friend class generator_iterator_base<generator_iterator<TValue, v_value_count, v_value_offset>,
                                         TValue,
                                         v_value_count,
                                         v_value_offset>;

private:
    void setValueCount(int) {}
};

/// @brief Uses a generator function to iterate over a potentially varying number of values.
template <typename TValue, int v_value_offset>
class generator_iterator<TValue, LUA_MULTRET, v_value_offset>
    : public generator_iterator_base<generator_iterator<TValue, LUA_MULTRET, v_value_offset>,
                                     TValue,
                                     LUA_MULTRET,
                                     v_value_offset> {
public:
    using generator_iterator_base<generator_iterator<TValue, LUA_MULTRET, v_value_offset>,
                                  TValue,
                                  LUA_MULTRET,
                                  v_value_offset>::generator_iterator_base;

    friend class generator_iterator_base<generator_iterator<TValue, LUA_MULTRET, v_value_offset>,
                                         TValue,
                                         LUA_MULTRET,
                                         v_value_offset>;

    friend class generator_index_range_iterator<v_value_offset>;

private:
    void setValueCount(int value_count) { value_count_ = value_count; }

    int value_count_;
};

/// @brief Uses a generator function to generate a single value on each step.
template <int v_value_offset>
class generator_index_iterator : public generator_iterator<StackIndexResult, 1 + v_value_offset, v_value_offset> {
public:
    using generator_iterator<StackIndexResult, 1 + v_value_offset, v_value_offset>::generator_iterator;

    auto operator*() const { return this->state_->stackIndex(*this->value_index_ + v_value_offset).asResult(); }
};

/// @brief Uses a generator function to generate a fixed number of values on each step.
template <int v_value_count, int v_value_offset>
class generator_indices_iterator
    : public generator_iterator<StackIndicesResult<v_value_count>, v_value_count + v_value_offset, v_value_offset> {
public:
    using generator_iterator<StackIndicesResult<v_value_count>, v_value_count + v_value_offset, v_value_offset>::
        generator_iterator;

    auto operator*() const
    {
        return this->state_->template stackIndices<v_value_count>(*this->value_index_ + v_value_offset).asResult();
    }
};

/// @brief Uses a generator function to generate a varying number of values on each step.
template <int v_value_offset>
class generator_index_range_iterator : public generator_iterator<StackIndexRangeResult, LUA_MULTRET, v_value_offset> {
public:
    using generator_iterator<StackIndexRangeResult, LUA_MULTRET, v_value_offset>::generator_iterator;

    auto operator*() const
    {
        return this->state_->stackIndexRange(*this->value_index_ + v_value_offset, this->value_count_ - v_value_offset)
            .asResult();
    }
};

/// @brief Combines different iterators into a single type.
/// @remark For when the iterator is only known at runtime.
template <typename... TIterators>
class iterator_variant : public std::variant<TIterators...> {
public:
    using difference_type = lua_Integer;
    using value_type = dutils::remove_cvref_t<decltype((*std::declval<TIterators>(), ...))>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::input_iterator_tag;

    using std::variant<TIterators...>::variant;

    iterator_variant& operator++()
    {
        std::visit([](auto& iterator) { ++iterator; }, *this);
        return *this;
    }

    void operator++(int) { ++*this; }

    bool operator==(const iterator_variant& other) const
    {
        return std::visit(
            dutils::Overloaded{
                std::equal_to<TIterators>()...,
                [](...) { return false; },
            },
            *this,
            other);
    }

    bool operator!=(const iterator_variant& other) const { return !(*this == other); }

    auto operator*() const
    {
        return std::visit([](const auto& iterator) -> value_type { return *iterator; }, *this);
    }
};

/// @brief A non-polymorphic base class for different iteration wrappers.
class IterationWrapperBase {
public:
    IterationWrapperBase(detail::StateBase& state, int table_index)
        : state_(state)
        , table_index_(table_index)
    {}

protected:
    detail::StateBase& state_;
    int table_index_;

private:
    ScopedStack scoped_stack_{state_};
};

/// @brief Provides iteration functionality using the specified iterator.
template <typename TIterator>
class IterationWrapper : public IterationWrapperBase {
public:
    using IterationWrapperBase::IterationWrapperBase;

    auto begin() const { return TIterator(state_, table_index_); }
    auto end() const { return TIterator(); }
};

/// @brief Provides iteration functionality using the specified iterator, querying the table length in advance.
template <template <bool> typename TIterator, bool v_raw>
class IndexLengthIterationWrapper : public IterationWrapperBase {
public:
    using IterationWrapperBase::IterationWrapperBase;

    auto begin() const { return TIterator<v_raw>(state_, table_index_, 1, size_); }
    auto end() const { return TIterator<v_raw>(state_, table_index_, size_ + 1, size_); }

private:
    lua_Integer size_ = v_raw ? static_cast<lua_Integer>(state_.rawLength(table_index_)) : state_.length(table_index_);
};

/// @brief Provides iteration functionality for generator functions using the specified iterator.
template <typename TIterator>
class GeneratorIterationWrapper {
public:
    GeneratorIterationWrapper(detail::StateBase& state, int base_index, int input_count)
        : state_(state)
        , base_index_(base_index)
        , input_count_(input_count)
    {}

    auto begin() const { return TIterator(state_, base_index_, input_count_); }
    auto end() const { return TIterator(); }

private:
    detail::StateBase& state_;
    ScopedStack scoped_stack_{state_};
    int base_index_;
    int input_count_;
};

/// @brief Combines different iterator wrappers into a single type.
/// @remark For when the iterator is only known at runtime.
template <typename... TIterationWrappers>
class IterationWrapperVariant : public std::variant<TIterationWrappers...> {
public:
    using std::variant<TIterationWrappers...>::variant;

    using ResultType = iterator_variant<decltype(std::declval<const TIterationWrappers>().begin())...>;

    ResultType begin() const
    {
        return std::visit([](const auto& wrapper) -> ResultType { return wrapper.begin(); }, *this);
    }

    ResultType end() const
    {
        return std::visit([](const auto& wrapper) -> ResultType { return wrapper.end(); }, *this);
    }
};

// --- Convert Specializations ---

template <typename TStateRef>
struct Convert<TStateRef, std::enable_if_t<std::is_same_v<std::remove_cv_t<TStateRef>, StateRef>>> {
    // --- Check ---

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 0;

    static StateRef& check(StateRef& state, int) { return state; }

    // --- Push ---

    static constexpr bool can_push = false;
};

namespace detail {

struct ConvertAnyIndexBase {
    // --- Check ---

    static constexpr bool can_check = true;

    static constexpr bool isExact(lua_State*, int) { return true; }
    static constexpr bool isValid(lua_State*, int) { return true; }

    // --- Push ---

    static constexpr bool can_push = true;
};

template <typename TIndex>
struct ConvertIndex : ConvertAnyIndexBase {
    // --- Check ---

    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return "any"; }

    // --- Push ---

    static constexpr std::optional<int> push_count = 1;

    static std::string getPushTypename() { return getCheckTypename(); }

    static void push(lua_State* state, TIndex index)
    {
        assert(index.state().state() == state);
        lua_pushvalue(state, index.index());
    }
};

template <typename TStackIndex>
struct ConvertStackIndex : ConvertIndex<TStackIndex> {
    // --- Check ---

    static std::optional<TStackIndex> at(StateRef& state, int pos)
    {
        state.maxFuncArg(pos);
        return TStackIndex(state, pos);
    }

    static TStackIndex check(StateRef& state, int arg)
    {
        state.maxFuncArg(arg);
        return TStackIndex(state, arg);
    }
};

template <typename TIndices, int v_count>
struct ConvertIndices : ConvertAnyIndexBase {
    // --- Check ---

    static constexpr std::optional<int> check_count = v_count;

    static std::string getCheckTypename() { return "any<" + std::to_string(v_count) + ">"; }

    // --- Push ---

    static constexpr std::optional<int> push_count = v_count;

    static std::string getPushTypename() { return getCheckTypename(); }

    static void push(lua_State* state, TIndices indices)
    {
        assert(indices.state().state() == state);
        pushHelper(state, indices, std::make_index_sequence<v_count>{});
    }

private:
    template <std::size_t... v_indices>
    static void pushHelper(lua_State* state, TIndices indices, std::index_sequence<v_indices...>)
    {
        (lua_pushvalue(state, indices[v_indices].index()), ...);
    }
};

template <typename TStackIndices, int v_count>
struct ConvertStackIndices : ConvertIndices<TStackIndices, v_count> {
    // --- Check ---

    static std::optional<TStackIndices> at(StateRef& state, int pos)
    {
        state.maxFuncArg(pos + v_count - 1);
        return TStackIndices(state, pos);
    }

    static TStackIndices check(StateRef& state, int arg)
    {
        state.maxFuncArg(arg + v_count - 1);
        return TStackIndices(state, arg);
    }
};

template <typename TIndexRange>
struct ConvertIndexRange : ConvertAnyIndexBase {
    // --- Check ---

    static constexpr std::optional<int> check_count = std::nullopt;

    static std::string getCheckTypename() { return "..."; }

    // --- Push ---

    static constexpr std::optional<int> push_count = std::nullopt;

    static std::string getPushTypename() { return getCheckTypename(); }

    static void push(lua_State* state, TIndexRange index_range)
    {
        assert(index_range.state().state() == state);
        for (auto index : index_range)
            lua_pushvalue(state, index.index());
    }

    static int getPushCount(TIndexRange index_range) { return index_range.size(); }
};

template <typename TStackIndexRange>
struct ConvertStackIndexRange : ConvertIndexRange<TStackIndexRange> {
    // --- Check ---

    static std::optional<TStackIndexRange> at(StateRef& state, int pos)
    {
        return TStackIndexRange(state, pos, state.size() - pos + 1);
    }

    static TStackIndexRange check(StateRef& state, int arg)
    {
        return TStackIndexRange(state, arg, state.size() - arg + 1);
    }
};

template <typename>
struct ConvertAnyIndex;

template <typename TState, StackIndexType v_type>
struct ConvertAnyIndex<StackIndex<TState, v_type>> : ConvertStackIndex<StackIndex<TState, v_type>> {};

template <typename TState>
struct ConvertAnyIndex<RegistryIndex<TState>> : ConvertIndex<RegistryIndex<TState>> {};

template <typename TState>
struct ConvertAnyIndex<UpvalueIndex<TState>> : ConvertIndex<UpvalueIndex<TState>> {};

template <typename TState, int v_count, StackIndexType v_type>
struct ConvertAnyIndex<StackIndices<TState, v_count, v_type>>
    : ConvertStackIndices<StackIndices<TState, v_count, v_type>, v_count> {};

template <typename TState, int v_count>
struct ConvertAnyIndex<UpvalueIndices<TState, v_count>> : ConvertIndices<UpvalueIndices<TState, v_count>, v_count> {};

template <typename TState, StackIndexType v_type>
struct ConvertAnyIndex<StackIndexRange<TState, v_type>> : ConvertStackIndexRange<StackIndexRange<TState, v_type>> {};

template <typename TState>
struct ConvertAnyIndex<UpvalueIndexRange<TState>> : ConvertIndexRange<UpvalueIndexRange<TState>> {};

} // namespace detail

template <typename TIndex>
struct Convert<TIndex, std::enable_if_t<is_any_index_v<TIndex>>> : detail::ConvertAnyIndex<std::remove_cv_t<TIndex>> {};

// --- ClassInfo Specializations ---

namespace detail {

template <typename TArgsTuple, typename TResultTuple = std::tuple<>>
struct args_tuple_without_empty {
    using type = TResultTuple;
};

template <typename TFirst, typename... TRest, typename... TResults>
struct args_tuple_without_empty<std::tuple<TFirst, TRest...>, std::tuple<TResults...>>
    : args_tuple_without_empty<std::tuple<TRest...>,
                               std::conditional_t<convert_checks_exactly_v<TFirst, 0>,
                                                  std::tuple<TResults...>,
                                                  std::tuple<TResults..., TFirst>>> {};

template <typename TRet, typename... TArgs>
static std::string formatFunctionSignature()
{
    return "function(" +
           Convert<typename args_tuple_without_empty<std::tuple<std::remove_reference_t<TArgs>...>>::type>::
               getPushTypename() +
           ")" + [] {
               if constexpr (std::is_void_v<TRet>)
                   return std::string();
               else
                   return " -> " + std::string(Convert<std::remove_reference_t<TRet>>::getPushTypename());
           }();
}

} // namespace detail

template <typename TRet, typename... TArgs>
struct ClassInfo<std::function<TRet(TArgs...)>> : DefaultClassInfo {
    using Signature = TRet(TArgs...);

    static std::string getCheckTypename() { return detail::formatFunctionSignature<TRet, TArgs...>(); }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr auto metamethods()
    {
        constexpr auto call =
            +[](const std::function<Signature>& func, TArgs... args) { return func(std::forward<TArgs>(args)...); };
        return std::array{reg<call>("__call")};
    }
};

template <typename TRet, typename... TArgs>
struct ClassInfo<FunctionUnsafe<TRet(TArgs...)>> : DefaultClassInfo {
    using Signature = TRet(TArgs...);

    static std::string getCheckTypename() { return detail::formatFunctionSignature<TRet, TArgs...>(); }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr auto metamethods()
    {
        constexpr auto call = +[](const FunctionUnsafe<Signature>& function_unsafe, TArgs... args) {
            return function_unsafe.func(std::forward<TArgs>(args)...);
        };
        return std::array{regUnsafe<call>("__call")};
    }
};

template <typename TRet, typename... TArgs>
struct ClassInfo<FunctionReturnException<TRet(TArgs...)>> : DefaultClassInfo {
    using Signature = TRet(TArgs...);

    static std::string getCheckTypename() { return detail::formatFunctionSignature<TRet, TArgs...>(); }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr auto metamethods()
    {
        constexpr auto call = +[](const FunctionReturnException<Signature>& function_return_exception, TArgs... args) {
            return function_return_exception.func(std::forward<TArgs>(args)...);
        };
        return std::array{regReturnException<call>("__call")};
    }
};

// --- Utility ---

/// @brief Function object that checks Lua arguments for a certain type.
template <typename T>
struct ArgCheck {
    template <typename TArg>
    auto operator()(TArg&& arg) const
    {
        return std::forward<TArg>(arg).template check<T>();
    }
};

/// @brief Function object that converts Lua values into an optional of the given type.
template <typename T>
struct ValueTo {
    template <typename TIndex>
    auto operator()(TIndex&& index) const
    {
        return std::forward<TIndex>(index).template to<T>();
    }
};

/// @brief Can be used to provide __pairs for custom ClassInfo specializations.
inline auto indextable_pairs(Arg value)
{
    constexpr auto next = +[](StateRef& lua, Arg table, Arg key) {
        auto result = table.next(std::move(key));
        return result ? VarArgs(*result) : VarArgs(lua.pushNil());
    };
    return std::tuple{&wrap<next>, value.getMetafield("indextable")};
}

namespace detail {

/// @brief Requires either of the two arguments to be an index and returns the associated state.
template <typename TLeft, typename TRight>
inline auto& stateOf(TLeft& lhs, TRight& rhs)
{
    if constexpr (is_index_v<TLeft>)
        return lhs.state();
    else
        return rhs.state();
}

/// @brief Whether any argument is any index type.
template <typename... TArgs>
struct any_index : std::disjunction<is_any_index<TArgs>...> {};

template <typename... TArgs>
inline constexpr auto any_index_v = any_index<TArgs...>::value;

/// @brief Whether all arguments are checkable using Convert.
template <typename... TArgs>
struct all_pushable : std::conjunction<convert_can_push<TArgs>...> {};

template <typename... TArgs>
inline constexpr auto all_pushable_v = all_pushable<TArgs...>::value;

/// @brief If any index type is present and the other types are pushable using Convert.
template <typename... TArgs>
inline constexpr auto op_arguments_valid = (any_index_v<std::remove_reference_t<TArgs>...> &&
                                            all_pushable_v<std::remove_reference_t<TArgs>...>);
// Parentheses above are just for formatting...

/// @brief Enable-if for arithmetic operations.
template <typename... TArgs>
using EnableArith = std::enable_if_t<op_arguments_valid<TArgs...>, dang::lua::StackIndexResult>;

/// @brief Enable-if for compare operations.
template <typename... TArgs>
using EnableCompare = std::enable_if_t<op_arguments_valid<TArgs...>, bool>;

template <typename TLeft, typename TRight>
inline auto operator+(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Add>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator-(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Sub>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator*(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Mul>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator%(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Mod>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

// Would be ambiguous with binary xor
/*
template <typename TLeft, typename TRight>
inline auto operator^(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Pow>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator/(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::Div>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

// Integer division works different in C++
/*
template <typename TLeft, typename TRight>
inline auto operator//(TLeft&& lhs, TRight&& rhs) -> EnableIfAnyIndex<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::IDiv>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator&(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::BinaryAnd>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator|(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::BinaryOr>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

// Would be ambiguous with pow
/*
template <typename TLeft, typename TRight>
inline auto operator^(TLeft&& lhs, TRight&& rhs) ->EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::BinaryXOr>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}
*/

template <typename TLeft, typename TRight>
inline auto operator<<(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::LeftShift>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename TLeft, typename TRight>
inline auto operator>>(TLeft&& lhs, TRight&& rhs) -> EnableArith<TLeft, TRight>
{
    return stateOf(lhs, rhs).template arith<ArithOp::RightShift>(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
}

template <typename T>
inline auto operator-(T&& operand) -> EnableArith<T>
{
    return operand.stack().template arith<ArithOp::UnaryMinus>(std::forward<T>(operand));
}

template <typename T>
inline auto operator~(T&& operand) -> EnableArith<T>
{
    return operand.stack().template arith<ArithOp::BinaryNot>(std::forward<T>(operand));
}

template <typename TLeft, typename TRight>
inline auto operator==(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return stateOf(left, right).compare(CompareOp::Equal, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator!=(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return !(left == right);
}

template <typename TLeft, typename TRight>
inline auto operator<(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return stateOf(left, right).compare(CompareOp::LessThan, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator<=(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return stateOf(left, right).compare(CompareOp::LessEqual, std::forward<TLeft>(left), std::forward<TRight>(right));
}

template <typename TLeft, typename TRight>
inline auto operator>(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return right < left;
}

template <typename TLeft, typename TRight>
inline auto operator>=(TLeft&& left, TRight&& right) -> EnableCompare<TLeft, TRight>
{
    return right <= left;
}

// --- StateBase Implementation ---

inline PairsIterationWrapper StateBase::pairs(int index)
{
    if (auto pairs = getMetafield(index, "__pairs")) {
        auto result = std::move(*pairs).call<3>(stackIndex(index));
        return GeneratorIterationWrapper<generator_indices_iterator<2, 0>>(*this, result.first(), result.size());
    }
    return pairsRaw(index);
}

inline KeysIterationWrapper StateBase::keys(int index)
{
    if (auto pairs = getMetafield(index, "__pairs")) {
        auto result = std::move(*pairs).call<3>(stackIndex(index));
        return GeneratorIterationWrapper<generator_index_iterator<0>>(*this, result.first(), result.size());
    }
    return keysRaw(index);
}

inline ValuesIterationWrapper StateBase::values(int index)
{
    if (auto pairs = getMetafield(index, "__pairs")) {
        auto result = std::move(*pairs).call<3>(stackIndex(index));
        return GeneratorIterationWrapper<generator_index_iterator<1>>(*this, result.first(), result.size());
    }
    return valuesRaw(index);
}

inline PairsRawIterationWrapper StateBase::pairsRaw(int index) { return {*this, index}; }

inline KeysRawIterationWrapper StateBase::keysRaw(int index) { return {*this, index}; }

inline ValuesRawIterationWrapper StateBase::valuesRaw(int index) { return {*this, index}; }

inline IPairsIterationWrapper StateBase::ipairs(int index) { return {*this, index}; }

inline IKeysIterationWrapper StateBase::ikeys(int index) { return {*this, index}; }

inline IValuesIterationWrapper StateBase::ivalues(int index) { return {*this, index}; }

inline IPairsLenIterationWrapper StateBase::ipairsLen(int index) { return {*this, index}; }

inline IKeysLenIterationWrapper StateBase::ikeysLen(int index) { return {*this, index}; }

inline IValuesLenIterationWrapper StateBase::ivaluesLen(int index) { return {*this, index}; }

inline IPairsRawIterationWrapper StateBase::ipairsRaw(int index) { return {*this, index}; }

inline IKeysRawIterationWrapper StateBase::ikeysRaw(int index) { return {*this, index}; }

inline IValuesRawIterationWrapper StateBase::ivaluesRaw(int index) { return {*this, index}; }

} // namespace detail

/*
#ifdef _DEBUG
namespace detail::force_include
{

// forcefully include these functions for debugger visualization
extern const auto State_formatDebug = &StateBase::formatDebug;

}
#endif
*/

} // namespace dang::lua

namespace std {

template <typename TState, int v_count, dang::lua::detail::StackIndexType v_type>
struct tuple_size<dang::lua::detail::StackIndices<TState, v_count, v_type>>
    : std::integral_constant<std::size_t, v_count> {};

template <std::size_t v_index, typename TState, int v_count, dang::lua::detail::StackIndexType v_type>
struct tuple_element<v_index, dang::lua::detail::StackIndices<TState, v_count, v_type>> {
    using type = dang::lua::detail::StackIndex<TState, dang::lua::detail::StackIndexType::Result>;
};

template <typename TState, int v_count>
struct tuple_size<dang::lua::detail::UpvalueIndices<TState, v_count>> : std::integral_constant<std::size_t, v_count> {};

template <std::size_t v_index, typename TState, int v_count>
struct tuple_element<v_index, dang::lua::detail::UpvalueIndices<TState, v_count>> {
    using type = dang::lua::detail::UpvalueIndex<TState>;
};

} // namespace std
