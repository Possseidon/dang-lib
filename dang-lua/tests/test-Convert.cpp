#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <string>
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
        if (lua_pcall(state_, 0, 0, 0) == LUA_OK)
            return std::string();
        return dlua::Convert<std::string>::at(state_, -1).value_or(std::string());
    }

private:
    lua_State* state_;
};

template <typename... T>
using maybe_cref = std::tuple<T..., const T..., T&..., const T&..., T&&..., const T&&...>;

// --- Convert<userdata>

struct DefaultClassInfoTag {};

// single helper
struct TableClassInfoTag {};
struct MetatableClassInfoTag {};
struct PropertyClassInfoTag {};

// combined helper
struct TableMetatableClassInfoTag {};
struct TablePropertiesClassInfoTag {};
struct MetatablePropertiesClassInfoTag {};
struct TableMetatablePropertiesClassInfoTag {};

struct AllowTableInitializationClassInfoTag {};

template <typename TClassInfoTag = DefaultClassInfoTag>
struct TestClass {
    TestClass() = default;

    TestClass(std::string name, int data)
        : name(name)
        , data(data)
    {}

    std::string name;
    int data = 0;
};

template <typename TClass>
int luaGetName(lua_State* state)
{
    dlua::Convert<std::string>::push(state, dlua::Convert<TClass>::check(state, 1).name);
    return 1;
}

template <typename TClass>
int luaSetName(lua_State* state)
{
    auto& value = dlua::Convert<TClass>::check(state, 1);
    value.name = dlua::Convert<std::string>::check(state, 2);
    return 0;
}

template <typename TClass>
int luaGetData(lua_State* state)
{
    dlua::Convert<int>::push(state, dlua::Convert<TClass>::check(state, 1).data);
    return 1;
}

template <typename TClass>
int luaSetData(lua_State* state)
{
    auto& value = dlua::Convert<TClass>::check(state, 1);
    value.data = dlua::Convert<int>::check(state, 2);
    return 0;
}

namespace dang::lua {

/// @brief Does not have any special behavior.
template <>
struct ClassInfo<TestClass<>> : DefaultClassInfo {
    static constexpr const char* className() { return "TestClass"; }
};

template <>
struct ClassInfo<TestClass<TableClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<TableClassInfoTag>;

    static constexpr auto table() { return std::array{luaL_Reg{"getName", luaGetName<Class>}}; }
};

template <>
struct ClassInfo<TestClass<MetatableClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<MetatableClassInfoTag>;

    static constexpr auto metatable() { return std::array{luaL_Reg{"__index", luaIndex}}; }

private:
    static int luaIndex(lua_State* state)
    {
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "getData") {
            lua_pushcfunction(state, luaGetData<Class>);
            return 1;
        }
        return 0;
    }
};

template <>
struct ClassInfo<TestClass<PropertyClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<PropertyClassInfoTag>;

    static constexpr auto properties()
    {
        return std::array{Property{"name", luaGetName<Class>, luaSetName<Class>},
                          Property{"nameReadOnly", luaGetName<Class>},
                          Property{"nameWriteOnly", nullptr, luaSetName<Class>}};
    }
};

template <>
struct ClassInfo<TestClass<TableMetatableClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<TableMetatableClassInfoTag>;

    static constexpr auto table() { return std::array{luaL_Reg{"getName", luaGetName<Class>}}; }
    static constexpr auto metatable() { return std::array{luaL_Reg{"__index", luaIndex}}; }

private:
    static int luaIndex(lua_State* state)
    {
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "getName") {
            return luaL_error(state, "ClassInfo::table() should have priority");
        }
        if (key == "getData") {
            lua_pushcfunction(state, luaGetData<Class>);
            return 1;
        }
        return 0;
    }
};

