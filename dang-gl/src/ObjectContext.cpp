#include "pch.h"
#include "ObjectContext.h"

namespace dang::gl
{

ObjectContext::ObjectContext(Window& window)
    : window_(window)
{
}

Window& ObjectContext::window() const
{
    return window_;
}

}
