#include "pch.h"
#include "Monitor.h"

namespace dang::gl
{

GLFWmonitor* Monitor::handle() const
{
    return handle_;
}

Monitor::operator GLFWmonitor* () const
{
    return handle_;
}

Monitor::Monitor(GLFWmonitor* monitor)
    : handle_(monitor)
{
}

}