template <>
struct ClassInfo<TestClass<TablePropertiesClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<TablePropertiesClassInfoTag>;

    static constexpr auto table()
    {
        return std::array{luaL_Reg{"name", luaGetName<Class>},
                          luaL_Reg{"nameWriteOnly", luaGetName<Class>},
                          luaL_Reg{"getName", luaGetName<Class>}};
    }
    static constexpr auto properties()
    {
        return std::array{Property{"name", luaGetName<Class>, luaSetName<Class>},
                          Property{"nameReadOnly", luaGetName<Class>},
                          Property{"nameWriteOnly", nullptr, luaSetName<Class>}};
    }
};

template <>
struct ClassInfo<TestClass<MetatablePropertiesClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<MetatablePropertiesClassInfoTag>;

    static constexpr auto metatable()
    {
        return std::array{luaL_Reg{"__index", luaIndex}, luaL_Reg{"__newindex", luaNewIndex}};
    }
    static constexpr auto properties()
    {
        return std::array{Property{"name", luaGetName<Class>, luaSetName<Class>},
                          Property{"nameReadOnly", luaGetName<Class>},
                          Property{"nameWriteOnly", nullptr, luaSetName<Class>}};
    }

private:
    static int luaIndex(lua_State* state)
    {
        auto& self = dlua::Convert<Class>::check(state, 1);
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "name") {
            return luaL_error(state, "property should have priority");
        }
        if (key == "nameWriteOnly" || key == "nameIndex") {
            dlua::Convert<std::string>::push(state, self.name);
            return 1;
        }
        if (key == "data") {
            dlua::Convert<int>::push(state, self.data);
            return 1;
        }
        return 0;
    }

    static int luaNewIndex(lua_State* state)
    {
        auto& self = dlua::Convert<Class>::check(state, 1);
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "name") {
            return luaL_error(state, "property should have priority");
        }
        if (key == "nameReadOnly" || key == "nameNewIndex") {
            self.name = dlua::Convert<std::string>::check(state, 3);
            return 0;
        }
        if (key == "data") {
            self.data = dlua::Convert<int>::check(state, 3);
            return 1;
        }
        return 0;
    }
};

template <>
struct ClassInfo<TestClass<TableMetatablePropertiesClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<TableMetatablePropertiesClassInfoTag>;

    static constexpr auto table()
    {
        return std::array{luaL_Reg{"name", luaGetName<Class>},
                          luaL_Reg{"nameReadOnly", luaGetName<Class>},
                          luaL_Reg{"nameWriteOnly", luaGetName<Class>},
                          luaL_Reg{"getName", luaGetName<Class>}};
    }
    static constexpr auto metatable()
    {
        return std::array{luaL_Reg{"__index", luaIndex}, luaL_Reg{"__newindex", luaNewIndex}};
    }
    static constexpr auto properties()
    {
        return std::array{Property{"name", luaGetName<Class>, luaSetName<Class>},
                          Property{"nameReadOnly", luaGetName<Class>},
                          Property{"nameWriteOnly", nullptr, luaSetName<Class>},
                          Property{"dataReadOnly", luaGetData<Class>}};
    }

private:
    static int luaIndex(lua_State* state)
    {
        auto& self = dlua::Convert<Class>::check(state, 1);
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "name" || key == "nameReadOnly" || key == "dataReadOnly") {
            return luaL_error(state, "property should have priority");
        }
        if (key == "nameWriteOnly" || key == "getName") {
            return luaL_error(state, "table should have priority");
        }
        if (key == "data" || key == "dataWriteOnly" || key == "getData") {
            dlua::Convert<int>::push(state, self.data);
            return 1;
        }
        return 0;
    }

    static int luaNewIndex(lua_State* state)
    {
        auto& self = dlua::Convert<Class>::check(state, 1);
        auto key = dlua::Convert<std::string>::check(state, 2);
        if (key == "name" || key == "nameWriteOnly") {
            return luaL_error(state, "property should have priority");
        }
        if (key == "nameReadOnly" || key == "nameNewIndex") {
            self.name = dlua::Convert<std::string>::check(state, 3);
            return 0;
        }
        if (key == "data" || key == "dataReadOnly") {
            self.data = dlua::Convert<int>::check(state, 3);
            return 0;
        }
        return 0;
    }
};

