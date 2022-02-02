#include <optional>
#include <string>
#include <variant>

#include "dang-lua/convert/Integer.h"
#include "dang-lua/convert/String.h"
#include "dang-lua/convert/Variant.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using variant_types = maybe_cv<std::variant<int, std::string>>;

TEMPLATE_LIST_TEST_CASE("Convert can check for std::variant.", "[lua][convert][variant][check]", variant_types)
{
    using namespace std::literals::string_literals;

    using Variant = std::remove_cv_t<TestType>;

    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has the disjunction of all options as a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "integer or string");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;
        SECTION("Convert::isExact returns true if any option is exact.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "test");
            CHECK(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true if any option is valid.")
        {
            CHECK_FALSE(Convert::isValid(*lua, 1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "test");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at returns the first valid value and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == Variant{42});
            lua_pushliteral(*lua, "42");
            CHECK(Convert::at(*lua, -1) == Variant{42});
            lua_pushliteral(*lua, "test");
            CHECK(Convert::at(*lua, -1) == Variant{"test"s});
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns the first valid value and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (integer or string expected, got no value)");
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == Variant{42});
            lua_pushliteral(*lua, "42");
            CHECK(Convert::check(*lua, -1) == Variant{42});
            lua_pushliteral(*lua, "test");
            CHECK(Convert::check(*lua, -1) == Variant{"test"s});
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (integer or string expected, got boolean)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push std::variant.", "[lua][convert][variant][push]", variant_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has the disjunction of all options as a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "integer|string");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes the value with the correct type on the stack.")
        {
            Convert::push(*lua, 42);
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::at(*lua, -1) == 42);
            Convert::push(*lua, "test"s);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "test"s);
        }
    }
}

// TODO: Add test with variant with monostate
