#pragma once

#include "ObjectBase.h"

namespace dang::gl
{

class Binding {
public:
    template <class TInfo>
    void bind(ObjectBase* object);

private:
    ObjectBase* bound_object_ = nullptr;
};

template<class TInfo>
inline void Binding::bind(ObjectBase* object)
{
    if (bound_object_ == object)
        return;
    TInfo::bind(object->handle());
    bound_object_ = object;
}

}
