#include <array>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "dang-lua/Convert.h"
#include "dang-utils/utils.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;
namespace dutils = dang::utils;

using pair_types = maybe_cv<std::pair<int, std::string>, std::tuple<int, std::string>>;

TEMPLATE_LIST_TEST_CASE("Convert can push std::pair and std::tuple.", "[lua][convert][tuple][push]", pair_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    SECTION("It pushes each value and has the conjunction of all values as a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 2);
        CHECK(Convert::getPushTypename() == "integer, string");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes all elements on the stack in order.")
        {
            Convert::push(*lua, {42, "test"s});
            CHECK(lua_type(*lua, 1) == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::at(*lua, 1) == 42);
            CHECK(lua_type(*lua, 2) == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::at(*lua, 2) == "test"s);
        }
    }
}

using array_types = maybe_cv<std::array<int, 3>>;

TEMPLATE_LIST_TEST_CASE("Convert can push std::array.", "[lua][convert][array][push]", array_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It pushes each value in the array and denotes the count in its typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 3);
        CHECK(Convert::getPushTypename() == "integer<3>");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes all elements on the stack in order.")
        {
            Convert::push(*lua, {42, 256, 1337});
            CHECK(lua_type(*lua, 1) == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::at(*lua, 1) == 42);
            CHECK(lua_type(*lua, 2) == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::at(*lua, 2) == 256);
            CHECK(lua_type(*lua, 3) == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::at(*lua, 3) == 1337);
        }
    }
}
