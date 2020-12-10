#include "pch.h"

#include "Objects/FBO.h"

namespace dang::gl {

FBO::AttachmentPoint::operator GLenum() const { return attachment_; }

FBO::AttachmentPoint::AttachmentPoint(GLenum attachment)
    : attachment_(attachment)
{}

FBO::~FBO()
{
    if (*this)
        objectContext().reset(handle());
}

void FBO::setLabel(std::optional<std::string> label)
{
    bind();
    Object::setLabel(std::move(label));
}

FBO::AttachmentPoint FBO::colorAttachment(std::size_t index) const
{
    if (index >= color_attachments_.size())
        throw FramebufferError("Framebuffer color attachment index too high.");
    return GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);
}

FBO::AttachmentPoint FBO::depthAttachment() const { return GL_DEPTH_ATTACHMENT; }

FBO::AttachmentPoint FBO::stencilAttachment() const { return GL_STENCIL_ATTACHMENT; }

FBO::AttachmentPoint FBO::depthStencilAttachment() const { return GL_DEPTH_STENCIL_ATTACHMENT; }

void FBO::bindDefault(Context& context, FramebufferTarget target)
{
    context.contextFor<ObjectType::Framebuffer>().bind(target, {});
}

void FBO::bind(FramebufferTarget target) const { objectContext().bind(target, handle()); }

void FBO::bindDefault(FramebufferTarget target) const { objectContext().bind(target, {}); }

std::optional<svec2> FBO::size() { return size_; }

bool FBO::anyAttachments() const
{
    auto is_attached = [](bool attached) { return attached; };
    return std::any_of(color_attachments_.begin(), color_attachments_.end(), is_attached) || depth_attachment_ ||
           stencil_attachment_ || depth_stencil_attachment_;
}

bool FBO::isAttached(AttachmentPoint attachment_point) const
{
    GLenum attachment = attachment_point;
    switch (attachment) {
    case GL_DEPTH_ATTACHMENT:
        return depth_attachment_;
    case GL_STENCIL_ATTACHMENT:
        return stencil_attachment_;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        return depth_stencil_attachment_;
    default:
        return color_attachments_[std::size_t{attachment - GL_COLOR_ATTACHMENT0}];
    }
}

void FBO::attach(const RBO& rbo, AttachmentPoint attachment_point)
{
    auto fbo_target = FramebufferTarget::DrawFramebuffer;
    auto rbo_target = RenderbufferTarget::Renderbuffer;
    bind(fbo_target);
    rbo.bind();
    glFramebufferRenderbuffer(
        toGLConstant(fbo_target), attachment_point, toGLConstant(rbo_target), rbo.handle().unwrap());
    updateSize(rbo.size());
    updateAttachmentPoint(attachment_point, true);
}

void FBO::detach(AttachmentPoint attachment_point)
{
    auto target = FramebufferTarget::DrawFramebuffer;
    bind(target);
    glFramebufferTexture(toGLConstant(target), attachment_point, Handle{}.unwrap(), 0);
    updateAttachmentPoint(attachment_point, false);
    if (!anyAttachments())
        size_ = std::nullopt;
}

FramebufferStatus FBO::status() const
{
    auto target = FramebufferTarget::DrawFramebuffer;
    bind(target);
    return static_cast<FramebufferStatus>(glCheckFramebufferStatus(toGLConstant(target)));
}

bool FBO::isComplete() const { return status() == FramebufferStatus::Complete; }

