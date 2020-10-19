#pragma once

#include "utils.h"

namespace dang::lua
{

/// <summary>Serves as a container for typenames that represent subclasses.</summary>
template <typename... TBases>
struct SubClassList {};

/// <summary>Can be specialized to provide information about the subclasses of a given class.</summary>
template <typename T>
struct SubClasses : SubClassList<> {};

template <typename T>
constexpr SubClasses<T> SubClassesOf{};

/// <summary>Can be specialized to provide a customized name for a class.</summary>
template <typename T>
const char* ClassName = typeid(T).name();

/// <summary>Can be specialized to provide a customized name for references to a class.</summary>
template <typename T>
const char* ClassNameRef = typeid(T*).name();

/// <summary>Returns empty index and metatable.</summary>
struct DefaultClassInfo {
    constexpr std::array<luaL_Reg, 0> table()
    {
        return {};
    }

    constexpr std::array<luaL_Reg, 0> metatable()
    {
        return {};
    }
};

/// <summary>Can be specialized to provide an index and metatable of a wrapped class.</summary>
template <typename T>
struct ClassInfo : DefaultClassInfo {};

/// <summary>Shorthand to access the index table of a wrapped class.</summary>
template <typename T>
const auto ClassTable = ClassInfo<T>().table();

/// <summary>Shorthand to access the metatable of a wrapped class.</summary>
template <typename T>
const auto ClassMetatable = ClassInfo<T>().metatable();

/// <summary>Can be specialized to provide an array of string names for a given enum to convert from and to Lua.</summary>
/// <remarks>The array needs to end with a "null" entry.</remarks>
template <typename T>
constexpr const char* EnumValues[1]{};

namespace detail
{

/// <summary>Somewhat similar to luaL_setfuncs, except it uses any kind of container.</summary>
template <typename T>
void setfuncs(lua_State* state, const T& funcs)
{
    for (const auto& func : funcs) {
        lua_pushcfunction(state, func.func);
        lua_setfield(state, -2, func.name);
    }
}

}

/*

--- Convert Protocol ---

static constexpr std::optional<int> PushCount = 1;
    -> How many items are pushed by push, usually 1
    -> Can be std::nullopt if the size varies, in which case the getPushCount function must be provided

static constexpr bool AllowNesting = true;
    -> Whether this type can be nested inside of tuples.

static bool isExact(lua_State* state, int pos);
    -> Whether the given stack positions type matches exactly.
    -> lua_type(state, pos) == T

static bool isValid(lua_State* state, int pos);
    -> Whether the given stack position is convertible.
    -> lua_isT(state, pos)

static std::optional<T> at(lua_State* state, int pos);
    -> Tries to convert the given stack position and returns std::nullopt on failure.
    -> lua_toT(state, arg)

static T check(lua_State* state, int arg);
    -> Tries to convert the given argument stack position and raises an argument error on failure.
    -> lua_checkT(state, arg)

static int getPushCount(const &T value);
    -> When PushCount is std::nullopt, this function returns the actual count for a given value

static std::string/std::string_view getPushTypename();
    -> Returns the typename of the value
    -> luaL_typename

static void push(lua_State* state, T value);
    -> Pushes the given value onto the stack using PushCount values
    -> lua_pushT(state, value)

// A full implementation would look like this:

template <>
struct Convert<T> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    static bool isExact(lua_State* state, int pos)
    {

    }

    static constexpr bool isValid(lua_State* state, int pos)
    {

    }

    static std::optional<T> at(lua_State* state, int pos)
    {

    }

    static T check(lua_State* state, int arg)
    {

    }

    static int getPushCount(const T& value)
    {

    }

    static constexpr std::string_view getPushTypename()
    {

    }

    static void push(lua_State* state, const T& value)
    {

    }
};


*/

/// <summary>A Lua class instance can either be its own value or reference an existing instance.</summary>
enum class StoreType {
    None,
    Value,
    Reference
};

/// <summary>Converts instances of classes and enums to and from Lua as either value or reference.</summary>
template <typename T>
struct Convert {
    static_assert(dlua::EnumValues<T>[std::size(dlua::EnumValues<T>) - 1] == nullptr, "EnumValues is not null-terminated");
    static_assert(!std::is_enum_v<T> || std::size(dlua::EnumValues<T>) > 1, "EnumValues is empty");

    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Checks whether the type matches any of the supplied sub classes.</summary>
    template <typename TFirst, typename... TRest>
    static StoreType type(lua_State* state, int pos, SubClassList<TFirst, TRest...>)
    {
        auto result = Convert<TFirst>::type(state, pos);
        if (result != StoreType::None)
            return result;
        return type<TRest...>(state, pos);
    }

