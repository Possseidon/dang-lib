#pragma once

#include "dang-gl/Objects/BufferContext.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

#include "dang-utils/enum.h"

namespace dang::gl {

// TODO: Move a lot of VBO functionality in here
// TODO: Lock mapped buffers again

/// @brief An OpenGL buffer for a template specified target.
template <BufferTarget v_target>
class BufferBase : public Object<ObjectType::Buffer> {
public:
    /// @brief Resets the bound buffer of the context, in case of the buffer still being bound.
    ~BufferBase()
    {
        if (*this)
            objectContext().reset(v_target, handle());
    }

    BufferBase(const BufferBase&) = delete;
    BufferBase& operator=(const BufferBase&) = delete;

    /// @brief Binds the buffer to the correct target.
    void bind() const { objectContext().bind(v_target, handle()); }

protected:
    BufferBase() = default;

    BufferBase(BufferBase&&) = default;
    BufferBase& operator=(BufferBase&&) = default;
};

} // namespace dang::gl
