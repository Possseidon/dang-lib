#pragma once

#include "ClearMask.h"
#include "Object.h"
#include "ObjectContext.h"
#include "ObjectType.h"
#include "RBO.h"
#include "Texture.h"

namespace dang::gl
{

/// <summary>An error caused by an invalid FBO operation.</summary>
class FramebufferError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

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

/// <summary>The different error states, which a framebuffer can be in.</summary>
enum class FramebufferStatus : GLenum {
    Undefined = GL_FRAMEBUFFER_UNDEFINED,
    Complete = GL_FRAMEBUFFER_COMPLETE,
    IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
    IncompleteMissingAttachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
    IncompleteDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
    IncompleteReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
    Unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
    IncompleteMultisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
    IncompleteLayerTargets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
};

/// <summary>A framebuffer object, which represents the destination (or source) of OpenGL render operations.</summary>
/// <remarks>Framebuffer objects can be attached with both textures and renderbuffer objects.</remarks>
class FBO : public Object<ObjectType::Framebuffer> {
public:

    /// <summary>Wraps any framebuffer attachment point.</summary>
    class AttachmentPoint {
    public:
        friend class FBO;

        operator GLenum() const;

    private:
        AttachmentPoint(GLenum attachment);

        GLenum attachment_;
    };

    using Object::Object;

    /// <summary>Resets the bound framebuffer of the context, in case of the framebuffer still being bound.</summary>
    ~FBO();

    /// <summary>Returns a color attachment point with the given index.</summary>
    AttachmentPoint colorAttachment(std::size_t index) const;
    /// <summary>Returns the depth attachment point.</summary>
    AttachmentPoint depthAttachment() const;
    /// <summary>Returns the stencil attachment point.</summary>
    AttachmentPoint stencilAttachment() const;
    /// <summary>Returns the depth-stencil attachment point.</summary>
    AttachmentPoint depthStencilAttachment() const;

    /// <summary>Binds the framebuffer to the given target, defaulting to both draw and read.</summary>
    void bind(FramebufferTarget target = FramebufferTarget::Framebuffer) const;

    /// <summary>Binds the default framebuffer to the given target of the specified window.</summary>
    static void bindDefault(Window& window, FramebufferTarget target = FramebufferTarget::Framebuffer);
    /// <summary>Binds the default framebuffer to the given target of the associated window.</summary>
    void bindDefault(FramebufferTarget target = FramebufferTarget::Framebuffer) const;

    /// <summary>Returns the forcibly common width and height of all attachments.</summary>
    std::optional<dmath::svec2> size();

    /// <summary>Whether the framebuffer has any attachment.</summary>
    bool anyAttachments() const;
    /// <summary>Whether the framebuffer has an attachment at the specified attachment point.</summary>
    bool isAttached(AttachmentPoint attachment_point) const;
    /// <summary>Attaches the given renderbuffer to the specified attachment point.</summary>
    void attach(const RBO& rbo, AttachmentPoint attachment_point);
    /// <summary>Detaches the current renderbuffer or texture from the specified attachment point.</summary>
    void detach(AttachmentPoint attachment_point);

    /// <summary>Returns the current status of the framebuffer.</summary>
    FramebufferStatus status() const;
    /// <summary>Whether the current status of the framebuffer is "complete".</summary>
    bool isComplete() const;
    /// <summary>Throws an exception with an appropriate message if the framebuffer is not complete.</summary>
    void checkComplete() const;

    /// <summary>Binds the framebuffer and fills it with the current clear color, depth and stencil values.</summary>
    void clear(ClearMask mask);

    /// <summary>Binds the default framebuffer and fills it with the current clear color, depth and stencil values.</summary>
    static void clearDefault(Window& window, ClearMask mask);
    /// <summary>Binds the default framebuffer and fills it with the current clear color, depth and stencil values.</summary>
    void clearDefault(ClearMask mask);

private:
    /// <summary>Used to keep track of the smallest width and height.</summary>
    void updateSize(dmath::svec2 size);
    /// <summary>Updates the given attachment point to being active or not.</summary>
    void updateAttachmentPoint(AttachmentPoint attachment_point, bool active);

    std::optional<dmath::svec2> size_;
    std::vector<bool> color_attachments_ = std::vector<bool>(window().state().max_color_attachments);
    bool depth_attachment_ = false;
    bool stencil_attachment_ = false;
    bool depth_stencil_attachment_ = false;
};

}
