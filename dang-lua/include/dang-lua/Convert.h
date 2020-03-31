#pragma once

namespace dang::lua
{

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

/// <summary>Provides class info for index and metatable of a wrapped class.</summary>
template <typename T>
struct ClassInfo : DefaultClassInfo {};

/// <summary>Shorthand to access the index table of a wrapped class.</summary>
template <typename T>
constexpr auto ClassTable = ClassInfo<T>().table();

/// <summary>Shorthand to access the metatable of a wrapped class.</summary>
template <typename T>
constexpr auto ClassMetatable = ClassInfo<T>().metatable();

/// <summary>Provides an array of string names for a given enum to convert from and to lua.</summary>
template <typename T>
constexpr const char* EnumValues[1]{};

namespace detail
{

/// <summary>Somewhat similar to luaL_setfuncs, except it uses any kind of container.</summary>
template <typename T>
void setfuncs(lua_State* L, const T& funcs)
{
    for (auto func : funcs) {
        lua_pushcfunction(L, func.func);
        lua_setfield(L, -2, func.name);
    }
}

}

/*
Convert Protocol:

bool isExact(lua_State* L, int pos);
    -> Wether the given stack positions type matches exactly.
    -> lua_type(L, pos) == T

bool isValid(lua_State* L, int pos);
    -> Wether the given stack position is convertible.
    -> lua_isT(L, pos)

int push(lua_State* L, T value);
    -> Pushes onto the stack the given value, returning how many items were pushed
    -> lua_pushT(L, value)

std::optional<T> at(lua_State* L, int pos);
    -> Tries to convert the given argument stack position and returns std::nullopt on failure.
    -> lua_toT(L, arg)

T check(lua_State* L, int arg);
    -> Tries to convert the given argument stack position and raises an error on failure.
    -> lua_checkT(L, arg)

*/

/// <summary>Converts enums and instances of classes to and from lua as either value or reference.</summary>
template <typename T>
struct Convert {
    static_assert(dlua::EnumValues<T>[std::size(dlua::EnumValues<T>) - 1] == nullptr, "EnumValues is not null-terminated");
    static_assert(!std::is_enum_v<T> || std::size(dlua::EnumValues<T>) > 1, "EnumValues is empty");

    /// <summary>A lua class instance can either be its own value or reference an existing instance.</summary>
    enum class Type {
        None,
        Value,
        Reference
    };

    /// <summary>Returns, wether a stack position is a value, reference or neither.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static Type type(lua_State* L, int pos)
    {
        if (luaL_testudata(L, pos, typeid(T).name()))
            return Type::Value;
        if (luaL_testudata(L, pos, typeid(T*).name()))
            return Type::Reference;
        return Type::None;
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

    /// <summary>Returns, wether the stack position is a valid enum value or either a class value or reference.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        if constexpr (std::is_class_v<T>)
            return type(L, pos) != Type::None;
        else if constexpr (std::is_enum_v<T>) {
            return at(L, pos);
        }
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>Returns, wether the stack position is a valid enum value or either a class value or reference.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return isExact(L, pos);
    }

    /// <summary>Returns a reference to the value at the given stack position or std::nullopt on failure.</summary>
    static auto at(lua_State* L, int pos)
        -> std::optional<std::conditional_t<std::is_class_v<T>, std::reference_wrapper<T>, T>>
    {
        if constexpr (std::is_class_v<T>) {
            if (void* pointer = luaL_testudata(L, pos, typeid(T*).name()))
                return **static_cast<T**>(pointer);
            if (void* value = luaL_testudata(L, pos, typeid(T).name()))
                return *static_cast<T*>(value);
            return std::nullopt;
        }
        else if constexpr (std::is_enum_v<T>) {
            lua_pushvalue(L, pos);
            auto result = findEnumValue(lua_tostring(L, -1));
            lua_pop(L, 1);
            return result;
        }
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>Returns a reference to the value at the given argument stack position and raises an error on failure.</summary>
    static auto check(lua_State* L, int arg)
        -> std::conditional_t<std::is_class_v<T>, T&, T>
    {
        if constexpr (std::is_class_v<T>) {
            if (void* test = luaL_testudata(L, arg, typeid(T*).name()))
                return **static_cast<T**>(test);
            void* test = luaL_checkudata(L, arg, typeid(T).name());
            return *static_cast<T*>(test);
        }
        else if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(luaL_checkoption(L, arg, nullptr, EnumValues<T>));
        }
        else
            static_assert(false, "class or enum expected");
    }

    /// <summary>__gc, which is used to do cleanup for non-reference values.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static int cleanup(lua_State* L)
    {
        T* userdata = static_cast<T*>(lua_touserdata(L, 1));
        userdata->~T();
        return 0;
    }

