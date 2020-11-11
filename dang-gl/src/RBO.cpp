#include "pch.h"
#include "RBO.h"

namespace dang::gl
{

RBO::RBO(svec2 size, GLsizei samples, PixelInternalFormat format)
    : size_(size)
    , samples_(samples)
    , format_(format)
{
    bind();
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        samples,
        toGLConstant(format),
        size.x(),
        size.y());
}

RBO::~RBO()
{
    if (*this)
        objectContext().reset(handle());
}

RBO RBO::color(svec2 size, GLsizei samples)
{
    return RBO(size, samples, PixelInternalFormat::RGBA8);
}

RBO RBO::depth(svec2 size, GLsizei samples)
{
    return RBO(size, samples, PixelInternalFormat::DEPTH_COMPONENT);
}

RBO RBO::depthStencil(svec2 size, GLsizei samples)
{
    return RBO(size, samples, PixelInternalFormat::DEPTH_STENCIL);
}

RBO RBO::stencil(svec2 size, GLsizei samples)
{
    return RBO(size, samples, PixelInternalFormat::STENCIL_INDEX8);
}

void RBO::bind() const
{
    objectContext().bind(handle());
}

svec2 RBO::size() const
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
