#include "pch.h"

#include "GLFW.h"

#include <exception>
#include <sstream>
#include <iostream>

namespace dang::gl
{

GLFW GLFW::Instance;

void GLFW::setContext(GLFWwindow* window)
{
    if (window != window_) {
        window_ = window;
        glfwMakeContextCurrent(window);

        if (!gladInitialized && window) {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
                std::cerr << "Failed to initialize OpenGL." << std::endl;
                exit(EXIT_FAILURE);
            }
            gladInitialized = true;
        }
    }
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
    exit(EXIT_FAILURE);
}

}
