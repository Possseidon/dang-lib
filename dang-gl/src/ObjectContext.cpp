#include "pch.h"
#include "ObjectContext.h"

namespace dang::gl
{

ObjectContextBase::ObjectContextBase(Window& window)
    : window_(window)
{
}

Window& ObjectContextBase::window() const
{
    return window_;
}

}
