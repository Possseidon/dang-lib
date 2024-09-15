#include <optional>

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using optional_types = maybe_cv<std::optional<int>>;

TEMPLATE_LIST_TEST_CASE("Convert can check std::optional.", "[lua][convert][optional][check]", optional_types)
{
    using Convert = dlua::Convert<TestType>;
    using ConvertContained = dlua::Convert<int>;

    SECTION("It can be checked if the contained type can be checked and modifies its typename.")
    {
        STATIC_REQUIRE(Convert::can_check == ConvertContained::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "optional " + ConvertContained::getCheckTypename());
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true for exact values and nil/none.")
        {
            CHECK(Convert::isExact(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for valid values and nil/none.")
        {
            CHECK(Convert::isValid(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at returns an optional that may contain a std::nullopt and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1).value() == std::nullopt);
            lua_pushnil(*lua);
            CHECK(Convert::at(*lua, -1).value() == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1).value() == 42);
            lua_pushliteral(*lua, "42");
            CHECK(Convert::at(*lua, -1).value() == 42);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns an optional that may contain a std::nullopt and throws a Lua error otherwise.")
        {
            CHECK(Convert::check(*lua, 1) == std::nullopt);
            lua_pushnil(*lua);
            CHECK(Convert::check(*lua, -1) == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == 42);
            lua_pushliteral(*lua, "42");
            CHECK(Convert::check(*lua, -1) == 42);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (optional integer expected, got boolean)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push std::optional.", "[lua][convert][optional][push]", optional_types)
{
    using Convert = dlua::Convert<TestType>;
    using ConvertContained = dlua::Convert<int>;

    SECTION("It can be pushed if the contained type can be pushed and modifies its typename.")
    {
        STATIC_REQUIRE(Convert::can_push == ConvertContained::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == ConvertContained::getPushTypename() + "?");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes the value or nil.")
        {
            Convert::push(*lua, 42);
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            CHECK(lua_tointeger(*lua, -1) == 42);
            Convert::push(*lua, std::nullopt);
            CHECK(lua_type(*lua, -1) == LUA_TNIL);
        }
    }
}
