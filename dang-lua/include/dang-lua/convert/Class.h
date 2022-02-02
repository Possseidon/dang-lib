#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "dang-lua/NoreturnError.h"
#include "dang-lua/convert/Base.h"
#include "dang-lua/global.h"

namespace dang::lua {

/// @brief A named property on a user-defined Lua type with custom (optional) get and set function.
struct Property {
    const char* name;
    lua_CFunction get = nullptr;
    lua_CFunction set = nullptr;
};

namespace detail {

/// @brief Somewhat similar to luaL_setfuncs, except it uses any kind of container.
template <typename TFuncs>
void setFuncs(lua_State* state, const TFuncs& funcs)
{
    for (const auto& func : funcs) {
        assert(func.func != nullptr);
        lua_pushcfunction(state, func.func);
        lua_setfield(state, -2, func.name);
    }
}

/// @brief Returns the number of property accessors which aren't null.
template <typename TProps>
auto countProperties(const TProps& props, lua_CFunction Property::*accessor)
{
    using std::begin, std::end;
    return std::count_if(begin(props), end(props), std::mem_fn(accessor));
}

/// @brief Sets all property accessors as fields unless they are null.
template <typename TProps>
void setPropertyFuncs(lua_State* state, const TProps& props, lua_CFunction Property::*accessor)
{
    for (const auto& prop : props) {
        if (prop.*accessor == nullptr)
            continue;
        lua_pushcfunction(state, prop.*accessor);
        lua_setfield(state, -2, prop.name);
    }
}

/// @brief A Lua class instance can either be its own value or reference an existing instance.
enum class ClassStoreType { Value, Reference };

/// @brief Swaps between value and reference store type.
inline constexpr auto otherClassStoreType(ClassStoreType class_store_type)
{
    switch (class_store_type) {
    case ClassStoreType::Value:
        return ClassStoreType::Reference;
    case ClassStoreType::Reference:
        return ClassStoreType::Value;
    }
}

/// @brief Provides a unique pointers for any given type.
template <typename, detail::ClassStoreType>
struct UniqueClassInfo {
    static void* id() { return reinterpret_cast<void*>(&id); }
};

} // namespace detail

/// @brief Serves as a container for typenames that represent subclasses.
template <typename...>
struct SubClassList {};

/// @brief Can be specialized to provide information about the subclasses of a given class.
template <typename>
struct SubClasses : SubClassList<> {};

template <typename TClass>
constexpr SubClasses<TClass> sub_classes{};

/// @brief A full ClassInfo implementation, meant to be inherited for convenience.
/// @remark Methods and properties can be any kind of collection and need not be constexpr.
struct DefaultClassInfo {
    static constexpr auto specialized = true;

    static std::string getCheckTypename() { return "<class>"; }
    static std::string getPushTypename() { return getCheckTypename(); }

    static constexpr auto allow_table_initialization = false;

    static constexpr std::array<luaL_Reg, 0> methods() { return {}; }
    static constexpr std::array<luaL_Reg, 0> metamethods() { return {}; }
    static constexpr std::array<Property, 0> properties() { return {}; }

    static void require() {}
};

/// @brief Can be specialized to provide an index and metatable of a wrapped class.
template <typename TClass, typename = void>
struct ClassInfo {
    /// @brief A flag to find out if a specific class is specialized and can be used with Convert.
    static constexpr auto specialized = false;
};

/// @brief Converts class types using ClassInfo specializations.
// TODO: Differentiate between constness of classes.
template <typename TClass>
struct Convert<TClass, std::enable_if_t<ClassInfo<std::remove_cv_t<TClass>>::specialized>> {
    using Class = std::remove_cv_t<TClass>;
    using Info = ClassInfo<Class>;
    using StoreType = detail::ClassStoreType;

    static constexpr bool trivially_destructible = std::is_trivially_destructible_v<Class>;

    template <StoreType>
    struct value_store;

    /// @brief Values are wrapped inside an optional for non-trivial destructors.
    template <>
    struct value_store<StoreType::Value> {
        using type = std::conditional_t<trivially_destructible, Class, std::optional<Class>>;
    };

