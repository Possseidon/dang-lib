#include <memory>
#include <optional>

#include "dang-lua/Allocator.h"
#include "dang-lua/State.h"

#include "catch2/catch.hpp"
#include "lua.hpp"
#include "shared/CheckedAllocator.h"
#include "shared/LuaEnums.h"
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

// --- StateBase (using State)

namespace {

int dummyPanicFunction(lua_State*) { return 0; }

} // namespace

TEST_CASE("Lua StateBase can check properties of the state itself.", "[lua][state]")
{
    auto lua = dlua::State();

    CHECK(lua.version() == LUA_VERSION_NUM);
    lua.checkVersion();

    CHECK(lua.status() == dlua::Status::Ok);
    CHECK_FALSE(lua.isYieldable());

    auto old_panic_function = lua.replacePanicFunction(dummyPanicFunction);
    CHECK(lua.replacePanicFunction(old_panic_function) == dummyPanicFunction);

    auto data = std::make_unique<int>();
    lua.extraspace() = data.get();
    CHECK(lua.extraspace() == data.get());
}

TEST_CASE("Lua StateBase can query and switch out the allocator.", "[lua][state]")
{
    // This test is a bit questionable as it relies on how Lua allocates things.
    // If this ever breaks for "no reason" it is probably a good idea to simplify or remove it.
    // Hot swapping an allocator isn't something very common anyway.

    auto lua = dlua::State();
    auto checked_allocator = CheckedAllocator();

    // Make sure all garbage is cleaned up.
    lua.gcCollect();

    // Hot swap the allocator.
    auto old_allocator = lua.getAllocator();
    lua.setAllocator(checked_allocator.allocator());

    // Allocate a table.
    lua.pushTable();
    checked_allocator.checkNotEmpty();

    // Remove the table and let the gc deallocate it.
    lua.pop();
    lua.gcCollect();
    checked_allocator.checkEmpty();

    // Put back the old allocator.
    lua.setAllocator(old_allocator);
}

TEST_CASE("Lua StateBase can work with the garbage collector.", "[lua][state]")
{
    auto lua = dlua::State();

    SECTION("A full garbage-collection cycle can be triggered.") { lua.gcCollect(); }
    SECTION("A single garbage-collection step can be triggered.") { lua.gcStep(1); }
    SECTION("It can be stopped and restarted.")
    {
        CHECK(lua.gcIsRunning());
        lua.gcStop();
        CHECK_FALSE(lua.gcIsRunning());
        lua.gcRestart();
        CHECK(lua.gcIsRunning());
    }
    SECTION("The current memory in use can be queried.")
    {
        auto bytes = lua.gcCount() * 1024 + lua.gcCountBytes();
        CHECK(bytes > 0);
    }
    SECTION("It can be switched between generational and incremental mode.")
    {
        lua.gcGenerational(0, 0);
        CHECK(lua.gcIncremental(0, 0, 0) == dlua::GCOption::Generational);
    }
}

