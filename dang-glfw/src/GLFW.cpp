#include "GLFW.h"

#include "dang-gl/Context/Context.h"

#include "Window.h"

namespace dang::glfw {

GLFW GLFW::instance;

void GLFW::setActiveWindow(Window* window)
{
    if (active_window_ == window)
        return;

    active_window_ = window;
    glfwMakeContextCurrent(window ? window->handle() : nullptr);
    dgl::Context::current = &window->context();

    if (!glad_initialized_ && window)
        initializeGlad();
}

double GLFW::time() const { return glfwGetTime(); }

void GLFW::setTime(double new_time) const { glfwSetTime(new_time); }

uint64_t GLFW::timerValue() const { return glfwGetTimerValue(); }

uint64_t GLFW::timerFrequency() const { return glfwGetTimerFrequency(); }

std::string GLFW::clipboardOrThrow() const
{
    const char* content = glfwGetClipboardString(nullptr);
    // technically throws when null is returned
    // check for null just in case
    return content ? content : std::string();
}

std::string GLFW::clipboardOrEmpty() const { return clipboard().value_or(std::string()); }

std::optional<std::string> GLFW::clipboard() const
{
    try {
        return clipboardOrThrow();
    }
    catch (GLFWError) {
        return std::nullopt;
    }
}

void GLFW::setClipboard(const std::string& content) { glfwSetClipboardString(nullptr, content.c_str()); }

Monitor GLFW::primaryMonitor() const { return primary_monitor_; }

const std::vector<Monitor>& GLFW::monitors() const { return monitors_; }

bool GLFW::hasActiveWindow() { return active_window_ != nullptr; }

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

GLFW::~GLFW() { glfwTerminate(); }

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
    // TODO: Add event for GLFW::joystickCallback
    (void)jid;
    (void)event;
}

void GLFW::monitorCallback(GLFWmonitor* monitor, int event)
{
    if (event == GLFW_CONNECTED) {
        instance.monitors_.emplace_back(monitor);
        instance.onConnectMonitor(monitor);
    }
    else if (event == GLFW_DISCONNECTED) {
        instance.onDisconnectMonitor(monitor);
        auto pos = std::find(instance.monitors_.begin(), instance.monitors_.end(), monitor);
        if (pos != instance.monitors_.end())
            instance.monitors_.erase(pos);
    }

    if (instance.primary_monitor_ != monitor) {
        instance.primary_monitor_ = monitor;
        instance.onPrimaryMonitorChange(monitor);
    }
}

} // namespace dang::glfw
