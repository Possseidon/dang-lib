#include "dang-gl/Objects/ObjectContext.h"

namespace dang::gl {

ObjectContextBase::ObjectContextBase(Context& context)
    : context_(context)
{}

Context& ObjectContextBase::context() const { return context_; }

} // namespace dang::gl
