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

const std::string& WindowInfo::title() const
{
    return title_;
}

void WindowInfo::setTitle(std::string title)
{
    title_ = std::move(title);
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

Window::Window(const WindowInfo& info)
    : handle_(info.createWindow())
    , title_(info.title())
{
    glfwSetWindowUserPointer(handle_, this);
    registerCallbacks();
}

Window::~Window()
{
    glfwDestroyWindow(handle_);
}

Window& Window::fromUserPointer(GLFWwindow* window)
{
    return *static_cast<Window*>(glfwGetWindowUserPointer(window));
}

GLFWwindow* Window::handle() const
{
    return handle_;
}

const std::string& Window::title() const
{
    return title_;
}

void Window::setTitle(const std::string& title)
{
    if (title == title_)
        return;
    glfwSetWindowTitle(handle_, title.c_str());
    title_ = title;
}

const dmath::ivec2& Window::framebufferSize() const
{
    return framebuffer_size_;
}

float Window::framebufferAspect() const
{
    return static_cast<float>(framebuffer_size_.x()) / framebuffer_size_.y();
}

void Window::adjustViewport() const
{
    glViewport(0, 0, framebuffer_size_.x(), framebuffer_size_.y());
}

bool Window::autoAdjustViewport() const
{
    return auto_adjust_viewport_;
}

void Window::setAutoAdjustViewport(bool auto_adjust_viewport)
{
    auto_adjust_viewport_ = auto_adjust_viewport;
    if (auto_adjust_viewport)
        adjustViewport();
}

const std::string& Window::textInput() const
{
    return text_input_;
}

bool Window::shouldClose() const
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
    onUpdate(*this);
}

void Window::render()
{
    activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    onRender(*this);
    glfwSwapBuffers(handle_);
}

void Window::pollEvents()
{
    text_input_.clear();
    glfwPollEvents();
}

void Window::step()
{
    update();
    render();
    pollEvents();
}

void Window::run()
{
    while (!shouldClose())
        step();
}

void Window::registerCallbacks()
{
    glfwSetCharCallback(handle_, charCallback);
    // Deprecated: glfwSetCharModsCallback(handle_, charModsCallback);
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
    Window& window = Window::fromUserPointer(window_handle);
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    window.text_input_ += converter.to_bytes(codepoint);
}

void Window::cursorEnterCallback(GLFWwindow* window_handle, int entered)
{
    Window& window = Window::fromUserPointer(window_handle);
    if (entered)
        window.onCursorEnter(window);
    else
        window.onCursorLeave(window);
}

void Window::cursorPosCallback(GLFWwindow* window_handle, double xpos, double ypos)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onCursorMove({ window, dmath::dvec2(xpos, ypos) });
}

void Window::dropCallback(GLFWwindow* window_handle, int path_count, const char* path_array[])
{
    Window& window = Window::fromUserPointer(window_handle);
    if (!window.onDropPaths)
        return;
    std::vector<fs::path> paths(path_count);
    std::transform(path_array, path_array + path_count, paths.begin(), fs::u8path<const char*, 0>);
    window.onDropPaths({ window, paths });
}

void Window::framebufferSizeCallback(GLFWwindow* window_handle, int width, int height)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.framebuffer_size_.x() = width;
    window.framebuffer_size_.y() = height;
    if (window.auto_adjust_viewport_)
        window.adjustViewport();
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
    window.onScroll({ window, dmath::dvec2(xoffset, yoffset) });
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
