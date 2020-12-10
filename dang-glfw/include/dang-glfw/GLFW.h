#pragma once

#include "dang-glfw/Monitor.h"

#include "dang-utils/event.h"

namespace dang::glfw {

class Window;

/// <summary>Any error caused by GLFW.</summary>
class GLFWError : public std::runtime_error {
    using runtime_error::runtime_error;
};

/// <summary>A singleton, managing the global GLFW state.</summary>
class GLFW {
public:
    using MonitorEvent = dutils::Event<Monitor>;

    /// <summary>The single GLFW singleton instance.</summary>
    static GLFW Instance;

    /// <summary>Whether there is an active window/context.</summary>
    bool hasActiveWindow();
    /// <summary>Returns the active window/context.</summary>
    Window& activeWindow();
    /// <summary>Sets the active window/context.</summary>
    void setActiveWindow(Window* window);

    /// <summary>The current runtime since the program has been started.</summary>
    double time() const;
    /// <summary>Sets the current time to a new value, from which it will count now.</summary>
    void setTime(double new_time) const;
    /// <summary>The current runtime since the program has been started in frequency-ticks.</summary>
    uint64_t timerValue() const;
    /// <summary>Returns, how many frequency-ticks are in a single second.</summary>
    uint64_t timerFrequency() const;

    /// <summary>Returns the content of the clipboard and throws, if it could not be queried.</summary>
    std::string clipboardOrThrow() const;
    /// <summary>Returns the content of the clipboard or an empty string, if it could not be queried.</summary>
    std::string clipboardOrEmpty() const;
    /// <summary>Returns the content of the clipboard or std::nullopt, if it could not be queried</summary>
    std::optional<std::string> clipboard() const;
    /// <summary>Sets the clipboard to the given value.</summary>
    void setClipboard(const std::string& content);

    /// <summary>Returns a wrapper to the current primary monitor.</summary>
    Monitor primaryMonitor() const;
    /// <summary>Returns a list of wrappers for every connected monitor.</summary>
    const std::vector<Monitor>& monitors() const;

    /// <summary>Triggered, when a new monitor has been connected.</summary>
    MonitorEvent onConnectMonitor;
    /// <summary>Triggered, when an existing monitor is disconnected.</summary>
    MonitorEvent onDisconnectMonitor;
    /// <summary>Triggered, when the primary monitor changes.</summary>
    MonitorEvent onPrimaryMonitorChange;

private:
    /// <summary>Initializes GLFW and registers all callbacks.</summary>
    GLFW();
    /// <summary>Terminates GLFW.</summary>
    ~GLFW();

    /// <summary>Initializes glad, if it is not initialized yet, and can only be called, once a window/context has been created.</summary>
    void initializeGlad();
    /// <summary>Initializes the wrappers for both monitor-list and primary monitor.</summary>
    void initializeMonitors();

    /// <summary>Formats an error message with description and error code.</summary>
    static std::string formatError(int error_code, const char* description);

    /// <summary>An error callback, which dumps the error message to std::cerr and calls std::exit with EXIT_FAILURE.</summary>
    /// <remarks>Used, before GLFW is fully initialized, as exceptions cannot be caught at that point in time.</remarks>
    static void exitingErrorCallback(int error_code, const char* description);
    /// <summary>An error callback, which throws a GLFWError exception.</summary>
    static void throwingErrorCallback(int error_code, const char* description);
    /// <summary>Handles joystick events.</summary>
    static void joystickCallback(int jid, int event);
    /// <summary>Handles monitor connect and disconnect events and checks for changes of the primary monitor.</summary>
    static void monitorCallback(GLFWmonitor* monitor, int event);

    bool glad_initialized_ = false;
    Window* active_window_ = nullptr;
    Monitor primary_monitor_ = nullptr;
    std::vector<Monitor> monitors_;
};

} // namespace dang::glfw
