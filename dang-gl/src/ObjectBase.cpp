#include "pch.h"
#include "ObjectBase.h"

namespace dang::gl
{

GLuint ObjectBase::handle()
{
    return handle_;
}

Window& ObjectBase::window()
{
    return window_;
}

ObjectBase::ObjectBase(GLuint handle, Window& window)
    : handle_(handle)
    , window_(window)
{
}

}