    /// <summary>Serves as an exit condition when the list of sub classes is depleted.</summary>
    static StoreType type(lua_State* state, int, SubClassList<>)
    {
        return StoreType::None;
    }

    /// <summary>Whether a stack position is a value, reference or neither.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static StoreType type(lua_State* state, int pos)
    {
        if (luaL_testudata(state, pos, ClassName<T>))
            return StoreType::Value;
        if (luaL_testudata(state, pos, ClassNameRef<T>))
            return StoreType::Reference;
        return type(state, pos, SubClassesOf<T>);
    }

    /// <summary>Finds the given string enum value or std::nullopt if not found.</summary>
    template <typename = std::enable_if_t<std::is_enum_v<T>>>
    static std::optional<T> findEnumValue(const char* value)
    {
        for (std::size_t i = 0; EnumValues<T>[i]; i++)
            if (std::strcmp(EnumValues<T>[i], value) == 0)
                return static_cast<T>(i);
        return std::nullopt;
    }

    /// <summary>Whether the stack position is a valid class value or reference, or an enum.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        if constexpr (std::is_class_v<T>)
            return type(state, pos) != StoreType::None;
        else if constexpr (std::is_enum_v<T>)
            return at(state, pos);
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>Whether the stack position is a valid class value or reference, or an enum.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return isExact(state, pos);
    }

    /// <summary>Goes through the full list of subclasses to try and convert the value.</summary>
    template <typename TFirst, typename... TRest>
    static auto at(lua_State* state, int pos, SubClassList<TFirst, TRest...>)
        -> std::optional<std::reference_wrapper<T>>
    {
        auto result = Convert<TFirst>::at(state, pos);
        return result ? result : at(state, pos, SubClassList<TRest...>{});
    }

    /// <summary>Exit condition when the subclass list is depleted.</summary>
    static auto at(lua_State*, int, SubClassList<>)
        -> std::optional<std::reference_wrapper<T>>
    {
        return std::nullopt;
    }

