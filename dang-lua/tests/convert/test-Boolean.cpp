#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using boolean_types = maybe_cv<bool>;

TEMPLATE_LIST_TEST_CASE("Convert can check for booleans.", "[lua][convert][boolean][check]", boolean_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "boolean");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for actual booleans.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushboolean(*lua, false);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for any value.")
        {
            CHECK(Convert::isValid(*lua, 1));
            lua_pushboolean(*lua, false);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at only returns false for false, nil and none and never returns std::nullopt.")
        {
            CHECK_FALSE(Convert::at(*lua, 1).value());
            lua_pushnil(*lua);
            CHECK_FALSE(Convert::at(*lua, -1).value());
            lua_pushboolean(*lua, false);
            CHECK_FALSE(Convert::at(*lua, -1).value());
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1).value());
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1).value());
        }
        SECTION("Convert::check works the same as Convert::at and never throws.")
        {
            CHECK_FALSE(Convert::check(*lua, 1));
            lua_pushnil(*lua);
            CHECK_FALSE(Convert::check(*lua, -1));
            lua_pushboolean(*lua, false);
            CHECK_FALSE(Convert::check(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK(Convert::check(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1));
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push booleans.", "[lua][convert][boolean][push]", boolean_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "boolean");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a boolean value on the stack.")
        {
            Convert::push(*lua, false);
            CHECK(lua_type(*lua, -1) == LUA_TBOOLEAN);
            CHECK_FALSE(lua_toboolean(*lua, -1));
            Convert::push(*lua, true);
            CHECK(lua_type(*lua, -1) == LUA_TBOOLEAN);
            CHECK(lua_toboolean(*lua, -1));
        }
    }
}
