#pragma once

#include "dang-math/bounds.h"
#include "dang-math/vector.h"

namespace dang::gl {

using GammaRamp = GLFWgammaramp;
using VideoMode = GLFWvidmode;

/// <summary>Wraps a GLFW monitor handle.</summary>
class Monitor {
public:
    /// <summary>Initializes the handle with a nullptr.</summary>
    Monitor() = default;
    /// <summary>Initializes the handle with the given monitor pointer.</summary>
    Monitor(GLFWmonitor* monitor);

    /// <summary>Returns the wrapped handle pointer.</summary>
    GLFWmonitor* handle() const;
    /// <summary>Allows for implicit conversion to the handle pointer.</summary>
    operator GLFWmonitor*() const;

    /// <summary>Returns a human-readable name for the monitor.</summary>
    // TODO: std::u8string
    std::string name() const;
    /// <summary>Returns the physical size of the monitor display in millimeters, if possible.</summary>
    dmath::ivec2 physicalSize() const;
    /// <summary>Returns the current DPI scaling of the monitor.</summary>
    dmath::vec2 contentScale() const;
    /// <summary>Returns the relative position of the monitor on the virtual screen.</summary>
    dmath::ivec2 pos() const;
    /// <summary>Returns the area of the screen, not occluded by the system taskbar.</summary>
    dmath::ibounds2 workarea() const;

    /// <summary>Generates an appropriate gamma ramp and sets it for the monitor.</summary>
    void setGamma(float gamma) const;
    /// <summary>Sets the monitors gamma ramp, which is reset automatically when the program exits.</summary>
    /// <remarks>On windows the gamma ramp must contain exactly 256 values.</remarks>
    void setGammaRamp(const GammaRamp& gamma_ramp) const;
    /// <summary>Returns the current gamma ramp of the monitor or throws an error on failure.</summary>
    const GammaRamp& gammaRamp() const;

    /// <summary>Returns the current video mode of the monitor, which depends on whether a fullscreen window is present or throws an error on failure.</summary>
    const VideoMode& videoMode() const;
    /// <summary>Returns a list of all by the monitor supported video modes or an empty list in case of errors.</summary>
    std::vector<VideoMode> videoModes() const;

private:
    GLFWmonitor* handle_ = nullptr;
};

} // namespace dang::gl
