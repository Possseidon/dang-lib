#pragma once

#include "ObjectBase.h"

namespace dang::gl
{

class ObjectContext;

/// <summary>A base class, for when the default binding mechanism is not favorable.</summary>
class ObjectBindingBase {
public:
    virtual ~ObjectBindingBase() = default;
};

/// <summary>A context specific object binding, which remembers the currently bound object to minimize redundant bind calls.</summary>
template <typename TContext = ObjectContext>
class ObjectBinding : public ObjectBindingBase {
public:
    /// <summary>Initializes the binding with a reference to the context.</summary>
    ObjectBinding(TContext& context)
        : context_(context)
    {
    }

    /// <summary>Returns the associated context.</summary>
    TContext& context() const
    {
        return context_;
    }

    /// <summary>Binds the object using the bind function of the TInfo struct, unless the object is already bound.</summary>
    template <typename TInfo>
    void bind(const ObjectBase& object)
    {
        if (bound_object_ == &object)
            return;
        TInfo::bind(object.handle());
        bound_object_ = &object;
    }

    /// <summary>Used in the move constructor of Object to update the bound object if necessary.</summary>
    template <typename TInfo>
    void move(const ObjectBase& from, const ObjectBase& to)
    {
        if (bound_object_ == &from)
            bound_object_ = &to;
    }

private:
    TContext& context_;
    const ObjectBase* bound_object_ = nullptr;
};

}