    /// @brief References are stored as a pointer to the object.
    template <>
    struct value_store<StoreType::Reference> {
        using type = Class*;
    };

    template <StoreType v_store_type>
    using value_store_t = typename value_store<v_store_type>::type;

    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;

    static std::string getCheckTypename() { return Info::getCheckTypename(); }

    /// @brief Whether the stack position is a valid class value or reference.
    static bool isExact(lua_State* state, int pos) { return classStoreType(state, pos).has_value(); }

    /// @brief Whether the stack position is a valid class value or can be converted to one.
    static bool isValid(lua_State* state, int pos)
    {
        return Info::allow_table_initialization && lua_istable(state, pos) || isExact(state, pos);
    }

    /// @brief Returns a reference to the value at the given stack position or std::nullopt on failure.
    static auto at(lua_State* state, int pos) -> std::optional<std::reference_wrapper<Class>>
    {
        if (auto value = atRaw<StoreType::Value>(state, pos)) {
            if constexpr (trivially_destructible) {
                return *value;
            }
            else {
                if (*value)
                    return **value;
                noreturn_luaL_error(state, ("attempt to use a closed " + getCheckTypename()).c_str());
            }
        }
        if (auto reference = atRaw<StoreType::Reference>(state, pos)) {
            return **reference;
        }
        return std::nullopt;
    }

    /// @brief Returns a reference to the value at the given argument stack position and raises an argument error on
    /// failure.
    static Class& check(lua_State* state, int arg)
    {
        if (auto result = at(state, arg))
            return *result;
        noreturn_luaL_typeerror(state, arg, getCheckTypename().c_str());
    }

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;

    /// @brief Returns the name of the class.
    static std::string getPushTypename() { return Info::getPushTypename(); }

    /// @brief Pushes the in place constructed value onto the stack.
    template <typename... TValues>
    static auto push(lua_State* state, TValues&&... values)
        -> std::enable_if_t<std::is_constructible_v<Class, std::decay_t<TValues>...>, Class&>
    {
        auto& result = pushRaw<StoreType::Value>(state, std::forward<TValues>(values)...);
        if constexpr (trivially_destructible) {
            return result;
        }
        else {
            return *result;
        }
    }

    /// @brief Pushes a reference to the value on the stack.
    static void push(lua_State* state, std::reference_wrapper<Class> value)
    {
        pushRaw<StoreType::Reference>(state, &value.get());
    }

private:
    template <StoreType v_store_type, typename... TValues>
    static auto pushRaw(lua_State* state, TValues&&... values) -> value_store_t<v_store_type>&
    {
        using Store = value_store_t<v_store_type>;
        auto userdata = static_cast<Store*>(lua_newuserdatauv(state, sizeof(Store), 0));
        if constexpr (v_store_type == StoreType::Reference) {
            static_assert(sizeof...(TValues) == 1);
            new (userdata) Store(values...);
        }
        else if constexpr (trivially_destructible) {
            new (userdata) Store(std::forward<TValues>(values)...);
        }
        else {
            new (userdata) Store(std::in_place, std::forward<TValues>(values)...);
        }
        pushMetatable<v_store_type>(state);
        lua_setmetatable(state, -2);
        return *userdata;
    }

    /// @brief Whether a stack position is a value, reference or neither.
    static std::optional<StoreType> classStoreType(lua_State* state, int pos)
    {
        if (testudata<StoreType::Value>(state, pos))
            return StoreType::Value;
        if (testudata<StoreType::Reference>(state, pos))
            return StoreType::Reference;
        return classStoreType(state, pos, sub_classes<Class>);
    }

    /// @brief Returns a pointer to the stored value at the given stack position or null on failure.
    template <StoreType v_store_type>
    static auto atRaw(lua_State* state, int pos) -> value_store_t<v_store_type>*
    {
        if constexpr (v_store_type == StoreType::Value && Info::allow_table_initialization) {
            if (lua_istable(state, pos)) {
                luaL_checkstack(state, 5, nullptr);

                auto abs_pos = lua_absindex(state, pos);
                auto& value = pushRaw<v_store_type>(state);

                lua_pushnil(state);
                while (lua_next(state, abs_pos)) {
                    // duplicate key and value
                    lua_pushvalue(state, -2);
                    lua_pushvalue(state, -2);
                    // userdata[key] = value
                    lua_settable(state, -5);
                    // pop value, leave key
                    lua_pop(state, 1);
                }

                lua_replace(state, pos);
                return &value;
            }
        }

        if (void* pointer = testudata<v_store_type>(state, pos))
            return static_cast<value_store_t<v_store_type>*>(pointer);

        return atRaw<v_store_type>(state, pos, sub_classes<Class>);
    }

