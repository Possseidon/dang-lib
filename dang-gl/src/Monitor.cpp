#include "pch.h"
#include "Monitor.h"

namespace dang::gl
{

Monitor::Monitor(GLFWmonitor* monitor)
    : handle_(monitor)
{
}

GLFWmonitor* Monitor::handle() const
{
    return handle_;
}

Monitor::operator GLFWmonitor* () const
{
    return handle_;
}

std::string Monitor::name() const
{
    return glfwGetMonitorName(handle_);
}

dmath::ivec2 Monitor::physicalSize() const
{
    dmath::ivec2 result;
    glfwGetMonitorPhysicalSize(handle_, &result.x(), &result.y());
    return result;
}

dmath::vec2 Monitor::contentScale() const
{
    dmath::vec2 result;
    glfwGetMonitorContentScale(handle_, &result.x(), &result.y());
    return result;
}

dmath::ivec2 Monitor::pos() const
{
    dmath::ivec2 result;
    glfwGetMonitorPos(handle_, &result.x(), &result.y());
    return result;
}

dmath::ibounds2 Monitor::workarea() const
{
    dmath::ibounds2 result;
    glfwGetMonitorWorkarea(handle_, &result.low.x(), &result.low.y(), &result.high.x(), &result.high.y());
    result.high += result.low;
    return result;
}

void Monitor::setGamma(float gamma) const
{
    glfwSetGamma(handle_, gamma);
}

void Monitor::setGammaRamp(const GammaRamp& gamma_ramp) const
{
    glfwSetGammaRamp(handle_, &gamma_ramp);
}

const GammaRamp& Monitor::gammaRamp() const
{
    return *glfwGetGammaRamp(handle_);
}

const VideoMode& Monitor::videoMode() const
{
    return *glfwGetVideoMode(handle_);
}

std::vector<VideoMode> Monitor::videoModes() const
{
    int count;
    const VideoMode* first = glfwGetVideoModes(handle_, &count);
    // on error, first is nullptr and count is zero, resulting in an empty vector
    return std::vector<VideoMode>(first, first + count);
}

}
