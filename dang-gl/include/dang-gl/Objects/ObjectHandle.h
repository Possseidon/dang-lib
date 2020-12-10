#pragma once

#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/global.h"

namespace dang::gl {

template <ObjectType Type>
class ObjectHandle {
public:
    ObjectHandle() = default;

    explicit ObjectHandle(GLuint handle) noexcept
        : handle_(handle)
    {}

    GLuint unwrap() const noexcept { return handle_; }

    friend bool operator==(ObjectHandle lhs, ObjectHandle rhs) noexcept { return lhs.handle_ == rhs.handle_; }

    friend bool operator!=(ObjectHandle lhs, ObjectHandle rhs) noexcept { return !(lhs == rhs); }

    explicit operator bool() const noexcept { return *this != ObjectHandle{}; }

    void swap(ObjectHandle& other) noexcept
    {
        using std::swap;
        swap(handle_, other.handle_);
    }

    friend void swap(ObjectHandle& lhs, ObjectHandle& rhs) noexcept { lhs.swap(rhs); }

private:
    GLuint handle_ = 0;
};

} // namespace dang::gl
