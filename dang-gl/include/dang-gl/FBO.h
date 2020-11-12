#pragma once

#include "BufferMask.h"
#include "FramebufferContext.h"
#include "MathTypes.h"
#include "Object.h"
#include "ObjectContext.h"
#include "ObjectHandle.h"
#include "ObjectType.h"
#include "ObjectWrapper.h"
#include "RBO.h"

namespace dang::gl
{

/// <summary>An error caused by an invalid FBO operation.</summary>
class FramebufferError : public std::runtime_error {
    using runtime_error::runtime_error;
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

/// <summary>The filtering method to use for framebuffer blitting.</summary>
/// <remarks>The linear filtering method only works for the color buffer.</remarks>
enum class BlitFilter {
    Nearest,
    Linear,

    COUNT
};

template <>
constexpr dutils::EnumArray<BlitFilter, GLenum> GLConstants<BlitFilter> = {
    GL_NEAREST,
    GL_LINEAR
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

    FBO() = default;
    /// <summary>Resets the bound framebuffer of the context, in case of the framebuffer still being bound.</summary>
    ~FBO();

    FBO(const FBO&) = delete;
    FBO(FBO&&) = default;
    FBO& operator=(const FBO&) = delete;
    FBO& operator=(FBO&&) = default;

    /// <summary>Sets an optional label for the object, which is used in by OpenGL generated debug messages.</summary>
    void setLabel(std::optional<std::string> label);

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
    static void bindDefault(Context& context, FramebufferTarget target = FramebufferTarget::Framebuffer);
    /// <summary>Binds the default framebuffer to the given target of the associated window.</summary>
    void bindDefault(FramebufferTarget target = FramebufferTarget::Framebuffer) const;

    /// <summary>Returns the forcibly common width and height of all attachments.</summary>
    std::optional<svec2> size();

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
    void clear(BufferMask mask = BufferMask::ALL);

    /// <summary>Binds the default framebuffer and fills it with the current clear color, depth and stencil values.</summary>
    static void clearDefault(Context& context, BufferMask mask = BufferMask::ALL);
    /// <summary>Binds the default framebuffer and fills it with the current clear color, depth and stencil values.</summary>
    void clearDefault(BufferMask mask = BufferMask::ALL);

    void blitFrom(const FBO& other, BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest);
    void blitFromDefault(BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest);
    void blitToDefault(BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest) const;

private:
    /// <summary>Used to keep track of the smallest width and height.</summary>
    void updateSize(svec2 size);
    /// <summary>Updates the given attachment point to being active or not.</summary>
    void updateAttachmentPoint(AttachmentPoint attachment_point, bool active);

    /// <summary>Helper to blit pixels from one framebuffer to another.</summary>
    static void blit(
        ObjectContext<ObjectType::Framebuffer>& context,
        Handle read_framebuffer,
        Handle draw_framebuffer,
        const ibounds2& src_rect,
        const ibounds2& dst_rect,
        BufferMask mask,
        BlitFilter filter);

    std::optional<svec2> size_;
    std::vector<bool> color_attachments_ = std::vector<bool>(context()->max_color_attachments);
    bool depth_attachment_ = false;
    bool stencil_attachment_ = false;
    bool depth_stencil_attachment_ = false;
};

}