TEST_CASE("Lua StateBase can wrap Index types for intuitive usage.", "[lua][state]")
{
    auto lua = dlua::State();
    lua.padWithNil(5);

    SECTION("Index Wrapping")
    {
        auto positive_index = lua.stackIndex(2);
        using PositiveIndex = decltype(positive_index);
        CHECK(positive_index.index() == 2);
        CHECK(positive_index.first() == 2);
        CHECK(positive_index.last() == 2);
        STATIC_REQUIRE(PositiveIndex::size() == 1);
        STATIC_REQUIRE_FALSE(PositiveIndex::empty());

        auto negative_index = lua.stackIndex(-2);
        using NegativeIndex = decltype(negative_index);
        CHECK(negative_index.index() == 4);
        CHECK(negative_index.first() == 4);
        CHECK(negative_index.last() == 4);
        STATIC_REQUIRE(NegativeIndex::size() == 1);
        STATIC_REQUIRE_FALSE(NegativeIndex::empty());

        auto registry_index = lua.registry();
        using RegistryIndex = decltype(registry_index);
        STATIC_REQUIRE(RegistryIndex::index() == LUA_REGISTRYINDEX);
        STATIC_REQUIRE(RegistryIndex::first() == LUA_REGISTRYINDEX);
        STATIC_REQUIRE(RegistryIndex::last() == LUA_REGISTRYINDEX);
        STATIC_REQUIRE(RegistryIndex::size() == 1);
        STATIC_REQUIRE_FALSE(RegistryIndex::empty());

        auto upvalue_index = lua.upvalue(3);
        using UpvalueIndex = decltype(upvalue_index);
        CHECK(upvalue_index.index() == lua_upvalueindex(3));
        CHECK(upvalue_index.first() == lua_upvalueindex(3));
        CHECK(upvalue_index.last() == lua_upvalueindex(3));
        STATIC_REQUIRE(UpvalueIndex::size() == 1);
        STATIC_REQUIRE_FALSE(UpvalueIndex::empty());
    }
    SECTION("Indices Wrapping")
    {
        auto positive_indices = lua.stackIndices<2>(2);
        using PositiveIndices = decltype(positive_indices);
        CHECK(positive_indices.first() == 2);
        CHECK(positive_indices.last() == 3);
        STATIC_REQUIRE(PositiveIndices::size() == 2);
        STATIC_REQUIRE_FALSE(PositiveIndices::empty());

        auto negative_indices = lua.stackIndices<2>(-3);
        using NegativeIndices = decltype(negative_indices);
        CHECK(negative_indices.first() == 3);
        CHECK(negative_indices.last() == 4);
        STATIC_REQUIRE(NegativeIndices::size() == 2);
        STATIC_REQUIRE_FALSE(NegativeIndices::empty());

        auto empty_stack_indices = lua.stackIndices<0>(1);
        using EmptyStackIndices = decltype(empty_stack_indices);
        STATIC_REQUIRE(EmptyStackIndices::size() == 0);
        STATIC_REQUIRE(EmptyStackIndices::empty());

        auto upvalue_indices = lua.upvalueIndices<2>(2);
        using UpvalueIndices = decltype(upvalue_indices);
        CHECK(upvalue_indices.first() == lua_upvalueindex(2));
        CHECK(upvalue_indices.last() == lua_upvalueindex(3));
        STATIC_REQUIRE(UpvalueIndices::size() == 2);
        STATIC_REQUIRE_FALSE(UpvalueIndices::empty());

        auto empty_upvalue_indices = lua.upvalueIndices<0>(1);
        using EmptyUpvalueIndices = decltype(empty_upvalue_indices);
        STATIC_REQUIRE(EmptyUpvalueIndices::size() == 0);
        STATIC_REQUIRE(EmptyUpvalueIndices::empty());
    }
    SECTION("IndexRange Wrapping")
    {
        auto positive_index_range = lua.stackIndexRange(2, 2);
        CHECK(positive_index_range.first() == 2);
        CHECK(positive_index_range.last() == 3);
        CHECK(positive_index_range.size() == 2);
        CHECK_FALSE(positive_index_range.empty());

        auto negative_index_range = lua.stackIndexRange(-3, 2);
        CHECK(negative_index_range.first() == 3);
        CHECK(negative_index_range.last() == 4);
        CHECK(negative_index_range.size() == 2);
        CHECK_FALSE(negative_index_range.empty());

        auto empty_stack_index_range = lua.stackIndexRange(1, 0);
        CHECK(empty_stack_index_range.size() == 0);
        CHECK(empty_stack_index_range.empty());

        auto upvalue_index_range = lua.upvalueIndexRange(2, 2);
        CHECK(upvalue_index_range.first() == lua_upvalueindex(2));
        CHECK(upvalue_index_range.last() == lua_upvalueindex(3));
        CHECK(upvalue_index_range.size() == 2);
        CHECK_FALSE(upvalue_index_range.empty());

        auto empty_upvalue_index_range = lua.upvalueIndexRange(1, 0);
        CHECK(empty_upvalue_index_range.size() == 0);
        CHECK(empty_upvalue_index_range.empty());
    }
    SECTION("Top and Bottom Wrapping")
    {
        CHECK(lua.bottom().index() == 1);
        CHECK(lua.top().index() == 5);

        CHECK(lua.bottom<2>().first() == 1);
        CHECK(lua.bottom<2>().last() == 2);
        CHECK(lua.top<2>().first() == 4);
        CHECK(lua.top<2>().last() == 5);

        CHECK(lua.bottom(2).first() == 1);
        CHECK(lua.bottom(2).last() == 2);
        CHECK(lua.top(2).first() == 4);
        CHECK(lua.top(2).last() == 5);
    }
}

