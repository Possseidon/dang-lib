#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

using checkable_string_types = maybe_cv<std::string, std::string_view, const char*>;

TEMPLATE_LIST_TEST_CASE("Convert can check for strings.", "[lua][convert][string][check]", checkable_string_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "string");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for strings.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushliteral(*lua, "test");
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for strings and numbers.")
        {
            CHECK_FALSE(Convert::isValid(*lua, 1));
            lua_pushliteral(*lua, "test");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            UNSCOPED_INFO("The number is not converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
    }
}

using pushable_string_types = maybe_cv<std::string, std::string_view, const char*, char*>;

TEMPLATE_LIST_TEST_CASE("Convert can push strings.", "[lua][convert][string][push]", pushable_string_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "string");
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can check for std::string.", "[lua][convert][string][check]", maybe_cv<std::string>)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::at returns valid strings and numbers and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushliteral(*lua, "test");
            CHECK(Convert::at(*lua, -1) == "test"s);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == "42"s);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns valid strings and numbers and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (string expected, got no value)");
            lua_pushliteral(*lua, "test");
            CHECK(Convert::check(*lua, -1) == "test"s);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == "42"s);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string expected, got boolean)");
        }
    }
}

using std_string_types = maybe_cv<std::string>;

TEMPLATE_LIST_TEST_CASE("Convert can push std::string.", "[lua][convert][string][push]", std_string_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a string on the stack.")
        {
            Convert::push(*lua, "test"s);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(Convert::at(*lua, -1) == "test"s);
        }
        SECTION("Convert::push preserves embedded null-terminators.")
        {
            Convert::push(*lua, "\0te\0st\0"s);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(Convert::at(*lua, -1) == "\0te\0st\0"s);
        }
    }
}

using std_string_view_types = maybe_cv<std::string_view>;

TEMPLATE_LIST_TEST_CASE("Convert can check for std::string_view.",
                        "[lua][convert][string][check]",
                        std_string_view_types)
{
    using namespace std::literals::string_view_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::at returns valid strings and numbers and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushliteral(*lua, "test");
            CHECK(Convert::at(*lua, -1) == "test"sv);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == "42"sv);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns valid strings and numbers and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (string expected, got no value)");
            lua_pushliteral(*lua, "test");
            CHECK(Convert::check(*lua, -1) == "test"sv);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == "42"sv);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string expected, got boolean)");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push std::string_view.", "[lua][convert][string][push]", std_string_view_types)
{
    using namespace std::literals::string_view_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a string on the stack.")
        {
            Convert::push(*lua, "test"sv);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(Convert::at(*lua, -1) == "test"sv);
        }
        SECTION("Convert::push preserves embedded null-terminators.")
        {
            Convert::push(*lua, "\0te\0st\0"sv);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(Convert::at(*lua, -1) == "\0te\0st\0"sv);
        }
    }
}

using char_array_types = std::tuple<const char[5]>;

TEMPLATE_LIST_TEST_CASE("Convert can push references to const char arrays.", "[lua][convert][string]", char_array_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "string");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push trims of a potential null-terminator.")
        {
            Convert::push(*lua, "test");
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "test"s);
        }
        SECTION("Convert::push preserves embedded null-terminators.")
        {
            const char value[5] = {'\0', 'a', '\0', 'b', '\0'};
            Convert::push(*lua, value);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "\0a\0b"s);
        }
    }
}

using checkable_cstring_types = maybe_cv<const char*>;

TEMPLATE_LIST_TEST_CASE("Convert can check for C-Style strings.",
                        "[lua][convert][string][check]",
                        checkable_cstring_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        auto is_valid = [](const char* str, const std::string& value) {
            CHECKED_IF(str != nullptr) { CHECK(str == value); }
        };

        SECTION("Convert::at returns valid strings and numbers and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushliteral(*lua, "test");
            is_valid(Convert::at(*lua, -1).value(), "test"s);
            lua_pushinteger(*lua, 42);
            is_valid(Convert::at(*lua, -1).value(), "42"s);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns valid string and numbers and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (string expected, got no value)");
            lua_pushliteral(*lua, "test");
            is_valid(Convert::check(*lua, -1), "test"s);
            lua_pushinteger(*lua, 42);
            is_valid(Convert::check(*lua, -1), "42"s);
            UNSCOPED_INFO("The number is converted to a string.");
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string expected, got boolean)");
        }
    }
}

using pushable_cstring_types = maybe_cv<const char*, char*>;

TEMPLATE_LIST_TEST_CASE("Convert can push C-Style strings.", "[lua][convert][string][push]", pushable_cstring_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a string on the stack.")
        {
            std::string string = "test";
            TestType text = string.data();
            Convert::push(*lua, text);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "test");
        }
    }
}
