#pragma once

#include "dang-utils/enum.h"

#include "BufferContext.h"
#include "Object.h"
#include "ObjectType.h"

namespace dang::gl {

// TODO: Move a lot of VBO functionality in here
// TODO: Lock mapped buffers again

/// <summary>An OpenGL buffer for a template specified target.</summary>
template <BufferTarget Target>
class BufferBase : public Object<ObjectType::Buffer> {
public:
    /// <summary>Resets the bound buffer of the context, in case of the buffer still being bound.</summary>
    ~BufferBase()
    {
        if (*this)
            objectContext().reset(Target, handle());
    }

    BufferBase(const BufferBase&) = delete;
    BufferBase& operator=(const BufferBase&) = delete;

    /// <summary>Binds the buffer to the correct target.</summary>
    void bind() const { objectContext().bind(Target, handle()); }

protected:
    BufferBase() = default;

    BufferBase(BufferBase&&) = default;
    BufferBase& operator=(BufferBase&&) = default;
};

} // namespace dang::gl
