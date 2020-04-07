#include "pch.h"
#include "Stack.h"

namespace dang::lua
{

StackPos::StackPos(lua_State* state, int pos)
    : state_(state)
    , pos_(lua_absindex(state, pos))
{
}

lua_State* StackPos::state() const
{
    return state_;
}

int StackPos::pos() const
{
    return pos_;
}

StackPos StackPos::push() const
{
    lua_pushvalue(state_, pos_);
    return StackPos(state_, -1);
}

StackPos StackPos::push(lua_State* L) const
{
    push();
    lua_xmove(state_, L, 1);
    return StackPos(L, -1);
}

void StackPos::pop() const
{
    assert(lua_gettop(state_) == pos_);
    lua_pop(state_, 1);
}

Type StackPos::type() const
{
    return static_cast<Type>(lua_type(state_, pos_));
}

StackPos VarStackPos::operator[](int pos) const
{
    return indexHelper<StackPos>(pos);
}

VarStackPos VarStackPos::substack(int from) const
{
    return substackHelper<VarStackPos>(from);
}

StackIterator<StackPos> VarStackPos::begin() const
{
    return beginHelper<StackPos>();
}

StackIterator<StackPos> VarStackPos::end() const
{
    return endHelper<StackPos>();
}

Arg VarArg::operator[](int pos) const
{
    return indexHelper<Arg>(pos);
}

VarArg VarArg::substack(int from) const
{
    return substackHelper<VarArg>(from);
}

StackIterator<Arg> VarArg::begin() const
{
    return StackIterator<Arg>(Arg(state(), pos()));
}

StackIterator<Arg> VarArg::end() const
{
    return StackIterator<Arg>(Arg(state(), pos() + size()));
}

Ret MultRet::operator[](int pos) const
{
    return indexHelper<Ret>(pos);
}

MultRet MultRet::substack(int from) const
{
    return substackHelper<MultRet>(from);
}

StackIterator<Ret> MultRet::begin() const
{
    return beginHelper<Ret>();
}

StackIterator<Ret> MultRet::end() const
{
    return endHelper<Ret>();
}

lua_State* VarStackPos::state() const
{
    return state_;
}

int VarStackPos::pos() const
{
    return pos_;
}

int VarStackPos::pushAll() const
{
    for (auto arg : *this)
        arg.push();
    return count_;
}

int VarStackPos::pushAll(lua_State* L) const
{
    for (auto arg : *this)
        arg.push(L);
    return count_;
}

bool VarStackPos::empty() const
{
    return count_ == 0;
}

int VarStackPos::size() const
{
    return count_;
}

int VarStackPos::max_size() const
{
    return count_;
}

}
