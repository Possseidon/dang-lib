#pragma once

#include "ObjectContext.h"
#include "ObjectType.h"

namespace dang::gl {

/// <summary>Specialization for GL-Programs using the default bindable context.</summary>
template <>
class ObjectContext<ObjectType::Program> : public ObjectContextBindable<ObjectType::Program> {
    using ObjectContextBindable::ObjectContextBindable;
};

} // namespace dang::gl
