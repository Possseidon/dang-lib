#include "pch.h"
#include "Window.h"

#include "GLFW.h"

namespace dang::gl
{

GLFWwindow* WindowInfo::createWindow() const
{
    glfwWindowHint(GLFW_RESIZABLE, resizable);
    glfwWindowHint(GLFW_VISIBLE, visible);
    glfwWindowHint(GLFW_DECORATED, decorated);
    glfwWindowHint(GLFW_FOCUSED, focused);
    glfwWindowHint(GLFW_AUTO_ICONIFY, auto_iconify);
    glfwWindowHint(GLFW_FLOATING, floating);
    glfwWindowHint(GLFW_MAXIMIZED, maximized);
    glfwWindowHint(GLFW_CENTER_CURSOR, center_cursor);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparent_framebuffer);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, focus_on_show);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, scale_to_monitor);

    glfwWindowHint(GLFW_RED_BITS, red_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_GREEN_BITS, green_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_BLUE_BITS, blue_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_ALPHA_BITS, alpha_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_DEPTH_BITS, depth_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_ACCUM_RED_BITS, accum_red_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_ACCUM_GREEN_BITS, accum_green_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_ACCUM_BLUE_BITS, accum_blue_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, accum_alpha_bits.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_AUX_BUFFERS, aux_buffers.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_SAMPLES, samples.value_or(GLFW_DONT_CARE));
    glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate.value_or(GLFW_DONT_CARE));

    glfwWindowHint(GLFW_STEREO, stereo);
    glfwWindowHint(GLFW_SRGB_CAPABLE, srgb_capable);
    glfwWindowHint(GLFW_DOUBLEBUFFER, doublebuffer);

    glfwWindowHint(GLFW_CLIENT_API, static_cast<int>(client_api));
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, static_cast<int>(context_api));
    auto [major, minor] = context_version;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, static_cast<int>(context_robustness));
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, static_cast<int>(context_release_behavior));

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, forward_compatible);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug_context);
    glfwWindowHint(GLFW_OPENGL_PROFILE, static_cast<int>(gl_profile));
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, cocoa_retina_framebuffer);
    glfwWindowHintString(GLFW_COCOA_FRAME_NAME, cocoa_frame_name.c_str());
    glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, cocoa_graphics_switching);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, x11_class_name.c_str());
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, x11_instance_name.c_str());

    // TODO: Monitor
    // TODO: Share
    return glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

Window::Window(const WindowInfo& info)
    : handle_(info.createWindow())
    , title_(info.title)
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

dmath::ivec2 Window::pos() const
{
    dmath::ivec2 result;
    glfwGetWindowPos(handle_, &result.x(), &result.y());
    return result;
}

void Window::move(dmath::ivec2 new_pos) const
{
    glfwSetWindowPos(handle_, new_pos.x(), new_pos.y());
}

dmath::ivec2 Window::size() const
{
    dmath::ivec2 result;
    glfwGetWindowSize(handle_, &result.x(), &result.y());
    return result;
}

void Window::resize(dmath::ivec2 new_size) const
{
    glfwSetWindowSize(handle_, new_size.x(), new_size.y());
}

dmath::vec2 Window::contentScale() const
{
    dmath::vec2 result;
    glfwGetWindowContentScale(handle_, &result.x(), &result.y());
    return result;
}

dmath::ivec2 Window::framebufferSize() const
{
    dmath::ivec2 result;
    glfwGetFramebufferSize(handle_, &result.x(), &result.y());
    return result;
}

float Window::aspect() const
{
    dmath::ivec2 framebuffer_size = framebufferSize();
    return static_cast<float>(framebuffer_size.x()) / framebuffer_size.y();
}

