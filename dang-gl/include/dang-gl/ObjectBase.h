#pragma once

namespace dang::gl
{

class Window;

class ObjectBase {
public:
    ObjectBase(const ObjectBase&) = delete;
    ObjectBase(ObjectBase&& other) noexcept;

    ObjectBase& operator=(const ObjectBase&) = delete;
    ObjectBase& operator=(ObjectBase&&) = delete;

    GLuint handle() const;
    Window& window() const;

protected:
    ObjectBase(GLuint handle, Window& window);

private:
    GLuint handle_;
    Window& window_;
};

}
