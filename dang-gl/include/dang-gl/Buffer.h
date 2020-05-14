#pragma once

#include "dang-utils/enum.h"

#include "Object.h"
#include "ObjectContext.h"
#include "ObjectType.h"

namespace dang::gl
{

// TODO: Move a lot of VBO functionality in here
// TODO: Lock mapped buffers again

/// <summary>Specializes the context class for buffer objects.</summary>
template <>
class ObjectContext<ObjectType::Buffer> : public ObjectContextBase {
public:
    using ObjectContextBase::ObjectContextBase;

    /// <summary>Binds the given buffer handle to the specified target, if it isn't bound already.</summary>
    void bind(BufferTarget target, GLuint handle)
    {
        if (bound_buffers_[target] == handle)
            return;
        ObjectWrapper<ObjectType::Buffer>::bind(target, handle);
        bound_buffers_[target] = handle;
    }

    /// <summary>Resets the bound buffer of the specified target, if the given handle is currently bound to it.</summary>
    void reset(BufferTarget target, GLuint handle)
    {
        if (bound_buffers_[target] == handle)
            bound_buffers_[target] = 0;
    }

private:
    dutils::EnumArray<BufferTarget, GLuint> bound_buffers_{};
};

/// <summary>An OpenGL buffer for a template specified target.</summary>
template <BufferTarget Target>
class BufferBase : public Object<ObjectType::Buffer> {
public:
    using Object::Object;

    /// <summary>Resets the bound buffer of the context, in case of the buffer still being bound.</summary>
    ~BufferBase()
    {
        context().reset(Target, handle());
    }

    /// <summary>Binds the buffer to the correct target.</summary>
    void bind() const
    {
        context().bind(Target, handle());
    }
};

}
