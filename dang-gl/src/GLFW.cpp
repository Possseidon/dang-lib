#include "pch.h"

#include "GLFW.h"

#include <sstream>
#include <iostream>

namespace dang::gl
{

GLFW GLFW::Instance;

void GLFW::setActiveWindow(Window* window)
{
    if (window == active_window_)
        return;

    active_window_ = window;
    glfwMakeContextCurrent(window ? window->handle() : nullptr);

    if (glad_initialized_ || !window)
        return;

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize OpenGL." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    glad_initialized_ = true;
}

bool GLFW::hasActiveWindow()
{
    return active_window_ != nullptr;
}

Window& GLFW::activeWindow()
{
    assert(hasActiveWindow());
    return *active_window_;
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

Window& ObjectBase::window()
{
    return window_;
}

ObjectBase::ObjectBase(GLuint handle, Window& window)
    : handle_(handle)
    , window_(window)
{
}

WindowInfo::WindowInfo()
{
}

dmath::ivec2 WindowInfo::size() const
{
    return size_;
}

void WindowInfo::setSize(dmath::ivec2 size)
{
    size_ = size;
}

int WindowInfo::width() const
{
    return size_.x();
}

void WindowInfo::setWidth(int width)
{
    size_.x() = width;
}

int WindowInfo::height() const
{
    return size_.y();
}

void WindowInfo::setHeight(int height)
{
    size_.y() = height;
}

std::string WindowInfo::title() const
{
    return title_;
}

void WindowInfo::setTitle(std::string title)
{
    title_ = title;
}

GLFWwindow* WindowInfo::createWindow() const
{
    return glfwCreateWindow(
        width(),
        height(),
        title_.c_str(),
        nullptr,
        nullptr);
}

Window::Window(GLFWwindow* handle)
    : handle_(handle)
{
}

Window::Window(const WindowInfo& info)
    : Window(info.createWindow())
{
}

Window::~Window()
{
    glfwDestroyWindow(handle_);
}

GLFWwindow* Window::handle()
{
    return handle_;
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(handle_);
}

void Window::update()
{
    dgl::GLFW::Instance.setActiveWindow(this);
    glfwPollEvents();
}

void Window::render()
{
    dgl::GLFW::Instance.setActiveWindow(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(handle_);
}

void Window::step()
{
    update();
    render();
}

void Window::run()
{
    while (!shouldClose()) {
        update();
        render();
    }
}

}