/// @brief Allows table initialization and comes with a setter for name.
template <>
struct ClassInfo<TestClass<AllowTableInitializationClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<AllowTableInitializationClassInfoTag>;

    static constexpr auto allow_table_initialization = true;
    static constexpr auto properties() { return std::array{Property{"name", nullptr, luaSetName<Class>}}; }
};

} // namespace dang::lua

TEMPLATE_LIST_TEST_CASE("Convert can work with custom class types as userdata.",
                        "[lua][convert][class]",
                        maybe_cref<TestClass<>>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and has the specialized name.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "TestClass");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::push can be used to push new instances.")
        {
            auto value = TestClass("test", 42);

            SECTION("... copy-constructed.") { Convert::push(*lua, value); }
            SECTION("... move-constructed.") { Convert::push(*lua, std::move(value)); }
            SECTION("... in-place-constructed.") { Convert::push(*lua, "test", 42); }

            CHECK(Convert::isExact(*lua, 1));
            CHECK(Convert::isValid(*lua, 1));

            auto& at_value = Convert::at(*lua, 1).value().get();
            CHECK(at_value.name == "test");
            CHECK(at_value.data == 42);

            auto& checked_value = Convert::check(*lua, 1);
            CHECK(checked_value.name == "test");
            CHECK(checked_value.data == 42);

            CHECK(&at_value == &checked_value);
        }
        SECTION("Convert::push can be used to push references.")
        {
            auto value = TestClass("test", 42);

            Convert::push(*lua, std::ref(value));

            CHECK(Convert::isExact(*lua, 1));
            CHECK(Convert::isValid(*lua, 1));

            auto& at_value = Convert::at(*lua, 1).value().get();
            CHECK(&at_value == &value);

            auto& checked_value = Convert::check(*lua, 1);
            CHECK(&checked_value == &value);
        }
    }
}

