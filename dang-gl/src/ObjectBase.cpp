#include "pch.h"
#include "ObjectBase.h"

namespace dang::gl
{

GLuint ObjectBase::handle() const
{
    return handle_;
}

Window& ObjectBase::window() const
{
    return window_;
}

ObjectBase::ObjectBase(GLuint handle, Window& window)
    : handle_(handle)
    , window_(window)
{
}

}
