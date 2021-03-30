#pragma once

#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Objects/BufferMask.h"
#include "dang-gl/Objects/FramebufferContext.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/Objects/RBO.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief The filtering method to use for framebuffer blitting.
/// @remark The linear filtering method only works for the color buffer.
enum class BlitFilter {
    Nearest,
    Linear,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::BlitFilter> : default_enum_count<dang::gl::BlitFilter> {};

} // namespace dang::utils

namespace dang::gl {

template <>
inline constexpr dutils::EnumArray<BlitFilter, GLenum> gl_constants<BlitFilter> = {GL_NEAREST, GL_LINEAR};

/// @brief An error caused by an invalid FBO operation.
class FramebufferError : public std::runtime_error {
    using runtime_error::runtime_error;
};

/// @brief The different error states, which a framebuffer can be in.
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

/// @brief A framebuffer object, which represents the destination (or source) of OpenGL render operations.
/// @remark Framebuffer objects can be attached with both textures and renderbuffer objects.
class FBO : public Object<ObjectType::Framebuffer> {
public:
    /// @brief Wraps any framebuffer attachment point.
    class AttachmentPoint {
    public:
        friend class FBO;

        operator GLenum() const;

    private:
        AttachmentPoint(GLenum attachment);

        GLenum attachment_;
    };

    FBO() = default;

    FBO(EmptyObject)
        : Object<ObjectType::Framebuffer>(empty_object)
    {}

    /// @brief Resets the bound framebuffer of the context, in case of the framebuffer still being bound.
    ~FBO();

    FBO(const FBO&) = delete;
    FBO(FBO&&) = default;
    FBO& operator=(const FBO&) = delete;
    FBO& operator=(FBO&&) = default;

    /// @brief Sets an optional label for the object, which is used in by OpenGL generated debug messages.
    void setLabel(std::optional<std::string> label);

    /// @brief Returns a color attachment point with the given index.
    AttachmentPoint colorAttachment(std::size_t index) const;
    /// @brief Returns the depth attachment point.
    AttachmentPoint depthAttachment() const;
    /// @brief Returns the stencil attachment point.
    AttachmentPoint stencilAttachment() const;
    /// @brief Returns the depth-stencil attachment point.
    AttachmentPoint depthStencilAttachment() const;

    /// @brief Binds the framebuffer to the given target, defaulting to both draw and read.
    void bind(FramebufferTarget target = FramebufferTarget::Framebuffer) const;

    /// @brief Binds the default framebuffer to the given target of the specified window.
    static void bindDefault(Context& context, FramebufferTarget target = FramebufferTarget::Framebuffer);
    /// @brief Binds the default framebuffer to the given target of the associated window.
    void bindDefault(FramebufferTarget target = FramebufferTarget::Framebuffer) const;

    /// @brief Returns the forcibly common width and height of all attachments.
    std::optional<svec2> size();

    /// @brief Whether the framebuffer has any attachment.
    bool anyAttachments() const;
    /// @brief Whether the framebuffer has an attachment at the specified attachment point.
    bool isAttached(AttachmentPoint attachment_point) const;
    /// @brief Attaches the given renderbuffer to the specified attachment point.
    void attach(const RBO& rbo, AttachmentPoint attachment_point);
    /// @brief Detaches the current renderbuffer or texture from the specified attachment point.
    void detach(AttachmentPoint attachment_point);

    /// @brief Returns the current status of the framebuffer.
    FramebufferStatus status() const;
    /// @brief Whether the current status of the framebuffer is "complete".
    bool isComplete() const;
    /// @brief Throws an exception with an appropriate message if the framebuffer is not complete.
    void checkComplete() const;

    /// @brief Binds the framebuffer and fills it with the current clear color, depth and stencil values.
    void clear(BufferMask mask = BufferMask::ALL);

    /// @brief Binds the default framebuffer and fills it with the current clear color, depth and stencil values.
    static void clearDefault(Context& context, BufferMask mask = BufferMask::ALL);
    /// @brief Binds the default framebuffer and fills it with the current clear color, depth and stencil values.
    void clearDefault(BufferMask mask = BufferMask::ALL);

    void blitFrom(const FBO& other, BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest);
    void blitFromDefault(BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest);
    void blitToDefault(BufferMask mask = BufferMask::ALL, BlitFilter filter = BlitFilter::Nearest) const;

private:
    /// @brief Used to keep track of the smallest width and height.
    void updateSize(svec2 size);
    /// @brief Updates the given attachment point to being active or not.
    void updateAttachmentPoint(AttachmentPoint attachment_point, bool active);

    /// @brief Helper to blit pixels from one framebuffer to another.
    static void blit(ObjectContext<ObjectType::Framebuffer>& context,
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

} // namespace dang::gl