    template <StoreType v_store_type>
    static auto checkRaw(lua_State* state, int pos) -> value_store_t<v_store_type>&
    {
        if (auto pointer = atRaw<v_store_type>(state, pos))
            return *pointer;
        noreturn_luaL_typeerror(state, pos, getCheckTypename().c_str());
    }

    /// @brief Pushes the metatable for a value or reference instance onto the stack.
    template <StoreType v_store_type>
    static void pushMetatable(lua_State* state)
    {
        if (!newmetatable<v_store_type>(state))
            return;

        detail::setFuncs(state, Info::metamethods());

        registerIndex<v_store_type>(state);
        registerNewindex<v_store_type>(state);
        registerDisplayName(state);
        if constexpr (canClose(v_store_type)) {
            registerCloseMetamethods(state);
        }

        protectMetatable(state);
    }

    /// @brief Pushes this types metatable or nil on the stack.
    template <StoreType v_store_type>
    static int getmetatable(lua_State* state)
    {
        return lua_rawgetp(state, LUA_REGISTRYINDEX, detail::UniqueClassInfo<Class, v_store_type>::id());
    }

    /// @brief Creates a new metatable for this type unless it already exists and pushes it on the stack in either case.
    template <StoreType v_store_type>
    static bool newmetatable(lua_State* state)
    {
        if (getmetatable<v_store_type>(state))
            return false;
        lua_pop(state, 1);
        lua_newtable(state);
        lua_pushvalue(state, -1);
        lua_rawsetp(state, LUA_REGISTRYINDEX, detail::UniqueClassInfo<Class, v_store_type>::id());
        return true;
    }

    /// @brief Checks if the given argument has the correct metatable and returns the userdata pointer.
    template <StoreType v_store_type>
    static void* testudata(lua_State* state, int arg)
    {
        void* value = lua_touserdata(state, arg);
        if (!value || !lua_getmetatable(state, arg))
            return nullptr;
        lua_rawgetp(state, LUA_REGISTRYINDEX, detail::UniqueClassInfo<Class, v_store_type>::id());
        if (!lua_rawequal(state, -2, -1))
            value = nullptr;
        lua_pop(state, 2);
        return value;
    }

    /// @brief Checks whether the type matches any of the supplied sub classes.
    template <typename TFirst, typename... TRest>
    static std::optional<StoreType> classStoreType(lua_State* state, int pos, SubClassList<TFirst, TRest...>)
    {
        if (auto result = Convert<TFirst>::classStoreType(state, pos))
            return result;
        return classStoreType(state, pos, SubClassList<TRest...>());
    }

    /// @brief Serves as an exit condition when the list of sub classes is depleted.
    static std::optional<StoreType> classStoreType(lua_State*, int, SubClassList<>) { return std::nullopt; }

    /// @brief Goes through the full list of subclasses to try and convert the value.
    template <StoreType v_store_type, typename TFirst, typename... TRest>
    static auto atRaw(lua_State* state, int pos, SubClassList<TFirst, TRest...>) -> value_store_t<v_store_type>*
    {
        if (auto result = Convert<TFirst>::atRaw(state, pos))
            return result;
        return atRaw(state, pos, SubClassList<TRest...>());
    }

    /// @brief Exit condition when the subclass list is depleted.
    template <StoreType v_store_type>
    static auto atRaw(lua_State*, int, SubClassList<>) -> value_store_t<v_store_type>*
    {
        return nullptr;
    }

