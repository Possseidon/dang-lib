#pragma once

#include "dang-glfw/Monitor.h"
#include "dang-glfw/global.h"

#include "dang-utils/event.h"

namespace dang::glfw {

class Window;

/// @brief Any error caused by GLFW.
class GLFWError : public std::runtime_error {
    using runtime_error::runtime_error;
};

/// @brief A singleton, managing the global GLFW state.
class GLFW {
public:
    using MonitorEvent = dutils::Event<Monitor>;

    /// @brief The single GLFW singleton instance.
    static GLFW instance;

    /// @brief Whether there is an active window/context.
    bool hasActiveWindow();
    /// @brief Returns the active window/context.
    Window& activeWindow();
    /// @brief Sets the active window/context.
    void setActiveWindow(Window* window);

    /// @brief The current runtime since the program has been started.
    double time() const;
    /// @brief Sets the current time to a new value, from which it will count now.
    void setTime(double new_time) const;
    /// @brief The current runtime since the program has been started in frequency-ticks.
    uint64_t timerValue() const;
    /// @brief Returns, how many frequency-ticks are in a single second.
    uint64_t timerFrequency() const;

    /// @brief Returns the content of the clipboard and throws, if it could not be queried.
    std::string clipboardOrThrow() const;
    /// @brief Returns the content of the clipboard or an empty string, if it could not be queried.
    std::string clipboardOrEmpty() const;
    /// @brief Returns the content of the clipboard or std::nullopt, if it could not be queried
    std::optional<std::string> clipboard() const;
    /// @brief Sets the clipboard to the given value.
    void setClipboard(const std::string& content);

    /// @brief Returns a wrapper to the current primary monitor.
    Monitor primaryMonitor() const;
    /// @brief Returns a list of wrappers for every connected monitor.
    const std::vector<Monitor>& monitors() const;

    /// @brief Triggered, when a new monitor has been connected.
    MonitorEvent onConnectMonitor;
    /// @brief Triggered, when an existing monitor is disconnected.
    MonitorEvent onDisconnectMonitor;
    /// @brief Triggered, when the primary monitor changes.
    MonitorEvent onPrimaryMonitorChange;

private:
    /// @brief Initializes GLFW and registers all callbacks.
    GLFW();
    /// @brief Terminates GLFW.
    ~GLFW();

    /// @brief Initializes glad, if it is not initialized yet, and can only be called, once a window/context has been
    /// created.
    void initializeGlad();
    /// @brief Initializes the wrappers for both monitor-list and primary monitor.
    void initializeMonitors();

    /// @brief Formats an error message with description and error code.
    static std::string formatError(int error_code, const char* description);

    /// @brief An error callback, which dumps the error message to std::cerr and calls std::exit with EXIT_FAILURE.
    /// @remark Used, before GLFW is fully initialized, as exceptions cannot be caught at that point in time.
    static void exitingErrorCallback(int error_code, const char* description);
    /// @brief An error callback, which throws a GLFWError exception.
    static void throwingErrorCallback(int error_code, const char* description);
    /// @brief Handles joystick events.
    static void joystickCallback(int jid, int event);
    /// @brief Handles monitor connect and disconnect events and checks for changes of the primary monitor.
    static void monitorCallback(GLFWmonitor* monitor, int event);

    bool glad_initialized_ = false;
    Window* active_window_ = nullptr;
    Monitor primary_monitor_ = nullptr;
    std::vector<Monitor> monitors_;
};

} // namespace dang::glfw