    /// <summary>Returns a reference to the value at the given stack position or std::nullopt on failure.</summary>
    static auto at(lua_State* state, int pos)
        -> std::optional<std::conditional_t<std::is_class_v<T>, std::reference_wrapper<T>, T>>
    {
        if constexpr (std::is_class_v<T>) {
            if (void* value = luaL_testudata(state, pos, ClassName<T>))
                return *static_cast<T*>(value);
            if (void* pointer = luaL_testudata(state, pos, ClassNameRef<T>))
                return **static_cast<T**>(pointer);
            return at(state, pos, SubClassesOf<T>);
        }
        else if constexpr (std::is_enum_v<T>) {
            lua_pushvalue(state, pos);
            auto result = findEnumValue(lua_tostring(state, -1));
            lua_pop(state, 1);
            return result;
        }
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>Returns a reference to the value at the given argument stack position and raises an argument error on failure.</summary>
    static auto check(lua_State* state, int arg)
        -> std::conditional_t<std::is_class_v<T>, T&, T>
    {
        if constexpr (std::is_class_v<T>) {
            if (auto result = at(state, arg))
                return *result;
            throw luaL_checkudata(state, arg, ClassName<T>);
        }
        else if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(luaL_checkoption(state, arg, nullptr, EnumValues<T>));
        }
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>__gc, which is used to do cleanup for non-reference values.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static int cleanup(lua_State* state)
    {
        T* userdata = static_cast<T*>(lua_touserdata(state, 1));
        userdata->~T();
        return 0;
    }

    /// <summary>__index, which first checks the original index table, and then tries to call the customized __index method.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static int customIndex(lua_State* state)
    {
        lua_pushvalue(state, lua_upvalueindex(1));
        lua_pushvalue(state, -2);
        if (lua_gettable(state, -2) != LUA_TNIL)
            return 1;
        lua_pop(state, 2);
        lua_pushvalue(state, lua_upvalueindex(2));
        lua_insert(state, -3);
        lua_call(state, 2, 1);
        return 1;
    }

    /// <summary>Pushes the metatable for a value instance onto the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void pushValueMetatable(lua_State* state)
    {
        if (!luaL_newmetatable(state, ClassName<T>))
            return;
        detail::setfuncs(state, ClassMetatable<T>);
        lua_pushcfunction(state, cleanup);
        lua_setfield(state, -2, "__gc");
        pushReferenceMetatable(state);
        if (!luaL_getmetafield(state, -1, "__index")) {
            lua_createtable(state, 0, static_cast<int>(ClassTable<T>.size()));
            detail::setfuncs(state, ClassTable<T>);
        }
        if (lua_getfield(state, -3, "__index") != LUA_TNIL)
            lua_pushcclosure(state, customIndex, 2);
        else
            lua_pop(state, 1);
        lua_setfield(state, -3, "__index");
        lua_pop(state, 1);
    }

    /// <summary>Pushes the metatable for a reference instance onto the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void pushReferenceMetatable(lua_State* state)
    {
        if (!luaL_newmetatable(state, ClassNameRef<T>))
            return;
        detail::setfuncs(state, ClassMetatable<T>);
        pushValueMetatable(state);
        if (!luaL_getmetafield(state, -1, "__index")) {
            lua_createtable(state, 0, static_cast<int>(ClassTable<T>.size()));
            detail::setfuncs(state, ClassTable<T>);
        }
        if (lua_getfield(state, -3, "__index") != LUA_TNIL)
            lua_pushcclosure(state, customIndex, 2);
        else
            lua_pop(state, 1);
        lua_setfield(state, -3, "__index");
        lua_pop(state, 1);
    }

    /// <summary>Returns the name of the class or enum.</summary>
    static constexpr std::string_view getPushTypename()
    {
        return ClassName<T>;
    }

    /// <summary>Pushes the in place constructed value onto the stack.</summary>
    template <typename... TArgs, typename = std::enable_if_t<std::is_class_v<T>>>
    static void push(lua_State* state, TArgs&&... values)
    {
        T* userdata = static_cast<T*>(lua_newuserdata(state, sizeof(T)));
        new (userdata) T(std::forward<TArgs>(values)...);
        pushValueMetatable(state);
        lua_setmetatable(state, -2);
    }

    /// <summary>Pushes a string for the given enum value on the stack.</summary>
    template <typename = std::enable_if_t<std::is_enum_v<T>>>
    static void push(lua_State* state, T value)
    {
        lua_pushstring(state, EnumValues<T>[static_cast<std::size_t>(value)]);
    }

    /// <summary>Pushes a reference to the value on the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void push(lua_State* state, std::reference_wrapper<T> value)
    {
        pushRef(state, value.get());
    }

    /// <summary>Pushes a reference to the value on the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void pushRef(lua_State* state, T& value)
    {
        T** userdata = static_cast<T**>(lua_newuserdata(state, sizeof(T*)));
        *userdata = &value;
        pushReferenceMetatable(state);
        lua_setmetatable(state, -2);
    }
};

template <typename T> struct Convert<T&> : Convert<T> {};
template <typename T> struct Convert<T&&> : Convert<T> {};
template <typename T> struct Convert<const T> : Convert<T> {};
template <typename T> struct Convert<std::reference_wrapper<T>> : Convert<T> {};

/// <summary>Converts nothing.</summary>
template <>
struct Convert<void> {
    static constexpr std::optional<int> PushCount = 0;
    static constexpr bool AllowNesting = true;
};

/// <summary>Converts nil values.</summary>
template <typename TNil>
struct ConvertNil {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the given stack position is nil.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_isnil(state, pos);
    }

