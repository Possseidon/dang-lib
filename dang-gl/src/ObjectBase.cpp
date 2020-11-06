#include "pch.h"
#include "ObjectBase.h"

namespace dang::gl
{

ObjectBase::Handle ObjectBase::handle() const noexcept
{
    return handle_;
}

Window& ObjectBase::window() const noexcept
{
    return *window_;
}

ObjectBase::operator bool() const noexcept
{
    return handle_ != InvalidHandle;
}

ObjectBase::ObjectBase(Handle handle, Window& window) noexcept
    : handle_(handle)
    , window_(&window)
{
}

ObjectBase::ObjectBase(ObjectBase&& other) noexcept
    : ObjectBase()
{
    swap(other);
}

ObjectBase& ObjectBase::operator=(ObjectBase&& other) noexcept
{
    swap(other);
    return *this;
}

void ObjectBase::swap(ObjectBase& other) noexcept
{
    using std::swap;
    swap(handle_, other.handle_);
    swap(window_, other.window_);
}

}
