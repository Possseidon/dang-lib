#include "pch.h"

#include "Context/State.h"

namespace dang::gl {

detail::StatePropertyBase::StatePropertyBase(State& state)
    : state_(state)
    , index_(state.property_count_++)
{}

ScopedState State::scoped() { return ScopedState(*this); }

ScopedState::ScopedState(State& state)
    : state_(state)
{
    state.push();
}

ScopedState::~ScopedState() { state_.pop(); }

State& ScopedState::operator*() const { return state_; }

State* ScopedState::operator->() const { return &state_; }

void State::push() { state_backup_.emplace(); }

void State::pop() { state_backup_.pop(); }

} // namespace dang::gl