    /// <summary>Whether the given stack position is nil or none.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return lua_isnoneornil(state, pos);
    }

    /// <summary>Returns an instance of TNil for nil and none values, and std::nullopt otherwise.</summary>
    static std::optional<TNil> at(lua_State* state, int pos)
    {
        if (lua_isnoneornil(state, pos))
            return TNil();
        return std::nullopt;
    }

    /// <summary>Returns an instance of TNil and raises an error if the value is neither nil nor none.</summary>
    static TNil check(lua_State* state, int arg)
    {
        if (lua_isnoneornil(state, arg))
            return nullptr;
        throw luaL_error(state, "expected a nil value");
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "nil"sv;
    }

    /// <summary>Pushes a nil value on the stack.</summary>
    static void push(lua_State* state, TNil = {})
    {
        lua_pushnil(state);
    }
};

template <> struct Convert<std::nullptr_t> : ConvertNil<std::nullptr_t> {};
template <> struct Convert<std::nullopt_t> : ConvertNil<std::nullopt_t> {};
template <> struct Convert<std::monostate> : ConvertNil<std::monostate> {};

/// <summary>Allows for conversion between Lua boolean and C++ bool.</summary>
template <>
struct Convert<bool> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the given stack position contains an actual boolean.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_isboolean(state, pos);
    }

    /// <summary>Always returns true, as everything is convertible to boolean.</summary>
    static constexpr bool isValid(lua_State*, int)
    {
        return true;
    }

    /// <summary>Converts the given stack position and never returns std::nullopt.</summary>
    static std::optional<bool> at(lua_State* state, int pos)
    {
        return lua_toboolean(state, pos);
    }

    /// <summary>Converts the given stack position and never raises an error.</summary>
    static bool check(lua_State* state, int arg)
    {
        return lua_toboolean(state, arg);
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "boolean"sv;
    }

    /// <summary>Pushes the given boolean on the stack.</summary>
    static void push(lua_State* state, bool value)
    {
        lua_pushboolean(state, value);
    }
};

/// <summary>Allows for conversion between Lua numbers and C++ floating point types.</summary>
template <typename T>
struct ConvertFloatingPoint {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");

    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the stack position contains an actual number.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_type(state, pos) == LUA_TNUMBER;
    }

    /// <summary>Whether the stack position contains a number or a string, convertible to a number.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return lua_isnumber(state, pos);
    }

    /// <summary>Converts the given argument stack position into a Lua number and returns std::nullopt on failure.</summary>
    static std::optional<T> at(lua_State* state, int pos)
    {
        int isnum;
        lua_Number result = lua_tonumberx(state, pos, &isnum);
        if (isnum)
            return static_cast<T>(result);
        return std::nullopt;
    }

    /// <summary>Converts the given argument stack position into a floating point type and raises an error on failure.</summary>
    static T check(lua_State* state, int arg)
    {
        return static_cast<T>(luaL_checknumber(state, arg));
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "number"sv;
    }

    /// <summary>Pushes the given number on the stack.</summary>
    static void push(lua_State* state, T value)
    {
        lua_pushnumber(state, static_cast<lua_Number>(value));
    }
};

template <> struct Convert<float> : ConvertFloatingPoint<float> {};
template <> struct Convert<double> : ConvertFloatingPoint<double> {};
template <> struct Convert<long double> : ConvertFloatingPoint<long double> {};

