#include <tuple>
#include <utility>

#include "dang-lua/Reference.h"
#include "dang-lua/convert/Reference.h"
#include "dang-lua/convert/Tuple.h"

#include "catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using reference_types = maybe_cv<dlua::Reference>;

TEMPLATE_LIST_TEST_CASE("Convert cannot check for reference values.",
                        "[lua][convert][reference][check]",
                        reference_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It cannot be checked.") { STATIC_REQUIRE_FALSE(Convert::can_check); }
}

TEMPLATE_LIST_TEST_CASE("Convert can push references.", "[lua][convert][reference][push]", reference_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "reference");
    }
    SECTION("Given a Lua state and a valid reference.")
    {
        LuaState lua;
        lua_pushinteger(*lua, 42);
        auto reference = dlua::Reference::consume(*lua);

        SECTION("Convert::push pushes the referenced value on the stack.")
        {
            Convert::push(*lua, reference);
            CHECK(lua_tointeger(*lua, -1) == 42);
        }
    }
    SECTION("It can be nested inside a tuple.")
    {
        LuaState lua;
        lua_pushinteger(*lua, 1);
        auto reference1 = dlua::Reference::consume(*lua);
        lua_pushinteger(*lua, 2);
        auto reference2 = dlua::Reference::consume(*lua);

        auto references = std::tuple(std::move(reference1), std::move(reference2));

        dlua::Convert<decltype(references)>::push(*lua, references);

        CHECK(lua_tointeger(*lua, 1) == 1);
        CHECK(lua_tointeger(*lua, 2) == 2);
    }
}
