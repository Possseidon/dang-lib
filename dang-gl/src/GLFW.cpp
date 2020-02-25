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

    if (!glad_initialized_ && window)
        initializeGlad();
}

std::string GLFW::clipboardOrThrow() const
{
    const char* content = glfwGetClipboardString(nullptr);
    // technically throws when null is returned
    // check for null just in case
    return content ? content : std::string();
}

std::string GLFW::clipboardOrEmpty() const
{
    return clipboard().value_or(std::string());
}

std::optional<std::string> GLFW::clipboard() const
{
    try {
        return clipboardOrThrow();
    }
    catch (GLFWError) {
        return std::nullopt;
    }
}

void GLFW::setClipboard(const std::string& content)
{
    glfwSetClipboardString(nullptr, content.c_str());
}

Monitor GLFW::primaryMonitor() const
{
    return primary_monitor_;
}

const std::vector<Monitor>& GLFW::monitors() const
{
    return monitors_;
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
    glfwSetErrorCallback(exitingErrorCallback);
    glfwInit();
    glfwSetErrorCallback(throwingErrorCallback);
    glfwSetJoystickCallback(joystickCallback);
    glfwSetMonitorCallback(monitorCallback);
    initializeMonitors();
}

GLFW::~GLFW()
{
    glfwTerminate();
}

void GLFW::initializeGlad()
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize OpenGL." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    glad_initialized_ = true;
}

void GLFW::initializeMonitors()
{
    int count;
    GLFWmonitor** first = glfwGetMonitors(&count);
    monitors_.assign(first, first + count);
    primary_monitor_ = glfwGetPrimaryMonitor();
}

std::string GLFW::formatError(int error_code, const char* description)
{
    std::stringstream ss;
    ss << description << "[0x" << std::hex << error_code << "]";
    return ss.str();
}

void GLFW::exitingErrorCallback(int error_code, const char* description)
{
    std::cerr << formatError(error_code, description) << std::endl;
    std::exit(EXIT_FAILURE);
}

void GLFW::throwingErrorCallback(int error_code, const char* description)
{
    throw GLFWError(formatError(error_code, description));
}

void GLFW::joystickCallback(int jid, int event)
{
    // TODO: Event
    (void)jid;
    (void)event;
}

void GLFW::monitorCallback(GLFWmonitor* monitor, int event)
{
    if (event == GLFW_CONNECTED) {
        Instance.monitors_.emplace_back(monitor);
        Instance.onConnectMonitor(monitor);
    }
    else if (event == GLFW_DISCONNECTED) {
        Instance.onDisconnectMonitor(monitor);
        auto pos = std::find(Instance.monitors_.begin(), Instance.monitors_.end(), monitor);
        if (pos != Instance.monitors_.end())
            Instance.monitors_.erase(pos);
    }

    if (Instance.primary_monitor_ != monitor) {
        Instance.primary_monitor_ = monitor;
        Instance.onPrimaryMonitorChange(monitor);
    }
}

}