    /// @brief Registers the __index metamethod to the metatable on the top of the stack.
    template <StoreType v_store_type>
    static void registerIndex(lua_State* state)
    {
        if (getmetatable<otherClassStoreType(v_store_type)>(state) != LUA_TNIL) {
            lua_getfield(state, -1, "__index");
            lua_setfield(state, -3, "__index");
            lua_pop(state, 1);
            return;
        }
        lua_pop(state, 1);

        int pushed = 0;

        // Push table for properties.
        const auto& properties = Info::properties();
        auto get_count = detail::countProperties(properties, &Property::get);
        auto has_properties = get_count > 0;
        if (has_properties) {
            lua_createtable(state, 0, static_cast<int>(get_count));
            pushed++;
            detail::setPropertyFuncs(state, properties, &Property::get);
            lua_pushvalue(state, -1);
            lua_setfield(state, -2 - pushed, "get");
        }

        // Push methods.
        const auto& methods = Info::methods();
        constexpr auto can_close = canClose(v_store_type);
        auto has_indextable = !methods.empty() || can_close;
        if (has_indextable) {
            lua_createtable(state, 0, static_cast<int>(methods.size()));
            pushed++;
            if constexpr (can_close) {
                registerCloseMethods(state);
            }
            detail::setFuncs(state, methods);
            lua_pushvalue(state, -1);
            lua_setfield(state, -2 - pushed, "indextable");
        }

        // Push metamethods __index.
        auto has_indexfunction = lua_getfield(state, -1 - pushed, "__index") != LUA_TNIL;
        if (has_indexfunction)
            pushed++;
        else
            lua_pop(state, 1);

        if (pushed == 0)
            return;

        if (has_properties) {
            if (has_indextable) {
                if (has_indexfunction)
                    lua_pushcclosure(state, customIndex<1, 2, 3>, 3);
                else
                    lua_pushcclosure(state, customIndex<1, 2, 0>, 2);
            }
            else {
                if (has_indexfunction)
                    lua_pushcclosure(state, customIndex<1, 0, 2>, 2);
                else
                    lua_pushcclosure(state, customIndex<1, 0, 0>, 1);
            }
        }
        else if (has_indextable && has_indexfunction)
            lua_pushcclosure(state, customIndex<0, 1, 2>, 2);
        // else leave singular index table or function on the stack

        lua_setfield(state, -2, "__index");
    }

    /// @brief Registers the __newindex metamethod to the metatable on the top of the stack.
    template <StoreType v_store_type>
    static void registerNewindex(lua_State* state)
    {
        if (getmetatable<otherClassStoreType(v_store_type)>(state) != LUA_TNIL) {
            lua_getfield(state, -1, "__newindex");
            lua_setfield(state, -3, "__newindex");
            lua_pop(state, 1);
            return;
        }
        lua_pop(state, 1);

        int pushed = 0;

        // Push table for properties.
        const auto& properties = Info::properties();
        auto set_count = detail::countProperties(properties, &Property::set);
        auto has_properties = set_count > 0;
        if (has_properties) {
            lua_createtable(state, 0, static_cast<int>(set_count));
            pushed++;
            detail::setPropertyFuncs(state, properties, &Property::set);
            lua_pushvalue(state, -1);
            lua_setfield(state, -2 - pushed, "set");
        }

        // Push metamethods __newindex.
        auto has_newindex = lua_getfield(state, -1 - pushed, "__newindex") != LUA_TNIL;
        if (has_newindex)
            pushed++;
        else
            lua_pop(state, 1);

        if (pushed == 0)
            return;

        if (has_properties) {
            if (has_newindex)
                lua_pushcclosure(state, customNewindex<1, 2>, 2);
            else
                lua_pushcclosure(state, customNewindex<1, 0>, 1);
        }
        else if (has_newindex)
            lua_pushcclosure(state, customNewindex<0, 1>, 1);

        lua_setfield(state, -2, "__newindex");
    }

    /// @brief Registers the display name provided by `ClassInfo` to the metatable on the top of the stack.
    static void registerDisplayName(lua_State* state)
    {
        lua_pushstring(state, getCheckTypename().c_str());
        lua_setfield(state, -2, "__name");
    }

    static constexpr bool canClose(StoreType store_type)
    {
        return store_type == StoreType::Value && !trivially_destructible;
    }