/// <summary>Allows for conversion between Lua integers and C++ integral types.</summary>
template <typename T>
struct ConvertIntegral {
    static_assert(std::is_integral_v<T>, "T must be integral");

    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Checks, whether the given Lua integer fits into the range of the C++ integral type.</summary>
    static constexpr bool checkRange([[maybe_unused]] lua_Integer value)
    {
        if constexpr (std::is_same_v<T, std::uint64_t>) {
            return value >= 0;
        }
        else {
            if constexpr (std::numeric_limits<T>::max() < std::numeric_limits<lua_Integer>::max()) {
                if (value > std::numeric_limits<T>::max())
                    return false;
            }
            if constexpr (std::numeric_limits<T>::min() > std::numeric_limits<lua_Integer>::min()) {
                if (value < std::numeric_limits<T>::min())
                    return false;
            }
            return true;
        }
    }

    /// <summary>Whether the value at the given stack position is an integer and fits the C++ integral type.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        if (lua_type(state, pos) == LUA_TNUMBER)
            return false;
        int isnum;
        lua_Number value = lua_tointegerx(state, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// <summary>Whether the value at the given stack position is an integer or a string convertible to an integer and fits the C++ integral type.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        int isnum;
        lua_Number value = lua_tointegerx(state, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// <summary>Returns an error message for the given number not being in the correct range.</summary>
    static std::string getRangeErrorMessage(lua_Integer value)
    {
        return "value " + std::to_string(value) + " must be in range " +
            std::to_string(std::numeric_limits<T>::min()) + " .. " + std::to_string(std::numeric_limits<T>::max());
    }

    /// <summary>Converts the given argument stack position into an integral type and returns std::nullopt on failure.</summary>
    static std::optional<T> at(lua_State* state, int pos)
    {
        int isnum;
        lua_Integer result = lua_tointegerx(state, pos, &isnum);
        if (isnum && checkRange(result))
            return static_cast<T>(result);
        return std::nullopt;
    }

    /// <summary>Converts the given argument stack position into an integral type and raises an error on failure.</summary>
    static T check(lua_State* state, int arg)
    {
        lua_Integer result = luaL_checkinteger(state, arg);
        if (!checkRange(result))
            throw luaL_argerror(state, arg, getRangeErrorMessage(result).c_str());
        return static_cast<T>(result);
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "integer"sv;
    }

    /// <summary>Pushes the given integer on the stack.</summary>
    static void push(lua_State* state, T value)
    {
        lua_pushinteger(state, static_cast<lua_Integer>(value));
    }
};

template <> struct Convert<std::int8_t> : ConvertIntegral<std::int8_t> {};
template <> struct Convert<std::uint8_t> : ConvertIntegral<std::uint8_t> {};
template <> struct Convert<std::int16_t> : ConvertIntegral<std::int16_t> {};
template <> struct Convert<std::uint16_t> : ConvertIntegral<std::uint16_t> {};
template <> struct Convert<std::int32_t> : ConvertIntegral<std::int32_t> {};
template <> struct Convert<std::uint32_t> : ConvertIntegral<std::uint32_t> {};
template <> struct Convert<std::int64_t> : ConvertIntegral<std::int64_t> {};
template <> struct Convert<std::uint64_t> : ConvertIntegral<std::uint64_t> {};

/// <summary>Allows for conversion between Lua strings and std::string.</summary>
template <>
struct Convert<std::string> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the value at the given stack position is a string.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_type(state, pos) == LUA_TSTRING;
    }

    /// <summary>Whether the value at the given stack position is a string or a number.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return lua_isstring(state, pos);
    }

    /// <summary>Checks, whether the given argument stack position is a string or number and returns std::nullopt on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::optional<std::string> at(lua_State* state, int pos)
    {
        std::size_t length;
        const char* string = lua_tolstring(state, pos, &length);
        if (string)
            return std::string(string, length);
        return std::nullopt;
    }

    /// <summary>Checks, whether the given argument stack position is a string or number and raises an error on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::string check(lua_State* state, int arg)
    {
        std::size_t length;
        const char* string = luaL_checklstring(state, arg, &length);
        return std::string(string, length);
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "string"sv;
    }

    /// <summary>Pushes the given string onto the stack.</summary>
    static void push(lua_State* state, const std::string& value)
    {
        lua_pushlstring(state, value.c_str(), value.size());
    }
};

