#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Specialization for VertexArrays using the default bindable context.
template <>
class ObjectContext<ObjectType::VertexArray> : public ObjectContextBindable<ObjectType::VertexArray> {
    using ObjectContextBindable::ObjectContextBindable;
};

} // namespace dang::gl
