#pragma once

#include "Object.h"
#include "ObjectType.h"
#include "PixelInternalFormat.h"
#include "RenderbufferContext.h"

namespace dang::gl
{

/// <summary>A renderbuffer object containing image data, specifially used together with framebuffer objects.</summary>
/// <remarks>Natively supports multisampling.</remarks>
class RBO : public Object<ObjectType::Renderbuffer> {
public:
    /// <summary>Initializes the renderbuffer with the given size, format and optional multisampling-count.</summary>
    explicit RBO(dmath::svec2 size, PixelInternalFormat format = PixelInternalFormat::RGBA8, GLsizei samples = 0);
    /// <summary>Resets the bound renderbuffer of the context, in case of the renderbuffer still being bound.</summary>
    ~RBO();

    /// <summary>Bind the renderbuffer.</summary>
    void bind() const;

    /// <summary>Returns the width and height of the renderbuffer.</summary>
    dmath::svec2 size() const;
    /// <summary>Returns the pixel format of the renderbuffer.</summary>
    PixelInternalFormat format() const;
    /// <summary>Returns the sample count for multisampled renderbuffers or zero for non-multisampled ones.</summary>
    GLsizei samples() const;

private:
    dmath::svec2 size_;
    PixelInternalFormat format_;
    GLsizei samples_;
};

}