    /// <summary>__index, which first checks the original index table, and then tries to call the customized __index method.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static int customIndex(lua_State* L)
    {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_pushvalue(L, -2);
        if (lua_gettable(L, -2) != LUA_TNIL)
            return 1;
        lua_pop(L, 2);
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_insert(L, -3);
        lua_call(L, 2, 1);
        return 1;
    }

    /// <summary>Pushes the metatable for a value instance onto the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void pushValueMetatable(lua_State* L)
    {
        if (luaL_newmetatable(L, typeid(T).name())) {
            // luaL_setfuncs(L, ClassMetatable<T>, 0);
            detail::setfuncs(L, ClassMetatable<T>);
            lua_pushcfunction(L, cleanup);
            lua_setfield(L, -2, "__gc");
            pushPointerMetatable(L);
            if (!luaL_getmetafield(L, -1, "__index")) {
                lua_createtable(L, 0, static_cast<int>(ClassTable<T>.size()));
                detail::setfuncs(L, ClassTable<T>);
            }
            if (lua_getfield(L, -3, "__index") != LUA_TNIL)
                lua_pushcclosure(L, customIndex, 2);
            else
                lua_pop(L, 1);
            lua_setfield(L, -3, "__index");
            lua_pop(L, 1);
        }
    }

    /// <summary>Pushes the metatable for a reference instance onto the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static void pushPointerMetatable(lua_State* L)
    {
        if (luaL_newmetatable(L, typeid(T*).name())) {
            // luaL_setfuncs(L, ClassMetatable<T>, 0);
            detail::setfuncs(L, ClassMetatable<T>);
            pushValueMetatable(L);
            if (!luaL_getmetafield(L, -1, "__index")) {
                lua_createtable(L, 0, static_cast<int>(ClassTable<T>.size()));
                detail::setfuncs(L, ClassTable<T>);
            }
            if (lua_getfield(L, -3, "__index") != LUA_TNIL)
                lua_pushcclosure(L, customIndex, 2);
            else
                lua_pop(L, 1);
            lua_setfield(L, -3, "__index");
            lua_pop(L, 1);
        }
    }

    /// <summary>Pushes the in place constructed non-reference value onto the stack.</summary>
    template <typename... TArgs, typename = std::enable_if_t<std::is_class_v<T>>>
    static int push(lua_State* L, TArgs&&... values)
    {
        T* userdata = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
        new (userdata) T(std::forward<TArgs>(values)...);
        pushValueMetatable(L);
        lua_setmetatable(L, -2);
        return 1;
    }

    /// <summary>Pushes a string for the given enum value on the stack.</summary>
    template <typename = std::enable_if_t<std::is_enum_v<T>>>
    static int push(lua_State* L, T value)
    {
        lua_pushstring(L, EnumValues<T>[static_cast<std::size_t>(value)]);
        return 1;
    }

    /// <summary>Pushes a reference to the value onto the stack.</summary>
    template <typename = std::enable_if_t<std::is_class_v<T>>>
    static int pushRef(lua_State* L, T& value)
    {
        T** userdata = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
        *userdata = &value;
        pushPointerMetatable(L);
        lua_setmetatable(L, -2);
        return 1;
    }
};

template <typename T>
struct Convert<T&> : Convert<T> {};

template <typename T>
struct Convert<T*> : Convert<T> {};

template <typename T>
struct Convert<const T> : Convert<T> {};

/// <summary>Allows for conversion between lua boolean and C++ bool.</summary>
template <>
struct Convert<bool> {
    /// <summary>Returns, wether the given stack position contains an actual boolean.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return lua_isboolean(L, pos);
    }

    /// <summary>Always returns true, as everything is convertible to boolean.</summary>
    static constexpr bool isValid(lua_State*, int)
    {
        return true;
    }

    /// <summary>Converts the given stack position and never returns std::nullopt.</summary>
    static std::optional<bool> at(lua_State* L, int pos)
    {
        return lua_toboolean(L, pos);
    }

    /// <summary>Converts the given stack position and never raises an error.</summary>
    static bool check(lua_State* L, int arg)
    {
        return lua_toboolean(L, arg);
    }

    /// <summary>Pushes the given boolean on the stack.</summary>
    static int push(lua_State* L, bool value)
    {
        lua_pushboolean(L, value);
        return 1;
    }
};

/// <summary>Allows for conversion between lua numbers and C++ floating point types.</summary>
template <typename T>
struct ConvertFloatingPoint {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");

