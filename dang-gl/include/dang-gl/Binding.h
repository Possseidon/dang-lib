#pragma once

#include "ObjectBase.h"

namespace dang::gl
{

class Binding {
public:
    template <class TInfo>
    void bind(const ObjectBase& object)
    {
        if (bound_object_ == &object)
            return;
        TInfo::bind(object.handle());
        bound_object_ = &object;
    }

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
