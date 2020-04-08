#pragma once

namespace dang::lua
{

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
    COUNT = LUA_NUMTAGS
};

/// <summary>Operations for lua_arith.</summary>
enum class ArithOp {
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
    UnaryMinus = LUA_OPUNM,
    BinaryNot = LUA_OPBNOT
};

/// <summary>Operations for lua_compare.</summary>
enum class CompareOp {
    Equal = LUA_OPEQ,
    LessThan = LUA_OPLT,
    LessEqual = LUA_OPLE,
};

}
