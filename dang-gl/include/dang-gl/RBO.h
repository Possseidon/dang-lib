#pragma once

#include "Object.h"
#include "ObjectBase.h"
#include "ObjectType.h"
#include "PixelInternalFormat.h"

namespace dang::gl
{

/// <summary>Specializes the context class for renderbuffer objects.</summary>
template <>
class ObjectContext<ObjectType::Renderbuffer> : public ObjectContextBase {
public:
    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given renderbuffer handle, unless it is bound already.</summary>
    void bind(ObjectBase::Handle handle)
    {
        if (bound_renderbuffer_ == handle)
            return;
        ObjectWrapper<ObjectType::Renderbuffer>::bind(RenderbufferTarget::Renderbuffer, handle);
        bound_renderbuffer_ = handle;
    }

    /// <summary>Resets the bound renderbuffer, if the given handle is currently bound.</summary>
    void reset(ObjectBase::Handle handle)
    {
        if (bound_renderbuffer_ != handle)
            return;
        ObjectWrapper<ObjectType::Renderbuffer>::bind(RenderbufferTarget::Renderbuffer, ObjectBase::InvalidHandle);
        bound_renderbuffer_ = ObjectBase::InvalidHandle;
    }

private:
    ObjectBase::Handle bound_renderbuffer_;
};

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
