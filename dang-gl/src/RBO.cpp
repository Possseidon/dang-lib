#include "pch.h"
#include "RBO.h"

namespace dang::gl
{

RBO::RBO(dmath::svec2 size, PixelInternalFormat format, GLsizei samples)
    : size_(size)
    , format_(format)
    , samples_(samples)
{
    bind();
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        samples,
        toGLConstant(format),
        static_cast<GLsizei>(size.x()),
        static_cast<GLsizei>(size.y()));
}

RBO::~RBO()
{
    context().reset(handle());
}

void RBO::bind() const
{
    context().bind(handle());
}

dmath::svec2 RBO::size() const
{
    return size_;
}

PixelInternalFormat RBO::format() const
{
    return format_;
}

GLsizei RBO::samples() const
{
    return samples_;
}

}
