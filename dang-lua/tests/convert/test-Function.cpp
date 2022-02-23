#include <optional>

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

int dummyLuaFunction(lua_State*) { return 0; }

using cfunction_types = maybe_cv<lua_CFunction>;

TEMPLATE_LIST_TEST_CASE("Convert can check for Lua C functions.", "[lua][convert][function][check]", cfunction_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "C function");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact and Convert::isValid only return true for C functions.")
        {
            auto isExactAndValid = GENERATE(&Convert::isExact, &Convert::isValid);

            CHECK_FALSE(isExactAndValid(*lua, 1));
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(isExactAndValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(isExactAndValid(*lua, -1));
        }
        SECTION("Convert::at returns C functions and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(Convert::at(*lua, -1) == dummyLuaFunction);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns C functions and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (C function expected, got no value)");
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(Convert::check(*lua, -1) == dummyLuaFunction);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (C function expected, got boolean)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push Lua C functions.", "[lua][convert][function][push]", cfunction_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "C function");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a C function.")
        {
            Convert::push(*lua, dummyLuaFunction);
            CHECK(lua_type(*lua, -1) == LUA_TFUNCTION);
            CHECK(lua_tocfunction(*lua, -1) == dummyLuaFunction);
        }
    }
}