TEST_CASE("Lua StateBase can statically check the type of raw Lua indices.", "[lua][state]")
{
    SECTION("State::isStack")
    {
        STATIC_REQUIRE(dlua::State::isStack(1));
        STATIC_REQUIRE(dlua::State::isStack(10));
        STATIC_REQUIRE(dlua::State::isStack(-1));
        STATIC_REQUIRE(dlua::State::isStack(-10));
        STATIC_REQUIRE_FALSE(dlua::State::isStack(LUA_REGISTRYINDEX));
        STATIC_REQUIRE_FALSE(dlua::State::isStack(lua_upvalueindex(1)));
    }
    SECTION("State::isPseudo")
    {
        STATIC_REQUIRE_FALSE(dlua::State::isPseudo(1));
        STATIC_REQUIRE_FALSE(dlua::State::isPseudo(10));
        STATIC_REQUIRE_FALSE(dlua::State::isPseudo(-1));
        STATIC_REQUIRE_FALSE(dlua::State::isPseudo(-10));
        STATIC_REQUIRE(dlua::State::isPseudo(LUA_REGISTRYINDEX));
        STATIC_REQUIRE(dlua::State::isPseudo(lua_upvalueindex(1)));
    }
    SECTION("State::isRegistry")
    {
        STATIC_REQUIRE_FALSE(dlua::State::isRegistry(1));
        STATIC_REQUIRE_FALSE(dlua::State::isRegistry(10));
        STATIC_REQUIRE_FALSE(dlua::State::isRegistry(-1));
        STATIC_REQUIRE_FALSE(dlua::State::isRegistry(-10));
        STATIC_REQUIRE(dlua::State::isRegistry(LUA_REGISTRYINDEX));
        STATIC_REQUIRE_FALSE(dlua::State::isRegistry(lua_upvalueindex(1)));
    }
    SECTION("State::isUpvalue")
    {
        STATIC_REQUIRE_FALSE(dlua::State::isUpvalue(1));
        STATIC_REQUIRE_FALSE(dlua::State::isUpvalue(10));
        STATIC_REQUIRE_FALSE(dlua::State::isUpvalue(-1));
        STATIC_REQUIRE_FALSE(dlua::State::isUpvalue(-10));
        STATIC_REQUIRE_FALSE(dlua::State::isUpvalue(LUA_REGISTRYINDEX));
        STATIC_REQUIRE(dlua::State::isUpvalue(lua_upvalueindex(1)));
    }
}

TEST_CASE("Lua StateBase can check and convert indices depending on the current stack size.", "[lua][state]")
{
    auto lua = dlua::State();
    lua.padWithNil(4);

    SECTION("It can check if a positive index is at the bottom/top of the stack or a given offset away from it.")
    {
        STATIC_REQUIRE(dlua::State::isIndexBottom(1));
        STATIC_REQUIRE(dlua::State::isIndexBottom(2, 1));
        CHECK(lua.isIndexTop(4));
        CHECK(lua.isIndexTop(3, 1));
    }
    SECTION("It can return the offset of a given index away from the bottom/top of the stack.")
    {
        STATIC_REQUIRE(dlua::State::indexOffsetFromBottom(1) == 0);
        STATIC_REQUIRE(dlua::State::indexOffsetFromBottom(2) == 1);
        CHECK(lua.indexOffsetFromTop(4) == 0);
        CHECK(lua.indexOffsetFromTop(3) == 1);
    }
    SECTION("It can turn any index into an absolute index, leaving pseudo indices.")
    {
        CHECK(lua.absIndex(1) == 1);
        CHECK(lua.absIndex(2) == 2);
        CHECK(lua.absIndex(-1) == 4);
        CHECK(lua.absIndex(-2) == 3);
        CHECK(lua.absIndex(LUA_REGISTRYINDEX) == LUA_REGISTRYINDEX);
        CHECK(lua.absIndex(lua_upvalueindex(1)) == lua_upvalueindex(1));
        CHECK(lua.absIndex(lua_upvalueindex(2)) == lua_upvalueindex(2));
    }
    SECTION("It can turn stack indices into absolute indices.")
    {
        CHECK(lua.absStackIndex(1) == 1);
        CHECK(lua.absStackIndex(2) == 2);
        CHECK(lua.absStackIndex(-1) == 4);
        CHECK(lua.absStackIndex(-2) == 3);
    }
}

