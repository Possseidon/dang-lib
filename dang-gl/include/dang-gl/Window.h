#pragma once

#include "dang-math/vector.h"
#include "dang-math/bounds.h"
#include "dang-utils/enum.h"
#include "dang-utils/event.h"
#include "dang-utils/NonCopyable.h"

#include "Binding.h"
#include "BindingPoint.h"
#include "Input.h"
#include "Monitor.h"

namespace dang::gl
{

using GLVersion = std::tuple<int, int>;
using GLVersionFull = std::tuple<int, int, int>;

enum class ClientAPI {
    None = GLFW_NO_API,
    OpenGL = GLFW_OPENGL_API,
    OpenGLES = GLFW_OPENGL_ES_API
};

enum class ContextAPI {
    Native = GLFW_NATIVE_CONTEXT_API,
    EGL = GLFW_EGL_CONTEXT_API,
    OSMesa = GLFW_OSMESA_CONTEXT_API
};

enum class ContextRobustness {
    None = GLFW_NO_ROBUSTNESS,
    NoResetNotification = GLFW_NO_RESET_NOTIFICATION,
    LoseContextOnReset = GLFW_LOSE_CONTEXT_ON_RESET
};

enum class ContextReleaseBehavior {
    Any = GLFW_ANY_RELEASE_BEHAVIOR,
    Flush = GLFW_RELEASE_BEHAVIOR_FLUSH,
    None = GLFW_RELEASE_BEHAVIOR_NONE
};

enum class GLProfile {
    Any = GLFW_OPENGL_ANY_PROFILE,
    Core = GLFW_OPENGL_CORE_PROFILE,
    Compatibility = GLFW_OPENGL_COMPAT_PROFILE
};

struct WindowInfo {
    GLFWwindow* createWindow() const;

    dmath::ivec2 size;
    int& width = size.x();
    int& height = size.y();
    std::string title;

    Window* share = nullptr;

    bool resizable = true;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
    bool auto_iconify = true;
    bool floating = false;
    bool maximized = false;
    bool center_cursor = false;
    bool transparent_framebuffer = false;
    bool focus_on_show = true;
    bool scale_to_monitor = false;

    struct Framebuffer {
        std::optional<int> red_bits = 8;
        std::optional<int> green_bits = 8;
        std::optional<int> blue_bits = 8;
        std::optional<int> alpha_bits = 8;
        std::optional<int> depth_bits = 24;
        std::optional<int> stencil_bits = 8;
        std::optional<int> accum_red_bits = 0;
        std::optional<int> accum_green_bits = 0;
        std::optional<int> accum_blue_bits = 0;
        std::optional<int> accum_alpha_bits = 0;
        std::optional<int> aux_buffers = 0;
        std::optional<int> samples = 0;

        bool stereo = false;
        bool srgb_capable = false;
        bool doublebuffer = true;
    } framebuffer;

    Monitor monitor;
    std::optional<int> monitor_refresh_rate = std::nullopt;

    ClientAPI client_api = ClientAPI::OpenGL;

    struct Context {
        ContextAPI api = ContextAPI::Native;
        GLVersion version = { 1, 0 };

        ContextRobustness robustness = ContextRobustness::None;
        ContextReleaseBehavior release_behavior = ContextReleaseBehavior::Any;
        bool no_error = false;

        bool forward_compatible = false;
        bool debug = false;
        GLProfile profile = GLProfile::Any;
    } context;

    struct Cocoa {
        bool retina_framebuffer = true;
        std::string frame_name;
        bool graphics_switching = false;
    } cocoa;

    struct X11 {
        std::string class_name;
        std::string instance_name;
    } x11;
};

class Window : public dutils::NonCopyable {
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
    void move(dmath::ivec2 new_pos) const;

    dmath::ivec2 size() const;
    void resize(dmath::ivec2 new_size) const;

    dmath::ivec2 framebufferSize() const;
    float aspect() const;

    dmath::vec2 contentScale() const;

    bool isResizable() const;
    void setResizable(bool resizable) const;

    std::optional<int> minWidth() const;
    std::optional<int> minHeight() const;
    std::optional<int> maxWidth() const;
    std::optional<int> maxHeight() const;
    void setSizeLimits(std::optional<int> min_width, std::optional<int> min_height, std::optional<int> max_width, std::optional<int> max_height);
    void setMinSize(std::optional<int> min_width, std::optional<int> min_height);
    void setMaxSize(std::optional<int> max_width, std::optional<int> max_height);

    void adjustViewport() const;
    bool autoAdjustViewport() const;
    void setAutoAdjustViewport(bool auto_adjust_viewport);

    std::optional<dmath::ivec2> aspectRatio() const;
    void setAspectRatio(std::optional<dmath::ivec2> aspect_ratio);
    void freezeAspectRatio();

    float opacity() const;
    void setOpacity(float new_opacity) const;

    bool isIconified() const;
    void iconify() const;
    bool autoIconify() const;
    void setAutoIconify(bool auto_iconify) const;

    bool isMaximized() const;
    void maximize() const;

    void restore() const;

    bool isVisible() const;
    void hide() const;
    void show() const;

    bool isFocused() const;
    void focus() const;
    bool focusOnShow() const;
    void setFocusOnShow(bool focus_on_show) const;

    void requestAttention() const;

    bool isHovered() const;

    bool isDecorated() const;
    void setDecorated(bool decorated) const;

    bool isFloating() const;
    void setFloating(bool floating) const;

    bool transparentFramebuffer() const;

    ClientAPI clientAPI() const;
    ContextAPI contextAPI() const;
    GLVersionFull glVersion() const;
    bool forwardCompatible() const;
    bool debugContext() const;
    GLProfile glProfile() const;
    ContextReleaseBehavior contextReleaseBehavior() const;
    bool contextNoError() const;
    ContextRobustness contextRobustness() const;

    const std::string& textInput() const;

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

    void updateSizeLimits() const;

    GLFWwindow* handle_ = nullptr;
    std::string title_;
    dmath::ibounds2 size_limits_;
    std::optional<dmath::ivec2> aspect_ratio_;
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
