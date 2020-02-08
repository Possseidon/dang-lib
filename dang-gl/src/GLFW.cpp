#include "pch.h"

#include "GLFW.h"

#include <sstream>
#include <iostream>

namespace dang::gl
{

GLFW GLFW::Instance;

void GLFW::setContext(GLFWwindow* window)
{
    if (window == active_window_)
        return;

    active_window_ = window;
    glfwMakeContextCurrent(window);

    if (glad_initialized_ || !window)
        return;

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize OpenGL." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    glad_initialized_ = true;
}

GLFW::GLFW()
{
    glfwSetErrorCallback(errorCallback);
    glfwInit();
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

GLuint ObjectBase::handle()
{
    return handle_;
}

ObjectBase::ObjectBase(GLuint handle, Binding& binding)
    : handle_(handle)
    , binding_(binding)
{
}

}
