#pragma once

#include "ObjectContext.h"
#include "ObjectHandle.h"
#include "ObjectType.h"
#include "ObjectWrapper.h"

namespace dang::gl {

/// <summary>Specializes the context class for buffer objects.</summary>
template <>
class ObjectContext<ObjectType::Buffer> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Buffer>;
    using Wrapper = ObjectWrapper<ObjectType::Buffer>;

    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given buffer handle to the specified target, if it isn't bound already.</summary>
    void bind(BufferTarget target, Handle handle)
    {
        if (bound_buffers_[target] == handle)
            return;
        Wrapper::bind(target, handle);
        bound_buffers_[target] = handle;
    }

    /// <summary>Resets the bound buffer of the specified target, if the given handle is currently bound to it.</summary>
    void reset(BufferTarget target, Handle handle)
    {
        if (bound_buffers_[target] != handle)
            return;
        Wrapper::bind(target, {});
        bound_buffers_[target] = {};
    }

private:
    dutils::EnumArray<BufferTarget, Handle> bound_buffers_{};
};

} // namespace dang::gl
