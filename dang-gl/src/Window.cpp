#include "pch.h"
#include "Window.h"

#include "GLFW.h"

namespace dang::gl
{

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
    /* TODO:
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    */
    return glfwCreateWindow(width(), height(), title_.c_str(), nullptr, nullptr);
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

void Window::activate()
{
    dgl::GLFW::Instance.setActiveWindow(this);
}

void Window::update()
{
    activate();
    glfwPollEvents();
    onUpdate(*this);
}

void Window::render()
{
    activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    onRender(*this);
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
