#include "pch.h"
#include "GLFW.h"

#include "Window.h"

namespace dang::gl
{

GLFW GLFW::Instance;

void GLFW::setActiveWindow(Window* window)
{
    if (window == active_window_)
        return;

    active_window_ = window;
    glfwMakeContextCurrent(window ? window->handle() : nullptr);

    if (glad_initialized_ || !window)
        return;

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize OpenGL." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    glad_initialized_ = true;
}

bool GLFW::hasActiveWindow()
{
    return active_window_ != nullptr;
}

Window& GLFW::activeWindow()
{
    assert(hasActiveWindow());
    return *active_window_;
}

GLFW::GLFW()
{
    glfwSetErrorCallback(errorCallback);
    glfwInit();
    glfwSetJoystickCallback(joystickCallback);
    glfwSetMonitorCallback(monitorCallback);
}

GLFW::~GLFW()
{
    glfwTerminate();
}

std::string GLFW::formatError(int error_code, const char* description)
{
    std::stringstream ss;
    ss << description << "[0x" << std::hex << error_code << "]";
    return ss.str();
}

void GLFW::errorCallback(int error_code, const char* description)
{
    std::cerr << formatError(error_code, description) << std::endl;
    std::exit(EXIT_FAILURE);
}

void GLFW::joystickCallback(int jid, int event)
{
    // TODO: Event
    (void)jid;
    (void)event;
}

void GLFW::monitorCallback(GLFWmonitor* monitor, int event)
{
    // TODO: Event
    (void)monitor;
    (void)event;
}

}
