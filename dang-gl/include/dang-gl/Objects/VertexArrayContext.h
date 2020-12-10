#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectType.h"

namespace dang::gl {

/// <summary>Specialization for VertexArrays using the default bindable context.</summary>
template <>
class ObjectContext<ObjectType::VertexArray> : public ObjectContextBindable<ObjectType::VertexArray> {
    using ObjectContextBindable::ObjectContextBindable;
};

} // namespace dang::gl