    /// @brief Registers the cleanup function on the metatable on the top of the stack.
    static void registerCloseMetamethods(lua_State* state)
    {
        static_assert(!trivially_destructible);
        lua_pushcfunction(state, gc);
        lua_setfield(state, -2, "__gc");
        lua_pushcfunction(state, close);
        lua_setfield(state, -2, "__close");
    }

    /// @brief Registers the cleanup function on the metatable on the top of the stack.
    static void registerCloseMethods(lua_State* state)
    {
        static_assert(!trivially_destructible);
        lua_pushcfunction(state, close);
        lua_setfield(state, -2, "close");
        lua_pushcfunction(state, closed);
        lua_setfield(state, -2, "closed");
    }

    /// @brief Protects the metatable on the stop of the stack with `false`.
    static void protectMetatable(lua_State* state)
    {
        lua_pushboolean(state, false);
        lua_setfield(state, -2, "__metatable");
    }

    // --- Lua functions ---

    /// @brief Destroys the entire optional.
    static int gc(lua_State* state)
    {
        static_assert(!trivially_destructible);
        using Store = value_store_t<StoreType::Value>;
        auto value = static_cast<Store*>(lua_touserdata(state, 1));
        assert(value);
        value->~Store();
        return 0;
    }

    /// @brief Destroys the value by resetting the optional.
    static int close(lua_State* state)
    {
        static_assert(!trivially_destructible);
        checkRaw<StoreType::Value>(state, 1).reset();
        return 0;
    }

    /// @brief Whether the value has been closed, i.e. whether the optional is empty.
    static int closed(lua_State* state)
    {
        static_assert(!trivially_destructible);
        lua_pushboolean(state, !checkRaw<StoreType::Value>(state, 1).has_value());
        return 1;
    }

    /// @brief Handles checking properties, the original index table and calling the __index function in this order.
    /// @remark Upvalue indices to use for each style are passed as template parameter and can be 0 to skip entirely.
    template <int v_properties, int v_indextable, int v_indexfunction>
    static int customIndex(lua_State* state)
    {
        if constexpr (v_properties != 0) {
            lua_pushvalue(state, -1);
            if (lua_gettable(state, lua_upvalueindex(v_properties)) != LUA_TNIL) {
                lua_pushvalue(state, 1);
                lua_call(state, 1, 1);
                return 1;
            }
            lua_pop(state, 1);
        }

        if constexpr (v_indextable != 0) {
            lua_pushvalue(state, -1);
            if (lua_gettable(state, lua_upvalueindex(v_indextable)) != LUA_TNIL)
                return 1;
            lua_pop(state, 1);
        }

        if constexpr (v_indexfunction != 0) {
            lua_pushvalue(state, lua_upvalueindex(v_indexfunction));
            lua_insert(state, -3);
            lua_call(state, 2, 1);
            return 1;
        }
        else {
            return 0;
        }
    }

    /// @brief Handles properties and calling the original __newindex function in this order.
    /// @remark Upvalue indices to use for each style are passed as template parameter and can be 0 to skip entirely.
    template <int v_properties, int v_indexfunction>
    static int customNewindex(lua_State* state)
    {
        if constexpr (v_properties != 0) {
            lua_pushvalue(state, -2);
            if (lua_gettable(state, lua_upvalueindex(v_properties)) != LUA_TNIL) {
                lua_pushvalue(state, 1);
                lua_pushvalue(state, 3);
                lua_call(state, 2, 0);
                return 0;
            }
            lua_pop(state, 1);
        }

        if constexpr (v_indexfunction != 0) {
            lua_pushvalue(state, lua_upvalueindex(v_indexfunction));
            lua_insert(state, -4);
            lua_call(state, 3, 0);
            return 0;
        }
        else {
            std::string name(getPushTypename());
            if (lua_type(state, 2) == LUA_TSTRING) {
                auto prop = lua_tostring(state, 2);
                noreturn_luaL_error(state, ("cannot write property " + name + "." + prop).c_str());
            }
            noreturn_luaL_error(state, ("attempt to index a " + name + " value").c_str());
        }
    }
};

} // namespace dang::lua
