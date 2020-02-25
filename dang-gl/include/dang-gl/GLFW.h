#pragma once

#include "dang-utils/event.h"

#include "Monitor.h"

namespace dang::gl
{

class Window;

class GLFWError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class GLFW {
public:
    using MonitorEvent = dutils::Event<Monitor>;

    static GLFW Instance;

    bool hasActiveWindow();
    Window& activeWindow();
    void setActiveWindow(Window* window);

    std::string clipboardOrThrow() const;
    std::string clipboardOrEmpty() const;
    std::optional<std::string> clipboard() const;
    void setClipboard(const std::string& content);

    Monitor primaryMonitor() const;
    const std::vector<Monitor>& monitors() const;

    MonitorEvent onConnectMonitor;
    MonitorEvent onDisconnectMonitor;
    MonitorEvent onPrimaryMonitorChange;

private:
    GLFW();
    ~GLFW();

    void initializeGlad();
    void initializeMonitors();

    static std::string formatError(int error_code, const char* description);

    static void exitingErrorCallback(int error_code, const char* description);
    static void throwingErrorCallback(int error_code, const char* description);
    static void joystickCallback(int jid, int event);
    static void monitorCallback(GLFWmonitor* monitor, int event);

    bool glad_initialized_ = false;
    Window* active_window_ = nullptr;
    Monitor primary_monitor_ = nullptr;
    std::vector<Monitor> monitors_;
};

}