    /// <summary>Returns, wether the stack position contains an actual number.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return lua_type(L, pos) == LUA_TNUMBER;
    }

    /// <summary>Returns, wether the stack position contains a number or a string, convertible to a number.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return lua_isnumber(L, pos);
    }

    /// <summary>Converts the given argument stack position into a lua number and returns std::nullopt on failure.</summary>
    static std::optional<T> at(lua_State* L, int pos)
    {
        int isnum;
        lua_Number result = lua_tonumberx(L, pos, &isnum);
        if (isnum)
            return static_cast<T>(result);
        return std::nullopt;
    }

    /// <summary>Converts the given argument stack position into a floating point type and raises an error on failure.</summary>
    static T check(lua_State* L, int arg)
    {
        return static_cast<T>(luaL_checknumber(L, arg));
    }

    /// <summary>Pushes the given number on the stack.</summary>
    static int push(lua_State* L, T value)
    {
        lua_pushnumber(L, static_cast<lua_Number>(value));
        return 1;
    }
};

template <>
struct Convert<float> : ConvertFloatingPoint<float> {};

template <>
struct Convert<double> : ConvertFloatingPoint<double> {};

template <>
struct Convert<long double> : ConvertFloatingPoint<long double> {};

/// <summary>Allows for conversion between lua integers and C++ integral types.</summary>
template <typename T>
struct ConvertIntegral {
    static_assert(std::is_integral_v<T>, "T must be integral");

    /// <summary>Checks, wether the given lua integer fits into the range of the C++ integral type.</summary>
    static bool checkRange([[maybe_unused]] lua_Integer value)
    {
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

    /// <summary>Returns, wether the value at the given stack position is an integer or a string convertible to an integer and fits the C++ integral type.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        if (lua_type(L, pos) == LUA_TNUMBER)
            return false;
        int isnum;
        lua_Number value = lua_tointegerx(L, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// <summary>Returns, wether the value at the given stack position is an integer or a string convertible to an integer and fits the C++ integral type.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        int isnum;
        lua_Number value = lua_tointegerx(L, pos, &isnum);
        return isnum && checkRange(value);
    }

    /// <summary>Returns an error message for the given number not being in the correct range.</summary>
    static std::string getRangeErrorMessage(lua_Integer value)
    {
        return "value " + std::to_string(value) + " must be in range " +
            std::to_string(std::numeric_limits<T>::min()) + " .. " + std::to_string(std::numeric_limits<T>::max());
    }

    /// <summary>Converts the given argument stack position into an integral type and returns std::nullopt on failure.</summary>
    static std::optional<T> at(lua_State* L, int pos)
    {
        int isnum;
        lua_Integer result = lua_tointegerx(L, pos, &isnum);
        if (isnum && checkRange(result))
            return static_cast<T>(result);
        return std::nullopt;
    }

    /// <summary>Converts the given argument stack position into an integral type and raises an error on failure.</summary>
    static T check(lua_State* L, int arg)
    {
        lua_Integer result = luaL_checkinteger(L, arg);
        luaL_argcheck(L, checkRange(result), arg, getRangeErrorMessage(result).c_str());
        return static_cast<T>(result);
    }

    /// <summary>Pushes the given integer on the stack.</summary>
    static int push(lua_State* L, T value)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        return 1;
    }
};

template <>
struct Convert<std::int8_t> : ConvertIntegral<std::int8_t> {};

template <>
struct Convert<std::uint8_t> : ConvertIntegral<std::uint8_t> {};

template <>
struct Convert<std::int16_t> : ConvertIntegral<std::int16_t> {};

template <>
struct Convert<std::uint16_t> : ConvertIntegral<std::uint16_t> {};

template <>
struct Convert<std::int32_t> : ConvertIntegral<std::int32_t> {};

template <>
struct Convert<std::uint32_t> : ConvertIntegral<std::uint32_t> {};

template <>
struct Convert<std::int64_t> : ConvertIntegral<std::int64_t> {};

// uint64_t does not fit all possible lua_Integer values and is omitted for now

/// <summary>Allows for conversion between lua strings and C++ std::string.</summary>
template <>
struct Convert<std::string> {
    /// <summary>Returns, wether the value at the given stack position is a string.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return lua_type(L, pos) == LUA_TSTRING;
    }

    /// <summary>Returns, wether the value at the given stack position is a string or a number.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return lua_isstring(L, pos);
    }

    /// <summary>Checks, wether the given argument stack position is a string or number and returns std::nullopt on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::optional<std::string> at(lua_State* L, int pos)
    {
        std::size_t length;
        const char* string = lua_tolstring(L, pos, &length);
        if (string)
            return std::string(string, length);
        return std::nullopt;
    }

    /// <summary>Checks, wether the given argument stack position is a string or number and raises an error on failure.</summary>
    /// <remarks>Numbers are actually converted to a string in place.</remarks>
    static std::string check(lua_State* L, int arg)
    {
        std::size_t length;
        const char* string = luaL_checklstring(L, arg, &length);
        return std::string(string, length);
    }

    /// <summary>Pushes the given string onto the stack.</summary>
    static int push(lua_State* L, const std::string& value)
    {
        lua_pushlstring(L, value.c_str(), value.size());
        return 1;
    }
};

