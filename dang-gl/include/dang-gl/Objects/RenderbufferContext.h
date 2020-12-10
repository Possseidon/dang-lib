#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// <summary>Specializes the context class for renderbuffer objects.</summary>
template <>
class ObjectContext<ObjectType::Renderbuffer> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Renderbuffer>;
    using Wrapper = ObjectWrapper<ObjectType::Renderbuffer>;

    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given renderbuffer handle, unless it is bound already.</summary>
    void bind(Handle handle)
    {
        if (bound_renderbuffer_ == handle)
            return;
        Wrapper::bind(RenderbufferTarget::Renderbuffer, handle);
        bound_renderbuffer_ = handle;
    }

    /// <summary>Resets the bound renderbuffer, if the given handle is currently bound.</summary>
    void reset(Handle handle)
    {
        if (bound_renderbuffer_ != handle)
            return;
        Wrapper::bind(RenderbufferTarget::Renderbuffer, {});
        bound_renderbuffer_ = {};
    }

private:
    Handle bound_renderbuffer_;
};

} // namespace dang::gl