void Window::adjustViewport() const
{
    dmath::ivec2 framebuffer_size = framebufferSize();
    glViewport(0, 0, framebuffer_size.x(), framebuffer_size.y());
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

std::optional<int> Window::minWidth() const
{
    int value = size_limits_.low.x();
    return value != GLFW_DONT_CARE ? std::optional(value) : std::nullopt;
}

std::optional<int> Window::minHeight() const
{
    int value = size_limits_.low.y();
    return value != GLFW_DONT_CARE ? std::optional(value) : std::nullopt;
}

std::optional<int> Window::maxWidth() const
{
    int value = size_limits_.high.x();
    return value != GLFW_DONT_CARE ? std::optional(value) : std::nullopt;
}

std::optional<int> Window::maxHeight() const
{
    int value = size_limits_.high.y();
    return value != GLFW_DONT_CARE ? std::optional(value) : std::nullopt;
}

void Window::setSizeLimits(std::optional<int> min_width, std::optional<int> min_height, std::optional<int> max_width, std::optional<int> max_height)
{
    size_limits_ = {
        { min_width.value_or(GLFW_DONT_CARE), min_height.value_or(GLFW_DONT_CARE) },
        { max_width.value_or(GLFW_DONT_CARE), max_height.value_or(GLFW_DONT_CARE) } };
    updateSizeLimits();
}

void Window::setMinSize(std::optional<int> min_width, std::optional<int> min_height)
{
    size_limits_.low.x() = min_width.value_or(GLFW_DONT_CARE);
    size_limits_.low.y() = min_height.value_or(GLFW_DONT_CARE);
    updateSizeLimits();
}

void Window::setMaxSize(std::optional<int> max_width, std::optional<int> max_height)
{
    size_limits_.high.x() = max_width.value_or(GLFW_DONT_CARE);
    size_limits_.high.y() = max_height.value_or(GLFW_DONT_CARE);
    updateSizeLimits();
}

std::optional<dmath::ivec2> Window::aspectRatio() const
{
    return aspect_ratio_;
}

void Window::setAspectRatio(std::optional<dmath::ivec2> aspect_ratio)
{
    aspect_ratio_ = aspect_ratio;
    if (aspect_ratio)
        glfwSetWindowAspectRatio(handle_, aspect_ratio->x(), aspect_ratio->y());
    else
        glfwSetWindowAspectRatio(handle_, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Window::freezeAspectRatio()
{
    setAspectRatio(framebufferSize());
}

float Window::opacity() const
{
    return glfwGetWindowOpacity(handle_);
}

void Window::setOpacity(float new_opacity) const
{
    glfwSetWindowOpacity(handle_, new_opacity);
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
    if (!text_input_.empty())
        onType(*this);
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

void Window::framebufferSizeCallback(GLFWwindow* window_handle, int, int)
{
    Window& window = Window::fromUserPointer(window_handle);
    if (window.auto_adjust_viewport_)
        window.adjustViewport();
    window.onFramebufferResize(window);
}

void Window::keyCallback(GLFWwindow* window_handle, int key, int scancode, int action, int mods)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onKey({ window, static_cast<KeyAction>(action), KeyData(static_cast<Key>(key), scancode), static_cast<ModifierKeys>(mods) });
}

void Window::mouseButtonCallback(GLFWwindow* window_handle, int button, int action, int mods)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onButton({ window, static_cast<ButtonAction>(action), static_cast<Button>(button), static_cast<ModifierKeys>(mods) });
}

void Window::scrollCallback(GLFWwindow* window_handle, double xoffset, double yoffset)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onScroll({ window, dmath::dvec2(xoffset, yoffset) });
}

void Window::windowCloseCallback(GLFWwindow* window_handle)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onClose(window);
}

void Window::windowContentScaleCallback(GLFWwindow* window_handle, float, float)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onContentScale(window);
}

void Window::windowFocusCallback(GLFWwindow* window_handle, int focused)
{
    Window& window = Window::fromUserPointer(window_handle);
    if (focused)
        window.onFocus(window);
    else
        window.onUnfocus(window);
}

void Window::windowIconifyCallback(GLFWwindow* window_handle, int iconified)
{
    Window& window = Window::fromUserPointer(window_handle);
    if (iconified)
        window.onIconify(window);
    else {
        window.onUniconify(window);
        window.onRestore(window);
    }
}

void Window::windowMaximizeCallback(GLFWwindow* window_handle, int maximized)
{
    Window& window = Window::fromUserPointer(window_handle);
    if (maximized)
        window.onMaximize(window);
    else {
        window.onUnmaximize(window);
        window.onRestore(window);
    }
}

void Window::windowPosCallback(GLFWwindow* window_handle, int, int)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onMove(window);
}

void Window::windowRefreshCallback(GLFWwindow* window_handle)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.render();
}

void Window::windowSizeCallback(GLFWwindow* window_handle, int, int)
{
    Window& window = Window::fromUserPointer(window_handle);
    window.onResize(window);
}

void Window::updateSizeLimits() const
{
    glfwSetWindowSizeLimits(handle_,
        size_limits_.low.x(), size_limits_.low.y(),
        size_limits_.high.x(), size_limits_.high.y());
}

}