/// <summary>Allows for conversion between Lua strings and std::string_view.</summary>
template <>
struct Convert<std::string_view> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the value at the given stack position is a string.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_type(state, pos) == LUA_TSTRING;
    }

    /// <summary>Whether the value at the given stack position is a string or a number.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return lua_isstring(state, pos);
    }

    /// <summary>Checks, whether the given argument stack position is a string or number and returns std::nullopt on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::optional<std::string_view> at(lua_State* state, int pos)
    {
        std::size_t length;
        const char* string = lua_tolstring(state, pos, &length);
        if (string)
            return std::string_view(string, length);
        return std::nullopt;
    }

    /// <summary>Checks, whether the given argument stack position is a string or number and raises an error on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::string_view check(lua_State* state, int arg)
    {
        std::size_t length;
        const char* string = luaL_checklstring(state, arg, &length);
        return std::string_view(string, length);
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "string"sv;
    }

    /// <summary>Pushes the given string onto the stack.</summary>
    static void push(lua_State* state, std::string_view value)
    {
        lua_pushlstring(state, value.data(), value.size());
    }
};

/// <summary>Allows pushing of char arrays as strings.</summary>
template <std::size_t Count>
struct Convert<char[Count]> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "string"sv;
    }

    /// <summary>Pushes the given string onto the stack, shortening a potential null-termination.</summary>
    static void push(lua_State* state, const char(&value)[Count])
    {
        lua_pushlstring(state, value, value[Count - 1] ? Count : Count - 1);
    }
};

/// <summary>Allows pushing of C-Style strings.</summary>
template <>
struct Convert<const char*> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "string"sv;
    }

    /// <summary>Pushes the given null-terminated string onto the stack.</summary>
    static void push(lua_State* state, const char* value)
    {
        lua_pushstring(state, value);
    }
};

/// <summary>Allows pushing of C-Style strings.</summary>
template <>
struct Convert<char*> : Convert<const char*> {};

/// <summary>Allows for conversion of C functions.</summary>
template <>
struct Convert<lua_CFunction> {
    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the value at the given stack position is a C function.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_iscfunction(state, pos);
    }

    /// <summary>Whether the value at the given stack position is a C function.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return isExact(state, pos);
    }

    /// <summary>Checks, whether the given argument stack position is a C function and returns std::nullopt on failure.</summary>
    static std::optional<lua_CFunction> at(lua_State* state, int pos)
    {
        if (auto result = lua_tocfunction(state, pos))
            return result;
        return std::nullopt;
    }

    /// <summary>Checks, whether the given argument stack position is a C function and raises an error on failure.</summary>
    static lua_CFunction check(lua_State* state, int arg)
    {
        if (auto result = lua_tocfunction(state, arg))
            return result;
        throw luaL_argerror(state, arg, "C function expected");
    }

    static constexpr std::string_view getPushTypename()
    {
        using namespace std::literals;
        return "function"sv;
    }

    /// <summary>Pushes the given C function onto the stack.</summary>
    static void push(lua_State* state, lua_CFunction value)
    {
        lua_pushcfunction(state, value);
    }
};

template <> struct Convert<int(&)(lua_State*)> : Convert<lua_CFunction> {};
template <> struct Convert<int(lua_State*)> : Convert<lua_CFunction> {};

/// <summary>Allows for conversion for possible nil values using std::optional.</summary>
template <typename T>
struct Convert<std::optional<T>> {
    using Base = Convert<T>;

