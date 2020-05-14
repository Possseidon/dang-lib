#include "pch.h"
#include "FBO.h"

namespace dang::gl
{

FBO::~FBO()
{
    context().reset(handle());
}

void FBO::bindDefault(Window& window, FramebufferTarget target)
{
    window.objectContext<ObjectType::Framebuffer>().bind(target, 0);
}

void FBO::bind(FramebufferTarget target)
{
    context().bind(target, handle());
}

void FBO::bindDefault(FramebufferTarget target)
{
    context().bind(target, 0);
}

}
