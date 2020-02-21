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
    glfwSetWindowUserPointer(handle, this);
    registerCallbacks();
}

Window::Window(const WindowInfo& info)
    : Window(info.createWindow())
{
}

Window::~Window()
{
    glfwDestroyWindow(handle_);
}

Window& Window::fromUserPointer(GLFWwindow* window)
{
    return *static_cast<Window*>(glfwGetWindowUserPointer(window));
}

GLFWwindow* Window::handle()
{
    return handle_;
}

const dmath::ivec2& Window::framebufferSize()
{
    return framebufferSize_;
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

void Window::registerCallbacks()
{
    glfwSetCharCallback(handle_, charCallback);
    glfwSetCharModsCallback(handle_, charModsCallback);
    glfwSetCursorEnterCallback(handle_, cursorEnterCallback);
    glfwSetCursorPosCallback(handle_, cursorPosCallback);
    glfwSetDropCallback(handle_, dropCallback);
    glfwSetFramebufferSizeCallback(handle_, framebufferSizeCallback);
    glfwSetKeyCallback(handle_, keyCallback);
    glfwSetMouseButtonCallback(handle_, mouseButtonCallback);
    glfwSetScrollCallback(handle_, scrollCallback);
    glfwSetWindowCloseCallback(handle_, windowCloseCallback);
    glfwSetWindowContentScaleCallback(handle_, windowContentScaleCallback);
    glfwSetWindowFocusCallback(handle_, windowFocusCallback);
    glfwSetWindowIconifyCallback(handle_, windowIconifyCallback);
    glfwSetWindowMaximizeCallback(handle_, windowMaximizeCallback);
    glfwSetWindowPosCallback(handle_, windowPosCallback);
    glfwSetWindowRefreshCallback(handle_, windowRefreshCallback);
    glfwSetWindowSizeCallback(handle_, windowSizeCallback);
}

void Window::charCallback(GLFWwindow* window_handle, unsigned int codepoint)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)codepoint;
}

void Window::charModsCallback(GLFWwindow* window_handle, unsigned int codepoint, int mods)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)codepoint;
    (void)mods;
}

void Window::cursorEnterCallback(GLFWwindow* window_handle, int entered)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)entered;
}

void Window::cursorPosCallback(GLFWwindow* window_handle, double xpos, double ypos)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)xpos;
    (void)ypos;
}

void Window::dropCallback(GLFWwindow* window_handle, int path_count, const char* paths[])
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)path_count;
    (void)paths;
}

void Window::framebufferSizeCallback(GLFWwindow* window_handle, int width, int height)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.framebufferSize_.x() = width;
    window.framebufferSize_.y() = height;
    window.onFramebufferResize(window);
}

void Window::keyCallback(GLFWwindow* window_handle, int key, int scancode, int action, int mods)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;
}

void Window::mouseButtonCallback(GLFWwindow* window_handle, int button, int action, int mods)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)button;
    (void)action;
    (void)mods;
}

void Window::scrollCallback(GLFWwindow* window_handle, double xoffset, double yoffset)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)xoffset;
    (void)yoffset;
}

void Window::windowCloseCallback(GLFWwindow* window_handle)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
}

void Window::windowContentScaleCallback(GLFWwindow* window_handle, float xscale, float yscale)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)xscale;
    (void)yscale;
}

void Window::windowFocusCallback(GLFWwindow* window_handle, int focused)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)focused;
}

void Window::windowIconifyCallback(GLFWwindow* window_handle, int iconified)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)iconified;
}

void Window::windowMaximizeCallback(GLFWwindow* window_handle, int maximized)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)maximized;
}

void Window::windowPosCallback(GLFWwindow* window_handle, int xpos, int ypos)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)xpos;
    (void)ypos;
}

void Window::windowRefreshCallback(GLFWwindow* window_handle)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.render();
}

void Window::windowSizeCallback(GLFWwindow* window_handle, int width, int height)
{
    // TODO: Event
    Window& window = Window::fromUserPointer(window_handle);
    (void)window;
    (void)width;
    (void)height;
}

}