TEST_CASE("Lua StateBase can do queries on the Lua stack.", "[lua][state]")
{
    auto lua = dlua::State();

    SECTION("Its size can be queried.")
    {
        CHECK(lua.size() == 0);
        CHECK(lua.empty());
        lua.padWithNil(3);
        CHECK(lua.size() == 3);
        CHECK_FALSE(lua.empty());
    }
    SECTION("The type of elements can be queried.")
    {
        SECTION("No value.")
        {
            CHECK(lua.type(1) == dlua::Type::None);
            CHECK(lua.typeName(1) == "no value");
            CHECK(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A nil value.")
        {
            lua.pushNil();
            CHECK(lua.type(1) == dlua::Type::Nil);
            CHECK(lua.typeName(1) == "nil");
            CHECK_FALSE(lua.isNone(1));
            CHECK(lua.isNil(1));
            CHECK(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A boolean.")
        {
            lua.push(true);
            CHECK(lua.type(1) == dlua::Type::Boolean);
            CHECK(lua.typeName(1) == "boolean");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("Light userdata.")
        {
            // TODO: Add a way to add light userdata.
            /*
            lua.push();
            CHECK(lua.type(1) == dlua::Type::LightUserdata);
            CHECK(lua.typeName(1) == "userdata");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
            */
        }
        SECTION("A number.")
        {
            lua.push(42.56);
            CHECK(lua.type(1) == dlua::Type::Number);
            CHECK(lua.typeName(1) == "number");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A number that is representable as an integer.")
        {
            lua.push(42.0);
            CHECK(lua.type(1) == dlua::Type::Number);
            CHECK(lua.typeName(1) == "number");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("An integer.")
        {
            lua.push(42);
            CHECK(lua.type(1) == dlua::Type::Number);
            CHECK(lua.typeName(1) == "number");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK(lua.isNumber(1));
            CHECK(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A string.")
        {
            lua.push("test");
            CHECK(lua.type(1) == dlua::Type::String);
            CHECK(lua.typeName(1) == "string");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A string that is convertible to a number.")
        {
            lua.push("42.56");
            CHECK(lua.type(1) == dlua::Type::String);
            CHECK(lua.typeName(1) == "string");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A string that is convertible to an integer.")
        {
            lua.push("42");
            CHECK(lua.type(1) == dlua::Type::String);
            CHECK(lua.typeName(1) == "string");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A table.")
        {
            lua.pushTable();
            CHECK(lua.type(1) == dlua::Type::Table);
            CHECK(lua.typeName(1) == "table");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A Lua function.")
        {
            lua.load("");
            CHECK(lua.type(1) == dlua::Type::Function);
            CHECK(lua.typeName(1) == "function");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("A C function.")
        {
            lua.push(+[](lua_State*) { return 0; });
            CHECK(lua.type(1) == dlua::Type::Function);
            CHECK(lua.typeName(1) == "function");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK(lua.isFunction(1));
            CHECK(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
        }
        SECTION("Full userdata.")
        {
            // TODO: ClassInfo for some tiny test struct.
            /*
            lua.pushNew<Test>();
            CHECK(lua.type(1) == dlua::Type::Userdata);
            CHECK(lua.typeName(1) == "userdata");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK(lua.isUserdata(1));
            CHECK_FALSE(lua.isThread(1));
            */
        }
        SECTION("A thread.")
        {
            // TODO: Implement pushThread properly again.
            /*
            lua.pushThread();
            CHECK(lua.type(1) == dlua::Type::Thread);
            CHECK(lua.typeName(1) == "thread");
            CHECK_FALSE(lua.isNone(1));
            CHECK_FALSE(lua.isNil(1));
            CHECK_FALSE(lua.isNoneOrNil(1));
            CHECK_FALSE(lua.isBoolean(1));
            CHECK_FALSE(lua.isLightUserdata(1));
            CHECK_FALSE(lua.isNumber(1));
            CHECK_FALSE(lua.isInteger(1));
            CHECK_FALSE(lua.isString(1));
            CHECK_FALSE(lua.isTable(1));
            CHECK_FALSE(lua.isFunction(1));
            CHECK_FALSE(lua.isCFunction(1));
            CHECK_FALSE(lua.isUserdata(1));
            CHECK(lua.isThread(1));
            */
        }
    }
}

TEST_CASE("Lua StateBase can check elements using Convert.", "[lua][state]")
{
    auto lua = dlua::State();
    lua.push(42);
    CHECK(lua.to<int>(1) == 42);
    CHECK(lua.check<int>(1) == 42);
}

TEST_CASE("Lua StateBase can check if the stack can be extended.")
{
    auto lua = dlua::State();

    SECTION("Asserting on acceptable indices.")
    {
        lua.assertAcceptable(20);
        lua.assertPushable(20);
        lua.assertPushableAuxiliary();
    }
    SECTION("Ensuring acceptable indices and returning false if it can't.")
    {
        CHECK(lua.checkAcceptable(21));
        CHECK(lua.checkPushable(21));
        CHECK(lua.checkPushableAuxiliary());
    }
    SECTION("Ensuring acceptable indices and throwing an error if it can't.")
    {
        lua.ensureAcceptable(21);
        lua.ensurePushable(21);
        lua.ensurePushableAuxiliary();
    }
}

TEST_CASE("Lua StateBase can push elements onto the stack and replace or remove existing ones.", "[lua][state]") {}

TEST_CASE("Lua StateBase can raise errors.", "[lua][state]") {}

TEST_CASE("Lua StateBase can compile Lua code.", "[lua][state]") {}

TEST_CASE("Lua StateBase can call functions.", "[lua][state]") {}

TEST_CASE("Lua StateBase can call a string of Lua code directly, compiling it on the fly.", "[lua][state]") {}

TEST_CASE("Lua StateBase can apply operations on elements.", "[lua][state]") {}

TEST_CASE("Lua StateBase can perform table access on elements.", "[lua][state]") {}

TEST_CASE("Lua StateBase provides various iteration wrappers.", "[lua][state]") {}

TEST_CASE("Lua StateBase can format any element.", "[lua][state]") {}

TEST_CASE("Lua StateBase can open libraries.", "[lua][state]") {}

TEST_CASE("Lua StateBase can turn elements into references.", "[lua][state]") {}

TEST_CASE("Lua StateBase can mark elements as to-be-closed.", "[lua][state]") {}

TEST_CASE("Lua StateBase wraps the functionality of the Debug Interface.", "[lua][state]") {}

// --- StateRef

TEST_CASE("Lua StateRef can be constructed from the lua_State passed to a C function.", "[lua][state]")
{
    LuaState owned_lua_state;

    auto pushed = GENERATE(0, 1, 5);

    for (int i = 0; i < pushed; i++)
        lua_pushinteger(*owned_lua_state, i);

    // Simulate a proper call, allowing LUA_MINSTACK (20) elements to be pushed safely.
    // See below for more info.
    luaL_checkstack(*owned_lua_state, LUA_MINSTACK, nullptr);

    // StateRef is meant exclusively for when Lua calls a C function.

    // 1. Stack Size (See https://www.lua.org/manual/5.4/manual.html#4.1.1)
    // Lua ensures, that LUA_MINSTACK (20) elements can be safely pushed.
    // When StateRef is created it assumes these 20 elements can safely be pushed.
    // Unless NDEBUG is set, StateRef keeps track of the stack size and asserts, that the stack doesn't overflow.

    // 2. Pushed Elements
    // When a C function is called, a variable number of elements will lie on the stack.
    // Creating a StateRef will therefore query the stack size once and store it.
    // Any operations on the StateRef will automatically update without any further size queries.
    // This should allow for better optimizations and avoid many calls to lua_gettop.
    // However each call always calls lua_gettop exactly once, which might not have been necessary.
    // Wrapped functions that do not use any StateRef or Arg parameter do not have this overhead.

    auto lua = dlua::StateRef(*owned_lua_state);

    CHECK(lua.state() == *owned_lua_state);
    CHECK(lua.size() == pushed);
}

TEST_CASE("Lua StateRef can be moved.", "[lua][state]")
{
    LuaState owned_lua_state;
    lua_pushinteger(*owned_lua_state, 42);
    luaL_checkstack(*owned_lua_state, LUA_MINSTACK, nullptr);

    auto lua = dlua::StateRef(*owned_lua_state);

    SECTION("Using move-constructor.")
    {
        auto moved_lua = std::move(lua);
        CHECK(moved_lua.state() == *owned_lua_state);
        CHECK(moved_lua.size() == 1);
    }
    SECTION("Using move-assignment.")
    {
        LuaState other_lua_state;

        auto moved_lua = dlua::StateRef(*other_lua_state);
        moved_lua = std::move(lua);
        CHECK(moved_lua.state() == *owned_lua_state);
        CHECK(moved_lua.size() == 1);
    }
}

TEST_CASE("Lua StateRef can be swapped.", "[lua][state]")
{
    LuaState owned_lua_state1;
    lua_pushinteger(*owned_lua_state1, 1);
    luaL_checkstack(*owned_lua_state1, LUA_MINSTACK, nullptr);

    LuaState owned_lua_state2;
    lua_pushinteger(*owned_lua_state2, 2);
    lua_pushinteger(*owned_lua_state2, 2);
    luaL_checkstack(*owned_lua_state2, LUA_MINSTACK, nullptr);

    auto lua1 = dlua::StateRef(*owned_lua_state1);
    auto lua2 = dlua::StateRef(*owned_lua_state2);

    SECTION("Using swap member function.") { lua1.swap(lua2); }
    SECTION("Using swap friend function.") { swap(lua1, lua2); }

    CHECK(lua1.state() == *owned_lua_state2);
    CHECK(lua1.size() == 2);
    CHECK(lua2.state() == *owned_lua_state1);
    CHECK(lua2.size() == 1);
}

TEST_CASE("Lua StateRef's underlying state can be checked and extracted.", "[lua][state]")
{
    LuaState owned_lua_state;

    auto lua = dlua::StateRef(*owned_lua_state);

    CHECK(lua.state() == *owned_lua_state);
    CHECK(std::move(lua).state() == *owned_lua_state);
}

// --- Thread

// --- State

TEST_CASE("Lua State can be constructed and closed.", "[lua][state]")
{
    auto checked_allocator = CheckedAllocator();
    auto allocator = checked_allocator.allocator();

    auto maybe_allocator = GENERATE_COPY(as<std::optional<dlua::Allocator>>{}, std::nullopt, allocator);

    auto check_close_function = [&](dlua::State& lua) {
        CHECK_FALSE(lua.closed());
        SECTION("Letting it go out of scope.") {}
        SECTION("Closing it explicitly.")
        {
            lua.close();
            CHECK(lua.closed());
            if (maybe_allocator)
                checked_allocator.checkEmpty();
        }
        SECTION("Closing it multiple times.")
        {
            lua.close();
            CHECK(lua.closed());
            if (maybe_allocator)
                checked_allocator.checkEmpty();

            lua.close();
            CHECK(lua.closed());
        }
    };

    SECTION("Using the constructor without standard libraries.")
    {
        auto lua = dlua::State(maybe_allocator);
        if (maybe_allocator)
            checked_allocator.checkNotEmpty();
        check_close_function(lua);
    }
    SECTION("Using the constructor with and without standard libraries.")
    {
        auto open_libs = GENERATE(true, false);
        auto lua = dlua::State(maybe_allocator, open_libs);
        if (maybe_allocator)
            checked_allocator.checkNotEmpty();
        check_close_function(lua);
    }
    SECTION("Using the withLibs function.")
    {
        auto lua = dlua::State::withLibs(maybe_allocator);
        if (maybe_allocator)
            checked_allocator.checkNotEmpty();
        check_close_function(lua);
    }

    if (maybe_allocator)
        checked_allocator.checkEmpty();
}

TEST_CASE("Lua State can be moved.", "[lua][state]")
{
    auto lua = dlua::State();
    lua.push(42);

    SECTION("Using move-constructor.")
    {
        auto moved_lua = std::move(lua);
        CHECK(moved_lua.to<int>(1) == 42);
    }
    SECTION("Using move-assignment.")
    {
        auto moved_lua = dlua::State();
        moved_lua = std::move(lua);
        CHECK(moved_lua.to<int>(1) == 42);
    }
}

TEST_CASE("Lua State can be swapped.", "[lua][state]")
{
    auto lua1 = dlua::State();
    lua1.push(1);

    auto lua2 = dlua::State();
    lua2.push(2);
    lua2.push(2);

    SECTION("Using swap member function.") { lua1.swap(lua2); }
    SECTION("Using swap friend function.") { swap(lua1, lua2); }

    CHECK(lua1.size() == 2);
    CHECK(lua1.to<int>(1) == 2);
    CHECK(lua1.to<int>(2) == 2);
    CHECK(lua2.size() == 1);
    CHECK(lua2.to<int>(1) == 1);
}
