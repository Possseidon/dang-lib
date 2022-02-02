#include <optional>
#include <string>

#include "dang-lua/convert/Enum.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

enum class TestEnum { First, Second, Third };

namespace dang::lua {

template <>
struct EnumInfo<TestEnum> : DefaultEnumInfo {
    static constexpr auto specialized = true;

    static std::string getCheckTypename() { return "CheckedTestEnum"; }
    static std::string getPushTypename() { return "PushedTestEnum"; }

    static constexpr const char* values[4]{"first", "second", "third"};
};

} // namespace dang::lua

TEMPLATE_LIST_TEST_CASE("Convert can check strings for valid enum values.",
                        "[lua][convert][enum][check]",
                        maybe_cv<TestEnum>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "CheckedTestEnum");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact and Convert::isValid return true for strings that are valid for this enum.")
        {
            auto isExactAndValid = GENERATE(&Convert::isExact, &Convert::isValid);

            CHECK_FALSE(isExactAndValid(*lua, 1));

            lua_pushliteral(*lua, "first");
            CHECK(isExactAndValid(*lua, -1));
            lua_pushliteral(*lua, "second");
            CHECK(isExactAndValid(*lua, -1));
            lua_pushliteral(*lua, "third");
            CHECK(isExactAndValid(*lua, -1));

            lua_pushliteral(*lua, "first_");
            CHECK_FALSE(isExactAndValid(*lua, -1));
            lua_pushliteral(*lua, "_first");
            CHECK_FALSE(isExactAndValid(*lua, -1));

            lua_pushinteger(*lua, 42);
            CHECK_FALSE(isExactAndValid(*lua, -1));
        }
        SECTION("Convert::at returns the enum value for valid strings and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);

            lua_pushliteral(*lua, "first");
            CHECK(Convert::at(*lua, -1) == TestEnum::First);
            lua_pushliteral(*lua, "second");
            CHECK(Convert::at(*lua, -1) == TestEnum::Second);
            lua_pushliteral(*lua, "third");
            CHECK(Convert::at(*lua, -1) == TestEnum::Third);

            lua_pushliteral(*lua, "first_");
            CHECK(Convert::at(*lua, -1) == std::nullopt);
            lua_pushliteral(*lua, "_first");
            CHECK(Convert::at(*lua, -1) == std::nullopt);

            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns the enum value for valid strings and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (string expected, got no value)");

            lua_pushliteral(*lua, "first");
            CHECK(Convert::at(*lua, -1) == TestEnum::First);
            lua_pushliteral(*lua, "second");
            CHECK(Convert::at(*lua, -1) == TestEnum::Second);
            lua_pushliteral(*lua, "third");
            CHECK(Convert::at(*lua, -1) == TestEnum::Third);

            CHECK(lua.shouldThrow([&] {
                lua_pushliteral(*lua, "first_");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (invalid option 'first_')");

            CHECK(lua.shouldThrow([&] {
                lua_pushliteral(*lua, "_first");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (invalid option '_first')");

            CHECK(lua.shouldThrow([&] {
                lua_pushinteger(*lua, 42);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (invalid option '42')");
        }
        SECTION("Convert::push pushes the string representation of the enum value on the stack.")
        {
            Convert::push(*lua, TestEnum::First);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "first");

            Convert::push(*lua, TestEnum::Second);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "second");

            Convert::push(*lua, TestEnum::Third);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "third");
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can check enum values, converting them from strings.",
                        "[lua][convert][enum][push]",
                        maybe_cv<TestEnum>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "PushedTestEnum");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes the string representation of the enum value on the stack.")
        {
            Convert::push(*lua, TestEnum::First);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "first");

            Convert::push(*lua, TestEnum::Second);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "second");

            Convert::push(*lua, TestEnum::Third);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, -1) == "third");
        }
    }
}
