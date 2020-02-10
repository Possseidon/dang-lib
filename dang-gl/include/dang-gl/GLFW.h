#pragma once

namespace dang::gl
{

class Window;

class GLFW {
public:
    static GLFW Instance;

    bool hasActiveWindow();
    Window& activeWindow();
    void setActiveWindow(Window* window);

private:
    GLFW();
    ~GLFW();

    static std::string formatError(int error_code, const char* description);
    static void errorCallback(int error_code, const char* description);

    bool glad_initialized_ = false;
    Window* active_window_ = nullptr;
};

}
