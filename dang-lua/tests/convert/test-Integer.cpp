#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

#include "dang-lua/convert/Integer.h"

#include "dang-utils/utils.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;
namespace dutils = dang::utils;

using integer_types = maybe_cv<std::int8_t,
                               std::int16_t,
                               std::int32_t,
                               std::int64_t,
                               std::uint8_t,
                               std::uint16_t,
                               std::uint32_t,
                               std::uint64_t>;

TEMPLATE_LIST_TEST_CASE("Convert can check for integers.", "[lua][convert][integer][check]", integer_types)
{
    using Integer = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    constexpr bool is_int64 = std::is_same_v<Integer, std::int64_t>;
    constexpr bool is_uint64 = std::is_same_v<Integer, std::uint64_t>;

    constexpr auto min_value = std::numeric_limits<Integer>::min();
    constexpr auto max_value = std::numeric_limits<Integer>::max();

    // lua_Integer can fit everything except big uint64 values
    constexpr auto lua_min_value = lua_Integer{min_value};
    constexpr auto lua_max_value = lua_Integer{is_uint64 ? 0 : max_value};

    SECTION("It can be checked from a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_check);
        STATIC_REQUIRE(Convert::check_count == 1);
        CHECK(Convert::getCheckTypename() == "integer");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true only for integers.")
        {
            CHECK_FALSE(Convert::isExact(*lua, 1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushnumber(*lua, 42.5);
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42.0");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42.5");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns for integers and convertible strings.")
        {
            CHECK_FALSE(Convert::isValid(*lua, 1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushnumber(*lua, 42.5);
            CHECK_FALSE(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42.5");
            CHECK_FALSE(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::isExact and Convert::isValid also make sure, that the integer is in range.")
        {
            auto isExactAndValid = GENERATE(&Convert::isExact, &Convert::isValid);

            lua_pushinteger(*lua, lua_min_value);
            CHECK(isExactAndValid(*lua, -1));
            if constexpr (!is_uint64) {
                lua_pushinteger(*lua, lua_max_value);
                CHECK(isExactAndValid(*lua, -1));
            }
            if constexpr (!is_int64) {
                lua_pushinteger(*lua, lua_min_value - 1);
                CHECK_FALSE(isExactAndValid(*lua, -1));
            }
            if constexpr (!is_int64 && !is_uint64) {
                lua_pushinteger(*lua, lua_max_value + 1);
                CHECK_FALSE(isExactAndValid(*lua, -1));
            }
        }
        SECTION("Convert::at returns the integer or convertible string and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushnumber(*lua, 42.5);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
            lua_pushliteral(*lua, "42");
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushliteral(*lua, "42.5");
            CHECK(Convert::at(*lua, -1) == std::nullopt);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::at returns std::nullopt when the number is not in range.")
        {
            lua_pushinteger(*lua, lua_min_value);
            CHECK(Convert::at(*lua, -1) == min_value);
            if constexpr (!is_uint64) {
                lua_pushinteger(*lua, lua_max_value);
                CHECK(Convert::at(*lua, -1) == max_value);
            }
            if constexpr (!is_int64) {
                lua_pushinteger(*lua, lua_min_value - 1);
                CHECK(Convert::at(*lua, -1) == std::nullopt);
            }
            if constexpr (!is_int64 && !is_uint64) {
                lua_pushinteger(*lua, lua_max_value + 1);
                CHECK(Convert::at(*lua, -1) == std::nullopt);
            }
        }
        SECTION("Convert::check returns the integer or convertible string and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (integer expected, got no value)");
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == Integer{42});
            lua_pushnumber(*lua, 42.0);
            CHECK(Convert::check(*lua, -1) == Integer{42});
            CHECK(lua.shouldThrow([&] {
                lua_pushnumber(*lua, 42.5);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (number has no integer representation)");
            lua_pushliteral(*lua, "42");
            CHECK(Convert::check(*lua, -1) == Integer{42});
            lua_pushliteral(*lua, "42.0");
            CHECK(Convert::check(*lua, -1) == Integer{42});
            CHECK(lua.shouldThrow([&] {
                lua_pushliteral(*lua, "42.5");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string cannot be converted to an integer)");
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (integer expected, got boolean)");
        }
        SECTION("Convert::check throws a Lua error when the number is not in range.")
        {
            [[maybe_unused]] auto msg_for = [&](lua_Integer value) {
                return "bad argument #1 to '?' (value " + std::to_string(value) + " must be in range " +
                       std::to_string(min_value) + " .. " + std::to_string(max_value) + ")";
            };

            lua_pushinteger(*lua, lua_min_value);
            CHECK(Convert::check(*lua, -1) == min_value);
            if constexpr (!is_uint64) {
                lua_pushinteger(*lua, lua_max_value);
                CHECK(Convert::check(*lua, -1) == max_value);
            }
            if constexpr (!is_int64) {
                CHECK(lua.shouldThrow([&] {
                    lua_pushinteger(*lua, lua_min_value - 1);
                    Convert::check(*lua, 1);
                }) == msg_for(lua_min_value - 1));
            }
            if constexpr (!is_int64 && !is_uint64) {
                CHECK(lua.shouldThrow([&] {
                    lua_pushinteger(*lua, lua_max_value + 1);
                    Convert::check(*lua, 1);
                }) == msg_for(lua_max_value + 1));
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can push integers.", "[lua][convert][integer][push]", integer_types)
{
    using Integer = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    constexpr bool is_uint64 = std::is_same_v<Integer, std::uint64_t>;
    [[maybe_unused]] constexpr auto max_value = std::numeric_limits<Integer>::max();

    SECTION("It can be pushed as a single value and has a typename.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "integer");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push pushes an integer on the stack.")
        {
            Convert::push(*lua, Integer{42});
            CHECK(lua_isinteger(*lua, -1));
            CHECK(lua_tointeger(*lua, -1) == 42);
            if constexpr (is_uint64) {
                INFO("Unsigned numbers that don't fit into a lua_Integer are converted to signed.");
                Convert::push(*lua, max_value);
                CHECK(lua_isinteger(*lua, -1));
                CHECK(lua_tointeger(*lua, -1) == -1);
            }
        }
    }
}
