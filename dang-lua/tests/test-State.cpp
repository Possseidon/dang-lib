#include "dang-lua/State.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

// --- Indices

using StackIndexTypes = maybe_cv<dlua::StackIndex, dlua::ConstStackIndex>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndex has correct type traits.", "[lua][index][type-traits]", StackIndexTypes)
{
    using StackIndex = TestType;

    STATIC_REQUIRE(dlua::is_index_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<StackIndex>);

    STATIC_REQUIRE(dlua::is_stack_index_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<StackIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndex>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndex>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<StackIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndex&&>);

    STATIC_REQUIRE(dlua::is_fixed_size_stack_index_v<StackIndex>);
}

using StackIndexResultTypes = maybe_cv<dlua::StackIndexResult, dlua::ConstStackIndexResult>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndexResult has correct type traits.",
                        "[lua][index][type-traits]",
                        StackIndexResultTypes)
{
    using StackIndexResult = TestType;

    STATIC_REQUIRE(dlua::is_index_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<StackIndexResult>);

    STATIC_REQUIRE(dlua::is_stack_index_v<StackIndexResult>);
    STATIC_REQUIRE(dlua::is_stack_index_result_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<StackIndexResult>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndexResult>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndexResult>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndexResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndexResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_result_v<StackIndexResult>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndexResult>);
    STATIC_REQUIRE(dlua::is_any_moved_stack_index_result_v<StackIndexResult&&>);

    STATIC_REQUIRE(dlua::is_fixed_size_stack_index_v<StackIndexResult>);
}

using RegistryIndexTypes = maybe_cv<dlua::RegistryIndex, dlua::ConstRegistryIndex>;
TEMPLATE_LIST_TEST_CASE("Lua RegistryIndex has correct type traits.", "[lua][index][type-traits]", RegistryIndexTypes)
{
    using RegistryIndex = TestType;

    STATIC_REQUIRE(dlua::is_index_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<RegistryIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<RegistryIndex>);

    STATIC_REQUIRE(dlua::is_pseudo_index_v<RegistryIndex>);
    STATIC_REQUIRE(dlua::is_registry_index_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<RegistryIndex>);

    STATIC_REQUIRE(dlua::is_any_index_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<RegistryIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<RegistryIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<RegistryIndex&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<RegistryIndex>);
}

using UpvalueIndexTypes = maybe_cv<dlua::UpvalueIndex, dlua::ConstUpvalueIndex>;
TEMPLATE_LIST_TEST_CASE("Lua UpvalueIndex has correct type traits.", "[lua][index][type-traits]", UpvalueIndexTypes)
{
    using UpvalueIndex = TestType;

    STATIC_REQUIRE(dlua::is_index_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<UpvalueIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<UpvalueIndex>);

    STATIC_REQUIRE(dlua::is_pseudo_index_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<UpvalueIndex>);
    STATIC_REQUIRE(dlua::is_upvalue_index_v<UpvalueIndex>);

    STATIC_REQUIRE(dlua::is_any_index_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<UpvalueIndex>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndex>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndex&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<UpvalueIndex>);
}

using StackIndicesTypes = maybe_cv<dlua::StackIndices<3>, dlua::ConstStackIndices<3>>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndices has correct type traits.", "[lua][index][type-traits]", StackIndicesTypes)
{
    using StackIndices = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<StackIndices>);
    STATIC_REQUIRE(dlua::is_indices_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<StackIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<StackIndices>);
    STATIC_REQUIRE(dlua::is_stack_indices_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<StackIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndices>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndices>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<StackIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndices&&>);

    STATIC_REQUIRE(dlua::is_fixed_size_stack_index_v<StackIndices>);
}

using StackIndicesResultTypes = maybe_cv<dlua::StackIndicesResult<3>, dlua::ConstStackIndicesResult<3>>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndicesResult has correct type traits.",
                        "[lua][index][type-traits]",
                        StackIndicesResultTypes)
{
    using StackIndicesResult = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_indices_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<StackIndicesResult>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_stack_indices_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_stack_indices_result_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<StackIndicesResult>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndicesResult>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndicesResult>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_result_v<StackIndicesResult>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndicesResult>);
    STATIC_REQUIRE(dlua::is_any_moved_stack_index_result_v<StackIndicesResult&&>);

    STATIC_REQUIRE(dlua::is_fixed_size_stack_index_v<StackIndicesResult>);
}

using UpvalueIndicesTypes = maybe_cv<dlua::UpvalueIndices<3>, dlua::ConstUpvalueIndices<3>>;
TEMPLATE_LIST_TEST_CASE("Lua UpvalueIndices has correct type traits.", "[lua][index][type-traits]", UpvalueIndicesTypes)
{
    using UpvalueIndices = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<UpvalueIndices>);
    STATIC_REQUIRE(dlua::is_indices_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_index_range_v<UpvalueIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<UpvalueIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<UpvalueIndices>);

    STATIC_REQUIRE(dlua::is_any_index_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<UpvalueIndices>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndices>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndices&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<UpvalueIndices>);
}

using StackIndexRangeTypes = maybe_cv<dlua::StackIndexRange, dlua::ConstStackIndexRange>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndexRange has correct type traits.",
                        "[lua][index][type-traits]",
                        StackIndexRangeTypes)
{
    using StackIndexRange = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<StackIndexRange>);
    STATIC_REQUIRE(dlua::is_index_range_v<StackIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<StackIndexRange>);
    STATIC_REQUIRE(dlua::is_stack_index_range_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<StackIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndexRange>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndexRange>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<StackIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndexRange&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<StackIndexRange>);
}

