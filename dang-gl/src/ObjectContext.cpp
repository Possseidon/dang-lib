#include "pch.h"
#include "ObjectContext.h"

namespace dang::gl
{

ObjectContextBase::ObjectContextBase(Context& context)
    : context_(context)
{
}

Context& ObjectContextBase::context() const
{
    return context_;
}

}
