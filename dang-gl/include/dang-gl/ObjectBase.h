#pragma once

namespace dang::gl
{

class Window;

/// <summary>The ultimate base class for all handle-based GL-Objects.</summary>
class ObjectBase {
public:
    /// <summary>Move-constructs the object and sets the handle of other to zero.</summary>
    ObjectBase(ObjectBase&& other) noexcept;

    /// <summary>Returns the handle of the GL-Object.</summary>
    GLuint handle() const;
    /// <summary>Returns the associated GL-Context in form of a window.</summary>
    Window& window() const;

protected:
    /// <summary>Initializes the GL-Object with the given handle and window.</summary>
    ObjectBase(GLuint handle, Window& window);

private:
    GLuint handle_;
    Window& window_;
};

}
