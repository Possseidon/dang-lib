#pragma once

#include "dang-math/vector.h"
#include "dang-utils/enum.h"
#include "dang-utils/event.h"

#include "Binding.h"
#include "BindingPoint.h"
#include "Input.h"

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

    struct EventInfoBase {
        Window& window;
    };

    struct CursorMoveInfo : EventInfoBase {
        dmath::dvec2 position;
    };

    struct ScrollInfo : EventInfoBase {
        dmath::dvec2 offset;
    };

    struct DropPathsInfo : EventInfoBase {
        std::vector<fs::path> paths;
    };

    struct ButtonInfo : EventInfoBase {
        ButtonAction action;
        Button button;
        ModifierKeys mods;
    };

    struct KeyInfo : EventInfoBase {
        KeyAction action;
        KeyData key;
        ModifierKeys mods;
    };

    using Event = dutils::Event<Window&>;
    using CursorMoveEvent = dutils::Event<CursorMoveInfo>;
    using ScrollEvent = dutils::Event<ScrollInfo>;
    using DropPathsEvent = dutils::Event<DropPathsInfo>;
    using KeyEvent = dutils::Event<KeyInfo>;
    using ButtonEvent = dutils::Event<ButtonInfo>;

    Window(const WindowInfo& info = WindowInfo());
    ~Window();

    static Window& fromUserPointer(GLFWwindow* window);

    GLFWwindow* handle() const;

    const std::string& title() const;
    void setTitle(const std::string& title);

    dmath::ivec2 pos() const;
    void move(const dmath::ivec2& new_pos) const;

    dmath::ivec2 size() const;
    void resize(const dmath::ivec2& new_size) const;

    dmath::vec2 contentScale() const;

    const dmath::ivec2& framebufferSize() const;
    float framebufferAspect() const;

    void adjustViewport() const;
    bool autoAdjustViewport() const;
    void setAutoAdjustViewport(bool auto_adjust_viewport);

    std::string_view textInput() const;

    template <class TInfo>
    typename TInfo::Binding& binding();

    void activate();

    void update();
    void render();
    void pollEvents();

    void step();
    void run();

    bool shouldClose() const;

    Event onUpdate;
    Event onRender;

    Event onClose;
    Event onContentScale;
    Event onFocus;
    Event onUnfocus;
    Event onIconify;
    Event onUniconify;
    Event onMaximize;
    Event onUnmaximize;
    Event onRestore;
    Event onMove;
    Event onResize;
    Event onFramebufferResize;

    Event onType;
    KeyEvent onKey;
    Event onCursorEnter;
    Event onCursorLeave;
    CursorMoveEvent onCursorMove;
    DropPathsEvent onDropPaths;
    ButtonEvent onButton;
    ScrollEvent onScroll;

private:
    void registerCallbacks();

    static void charCallback(GLFWwindow* window_handle, unsigned int codepoint);
    static void cursorEnterCallback(GLFWwindow* window_handle, int entered);
    static void cursorPosCallback(GLFWwindow* window_handle, double xpos, double ypos);
    static void dropCallback(GLFWwindow* window_handle, int path_count, const char* path_array[]);
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

    GLFWwindow* handle_ = nullptr;
    std::string title_;
    dmath::ivec2 framebuffer_size_;
    bool auto_adjust_viewport_ = true;
    std::string text_input_;
    dutils::EnumArray<BindingPoint, std::unique_ptr<Binding>> bindings_;
};

template<class TInfo>
inline typename TInfo::Binding& Window::binding()
{
    if (const auto& binding = bindings_[TInfo::BindingPoint])
        return static_cast<typename TInfo::Binding&>(*binding);
    return static_cast<typename TInfo::Binding&>(*(bindings_[TInfo::BindingPoint] = std::make_unique<TInfo::Binding>()));
}

}