using StackIndexRangeResultTypes = maybe_cv<dlua::StackIndexRangeResult, dlua::ConstStackIndexRangeResult>;
TEMPLATE_LIST_TEST_CASE("Lua StackIndexRangeResult has correct type traits.",
                        "[lua][index][type-traits]",
                        StackIndexRangeResultTypes)
{
    using StackIndexRangeResult = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_index_range_v<StackIndexRangeResult>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_stack_index_range_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_stack_index_range_result_v<StackIndexRangeResult>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<StackIndexRangeResult>);

    STATIC_REQUIRE(dlua::is_any_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_any_stack_index_result_v<StackIndexRangeResult>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<StackIndexRangeResult>);
    STATIC_REQUIRE(dlua::is_any_moved_stack_index_result_v<StackIndexRangeResult&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<StackIndexRangeResult>);
}

using UpvalueIndexRangeTypes = maybe_cv<dlua::UpvalueIndexRange, dlua::ConstUpvalueIndexRange>;
TEMPLATE_LIST_TEST_CASE("Lua UpvalueIndexRange has correct type traits.",
                        "[lua][index][type-traits]",
                        UpvalueIndexRangeTypes)
{
    using UpvalueIndexRange = TestType;

    STATIC_REQUIRE_FALSE(dlua::is_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_indices_v<UpvalueIndexRange>);
    STATIC_REQUIRE(dlua::is_index_range_v<UpvalueIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_stack_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_result_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_indices_result_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_stack_index_range_result_v<UpvalueIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_pseudo_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_registry_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_upvalue_index_v<UpvalueIndexRange>);

    STATIC_REQUIRE(dlua::is_any_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_any_stack_index_result_v<UpvalueIndexRange>);

    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndexRange>);
    STATIC_REQUIRE_FALSE(dlua::is_any_moved_stack_index_result_v<UpvalueIndexRange&&>);

    STATIC_REQUIRE_FALSE(dlua::is_fixed_size_stack_index_v<UpvalueIndexRange>);
}

// --- State

TEST_CASE("Lua State can be constructed from the lua_State passed to a C function.", "[lua][state]")
{
    LuaState owned_lua_state;

    auto pushed = GENERATE(as<std::size_t>{}, 0, 1, 5);

    for (std::size_t i = 0; i < pushed; i++)
        lua_pushinteger(*owned_lua_state, i);

    // Simulate a proper call, allowing LUA_MINSTACK (20) elements to be pushed safely.
    // See below for more info.
    luaL_checkstack(*owned_lua_state, LUA_MINSTACK, nullptr);

    // State is meant exclusively for when Lua calls a C function.

    // 1. Stack Size (See https://www.lua.org/manual/5.4/manual.html#4.1.1)
    // Lua ensures, that LUA_MINSTACK (20) elements can be safely pushed.
    // When State is created it assumes these 20 elements can safely be pushed.
    // Unless NDEBUG is set, State keeps track of the stack size and asserts, that the stack doesn't overflow.

    // 2. Pushed Elements
    // When a C function is called, a variable number of elements will lie on the stack.
    // Creating a State will therefore query the stack size once and store it.
    // Any operations on the State will automatically update without any further size queries.
    // This should allow for better optimizations and avoid many calls to lua_gettop.
    // However each call always calls lua_gettop exactly once, which might not have been necessary.
    // Wrapped functions that do not use any State or Arg parameter do not have this overhead.

    auto lua = dlua::State(*owned_lua_state);

    CHECK(lua.state() == *owned_lua_state);
    CHECK(lua.size() == pushed);
}

TEST_CASE("Lua State can be moved.", "[lua][state]")
{
    LuaState owned_lua_state;
    lua_pushinteger(*owned_lua_state, 42);
    luaL_checkstack(*owned_lua_state, LUA_MINSTACK, nullptr);

    auto lua = dlua::State(*owned_lua_state);

    SECTION("Using move-constructor.")
    {
        auto moved_lua = std::move(lua);
        CHECK(moved_lua.state() == *owned_lua_state);
        CHECK(moved_lua.size() == 1);
    }
    SECTION("Using move-assignment.")
    {
        LuaState other_lua_state;

        auto moved_lua = dlua::State(*other_lua_state);
        moved_lua = std::move(lua);
        CHECK(moved_lua.state() == *owned_lua_state);
        CHECK(moved_lua.size() == 1);
    }
}

TEST_CASE("Lua State can be swapped.", "[lua][state]")
{
    LuaState owned_lua_state1;
    lua_pushinteger(*owned_lua_state1, 1);
    luaL_checkstack(*owned_lua_state1, LUA_MINSTACK, nullptr);

    LuaState owned_lua_state2;
    lua_pushinteger(*owned_lua_state2, 2);
    lua_pushinteger(*owned_lua_state2, 2);
    luaL_checkstack(*owned_lua_state2, LUA_MINSTACK, nullptr);

    auto lua1 = dlua::State(*owned_lua_state1);
    auto lua2 = dlua::State(*owned_lua_state2);

    SECTION("Using swap member function.") { lua1.swap(lua2); }
    SECTION("Using swap friend function.") { swap(lua1, lua2); }

    CHECK(lua1.state() == *owned_lua_state2);
    CHECK(lua1.size() == 2);
    CHECK(lua2.state() == *owned_lua_state1);
    CHECK(lua2.size() == 1);
}

TEST_CASE("Lua State's underlying state can be checked and extracted.", "[lua][state]")
{
    LuaState owned_lua_state;

    auto lua = dlua::State(*owned_lua_state);

    CHECK(lua.state() == *owned_lua_state);
    CHECK(std::move(lua).state() == *owned_lua_state);
}

// --- OwnedState

// --- State and OwnedState
