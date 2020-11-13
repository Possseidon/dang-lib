#pragma once

#include "utils.h"

namespace dang::lua {

/// <summary>Several functions that report errors in the API use the following status codes to indicate different kinds of errors or other conditions.</summary>
enum class Status {
    Ok = LUA_OK,               // No errors.
    RuntimeError = LUA_ERRRUN, // A runtime error.
    MemoryError = LUA_ERRMEM,  // Memory allocation error. For such errors, Lua does not call the message handler.
    MessageHandlerError = LUA_ERRERR, // Error while running the message handler.
    SyntaxError = LUA_ERRSYNTAX,      // Syntax error during precompilation.
    Yield = LUA_YIELD,                // The thread (coroutine) yields.
    FileError = LUA_ERRFILE           // A file-related error; e.g., it cannot open or read the file.
};

/// <summary>The various options, which can be performed using lua_gc.</summary>
enum class GCOption {
    Collect = LUA_GCCOLLECT, // Performs a full garbage-collection cycle.
    Stop = LUA_GCSTOP,       // Stops the garbage collector.
    Restart = LUA_GCRESTART, // Restarts the garbage collector.
    Count = LUA_GCCOUNT,     // Returns the current amount of memory (in Kbytes) in use by Lua.
    CountBytes =
        LUA_GCCOUNTB, // Returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.
    Step =
        LUA_GCSTEP, // gc(..., int stepsize); Performs an incremental step of garbage collection, corresponding to the allocation of stepsize Kbytes.
    IsRunning = LUA_GCISRUNNING, // Returns a boolean that tells whether the collector is running (i.e., not stopped).
    Incremental =
        LUA_GCINC, // gc(..., int pause, int stepmul, int stepsize); Changes the collector to incremental mode with the given parameters and returns the previous mode.
    Generational =
        LUA_GCGEN, // gc(..., int minormul, int majormul); Changes the collector to generational mode with the given parameters and returns the previous mode.
};

/// <summary>Lua value types.</summary>
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

/// <summary>Possible operations for lua_arith.</summary>
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
    BinaryNot = LUA_OPBNOT
};

/// <summary>Possible operations for lua_compare.</summary>
enum class CompareOp {
    Equal = LUA_OPEQ,
    LessThan = LUA_OPLT,
    LessEqual = LUA_OPLE,
};

/// <summary>A list of all Lua standard libraries.</summary>
enum class StandardLibrary { Base, Coroutine, Table, IO, OS, String, Utf8, Math, Debug, Package, COUNT };

std::array<lua_CFunction, static_cast<std::size_t>(StandardLibrary::COUNT)> libraryFunctions = {luaopen_base,
                                                                                                luaopen_coroutine,
                                                                                                luaopen_table,
                                                                                                luaopen_io,
                                                                                                luaopen_os,
                                                                                                luaopen_string,
                                                                                                luaopen_utf8,
                                                                                                luaopen_math,
                                                                                                luaopen_debug,
                                                                                                luaopen_package};

std::array<const char*, static_cast<std::size_t>(StandardLibrary::COUNT)> libraryNames = {"_G",
                                                                                          LUA_COLIBNAME,
                                                                                          LUA_TABLIBNAME,
                                                                                          LUA_IOLIBNAME,
                                                                                          LUA_OSLIBNAME,
                                                                                          LUA_STRLIBNAME,
                                                                                          LUA_UTF8LIBNAME,
                                                                                          LUA_MATHLIBNAME,
                                                                                          LUA_DBLIBNAME,
                                                                                          LUA_LOADLIBNAME};

enum class LoadMode { Default, Binary, Text, Both, COUNT };

std::array<const char*, static_cast<std::size_t>(LoadMode::COUNT)> loadModeNames = {nullptr, "b", "t", "bt"};

} // namespace dang::lua
