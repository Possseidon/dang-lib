#pragma once

#include <string>

namespace dang::gl
{

class GLFW {
public:
    static GLFW Instance;

    void setContext(GLFWwindow* window);

private:
    GLFW();
    ~GLFW();

    static std::string formatError(int error_code, const char* description);
    static void errorCallback(int error_code, const char* description);

    bool glad_initialized_ = false;
    GLFWwindow* active_window_ = nullptr;
};

}
