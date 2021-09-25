#include "dang-lua/Reference.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

TEST_CASE("Lua references default construct with an invalid state.", "[lua][reference]")
{
    auto reference = dlua::Reference();
    CHECK_FALSE(reference);
    CHECK(reference.state() == nullptr);
}

TEST_CASE("Lua references can consume elements from the stack, so that they can be pushed back later.",
          "[lua][reference]")
{
    LuaState lua;

    SECTION("References can store any value.")
    {
        lua_pushinteger(*lua, 42);
        auto reference = dlua::Reference::consume(*lua);

        CHECK(lua_gettop(*lua) == 0);

        reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_tointeger(*lua, -1) == 42);

        reference.push();
        CHECK(lua_gettop(*lua) == 2);
        CHECK(lua_tointeger(*lua, -1) == 42);
    }
    SECTION("References can store nil.")
    {
        lua_pushnil(*lua);
        auto reference = dlua::Reference::consume(*lua);

        CHECK(lua_gettop(*lua) == 0);

        reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_isnil(*lua, -1));

        reference.push();
        CHECK(lua_gettop(*lua) == 2);
        CHECK(lua_isnil(*lua, -1));
    }
}

TEST_CASE("Lua references can be moved.")
{
    LuaState lua;

    lua_pushinteger(*lua, 42);
    auto reference = dlua::Reference::consume(*lua);

    SECTION("Using move-constructor.")
    {
        auto moved_reference = std::move(reference);

        CHECK_FALSE(reference);
        CHECK(moved_reference);

        moved_reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_tointeger(*lua, -1) == 42);
    }
    SECTION("Using move-assignment.")
    {
        auto moved_reference = dlua::Reference();
        moved_reference = std::move(reference);

        CHECK_FALSE(reference);
        CHECK(moved_reference);

        moved_reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_tointeger(*lua, -1) == 42);
    }
}

TEST_CASE("Lua references can be copied.")
{
    LuaState lua;

    lua_pushinteger(*lua, 42);
    auto reference = dlua::Reference::consume(*lua);

    SECTION("Using copy-constructor.")
    {
        auto copied_reference = reference;

        CHECK(reference);
        CHECK(copied_reference);

        reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_tointeger(*lua, -1) == 42);

        copied_reference.push();
        CHECK(lua_gettop(*lua) == 2);
        CHECK(lua_tointeger(*lua, -1) == 42);
    }
    SECTION("Using copy-assignment.")
    {
        auto copied_reference = dlua::Reference();
        copied_reference = reference;

        CHECK(reference);
        CHECK(copied_reference);

        reference.push();
        CHECK(lua_gettop(*lua) == 1);
        CHECK(lua_tointeger(*lua, -1) == 42);

        copied_reference.push();
        CHECK(lua_gettop(*lua) == 2);
        CHECK(lua_tointeger(*lua, -1) == 42);
    }
}

TEST_CASE("Lua references can be swapped.")
{
    LuaState lua;

    lua_pushinteger(*lua, 1);
    auto reference1 = dlua::Reference::consume(*lua);
    lua_pushinteger(*lua, 2);
    auto reference2 = dlua::Reference::consume(*lua);

    swap(reference1, reference2);

    reference1.push();
    CHECK(lua_gettop(*lua) == 1);
    CHECK(lua_tointeger(*lua, -1) == 2);

    reference2.push();
    CHECK(lua_gettop(*lua) == 2);
    CHECK(lua_tointeger(*lua, -1) == 1);
}

TEMPLATE_LIST_TEST_CASE("Convert can work with references.", "[lua][convert][reference]", maybe_const<dlua::Reference>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1 and allows nesting.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
    }
    SECTION("Given a Lua state and a valid reference.")
    {
        LuaState lua;
        lua_pushinteger(*lua, 42);
        auto reference = dlua::Reference::consume(*lua);

        SECTION("Convert::push pushes the referenced value on the stack.")
        {
            Convert::push(*lua, reference);
            CHECK(lua_tointeger(*lua, -1) == 42);
        }
    }
    SECTION("It can be nested inside a tuple.")
    {
        LuaState lua;
        lua_pushinteger(*lua, 1);
        auto reference1 = dlua::Reference::consume(*lua);
        lua_pushinteger(*lua, 2);
        auto reference2 = dlua::Reference::consume(*lua);

        auto references = std::tuple(std::move(reference1), std::move(reference2));

        dlua::Convert<decltype(references)>::push(*lua, references);

        CHECK(lua_tointeger(*lua, 1) == 1);
        CHECK(lua_tointeger(*lua, 2) == 2);
    }
}
