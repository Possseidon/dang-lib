#include "dang-lua/convert/Fail.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using fail_types = maybe_cv<dlua::Fail>;

TEMPLATE_LIST_TEST_CASE("Convert cannot check for fail values.", "[lua][convert][fail][check]", fail_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It cannot be checked.") { STATIC_REQUIRE_FALSE(Convert::can_check); }
}

TEMPLATE_LIST_TEST_CASE("Convert can push fail values.", "[lua][convert][fail][push]", fail_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "fail");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a falsy value on the stack.")
        {
            Convert::push(*lua, dlua::fail);
            CHECK_FALSE(lua_toboolean(*lua, -1));
        }
    }
}
