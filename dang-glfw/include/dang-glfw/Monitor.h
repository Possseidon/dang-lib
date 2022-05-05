#pragma once

#include "dang-glfw/global.h"
#include "dang-math/bounds.h"
#include "dang-math/vector.h"

namespace dang::glfw {

using GammaRamp = GLFWgammaramp;
using VideoMode = GLFWvidmode;

/// @brief Wraps a GLFW monitor handle.
class Monitor {
public:
    /// @brief Initializes the handle with a nullptr.
    Monitor() = default;
    /// @brief Initializes the handle with the given monitor pointer.
    Monitor(GLFWmonitor* monitor);

    /// @brief Returns the wrapped handle pointer.
    GLFWmonitor* handle() const;
    /// @brief Allows for implicit conversion to the handle pointer.
    operator GLFWmonitor*() const;

    /// @brief Returns a human-readable name for the monitor.
    // TODO: std::u8string
    std::string name() const;
    /// @brief Returns the physical size of the monitor display in millimeters, if possible.
    dmath::ivec2 physicalSize() const;
    /// @brief Returns the current DPI scaling of the monitor.
    dmath::vec2 contentScale() const;
    /// @brief Returns the relative position of the monitor on the virtual screen.
    dmath::ivec2 pos() const;
    /// @brief Returns the area of the screen, not occluded by the system taskbar.
    dmath::ibounds2 workarea() const;

    /// @brief Generates an appropriate gamma ramp and sets it for the monitor.
    void setGamma(float gamma) const;
    /// @brief Sets the monitors gamma ramp, which is reset automatically when the program exits.
    /// @remark On windows the gamma ramp must contain exactly 256 values.
    void setGammaRamp(const GammaRamp& gamma_ramp) const;
    /// @brief Returns the current gamma ramp of the monitor or throws an error on failure.
    const GammaRamp& gammaRamp() const;

    /// @brief Returns the current video mode of the monitor, which depends on whether a fullscreen window is present or
    /// throws an error on failure.
    const VideoMode& videoMode() const;
    /// @brief Returns a list of all by the monitor supported video modes or an empty list in case of errors.
    std::vector<VideoMode> videoModes() const;

private:
    GLFWmonitor* handle_ = nullptr;
};

} // namespace dang::glfw
