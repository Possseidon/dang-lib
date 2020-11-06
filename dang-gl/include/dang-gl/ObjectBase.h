#pragma once

namespace dang::gl
{

class Window;

/// <summary>The ultimate base class for all handle-based GL-Objects.</summary>
class ObjectBase {
public:
    using Handle = GLuint;

    static constexpr Handle InvalidHandle = 0;

    ObjectBase(const ObjectBase&) = delete;
    ObjectBase& operator=(const ObjectBase&) = delete;

    /// <summary>Returns the handle of the GL-Object or InvalidHandle for default constructed objects.</summary>
    Handle handle() const noexcept;
    /// <summary>For valid objects, returns the associated GL-Context in form of a window.</summary>
    Window& window() const noexcept;

    /// <summary>Whether the object is valid.</summary>
    operator bool() const noexcept;

protected:
    ObjectBase() = default;

    /// <summary>Initializes the GL-Object with the given handle and window.</summary>
    ObjectBase(Handle handle, Window& window) noexcept;

    ObjectBase(ObjectBase&& other) noexcept;
    ObjectBase& operator=(ObjectBase&& other) noexcept;

    void swap(ObjectBase& other) noexcept;

private:
    Handle handle_ = InvalidHandle;
    Window* window_ = nullptr;
};

}
