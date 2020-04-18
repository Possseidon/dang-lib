#pragma once

#include "ObjectBase.h"

namespace dang::gl
{

/// <summary>A context specific object binding, which remembers the currently bound object to minimize redundant bind calls.</summary>
class Binding {
public:
    /// <summary>Binds the object using the bind function of the TInfo struct, unless the object is already bound.</summary>
    template <class TInfo>
    void bind(const ObjectBase& object)
    {
        if (bound_object_ == &object)
            return;
        TInfo::bind(object.handle());
        bound_object_ = &object;
    }

    /// <summary>Used in the move constructor of Object to update the bound object if necessary.</summary>
    template <class TInfo>
    void move(const ObjectBase& from, const ObjectBase& to)
    {
        if (bound_object_ == &from)
            bound_object_ = &to;
    }

private:
    const ObjectBase* bound_object_ = nullptr;
};

}
