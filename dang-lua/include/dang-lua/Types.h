#pragma once

#include "dang-lua/global.h"

#include "dang-utils/enum.h"

namespace dang::lua {

/// @brief Several functions that report errors in the API use the following status codes to indicate different kinds of
/// errors or other conditions.
enum class Status {
    Ok = LUA_OK,               // No errors.
    RuntimeError = LUA_ERRRUN, // A runtime error.
    MemoryError = LUA_ERRMEM,  // Memory allocation error. For such errors, Lua does not call the message handler.
    MessageHandlerError = LUA_ERRERR, // Error while running the message handler.
    SyntaxError = LUA_ERRSYNTAX,      // Syntax error during precompilation.
    Yield = LUA_YIELD,                // The thread (coroutine) yields.
    FileError = LUA_ERRFILE           // A file-related error; e.g., it cannot open or read the file.
};

/// @brief The various options, which can be performed using lua_gc.
enum class GCOption {
    Collect = LUA_GCCOLLECT, // Performs a full garbage-collection cycle.
    Stop = LUA_GCSTOP,       // Stops the garbage collector.
    Restart = LUA_GCRESTART, // Restarts the garbage collector.
    Count = LUA_GCCOUNT,     // Returns the current amount of memory (in Kbytes) in use by Lua.
    CountBytes =
        LUA_GCCOUNTB,  // Returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.
    Step = LUA_GCSTEP, // gc(..., int stepsize); Performs an incremental step of garbage collection, corresponding to
                       // the allocation of stepsize Kbytes.
    IsRunning = LUA_GCISRUNNING, // Returns a boolean that tells whether the collector is running (i.e., not stopped).
    Incremental = LUA_GCINC, // gc(..., int pause, int stepmul, int stepsize); Changes the collector to incremental mode
                             // with the given parameters and returns the previous mode.
    Generational = LUA_GCGEN, // gc(..., int minormul, int majormul); Changes the collector to generational mode with
                              // the given parameters and returns the previous mode.
};

/// @brief Lua value types.
enum class Type {
    None = LUA_TNONE,
    Nil = LUA_TNIL,
    Boolean = LUA_TBOOLEAN,
    LightUserdata = LUA_TLIGHTUSERDATA,
    Number = LUA_TNUMBER,
    String = LUA_TSTRING,
    Table = LUA_TTABLE,
    Function = LUA_TFUNCTION,
    Userdata = LUA_TUSERDATA,
    Thread = LUA_TTHREAD,

    COUNT = LUA_NUMTYPES
};

/// @brief Possible operations for lua_arith.
enum class ArithOp {
    // binary (pop 2, push 1)
    Add = LUA_OPADD,
    Sub = LUA_OPSUB,
    Mul = LUA_OPMUL,
    Mod = LUA_OPMOD,
    Pow = LUA_OPPOW,
    Div = LUA_OPDIV,
    IDiv = LUA_OPIDIV,
    BinaryAnd = LUA_OPBAND,
    BinaryOr = LUA_OPBOR,
    BinaryXOr = LUA_OPBXOR,
    LeftShift = LUA_OPSHL,
    RightShift = LUA_OPSHR,

    // unary (pop 1, push 1)
    UnaryMinus = LUA_OPUNM,
    BinaryNot = LUA_OPBNOT,

    COUNT
};

/// @brief Possible operations for lua_compare.
enum class CompareOp {
    Equal = LUA_OPEQ,
    LessThan = LUA_OPLT,
    LessEqual = LUA_OPLE,

    COUNT
};

/// @brief A list of all Lua standard libraries.
enum class StandardLibrary { Base, Coroutine, Table, IO, OS, String, Utf8, Math, Debug, Package, COUNT };

/// @brief Wether to load Lua code only as text or binary, or accept both.
enum class LoadMode { Default, Binary, Text, Both, COUNT };

} // namespace dang::lua

namespace dang::utils {

template <>
struct enum_count<dang::lua::Type> : default_enum_count<dang::lua::Type> {};

template <>
struct enum_count<dang::lua::ArithOp> : default_enum_count<dang::lua::ArithOp> {};

template <>
struct enum_count<dang::lua::CompareOp> : default_enum_count<dang::lua::CompareOp> {};

template <>
struct enum_count<dang::lua::StandardLibrary> : default_enum_count<dang::lua::StandardLibrary> {};

template <>
struct enum_count<dang::lua::LoadMode> : default_enum_count<dang::lua::LoadMode> {};

} // namespace dang::utils

namespace dang::lua {

inline const dutils::EnumArray<StandardLibrary, lua_CFunction> library_functions = {luaopen_base,
                                                                                    luaopen_coroutine,
                                                                                    luaopen_table,
                                                                                    luaopen_io,
                                                                                    luaopen_os,
                                                                                    luaopen_string,
                                                                                    luaopen_utf8,
                                                                                    luaopen_math,
                                                                                    luaopen_debug,
                                                                                    luaopen_package};

inline constexpr dutils::EnumArray<StandardLibrary, const char*> library_names = {"_G",
                                                                                  LUA_COLIBNAME,
                                                                                  LUA_TABLIBNAME,
                                                                                  LUA_IOLIBNAME,
                                                                                  LUA_OSLIBNAME,
                                                                                  LUA_STRLIBNAME,
                                                                                  LUA_UTF8LIBNAME,
                                                                                  LUA_MATHLIBNAME,
                                                                                  LUA_DBLIBNAME,
                                                                                  LUA_LOADLIBNAME};

inline constexpr dutils::EnumArray<LoadMode, const char*> load_mode_names = {nullptr, "b", "t", "bt"};

} // namespace dang::lua
