#include <optional>

#include "dang-lua/convert/Nil.h"

#include "dang-utils/utils.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;
namespace dutils = dang::utils;

using nil_types = maybe_cv<dlua::Nil>;

TEMPLATE_LIST_TEST_CASE("Convert can check for nil values.", "[lua][convert][nil][check]", nil_types)
{
    using Nil = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "nil");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for actual nil values.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for nil and none values.")
        {
            CHECK(Convert::isValid(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at returns an instance for nil/none and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == Nil());
            lua_pushnil(*lua);
            CHECK(Convert::at(*lua, -1) == Nil());
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check return an instance for nil/none and throws a Lua error otherwise.")
        {
            CHECK(Convert::check(*lua, 1) == Nil());
            lua_pushnil(*lua);
            CHECK(Convert::check(*lua, -1) == Nil());
            CHECK(lua.shouldThrow([&] {
                lua_pushinteger(*lua, 42);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (nil expected, got number)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push nil values.", "[lua][convert][nil][push]", nil_types)
{
    using Nil = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "nil");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes nil on the stack.")
        {
            Convert::push(*lua, Nil());
            CHECK(lua_type(*lua, -1) == LUA_TNIL);
        }
    }
}
