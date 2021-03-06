#pragma once

#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ObjectWrapper.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief Specializes the context class for buffer objects.
template <>
class ObjectContext<ObjectType::Buffer> : public ObjectContextBase {
public:
    using Handle = ObjectHandle<ObjectType::Buffer>;
    using Wrapper = ObjectWrapper<ObjectType::Buffer>;

    using ObjectContextBase::ObjectContextBase;

    /// @brief Binds the given buffer handle to the specified target, if it isn't bound already.
    void bind(BufferTarget target, Handle handle)
    {
        if (bound_buffers_[target] == handle)
            return;
        Wrapper::bind(target, handle);
        bound_buffers_[target] = handle;
    }

    /// @brief Resets the bound buffer of the specified target, if the given handle is currently bound to it.
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
