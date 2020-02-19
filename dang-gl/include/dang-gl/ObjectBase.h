#pragma once

namespace dang::gl
{

class Window;

class ObjectBase {
public:
    GLuint handle() const;
    Window& window() const;

protected:
    ObjectBase(GLuint handle, Window& window);

private:
    GLuint handle_;
    Window& window_;
};

}
