#include <array>
#include <functional>
#include <string>
#include <utility>

#include "dang-lua/Convert.h"

#include "catch2/catch.hpp"
#include "shared/LuaState.h"
#include "shared/utils.h"

namespace dlua = dang::lua;

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
    static std::string getCheckTypename() { return "TestClass"; }
    static std::string getPushTypename() { return getCheckTypename(); }
};

template <>
struct ClassInfo<TestClass<TableClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<TableClassInfoTag>;

    static constexpr auto methods() { return std::array{luaL_Reg{"getName", luaGetName<Class>}}; }
};

template <>
struct ClassInfo<TestClass<MetatableClassInfoTag>> : ClassInfo<TestClass<>> {
    using Class = TestClass<MetatableClassInfoTag>;

    static constexpr auto metamethods() { return std::array{luaL_Reg{"__index", luaIndex}}; }

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

    static constexpr auto methods() { return std::array{luaL_Reg{"getName", luaGetName<Class>}}; }
    static constexpr auto metamethods() { return std::array{luaL_Reg{"__index", luaIndex}}; }

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

    static constexpr auto methods()
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

    static constexpr auto metamethods()
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

    static constexpr auto methods()
    {
        return std::array{luaL_Reg{"name", luaGetName<Class>},
                          luaL_Reg{"nameReadOnly", luaGetName<Class>},
                          luaL_Reg{"nameWriteOnly", luaGetName<Class>},
                          luaL_Reg{"getName", luaGetName<Class>}};
    }
    static constexpr auto metamethods()
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

// TODO: Convert cannot check for custom class types.

TEMPLATE_LIST_TEST_CASE("Convert can push custom class types as userdata.",
                        "[lua][convert][class][push]",
                        maybe_cv<TestClass<>>)
{
    using Convert = dlua::Convert<TestType>;

    SECTION("It can be pushed as a single value and has the specialized name.")
    {
        STATIC_REQUIRE(Convert::can_push);
        STATIC_REQUIRE(Convert::push_count == 1);
        CHECK(Convert::getPushTypename() == "TestClass");
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

TEST_CASE("ClassInfo can specialize the behavior of a classes userdata.", "[lua][convert][class]")
{
    LuaState lua;

    SECTION("DefaultClassInfo does not provide any special behavior.")
    {
        using Class = TestClass<DefaultClassInfoTag>;
        using Convert = dlua::Convert<Class>;

        SECTION("Indexing operations throw a Lua error.")
        {
            // TODO: This also just returns nil now. Do I wanna keep it that way?

            // Not just __newindex, but also __index throws an error in this case.
            /*
            CHECK(lua.shouldThrow([&] {
                Convert::push(*lua, "test", 42);
                lua_getfield(*lua, -1, "name");
            }) == "attempt to index a TestClass value");
            */
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
        REQUIRE(lua_gettop(*lua) == 1);

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

// TODO: Test const classes once implemented.
// TODO: Test subclass mechanism.
