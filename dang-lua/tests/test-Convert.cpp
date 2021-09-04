#include <cstddef>
#include <optional>
#include <variant>

#include "dang-lua/Convert.h"

#include "dang-utils/utils.h"

#include "catch2/catch.hpp"

namespace dlua = dang::lua;
namespace dutils = dang::utils;

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

template <typename... T>
using maybe_cref = std::tuple<T..., const T..., T&..., const T&..., T&&..., const T&&...>;

// --- Convert<userdata>

struct TestClass {};

// --- Convert<enum>

enum class TestEnum { First, Second, Third };

namespace dang::lua {

template <>
inline constexpr const char* enum_values<TestEnum>[4] = {"first", "second", "third"};

template <>
inline constexpr std::string_view enum_name<TestEnum> = "TestEnum";

} // namespace dang::lua

TEMPLATE_LIST_TEST_CASE("Convert can work with enum values, converting them to and from strings.",
                        "[lua][convert][enum]",
                        maybe_cref<TestEnum>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and has the specialized name.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "TestEnum");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact and Convert::isValid return true for strings that are valid for this enum.")
        {
            auto isExactAndValid = GENERATE(&Convert::isExact, &Convert::isValid);

            CHECK_FALSE(isExactAndValid(*lua, 1));

            lua_pushstring(*lua, "first");
            CHECK(isExactAndValid(*lua, -1));
            lua_pushstring(*lua, "second");
            CHECK(isExactAndValid(*lua, -1));
            lua_pushstring(*lua, "third");
            CHECK(isExactAndValid(*lua, -1));

            lua_pushstring(*lua, "first_");
            CHECK_FALSE(isExactAndValid(*lua, -1));
            lua_pushstring(*lua, "_first");
            CHECK_FALSE(isExactAndValid(*lua, -1));

            lua_pushinteger(*lua, 42);
            CHECK_FALSE(isExactAndValid(*lua, -1));
        }
        SECTION("Convert::at returns the enum value for valid strings and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);

            lua_pushstring(*lua, "first");
            CHECK(Convert::at(*lua, -1) == TestEnum::First);
            lua_pushstring(*lua, "second");
            CHECK(Convert::at(*lua, -1) == TestEnum::Second);
            lua_pushstring(*lua, "third");
            CHECK(Convert::at(*lua, -1) == TestEnum::Third);

            lua_pushstring(*lua, "first_");
            CHECK(Convert::at(*lua, -1) == std::nullopt);
            lua_pushstring(*lua, "_first");
            CHECK(Convert::at(*lua, -1) == std::nullopt);

            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns the enum value for valid strings and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }));

            lua_pushstring(*lua, "first");
            CHECK(Convert::at(*lua, -1) == TestEnum::First);
            lua_pushstring(*lua, "second");
            CHECK(Convert::at(*lua, -1) == TestEnum::Second);
            lua_pushstring(*lua, "third");
            CHECK(Convert::at(*lua, -1) == TestEnum::Third);

            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "first_");
                Convert::check(*lua, -1);
            }));

            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "_first");
                Convert::check(*lua, -1);
            }));

            CHECK(lua.shouldThrow([&] {
                lua_pushinteger(*lua, 42);
                Convert::check(*lua, -1);
            }));
        }
        SECTION("Convert::push pushes the string representation of the enum value on the stack.")
        {
            Convert::push(*lua, TestEnum::First);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(std::string(lua_tostring(*lua, -1)) == "first");

            Convert::push(*lua, TestEnum::Second);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(std::string(lua_tostring(*lua, -1)) == "second");

            Convert::push(*lua, TestEnum::Third);
            CHECK(lua_type(*lua, -1) == LUA_TSTRING);
            CHECK(std::string(lua_tostring(*lua, -1)) == "third");
        }
    }
}

// --- Convert<void>

// TODO: What was this actually used for? Maybe try and get rid of this...
TEST_CASE("Convert does nothing for void type.", "[lua][convert][void]")
{
    using Convert = dlua::Convert<void>;

    SECTION("It has a push count of 0 and disallows nesting.")
    {
        STATIC_REQUIRE(Convert::push_count == 0);
        STATIC_REQUIRE_FALSE(Convert::allow_nesting);
    }
}

// --- Convert<nil>

using nil_types = maybe_cref<std::nullptr_t, std::monostate>;
TEMPLATE_LIST_TEST_CASE("Convert can work with nil-like types.", "[lua][convert][nil]", nil_types)
{
    using Nil = dutils::remove_cvref_t<TestType>;
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
                Convert::check(*lua, -1);
            }));
        }
        SECTION("Convert::push pushes nil on the stack.")
        {
            Convert::push(*lua, Nil());
            CHECK(lua_type(*lua, -1) == LUA_TNIL);
        }
    }
}

// --- Convert<fail>

TEST_CASE("Convert supports pushing fail values.", "[lua][convert][fail]")
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

// --- Convert<boolean>

TEMPLATE_LIST_TEST_CASE("Convert can work with booleans.", "[lua][convert][boolean]", maybe_cref<bool>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and is named 'boolean'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "boolean");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for actual boolean values.")
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
        SECTION("Convert::push pushes a boolean value on the stack.")
        {
            Convert::push(*lua, false);
            CHECK(lua_type(*lua, -1) == LUA_TBOOLEAN);
            Convert::push(*lua, true);
            CHECK(lua_type(*lua, -1) == LUA_TBOOLEAN);
        }
    }
}

// --- Convert<number>

// --- Convert<integer>

// --- Convert<string>

// --- Convert<function>

// --- Convert<optional>

// --- Convert<pair>

// --- Convert<tuple>

// --- Convert<variant>