TEST_CASE("ClassInfo can specialize the behavior of a classes userdata.", "[lua][class]")
{
    LuaState lua;

    SECTION("DefaultClassInfo does not provide any special behavior.")
    {
        using Class = TestClass<DefaultClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Indexing operations throw a Lua error.")
        {
            // Not just __newindex, but also __index throws an error in this case.
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_getfield(*lua, -1, "name");
            }) == "attempt to index a TestClass value");
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushliteral(*lua, "test");
                lua_setfield(*lua, -2, "name");
            }) == "attempt to index a TestClass value");
        }
    }
    SECTION("ClassInfo can provide a simple table of methods.")
    {
        using Class = TestClass<TableClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Entries can be read.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "getName") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Reading invalid entries returns nil.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "invalid") == LUA_TNIL);
        }
        SECTION("Writing to an entry throws a Lua error.")
        {
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushinteger(*lua, 42);
                lua_setfield(*lua, -2, "getName");
            }) == "attempt to index a TestClass value");
        }
    }
    SECTION("ClassInfo can customize the metatable of a type.")
    {
        using Class = TestClass<MetatableClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        Convert::push(*lua, "test", 42);
        REQUIRE(lua_getfield(*lua, -1, "getData") == LUA_TFUNCTION);
        lua_pushvalue(*lua, -2);
        lua_call(*lua, 1, 1);
        CHECK(dlua::Convert<int>::check(*lua, -1) == 42);
    }
    SECTION("ClassInfo can provide a list of properties with getter and/or setter.")
    {
        using Class = TestClass<PropertyClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Properties can be read.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "name") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties can be read-only.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameReadOnly") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Write-only properties return nil when read.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameWriteOnly") == LUA_TNIL);
        }
        SECTION("Properties can be written.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "name");
            CHECK(value.name == "new");
        }
        SECTION("Properties can be write-only.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameWriteOnly");
            CHECK(value.name == "new");
        }
        SECTION("Read-only properties throw a Lua error when written.")
        {
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushliteral(*lua, "new");
                lua_setfield(*lua, -2, "nameReadOnly");
            }) == "cannot write property TestClass.nameReadOnly");
        }
        SECTION("Reading an invalid property returns nil.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "invalid") == LUA_TNIL);
        }
        SECTION("Writing an invalid property throws a Lua error.")
        {
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushinteger(*lua, 42);
                lua_setfield(*lua, -2, "invalid");
            }) == "cannot write property TestClass.invalid");
        }
    }
    SECTION("When both table and __index metafield are provided, the table has priority.")
    {
        using Class = TestClass<TableMetatableClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("ClassInfo::table() has priority.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "getName") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("If ClassInfo::table() does not contain the key, __index is still used.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "getData") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<int>::check(*lua, -1) == 42);
        }
    }
    SECTION("When both table and properties are available, properties have priority.")
    {
        using Class = TestClass<TablePropertiesClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Properties have priority when reading.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "name") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties can be read-only.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameReadOnly") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties have priority when writing.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "name");
            CHECK(value.name == "new");
        }
        SECTION("Properties can be write-only.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameWriteOnly");
            CHECK(value.name == "new");
        }
        SECTION("Reading a write-only property uses table as a fallback.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "nameWriteOnly") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Reading an invalid property uses table as a fallback.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "getName") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Writing a read-only property throws a Lua error, as table is not writable either.")
        {
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushinteger(*lua, 256);
                lua_setfield(*lua, -2, "dataReadOnly");
            }) == "cannot write property TestClass.dataReadOnly");
        }
        SECTION("Writing an invalid property throws a Lua error, as table is not writable either.")
        {
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_pushinteger(*lua, 256);
                lua_setfield(*lua, -2, "invalid");
            }) == "cannot write property TestClass.invalid");
        }
    }
    SECTION("When both __index metafield and properties are available, properties have priority.")
    {
        using Class = TestClass<MetatablePropertiesClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Properties have priority when reading.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "name") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties can be read-only.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameReadOnly") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties have priority when writing.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "name");
            CHECK(value.name == "new");
        }
        SECTION("Properties can be write-only.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameWriteOnly");
            CHECK(value.name == "new");
        }
        SECTION("Reading a write-only property uses __index as a fallback.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameWriteOnly") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Reading an invalid property uses __index as a fallback.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "data") == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::check(*lua, -1) == 42);
        }
        SECTION("Writing a read-only property uses __newindex as a fallback.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameReadOnly");
            CHECK(value.name == "new");
        }
        SECTION("Writing an invalid property uses __newindex as a fallback.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushinteger(*lua, 256);
            lua_setfield(*lua, -2, "data");
            CHECK(value.data == 256);
        }
    }
    SECTION("Properties have priority over table and table has priority over __index.")
    {
        using Class = TestClass<TableMetatablePropertiesClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Properties have priority when reading.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "name") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties can be read-only.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "nameReadOnly") == LUA_TSTRING);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Properties have priority when writing.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "name");
            CHECK(value.name == "new");
        }
        SECTION("Properties can be write-only.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameWriteOnly");
            CHECK(value.name == "new");
        }
        SECTION("Reading a write-only property uses table as a first fallback.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "nameWriteOnly") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Reading a write-only property uses __index as a second fallback.")
        {
            Convert::push(*lua, "test", 42);
            CHECK(lua_getfield(*lua, -1, "dataWriteOnly") == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::check(*lua, -1) == 42);
        }
        SECTION("Reading an invalid property uses table as a first fallback.")
        {
            Convert::push(*lua, "test", 42);
            REQUIRE(lua_getfield(*lua, -1, "getName") == LUA_TFUNCTION);
            lua_pushvalue(*lua, -2);
            lua_call(*lua, 1, 1);
            CHECK(dlua::Convert<std::string>::check(*lua, -1) == "test");
        }
        SECTION("Reading an invalid property uses __index as a second fallback.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            REQUIRE(lua_getfield(*lua, -1, "getData") == LUA_TNUMBER);
            CHECK(dlua::Convert<int>::check(*lua, -1) == 42);
        }
        SECTION("Writing a read-only property uses __newindex as a fallback.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameReadOnly");
            CHECK(value.name == "new");
        }
        SECTION("Writing an invalid property uses __newindex as a fallback.")
        {
            auto value = Class("test", 42);
            Convert::push(*lua, std::ref(value));
            lua_pushliteral(*lua, "new");
            lua_setfield(*lua, -2, "nameNewIndex");
            CHECK(value.name == "new");
        }
    }
    SECTION("A class can be enabled to implicitly convert from a table, assigning all key-value pairs of the table.")
    {
        using Class = TestClass<AllowTableInitializationClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        lua_newtable(*lua);
        lua_pushliteral(*lua, "test");
        lua_setfield(*lua, -2, "name");

        Class value;
        SECTION("Using Convert::at") { value = Convert::at(*lua, 1).value(); }
        SECTION("Using Convert::check") { value = Convert::check(*lua, 1); }

        CHECK(value.name == "test");
        CHECK(value.data == 0);

        UNSCOPED_INFO("The value is changed in place.");
        CHECK(lua_type(*lua, 1) == LUA_TUSERDATA);
    }
}

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
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (string expected, got no value)");

            lua_pushstring(*lua, "first");
            CHECK(Convert::at(*lua, -1) == TestEnum::First);
            lua_pushstring(*lua, "second");
            CHECK(Convert::at(*lua, -1) == TestEnum::Second);
            lua_pushstring(*lua, "third");
            CHECK(Convert::at(*lua, -1) == TestEnum::Third);

            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "first_");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (invalid option 'first_')");

            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "_first");
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
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (nil expected, got number)");
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

        SECTION("Convert::isExact returns true only for actual booleans.")
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
            CHECK_FALSE(lua_toboolean(*lua, -1));
            Convert::push(*lua, true);
            CHECK(lua_type(*lua, -1) == LUA_TBOOLEAN);
            CHECK(lua_toboolean(*lua, -1));
        }
    }
}