void FBO::checkComplete() const
{
    // Error messages excerpted from:
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
    switch (status()) {
    case FramebufferStatus::Complete:
        return;
    case FramebufferStatus::Undefined:
        throw FramebufferError(
            "The framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.");
    case FramebufferStatus::IncompleteAttachment:
        throw FramebufferError("Some attachment points are framebuffer incomplete.");
    case FramebufferStatus::IncompleteMissingAttachment:
        throw FramebufferError("The framebuffer does not have at least one image attached to it.");
    case FramebufferStatus::IncompleteDrawBuffer:
        throw FramebufferError("The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment "
                               "point(s) named by GL_DRAW_BUFFERi.");
    case FramebufferStatus::IncompleteReadBuffer:
        throw FramebufferError("GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
                               "is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
    case FramebufferStatus::Unsupported:
        throw FramebufferError("The combination of internal formats of the attached images violates an "
                               "implementation-dependent set of restrictions.");
    case FramebufferStatus::IncompleteMultisample:
        throw FramebufferError(
            "The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or the value of "
            "GL_TEXTURE_SAMPLES is not the same for all attached textures or the attached images are a mix of "
            "renderbuffers and textures and the value of GL_RENDERBUFFER_SAMPLES does not match the value of "
            "GL_TEXTURE_SAMPLES. Or the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached "
            "textures or the attached images are a mix of renderbuffers and textures and the value of "
            "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.");
    case FramebufferStatus::IncompleteLayerTargets:
        throw FramebufferError("A framebuffer attachment is layered and a populated attachment is not layered, or all "
                               "populated color attachments are not from textures of the same target.");
    }
    throw FramebufferError("The framebuffer is not complete for an unknown reason.");
}

void FBO::clear(BufferMask mask)
{
    bind(FramebufferTarget::DrawFramebuffer);
    glClear(static_cast<GLbitfield>(mask));
}

void FBO::clearDefault(Context& context, BufferMask mask)
{
    bindDefault(context, FramebufferTarget::DrawFramebuffer);
    glClear(static_cast<GLbitfield>(mask));
}

void FBO::clearDefault(BufferMask mask)
{
    bindDefault(FramebufferTarget::DrawFramebuffer);
    glClear(static_cast<GLbitfield>(mask));
}

void FBO::blitFrom(const FBO& other, BufferMask mask, BlitFilter filter)
{
    if (!size_ || !other.size_)
        return;
    ibounds2 src_rect(ivec2{*size_});
    ibounds2 dst_rect(ivec2{*other.size_});
    blit(objectContext(), other.handle(), handle(), src_rect, dst_rect, mask, filter);
}

void FBO::blitFromDefault(BufferMask mask, BlitFilter filter)
{
    if (!size_)
        return;
    ibounds2 src_rect(context().size());
    ibounds2 dst_rect(ivec2{*size_});
    blit(objectContext(), {}, handle(), src_rect, dst_rect, mask, filter);
}

void FBO::blitToDefault(BufferMask mask, BlitFilter filter) const
{
    if (!size_)
        return;
    ibounds2 src_rect(ivec2{*size_});
    ibounds2 dst_rect(context().size());
    blit(objectContext(), handle(), {}, src_rect, dst_rect, mask, filter);
}

void FBO::updateSize(svec2 size)
{
    if (!size_)
        size_ = size;
    if (*size_ != size)
        throw FramebufferError("Differently sized framebuffer attachments are not supported.");
}

void FBO::updateAttachmentPoint(AttachmentPoint attachment_point, bool active)
{
    GLenum attachment = attachment_point;
    switch (attachment) {
    case GL_DEPTH_ATTACHMENT:
        depth_attachment_ = active;
        break;
    case GL_STENCIL_ATTACHMENT:
        stencil_attachment_ = active;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        depth_stencil_attachment_ = active;
        break;
    default:
        color_attachments_[std::size_t{attachment - GL_COLOR_ATTACHMENT0}] = active;
    }
}

void FBO::blit(ObjectContext<ObjectType::Framebuffer>& context,
               Handle read_framebuffer,
               Handle draw_framebuffer,
               const ibounds2& src_rect,
               const ibounds2& dst_rect,
               BufferMask mask,
               BlitFilter filter)
{
    context.bind(FramebufferTarget::ReadFramebuffer, read_framebuffer);
    context.bind(FramebufferTarget::DrawFramebuffer, draw_framebuffer);
    const auto& src_size = src_rect.size();
    const auto& dst_size = dst_rect.size();
    glBlitFramebuffer(src_rect.low.x(),
                      src_rect.low.y(),
                      src_size.x(),
                      src_size.y(),
                      dst_rect.low.x(),
                      dst_rect.low.y(),
                      dst_size.x(),
                      dst_size.y(),
                      static_cast<GLbitfield>(mask),
                      toGLConstant(filter));
}

} // namespace dang::gl