    static_assert(Base::PushCount == 1, "Only single values can be optional.");

    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether the value at the given stack position is nil or a valid value.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return lua_isnoneornil(state, pos) || Base::isValid(state, pos);
    }

    /// <summary>Whether the value at the given stack position is nil or a valid value.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return isExact(state, pos);
    }

    /// <summary>Returns an optional containing a std::nullopt for nil values or a single std::nullopt for invalid values.</summary>
    static std::optional<std::optional<T>> at(lua_State* state, int pos)
    {
        if (lua_isnoneornil(state, pos))
            return std::optional<T>();
        auto result = Base::at(state, pos);
        if (result)
            return result;
        return std::nullopt;
    }

    /// <summary>Returns std::nullopt for nil values or raises an error for invalid values.</summary>
    static std::optional<T> check(lua_State* state, int arg)
    {
        if (lua_isnoneornil(state, arg))
            return std::nullopt;
        return Base::check(state, arg);
    }

    /// <summary>Pushes the given value or nil onto the stack.</summary>
    static int push(lua_State* state, std::optional<T> value)
    {
        if (value)
            Base::push(state, *value);
        else
            lua_pushnil(state);
        return 1;
    }
};

/// <summary>Returns the combined push count of all types or std::nullopt if any push count is not known at compile-time.</summary>
template <typename... TValues>
constexpr std::optional<int> CombinedPushCount = [] {
    if constexpr ((Convert<TValues>::PushCount && ...))
        return (0 + ... + *Convert<TValues>::PushCount);
    else
        return std::nullopt;
}();

/// <summary>Returns the combined push count of all values.</summary>
template <typename... TValues>
static constexpr int combinedPushCount(const TValues&... values)
{
    return (0 + ... + [&values] {
        if constexpr (Convert<TValues>::PushCount)
            return *Convert<TValues>::PushCount;
        else
            return Convert<TValues>::getPushCount(values);
        }());
}

/// <summary>Allows for conversion of multiple values using std::tuple.</summary>
template <typename... TValues>
struct Convert<std::tuple<TValues...>> {
    static constexpr std::optional<int> PushCount = CombinedPushCount<TValues...>;
    static constexpr bool AllowNesting = (Convert<TValues>::AllowNesting && ...);

    static_assert(AllowNesting, "Tuples do not allow nesting of stack indices.");

