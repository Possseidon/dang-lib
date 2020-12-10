#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Specializes the context class for framebuffer objects.
template <>
class ObjectContext<ObjectType::Framebuffer> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Framebuffer>;
    using Wrapper = ObjectWrapper<ObjectType::Framebuffer>;

    using ObjectContextBase::ObjectContextBase;

    /// @brief Binds the given buffer handle to the specified target, if it isn't bound already.
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

    /// @brief Resets the bound buffer of the specified target, if the given handle is currently bound to it.
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
