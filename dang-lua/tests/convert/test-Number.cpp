#include <optional>

#include "dang-lua/Convert.h"

#include "dang-utils/utils.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;
namespace dutils = dang::utils;

using number_types = maybe_cv<float, double>;

TEMPLATE_LIST_TEST_CASE("Convert can check for numbers.", "[lua][convert][number][check]", number_types)
{
    using Number = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "number");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for numbers and integers.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42.0");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for numbers, integers and convertible strings.")
        {
            CHECK_FALSE(Convert::isValid(*lua, 1));
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at returns the number or convertible string and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::at(*lua, -1) == Number{42});
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == Number{42});
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::at(*lua, -1) == Number{42});
            lua_pushliteral(*lua, "42");
            CHECK(Convert::at(*lua, -1) == Number{42});
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns the number or convertible string and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (number expected, got no value)");
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::check(*lua, -1) == Number{42});
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == Number{42});
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::check(*lua, -1) == Number{42});
            lua_pushliteral(*lua, "42");
            CHECK(Convert::check(*lua, -1) == Number{42});
            CHECK(lua.shouldThrow([&] {
                lua_pushliteral(*lua, "test");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string cannot be converted to a number)");
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (number expected, got boolean)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push numbers.", "[lua][convert][number][push]", number_types)
{
    using Number = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "number");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a number on the stack.")
        {
            Convert::push(*lua, Number{42});
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            CHECK(lua_tonumber(*lua, -1) == 42.0);
        }
    }
}

// --- Convert<integer>