template <std::size_t Count>
struct Convert<const char[Count]> {
    /// <summary>Pushes the given string onto the stack, shortening a potential null-termination.</summary>
    static int push(lua_State* L, const char(&value)[Count])
    {
        lua_pushlstring(L, value, value[Count - 1] ? Count : Count - 1);
        return 1;
    }
};

template <>
struct Convert<const char*> {
    /// <summary>Pushes the given null-terminated string onto the stack.</summary>
    static int push(lua_State* L, const char* value)
    {
        lua_pushstring(L, value);
        return 1;
    }
};

template <>
struct Convert<char*> : Convert<const char*> {};

/// <summary>Allows for conversion for possible nil values using std::optional.</summary>
template <typename T>
struct Convert<std::optional<T>> {
    using Base = Convert<T>;

    /// <summary>Returns, wether the value at the given stack position is nil or a valid value.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return lua_isnoneornil(L, pos) || Base::isValid(L, pos);
    }

    /// <summary>Returns, wether the value at the given stack position is nil or a valid value.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return isExact(L, pos);
    }

    /// <summary>Returns an optional containing a std::nullopt for nil values or a single std::nullopt for invalid values.</summary>
    static std::optional<std::optional<T>> at(lua_State* L, int pos)
    {
        if (lua_isnoneornil(L, pos))
            return std::optional<T>();
        auto result = Base::at(L, pos);
        if (result)
            return result;
        return std::nullopt;
    }

    /// <summary>Returns std::nullopt for nil values or raises an error for invalid values.</summary>
    static std::optional<T> check(lua_State* L, int arg)
    {
        if (lua_isnoneornil(L, arg))
            return std::nullopt;
        return Base::check(L, arg);
    }

    /// <summary>Pushes the given value or nil onto the stack.</summary>
    static int push(lua_State* L, std::optional<T> value)
    {
        if (value)
            return Base::push(L, *value);
        lua_pushnil(L);
        return 1;
    }
};

/// <summary>Allows for conversion for multiple values using std::tuple.</summary>
template <typename... TValues>
struct Convert<std::tuple<TValues...>> {

    /// <summary>Returns, wether all stack positions starting at pos are exact.</summary>
    static bool isExact(lua_State* L, int pos)
    {
        return isExactHelper(L, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Returns, wether all stack positions starting at pos are valid.</summary>
    static bool isValid(lua_State* L, int pos)
    {
        return isValidHelper(L, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Converts all stack positions starting at pos or std::nullopt on any failure of any.</summary>
    static std::optional<std::tuple<TValues...>> at(lua_State* L, int pos)
    {
        return atHelper(L, pos, std::index_sequence_for<TValues...>());
    }

    /// <summary>Converts all argument stack positions starting at arg and raises an error on failure of any.</summary>
    static std::tuple<TValues...> check(lua_State* L, int arg)
    {
        return checkHelper(L, arg, std::index_sequence_for<TValues...>());
    }

    /// <summary>Pushes all values in the tuple onto the stack and returns the count.</summary>
    static int push(lua_State* L, std::tuple<TValues...> values)
    {
        return std::apply(pushHelper, std::tuple_cat(std::tuple{ L }, values));
    }

private:
    template <std::size_t... Indices>
    static bool isExactHelper(lua_State* L, int pos, std::index_sequence<Indices...>)
    {
        pos = lua_absindex(L, pos);
        return (Convert<TValues>::isExact(L, pos + Indices) && ...);
    }

    template <std::size_t... Indices>
    static bool isValidHelper(lua_State* L, int pos, std::index_sequence<Indices...>)
    {
        pos = lua_absindex(L, pos);
        return (Convert<TValues>::isValid(L, pos + Indices) && ...);
    }

    template <std::size_t... Indices>
    static std::optional<std::tuple<TValues...>> atHelper(lua_State* L, int pos, std::index_sequence<Indices...>)
    {
        pos = lua_absindex(L, pos);
        std::tuple values{ Convert<TValues>::at(L, pos + Indices)... };
        if ((std::get<Indices>(values) && ...))
            return std::tuple{ *std::get<Indices>(values)... };
        return std::nullopt;
    }

    template <std::size_t... Indices>
    static std::tuple<TValues...> checkHelper(lua_State* L, int arg, std::index_sequence<Indices...>)
    {
        arg = lua_absindex(L, arg);
        return { Convert<TValues>::check(L, arg + Indices)... };
    }

    static int sum(std::initializer_list<int> counts)
    {
        return std::accumulate(counts.begin(), counts.end(), 0);
    }

    static int pushHelper(lua_State* L, TValues... values)
    {
        return sum({ Convert<TValues>::push(L, values)... });
    }
};

}
