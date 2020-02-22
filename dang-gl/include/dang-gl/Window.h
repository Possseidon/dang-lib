#pragma once

#include "dang-math/vector.h"
#include "dang-utils/enum.h"
#include "dang-utils/event.h"

#include "Binding.h"
#include "BindingPoint.h"

namespace dang::gl
{

class WindowInfo {
public:
    WindowInfo();

    dmath::ivec2 size() const;
    void setSize(dmath::ivec2 size);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    const std::string& title() const;
    void setTitle(std::string title);

    GLFWwindow* createWindow() const;

private:
    dmath::ivec2 size_;
    std::string title_;
};

class Window {
public:
    Window(const WindowInfo& info = WindowInfo());
    ~Window();

    static Window& fromUserPointer(GLFWwindow* window);

    GLFWwindow* handle() const;

    const std::string& title() const;
    void setTitle(const std::string& title);

    template <class TInfo>
    typename TInfo::Binding& binding();

    const dmath::ivec2& framebufferSize() const;

    const std::string& textInput() const;

    bool shouldClose() const;

    void activate();

    void update();
    void render();
    void pollEvents();

    void step();
    void run();

    dutils::Event<Window&> onUpdate;
    dutils::Event<Window&> onRender;
    dutils::Event<Window&> onFramebufferResize;

private:
    void registerCallbacks();

    static void charCallback(GLFWwindow* window_handle, unsigned int codepoint);
    static void charModsCallback(GLFWwindow* window_handle, unsigned int codepoint, int mods);
    static void cursorEnterCallback(GLFWwindow* window_handle, int entered);
    static void cursorPosCallback(GLFWwindow* window_handle, double xpos, double ypos);
    static void dropCallback(GLFWwindow* window_handle, int path_count, const char* paths[]);
    static void framebufferSizeCallback(GLFWwindow* window_handle, int width, int height);
    static void keyCallback(GLFWwindow* window_handle, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window_handle, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window_handle, double xoffset, double yoffset);
    static void windowCloseCallback(GLFWwindow* window_handle);
    static void windowContentScaleCallback(GLFWwindow* window_handle, float xscale, float yscale);
    static void windowFocusCallback(GLFWwindow* window_handle, int focused);
    static void windowIconifyCallback(GLFWwindow* window_handle, int iconified);
    static void windowMaximizeCallback(GLFWwindow* window_handle, int maximized);
    static void windowPosCallback(GLFWwindow* window_handle, int xpos, int ypos);
    static void windowRefreshCallback(GLFWwindow* window_handle);
    static void windowSizeCallback(GLFWwindow* window_handle, int width, int height);

    GLFWwindow* handle_;
    std::string title_;
    dutils::EnumArray<BindingPoint, std::unique_ptr<Binding>> bindings_;
    dmath::ivec2 framebuffer_size_;
    std::string text_input_;
};

template<class TInfo>
inline typename TInfo::Binding& Window::binding()
{
    if (const auto& binding = bindings_[TInfo::BindingPoint])
        return static_cast<typename TInfo::Binding&>(*binding);
    return static_cast<typename TInfo::Binding&>(*(bindings_[TInfo::BindingPoint] = std::make_unique<TInfo::Binding>()));
}

}
