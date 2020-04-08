#include "pch.h"
#include "Stack.h"

#include "Reference.h"

namespace dang::lua
{

StackPos::StackPos()
    : state_(nullptr)
    , pos_(0)
{
}

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

std::string StackPos::tostring() const
{
    std::size_t len;
    const char* string = luaL_tolstring(state_, pos_, &len);
    std::string result(string, string + len);
    lua_pop(state_, 1);
    return result;
}

StackPos StackPos::arithPush(ArithOp operation, StackPos other) const
{
    assert(operation != ArithOp::UnaryMinus && operation != ArithOp::BinaryNot);
    push();
    other.push(state_);
    lua_arith(state_, static_cast<int>(operation));
    return StackPos(state_);
}

Reference StackPos::arith(ArithOp operation, StackPos other) const
{
    arithPush(operation, other);
    return Reference::take(state_);
}

StackPos StackPos::arithPush(ArithOp operation) const
{
    assert(operation == ArithOp::UnaryMinus || operation == ArithOp::BinaryNot);
    push();
    lua_arith(state_, static_cast<int>(operation));
    return StackPos(state_);
}

Reference StackPos::arith(ArithOp operation) const
{
    arithPush(operation);
    return Reference::take(state_);
}

Reference operator+(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Add, rhs);
}

Reference operator-(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Sub, rhs);
}

Reference operator*(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Mul, rhs);
}

Reference operator/(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Div, rhs);
}

Reference StackPos::idiv(StackPos other)
{
    return arith(ArithOp::IDiv, other);
}

Reference operator%(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::Mod, rhs);
}

Reference operator&(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::BinaryAnd, rhs);
}

Reference operator|(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::BinaryOr, rhs);
}

Reference operator^(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::BinaryXOr, rhs);
}

Reference operator<<(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::LeftShift, rhs);
}

Reference operator>>(StackPos lhs, StackPos rhs)
{
    return lhs.arith(ArithOp::RightShift, rhs);
}

bool operator==(StackPos lhs, StackPos rhs)
{
    return lhs.compare(CompareOp::Equal, rhs);
}

bool operator!=(StackPos lhs, StackPos rhs)
{
    return !(lhs == rhs);
}

bool operator<(StackPos lhs, StackPos rhs)
{
    return lhs.compare(CompareOp::LessThan, rhs);
}

bool operator<=(StackPos lhs, StackPos rhs)
{
    return lhs.compare(CompareOp::LessEqual, rhs);
}

bool operator>(StackPos lhs, StackPos rhs)
{
    return rhs.compare(CompareOp::LessThan, lhs);
}

bool operator>=(StackPos lhs, StackPos rhs)
{
    return rhs.compare(CompareOp::LessEqual, lhs);
}

std::ostream& operator<<(std::ostream& ostream, StackPos pos)
{
    return ostream << pos.tostring();
}

Reference StackPos::operator-() const
{
    return arith(ArithOp::UnaryMinus);
}

Reference StackPos::operator~() const
{
    return arith(ArithOp::BinaryNot);
}

bool StackPos::compare(CompareOp operation, StackPos other)
{
    if (other.state_ != state_) {
        auto tmp = other.push(state_);
        bool result = lua_compare(state_, pos_, tmp.pos_, static_cast<int>(operation)) != 0;
        tmp.pop();
        return result;
    }
    return lua_compare(state_, pos_, other.pos_, static_cast<int>(operation)) != 0;
}

PairsWrapper StackPos::pairs() const
{
    return PairsWrapper(*this);
}

IPairsWrapper StackPos::ipairs() const
{
    return IPairsWrapper(*this);
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
