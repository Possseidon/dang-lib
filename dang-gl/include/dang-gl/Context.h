#pragma once

#include "dang-utils/enum.h"

#include "ObjectContext.h"
#include "ObjectType.h"
#include "State.h"

namespace dang::gl
{

class Context {
public:
    inline static Context* current = nullptr;

    void initialize();

    State& state()
    {
        return state_;
    }

    const State& state() const
    {
        return state_;
    }

    State* operator->()
    {
        return &state_;
    }

    const State* operator->() const
    {
        return &state_;
    }

    template <ObjectType Type>
    auto& contextFor()
    {
        return static_cast<ObjectContext<Type>&>(*object_contexts_[Type]);
    }

    template <ObjectType Type>
    auto& contextFor() const
    {
        return static_cast<const ObjectContext<Type>&>(*object_contexts_[Type]);
    }

private:
    /// <summary>Initializes the contexts for the different GL-Object types.</summary>
    template <ObjectType... Types>
    void createContexts(dutils::EnumSequence<ObjectType, Types...>);

    State state_;
    dutils::EnumArray<ObjectType, std::unique_ptr<ObjectContextBase>> object_contexts_;
};

}
