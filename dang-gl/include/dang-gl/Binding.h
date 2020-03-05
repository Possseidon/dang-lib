#pragma once

#include "ObjectBase.h"

namespace dang::gl
{

class Binding {
public:
    template <class TInfo>
    void bind(const ObjectBase* object);

private:
    const ObjectBase* bound_object_ = nullptr;
};

template<class TInfo>
inline void Binding::bind(const ObjectBase* object)
{
    if (bound_object_ == object)
        return;
    TInfo::bind(object->handle());
    bound_object_ = object;
}

}
