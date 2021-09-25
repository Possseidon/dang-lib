#include "dang-lua/State.h"

#include "catch2/catch.hpp"
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

// --- OwnedState

// --- State and OwnedState
