#pragma once

namespace dang::gl
{

class Monitor {
public:
    Monitor(GLFWmonitor* monitor);

    GLFWmonitor* handle() const;
    operator GLFWmonitor* () const;

private:
    GLFWmonitor* handle_;
};

}
