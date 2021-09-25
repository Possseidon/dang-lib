#pragma once

#include "dang-lua/Convert.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief Wraps a reference to a Lua value, that lives in the registry table.
/// @remark Uses luaL_ref and luaL_unref from the auxiliary library.
class Reference {
public:
    /// @brief Default constructs an invalid reference with a nullptr state.
    Reference() = default;

    /// @brief Turns the top of the stack into a reference, popping the value in the process.
    static Reference consume(lua_State* state) { return Reference(state); }
    /// @brief The Lua state must not be null, use the default constructor for empty references.
    static Reference consume(std::nullptr_t) = delete;

    /// @brief Removes the reference from the registry table.
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

    /// @brief Whether the reference is valid.
    explicit operator bool() const { return state_ != nullptr; }

    /// @brief The with the reference associated Lua state.
    lua_State* state() const { return state_; }

    /// @brief Pushes the referenced value on the stack.
    void push() const
    {
        assert(*this);
        lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
    }

private:
    /// @brief Turns the top of the stack into a reference, popping the value in the process.
    explicit Reference(lua_State* state)
        : state_(state)
        , ref_(luaL_ref(state, LUA_REGISTRYINDEX))
    {}

    lua_State* state_ = nullptr;
    int ref_ = LUA_NOREF;
};

// UniqueReference not necessary. Reference is very cheap to move on its own.

/// @brief Allows for easy sharing of the same reference.
using SharedReference = std::shared_ptr<Reference>;
using WeakReference = std::weak_ptr<Reference>;

template <typename TReference>
struct Convert<TReference, std::enable_if_t<std::is_same_v<std::remove_cv_t<TReference>, Reference>>> {
    static constexpr std::optional<int> push_count = 1;
    static constexpr bool allow_nesting = true;

    static void push([[maybe_unused]] lua_State* state, const Reference& reference)
    {
        assert(reference.state() == state);
        reference.push();
    }
};

} // namespace dang::lua