// --- Convert<number>

using number_types = maybe_cref<float, double>;
TEMPLATE_LIST_TEST_CASE("Convert can work with numbers.", "[lua][convert][number]", number_types)
{
    using Number = dutils::remove_cvref_t<TestType>;
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and is named 'number'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "number");
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
            lua_pushstring(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushstring(*lua, "42.0");
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
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushstring(*lua, "42");
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
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::at(*lua, -1) == Number{42});
            lua_pushstring(*lua, "42");
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
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::check(*lua, -1) == Number{42});
            lua_pushstring(*lua, "42");
            CHECK(Convert::check(*lua, -1) == Number{42});
            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "test");
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (string cannot be converted to a number)");
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (number expected, got boolean)");
        }
        SECTION("Convert::push pushes a number on the stack.")
        {
            Convert::push(*lua, Number{42});
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            CHECK(lua_tonumber(*lua, -1) == 42.0);
        }
    }
}

// --- Convert<integer>

using integer_types = maybe_cref<std::int8_t,
                                 std::int16_t,
                                 std::int32_t,
                                 std::int64_t,
                                 std::uint8_t,
                                 std::uint16_t,
                                 std::uint32_t,
                                 std::uint64_t>;
TEMPLATE_LIST_TEST_CASE("Convert can work with integers.", "[lua][convert][integer]", integer_types)
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

    SECTION("It has a push count of 1, allows nesting and is named 'integer'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "integer");
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
            lua_pushstring(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushstring(*lua, "42.0");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushstring(*lua, "42.5");
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
            lua_pushstring(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushstring(*lua, "42.5");
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
            lua_pushstring(*lua, "42");
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::at(*lua, -1) == Integer{42});
            lua_pushstring(*lua, "42.5");
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
            lua_pushstring(*lua, "42");
            CHECK(Convert::check(*lua, -1) == Integer{42});
            lua_pushstring(*lua, "42.0");
            CHECK(Convert::check(*lua, -1) == Integer{42});
            CHECK(lua.shouldThrow([&] {
                lua_pushstring(*lua, "42.5");
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

// --- Convert<string>

using string_types = maybe_cref<std::string, std::string_view, const char*, char*, const char[5], char[5]>;
TEMPLATE_LIST_TEST_CASE("Convert can work with strings.", "[lua][convert][string]", string_types)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and is named 'string'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "string");
    }
}

using validity_string_types = maybe_cref<std::string, std::string_view>;
TEMPLATE_LIST_TEST_CASE("Convert check for validity of std::string and std::string_view.",
                        "[lua][convert][string]",
                        validity_string_types)
{
    using Convert = dlua::Convert<TestType>;

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

TEMPLATE_LIST_TEST_CASE("Convert can work with std::string.", "[lua][convert][string]", maybe_cref<std::string>)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

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

TEMPLATE_LIST_TEST_CASE("Convert can work with std::string_view.",
                        "[lua][convert][string]",
                        maybe_cref<std::string_view>)
{
    using namespace std::literals::string_view_literals;

    using Convert = dlua::Convert<TestType>;

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

using char_array_types = maybe_cref<const char[5], char[5]>;
TEMPLATE_LIST_TEST_CASE("Convert can work with references to const char arrays.",
                        "[lua][convert][string]",
                        char_array_types)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;

    LuaState lua;

    SECTION("Convert::push trims of a potential null-terminator.")
    {
        Convert::push(*lua, "test");
        CHECK(dlua::Convert<std::string>::at(*lua, -1) == "test"s);
    }
    SECTION("Convert::push preserves embedded null-terminators.")
    {
        const char value[5] = {'\0', 'a', '\0', '\0', 'e'};
        Convert::push(*lua, value);
        CHECK(dlua::Convert<std::string>::at(*lua, -1) == "\0a\0\0e"s);
    }
}

TEMPLATE_LIST_TEST_CASE("Convert can work with C-Style strings.", "[lua][convert][string]", maybe_cref<const char*>)
{
    using namespace std::literals::string_literals;
    using Convert = dlua::Convert<TestType>;

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

using cstyle_strings = maybe_cref<const char*, char*>;
TEMPLATE_LIST_TEST_CASE("Convert can push C-Style strings.", "[lua][convert][string]", cstyle_strings)
{
    using Convert = dlua::Convert<TestType>;

    LuaState lua;

    SECTION("Convert::push pushes a string on the stack.")
    {
        Convert::push(*lua, "test");
        CHECK(dlua::Convert<std::string>::at(*lua, -1) == "test");
    }
}

// --- Convert<function>

int dummyLuaFunction(lua_State*) { return 0; }

TEMPLATE_LIST_TEST_CASE("Convert can work with Lua C functions.", "[lua][convert][function]", maybe_cref<lua_CFunction>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It has a push count of 1, allows nesting and is named 'C function'.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        STATIC_REQUIRE(Convert::getPushTypename() == "C function");
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact and Convert::isValid only return true for C functions.")
        {
            auto isExactAndValid = GENERATE(&Convert::isExact, &Convert::isValid);

            CHECK_FALSE(isExactAndValid(*lua, 1));
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(isExactAndValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(isExactAndValid(*lua, -1));
        }
        SECTION("Convert::at returns C functions and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1) == std::nullopt);
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(Convert::at(*lua, -1) == dummyLuaFunction);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns C functions and throws a Lua error otherwise.")
        {
            CHECK(lua.shouldThrow([&] { Convert::check(*lua, 1); }) ==
                  "bad argument #1 to '?' (C function expected, got no value)");
            lua_pushcfunction(*lua, dummyLuaFunction);
            CHECK(Convert::check(*lua, -1) == dummyLuaFunction);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (C function expected, got boolean)");
        }
        SECTION("Convert::push pushes a C function.")
        {
            Convert::push(*lua, dummyLuaFunction);
            CHECK(lua_type(*lua, -1) == LUA_TFUNCTION);
            CHECK(lua_tocfunction(*lua, -1) == dummyLuaFunction);
        }
    }
}

// --- Convert<optional>

TEMPLATE_LIST_TEST_CASE("Convert can work with std::optional.",
                        "[lua][convert][nil][optional]",
                        maybe_cref<std::optional<int>>)
{
    using namespace std::literals::string_literals;

    using Convert = dlua::Convert<TestType>;
    using ConvertContained = dlua::Convert<int>;

    SECTION("It has a push count of 1, allows nesting and is named the same as the contained type.")
    {
        STATIC_REQUIRE(Convert::push_count == 1);
        STATIC_REQUIRE(Convert::allow_nesting);
        CHECK(Convert::getPushTypename() == std::string(ConvertContained::getPushTypename()) + "?"s);
    }
    SECTION("Given a Lua state.")
    {
        LuaState lua;

        SECTION("Convert::isExact returns true for exact values and nil/none.")
        {
            CHECK(Convert::isExact(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isExact(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK_FALSE(Convert::isExact(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isExact(*lua, -1));
        }
        SECTION("Convert::isValid returns true for valid values and nil/none.")
        {
            CHECK(Convert::isValid(*lua, 1));
            lua_pushnil(*lua);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushinteger(*lua, 42);
            CHECK(Convert::isValid(*lua, -1));
            lua_pushliteral(*lua, "42");
            CHECK(Convert::isValid(*lua, -1));
            lua_pushboolean(*lua, true);
            CHECK_FALSE(Convert::isValid(*lua, -1));
        }
        SECTION("Convert::at returns an optional that may contain a std::nullopt and std::nullopt otherwise.")
        {
            CHECK(Convert::at(*lua, 1).value() == std::nullopt);
            lua_pushnil(*lua);
            CHECK(Convert::at(*lua, -1).value() == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::at(*lua, -1).value() == 42);
            lua_pushliteral(*lua, "42");
            CHECK(Convert::at(*lua, -1).value() == 42);
            lua_pushboolean(*lua, true);
            CHECK(Convert::at(*lua, -1) == std::nullopt);
        }
        SECTION("Convert::check returns an optional that may contain a std::nullopt and throws a Lua error otherwise.")
        {
            CHECK(Convert::check(*lua, 1) == std::nullopt);
            lua_pushnil(*lua);
            CHECK(Convert::check(*lua, -1) == std::nullopt);
            lua_pushinteger(*lua, 42);
            CHECK(Convert::check(*lua, -1) == 42);
            lua_pushliteral(*lua, "42");
            CHECK(Convert::check(*lua, -1) == 42);
            CHECK(lua.shouldThrow([&] {
                lua_pushboolean(*lua, true);
                Convert::check(*lua, 1);
            }) == "bad argument #1 to '?' (integer? expected, got boolean)");
        }
        SECTION("Convert::push pushes the value or nil.")
        {
            Convert::push(*lua, 42);
            CHECK(lua_type(*lua, -1) == LUA_TNUMBER);
            CHECK(lua_tointeger(*lua, -1) == 42);
            Convert::push(*lua, std::nullopt);
            CHECK(lua_type(*lua, -1) == LUA_TNIL);
        }
    }
}

// --- Convert<pair>

// --- Convert<tuple>

// --- Convert<variant>
