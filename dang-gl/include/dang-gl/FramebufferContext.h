#pragma once

#include "ObjectContext.h"
#include "ObjectHandle.h"
#include "ObjectType.h"
#include "ObjectWrapper.h"

namespace dang::gl {

/// <summary>Specializes the context class for framebuffer objects.</summary>
template <>
class ObjectContext<ObjectType::Framebuffer> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Framebuffer>;
    using Wrapper = ObjectWrapper<ObjectType::Framebuffer>;

    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given buffer handle to the specified target, if it isn't bound already.</summary>
    void bind(FramebufferTarget target, Handle handle)
    {
        switch (target) {
        case FramebufferTarget::Framebuffer:
            if (bound_draw_buffer_ == handle && bound_read_buffer_ == handle)
                return;
            Wrapper::bind(target, handle);
            bound_draw_buffer_ = handle;
            bound_read_buffer_ = handle;
            break;

        case FramebufferTarget::DrawFramebuffer:
            if (bound_draw_buffer_ == handle)
                return;
            Wrapper::bind(target, handle);
            bound_draw_buffer_ = handle;
            break;

        case FramebufferTarget::ReadFramebuffer:
            if (bound_read_buffer_ == handle)
                return;
            Wrapper::bind(target, handle);
            bound_read_buffer_ = handle;
            break;

        default:
            throw std::runtime_error("unknown framebuffer target");
        }
    }

    /// <summary>Resets the bound buffer of the specified target, if the given handle is currently bound to it.</summary>
    void reset(Handle handle)
    {
        if (bound_draw_buffer_ == handle) {
            Wrapper::bind(FramebufferTarget::DrawFramebuffer, {});
            bound_draw_buffer_ = {};
        }
        if (bound_read_buffer_ == handle) {
            Wrapper::bind(FramebufferTarget::ReadFramebuffer, {});
            bound_read_buffer_ = {};
        }
    }

private:
    Handle bound_draw_buffer_;
    Handle bound_read_buffer_;
};

} // namespace dang::gl
