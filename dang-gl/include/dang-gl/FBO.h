#pragma once

#include "Object.h"
#include "ObjectContext.h"
#include "ObjectType.h"

namespace dang::gl
{

/// <summary>Specializes the context class for framebuffer objects.</summary>
template <>
class ObjectContext<ObjectType::Framebuffer> : public ObjectContextBase {
public:
    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given buffer handle to the specified target, if it isn't bound already.</summary>
    void bind(FramebufferTarget target, GLuint handle)
    {
        switch (target) {
        case FramebufferTarget::Framebuffer:
            if (bound_draw_buffer_ == handle && bound_read_buffer_ == handle)
                return;
            ObjectWrapper<ObjectType::Framebuffer>::bind(target, handle);
            bound_draw_buffer_ = handle;
            bound_read_buffer_ = handle;
            break;

        case FramebufferTarget::DrawFramebuffer:
            if (bound_draw_buffer_ == handle)
                return;
            ObjectWrapper<ObjectType::Framebuffer>::bind(target, handle);
            bound_draw_buffer_ = handle;
            break;

        case FramebufferTarget::ReadFramebuffer:
            if (bound_read_buffer_ == handle)
                return;
            ObjectWrapper<ObjectType::Framebuffer>::bind(target, handle);
            bound_read_buffer_ = handle;
            break;
        }
    }

    /// <summary>Resets the bound buffer of the specified target, if the given handle is currently bound to it.</summary>
    void reset(GLuint handle)
    {
        if (bound_draw_buffer_ == handle) {
            ObjectWrapper<ObjectType::Framebuffer>::bind(FramebufferTarget::DrawFramebuffer, 0);
            bound_draw_buffer_ = 0;
        }
        if (bound_read_buffer_ == handle) {
            ObjectWrapper<ObjectType::Framebuffer>::bind(FramebufferTarget::ReadFramebuffer, 0);
            bound_read_buffer_ = 0;
        }
    }

private:
    GLuint bound_draw_buffer_;
    GLuint bound_read_buffer_;
};

/// <summary>A framebuffer object, which represents the destination (or source) of OpenGL render operations.</summary>
/// <remarks>Framebuffer objects can be attached with both textures and renderbuffer objects.</remarks>
class FBO : public Object<ObjectType::Framebuffer> {
public:
    using Object::Object;

    /// <summary>Resets the bound framebuffer of the context, in case of the framebuffer still being bound.</summary>
    ~FBO();

    /// <summary>Binds the default framebuffer to the given target of the specified window.</summary>
    static void bindDefault(Window& window, FramebufferTarget target = FramebufferTarget::Framebuffer);

    /// <summary>Binds the framebuffer to the given target, defaulting to both draw and read.</summary>
    void bind(FramebufferTarget target = FramebufferTarget::Framebuffer);
    /// <summary>Binds the default framebuffer to the given target of the associated window.</summary>
    void bindDefault(FramebufferTarget target = FramebufferTarget::Framebuffer);
};

}
