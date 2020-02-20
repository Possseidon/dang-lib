#include "pch.h"
#include "ObjectBase.h"

namespace dang::gl
{

ObjectBase::ObjectBase(ObjectBase&& other) noexcept
    : handle_(other.handle_)
    , window_(other.window_)
{
    other.handle_ = 0;
}

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
