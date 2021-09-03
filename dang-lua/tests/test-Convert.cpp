#include <cstddef>
#include <optional>
#include <variant>

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"

namespace dlua = dang::lua;

class LuaState {
public:
    LuaState()
        : state_(luaL_newstate())
    {}

    ~LuaState() { lua_close(state_); }

    LuaState(const LuaState&) = delete;
    LuaState(LuaState&&) = delete;
    LuaState& operator=(const LuaState&) = delete;
    LuaState& operator=(LuaState&&) = delete;

    auto operator*() { return state_; }

    template <typename TFunc>
    auto shouldThrow(TFunc func)
    {
        constexpr auto lua_func = +[](lua_State* state) {
            (*static_cast<TFunc*>(lua_touserdata(state, lua_upvalueindex(1))))();
            return 0;
        };
        lua_pushlightuserdata(state_, &func);
        lua_pushcclosure(state_, lua_func, 1);
        return lua_pcall(state_, 0, 0, 0) == LUA_ERRRUN;
    }

private:
    lua_State* state_;
};

TEST_CASE("Convert does nothing for void type.", "[lua][convert]")
{
    using Convert = dlua::Convert<void>;

    STATIC_REQUIRE(Convert::push_count == 0);
    STATIC_REQUIRE_FALSE(Convert::allow_nesting);
}

TEMPLATE_TEST_CASE("Convert can work with nil-like types.", "[lua][convert]", std::nullptr_t, std::monostate)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and is named 'nil'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "nil");
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
            CHECK(Convert::at(*lua, 1) == TestType());
            lua_pushnil(*lua);
            CHECK(Convert::at(*lua, -1) == TestType());
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check return an instance for nil/none and throws a Lua error otherwise.")
        {
            CHECK(Convert::check(*lua, 1) == TestType());
            lua_pushnil(*lua);
            CHECK(Convert::check(*lua, -1) == TestType());
            CHECK(lua.shouldThrow([&] {
                lua_pushinteger(*lua, 42);
                Convert::check(*lua, -1);
            }));
        }
        SECTION("Convert::push pushes nil on the stack.")
        {
            Convert::push(*lua, TestType());
            CHECK(lua_type(*lua, -1) == LUA_TNIL);
        }
    }
}

TEST_CASE("Convert supports pushing fail values.", "[lua][convert]")
{
    using Convert = dlua::Convert<dlua::Fail>;

    SECTION("It has a push count of 1, allows nesting and is named 'nil'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "fail");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes a falsy value on the stack.")
        {
            Convert::push(*lua, dlua::fail);
            CHECK_FALSE(lua_toboolean(*lua, -1));
        }
    }
}
