#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Specialization for GL-Programs using the default bindable context.
template <>
class ObjectContext<ObjectType::Program> : public ObjectContextBindable<ObjectType::Program> {
    using ObjectContextBindable::ObjectContextBindable;
};

} // namespace dang::gl
