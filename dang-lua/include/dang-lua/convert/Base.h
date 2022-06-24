#pragma once

#include "dang-lua/global.h"

namespace dang::lua {

/*

--- Convert Protocol ---

// Lua -> C (check)

static constexpr bool can_check = true;
    -> Whether this type can be converted from Lua to C using "at" and "check" functions.
    -> Everything below needs only be implemented if this is true.

static constexpr std::optional<int> check_count = 1;
    -> How many values of the Lua stack are used by "at" and "check" functions.
    -> std::nullopt means the entire rest of the stack is used.

static std::string getCheckTypename();
    -> Returns the typename that should be used in error messages.

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

// C -> Lua (push)

static constexpr bool can_push = true;
    -> Whether this type can be pushed from C to Lua.
    -> Everything below needs only be implemented if this is true.

static constexpr std::optional<int> push_count = 1;
    -> How many items are pushed by push, usually 1.
    -> Can be std::nullopt if the size varies, in which case the getPushCount function must be provided.

static int getPushCount(const &T value);
    -> When push_count is std::nullopt, this function returns the actual count for a concrete value.

static std::string getPushTypename();
    -> Returns a compact typename used in e.g. function signatures.

static void push(lua_State* state, T value);
    -> Pushes the given value onto the stack using push_count values.
    -> lua_pushT(state, value)

// A full implementation:

template <>
struct Convert<T> {
    static constexpr bool can_check = true;
    static constexpr std::optional<int> check_count = 1;
    static const std::string getCheckTypename();
    static bool isExact(lua_State* state, int pos);
    static bool isValid(lua_State* state, int pos);
    static std::optional<T> at(lua_State* state, int pos);
    static T check(lua_State* state, int arg);

    static constexpr bool can_push = true;
    static constexpr std::optional<int> push_count = 1;
    // static int getPushCount(const &T value);
    static std::string getPushTypename();
    static void push(lua_State* state, T value);
    // static void push(lua_State* state, const T& value);
};

*/

/// @brief SFINAE friendly base template that allows neither checking nor pushing.
template <typename, typename SFINAE = void>
struct Convert {
    static constexpr bool can_check = false;
    static constexpr bool can_push = false;
};

template <typename T>
struct convert_can_check : std::bool_constant<Convert<T>::can_check> {};

template <typename T>
inline constexpr bool convert_can_check_v = convert_can_check<T>::value;

template <typename T>
struct convert_can_push : std::bool_constant<Convert<T>::can_push> {};

template <typename T>
inline constexpr bool convert_can_push_v = convert_can_push<T>::value;

// The following type traits are mostly because MSVC has some SFINAE issues when testing check_count/push_count.
//   (Issue being them being a class-type "optional<int>", rather than primitives)
// They also simplify checking though, as testing for can_check/can_push is already taken care of.

/// @brief Whether the given type can be checked from an given exact amount of values.
template <typename, int v_count, typename SFINAE = void>
struct convert_checks_exactly : std::false_type {};

template <typename T, int v_count>
struct convert_checks_exactly<T, v_count, std::enable_if_t<Convert<T>::can_check>>
    : std::bool_constant<Convert<T>::check_count == v_count> {};

template <typename T, int v_count>
inline constexpr bool convert_checks_exactly_v = convert_checks_exactly<T, v_count>::value;

/// @brief Whether the given type can be pushed as a given exact amount of values.
template <typename, int v_count, typename SFINAE = void>
struct convert_pushes_exactly : std::false_type {};

template <typename T, int v_count>
struct convert_pushes_exactly<T, v_count, std::enable_if_t<Convert<T>::can_push>>
    : std::bool_constant<Convert<T>::push_count == v_count> {};

template <typename T, int v_count>
inline constexpr bool convert_pushes_exactly_v = convert_pushes_exactly<T, v_count>::value;

// TODO: Convert<T*> (or just void*?) to push light userdata

} // namespace dang::lua