    /// <summary>Whether all stack positions starting at pos are exact.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return isExactHelper(state, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Whether all stack positions starting at pos are valid.</summary>
    static bool isValid(lua_State* state, int pos)
    {
        return isValidHelper(state, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Converts all stack positions starting at pos or std::nullopt on any failure of any.</summary>
    static std::optional<std::tuple<TValues...>> at(lua_State* state, int pos)
    {
        return atHelper(state, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Converts all argument stack positions starting at arg and raises an error on failure of any.</summary>
    static std::tuple<TValues...> check(lua_State* state, int arg)
    {
        return checkHelper(state, arg, std::index_sequence_for<TValues...>());
    }

    /// <summary>Pushes all values in the tuple onto the stack and returns the count.</summary>
    static void push(lua_State* state, std::tuple<TValues...>&& values)
    {
        pushAll(state, std::index_sequence_for<TValues...>(), std::move(values));
    }

    /// <summary>Pushes all values in the tuple onto the stack and returns the count.</summary>
    static void push(lua_State* state, const std::tuple<TValues...>& values)
    {
        pushAll(state, std::index_sequence_for<TValues...>(), values);
    }

    /// <summary>Returns the total push count of all values in the tuple.</summary>
    template <typename = std::enable_if_t<!PushCount>>
    static constexpr int getPushCount(const std::tuple<TValues...>& values)
    {
        return std::apply(combinedPushCount, values);
    }

private:
    template <std::size_t... Indices>
    static bool isExactHelper(lua_State* state, int pos, std::index_sequence<Indices...>)
    {
        return (Convert<TValues>::isExact(state, pos + Indices) && ...);
    }

    template <std::size_t... Indices>
    static bool isValidHelper(lua_State* state, int pos, std::index_sequence<Indices...>)
    {
        return (Convert<TValues>::isValid(state, pos + Indices) && ...);
    }

    template <std::size_t... Indices>
    static std::optional<std::tuple<TValues...>> atHelper(lua_State* state, int pos, std::index_sequence<Indices...>)
    {
        std::tuple values{ Convert<TValues>::at(state, pos + Indices)... };
        if ((std::get<Indices>(values) && ...))
            return std::tuple{ *std::get<Indices>(values)... };
        return std::nullopt;
    }

    template <std::size_t... Indices>
    static std::tuple<TValues...> checkHelper(lua_State* state, int arg, std::index_sequence<Indices...>)
    {
        return { Convert<TValues>::check(state, arg + Indices)... };
    }

    template <std::size_t... Indices>
    static void pushAll(lua_State* state, std::index_sequence<Indices...>, std::tuple<TValues...>&& values)
    {
        (Convert<TValues>::push(state, std::move(std::get<Indices>(values))), ...);
    }

    template <std::size_t... Indices>
    static void pushAll(lua_State* state, std::index_sequence<Indices...>, const std::tuple<TValues...>& values)
    {
        (Convert<TValues>::push(state, std::get<Indices>(values)), ...);
    }
};

/// <summary>Allows for conversion of different values using std::variant.</summary>
template <typename... TOptions>
struct Convert<std::variant<TOptions...>> {
    static_assert(((Convert<TOptions>::PushCount == 1) && ...), "All variant options must have a push count of one.");

    using Variant = std::variant<TOptions...>;

    static constexpr std::optional<int> PushCount = 1;
    static constexpr bool AllowNesting = true;

    /// <summary>Whether at least one option matches exactly.</summary>
    static bool isExact(lua_State* state, int pos)
    {
        return (Convert<TOptions>::isExact(state, pos) || ...);
    }

    /// <summary>Whether at least one option is valid.</summary>
    static constexpr bool isValid(lua_State* state, int pos)
    {
        return (Convert<TOptions>::isValid(state, pos) || ...);
    }

    /// <summary>Returns the first type that does not return std::nullopt or returns std::nullopt itself if none was found.</summary>
    static std::optional<Variant> at(lua_State* state, int pos)
    {
        return atHelper(state, pos, TypeList<TOptions...>());
    }

    /// <summary>Returns the first type that does not return std::nullopt or raises and argument error if none was found.</summary>
    static Variant check(lua_State* state, int arg)
    {
        auto value = at(state, arg);
        if (value)
            return *value;
        std::string error = getPushTypename() + " expected, got " + luaL_typename(state, arg);
        throw luaL_argerror(state, arg, error.c_str());
    }

    /// <summary>Combines all possible options of the variant in the form: "a, b, c or d"</summary>
    static std::string getPushTypename()
    {
        return getPushTypenameHelper(TypeList<TOptions...>());
    }

    /// <summary>Pushes the value of the variant.</summary>
    static void push(lua_State* state, const Variant& variant)
    {
        std::visit([state](const auto& value) { Convert<decltype(value)>::push(state, (value)); }, variant);
    }

private:
    template <typename... TTypes>
    struct TypeList {};

    template <typename TFirst, typename... TRest>
    static std::optional<Variant> atHelper(lua_State* state, int pos, TypeList<TFirst, TRest...>)
    {
        auto value = Convert<TFirst>::at(state, pos);
        return value ? value : atHelper(state, pos, TypeList<TRest...>());
    }

    static std::optional<Variant> atHelper(lua_State*, int, TypeList<>)
    {
        return std::nullopt;
    }

    template <typename TFirst, typename... TRest>
    static std::string getPushTypenameHelper(TypeList<TFirst, TRest...>)
    {
        std::string result{ Convert<TFirst>::getPushTypename() };
        if constexpr (sizeof...(TRest) == 0)
            return result;
        else if constexpr (sizeof...(TRest) == 1)
            return result + " or " + getPushTypenameHelper(TypeList<TRest...>());
        else
            return result + ", " + getPushTypenameHelper(TypeList<TRest...>());
    }

};

}
