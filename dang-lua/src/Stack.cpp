#include "pch.h"
#include "Stack.h"

namespace dang::lua
{

StackPos::StackPos(lua_State* state, int pos)
    : state_(state)
    , pos_(pos)
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

void StackPos::push() const
{
    lua_pushvalue(state_, pos_);
}

void StackPos::push(lua_State* L) const
{
    push();
    if (L != state_)
        lua_xmove(state_, L, 1);
}

Type StackPos::type() const
{
    return static_cast<Type>(lua_type(state_, pos_));
}

inline StackIterator::StackIterator(StackPos arg)
    : arg_(arg)
{
}

inline StackIterator::reference StackIterator::operator*()
{
    return arg_;
}

inline StackIterator::pointer StackIterator::operator->()
{
    return &arg_;
}

bool operator==(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() == rhs.arg_.pos();
}

bool operator!=(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() != rhs.arg_.pos();
}

bool operator<(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() < rhs.arg_.pos();
}

bool operator<=(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() <= rhs.arg_.pos();
}

bool operator>(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() > rhs.arg_.pos();
}

bool operator>=(StackIterator lhs, StackIterator rhs)
{
    return lhs.arg_.pos() >= rhs.arg_.pos();
}

inline StackIterator& StackIterator::operator++()
{
    arg_.pos_++;
    return *this;
}

inline StackIterator StackIterator::operator++(int)
{
    auto old = *this;
    ++(*this);
    return old;
}

inline StackIterator& StackIterator::operator--()
{
    arg_.pos_--;
    return *this;
}

inline StackIterator StackIterator::operator--(int)
{
    auto old = *this;
    --(*this);
    return old;
}

inline StackIterator& StackIterator::operator+=(difference_type offset)
{
    arg_.pos_ += offset;
    return *this;
}

inline StackIterator StackIterator::operator+(difference_type offset) const
{
    return StackIterator(StackPos(arg_.state_, arg_.pos_ + offset));
}

inline StackIterator& StackIterator::operator-=(difference_type offset)
{
    arg_.pos_ -= offset;
    return *this;
}

inline StackIterator StackIterator::operator-(difference_type offset) const
{
    return StackIterator(StackPos(arg_.state_, arg_.pos_ - offset));
}

inline StackIterator::value_type StackIterator::operator[](difference_type offset) const
{
    return StackPos(arg_.state_, arg_.pos_ + offset);
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

StackPos VarStackPos::operator[](int pos) const
{
    return StackPos(state_, pos_ + pos - 1);
}

VarStackPos VarStackPos::substack(int from) const
{
    return VarStackPos(state_, pos_ + from - 1, std::max(0, count_ - from + 1));
}

StackIterator VarStackPos::begin() const
{
    return StackIterator(StackPos(state_, pos_));
}

StackIterator VarStackPos::end() const
{
    return StackIterator(StackPos(state_, pos_ + count_));
}

}
