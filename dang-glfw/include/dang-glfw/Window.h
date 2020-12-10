#pragma once

#include "dang-gl/Context/Context.h"
#include "dang-gl/Objects/BufferMask.h"

#include "dang-glfw/Input.h"
#include "dang-glfw/Monitor.h"
#include "dang-glfw/global.h"

#include "dang-math/bounds.h"
#include "dang-math/vector.h"

#include "dang-utils/event.h"

namespace dang::glfw {

struct GLVersion {
    int major;
    int minor;
};

struct GLVersionFull {
    int major;
    int minor;
    int revision;
};

enum class ClientAPI { None = GLFW_NO_API, OpenGL = GLFW_OPENGL_API, OpenGLES = GLFW_OPENGL_ES_API };

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

enum class VSync { Disabled = 0, Enabled = 1, Adaptive = -1 };

enum class CursorMode { Normal = GLFW_CURSOR_NORMAL, Hidden = GLFW_CURSOR_HIDDEN, Disabled = GLFW_CURSOR_DISABLED };

class Window;

struct WindowInfo {
    GLFWwindow* createWindow() const;

    dmath::ivec2 size = {1280, 720};
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
        GLVersion version = {1, 0};

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
        // Unlike most of GLFW, these are indeed ASCII encoded
        std::string class_name;
        std::string instance_name;
    } x11;
};

struct WindowEventInfo {
    Window& window;
};

struct CursorMoveInfo : WindowEventInfo {
    dmath::dvec2 window_pos;
    dmath::vec2 pos;
};

struct ScrollInfo : WindowEventInfo {
    dmath::dvec2 offset;
};

struct DropPathsInfo : WindowEventInfo {
    std::vector<fs::path> paths;
};

struct ButtonInfo : WindowEventInfo {
    ButtonAction action;
    Button button;
    ModifierKeys mods;
};

struct KeyInfo : WindowEventInfo {
    KeyAction action;
    KeyData key;
    ModifierKeys mods;
};

using WindowEvent = dutils::Event<Window>;
using CursorMoveEvent = dutils::Event<CursorMoveInfo>;
using ScrollEvent = dutils::Event<ScrollInfo>;
using DropPathsEvent = dutils::Event<DropPathsInfo>;
using KeyEvent = dutils::Event<KeyInfo>;
using ButtonEvent = dutils::Event<ButtonInfo>;

/// @brief Wraps the close to full capabilities of GLFW windows.
class Window {
public:
    /// @brief Creates a new GLFW window and activates it.
    Window(const WindowInfo& info = WindowInfo());
    /// @brief Destroys the GLFW window.
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    /// @brief Extracts a window from an existing handle, mainly used in GLFW callbacks.
    static Window& fromUserPointer(GLFWwindow* window);

    /// @brief Returns the handle of the GLFW window.
    GLFWwindow* handle() const;

    /// @brief Returns the OpenGL context of this window.
    const dgl::Context& context() const;
    /// @brief Returns the OpenGL context of this window.
    dgl::Context& context();

    // TODO: C++20 use std::u8string
    /// @brief Returns the title of the window.
    const std::string& title() const;
    /// @brief Sets the title of the window to the given string.
    void setTitle(const std::string& title);

    /// @brief Returns the current position of the window on the virtual screen.
    dmath::ivec2 pos() const;
    /// @brief Moves the window to the given position on the virtual screen.
    void move(dmath::ivec2 new_pos);

    /// @brief Returns the size of the window.
    dmath::ivec2 size() const;
    /// @brief Sets the window size to the given value.
    void resize(dmath::ivec2 new_size);

    /// @brief Returns the size of the framebuffer in pixels.
    dmath::ivec2 framebufferSize() const;
    /// @brief Returns the aspect (width/height) of the framebuffer.
    float aspect() const;

    /// @brief Returns the relative scale of the window, to accommodate for different monitor DPI settings.
    dmath::vec2 contentScale() const;

    /// @brief Whether the window is currently in fullscreen mode.
    bool isFullscreen() const;
    /// @brief If the window is in fullscreen mode, returns the monitor, which the window is displayed on, otherwise
    /// returns null.
    Monitor fullscreenMonitor() const;
    /// @brief Puts the window in fullscreen mode with optional supplied resolution and refresh rate.
    void makeFullscreen(std::optional<dmath::ivec2> size = std::nullopt,
                        std::optional<int> refresh_rate = std::nullopt);
    /// @brief Puts the window in fullscreen mode on a specific monitor.
    void makeFullscreen(Monitor monitor,
                        std::optional<dmath::ivec2> size = std::nullopt,
                        std::optional<int> refresh_rate = std::nullopt);
    /// @brief Restores the window from fullscreen mode, defaulting to the original position and size, before the
    /// fullscreen was activated.
    void restoreFullscreen(std::optional<dmath::ivec2> pos = std::nullopt,
                           std::optional<dmath::ivec2> size = std::nullopt);

    /// @brief Whether the user can resize the window.
    bool isResizable() const;
    /// @brief Sets, whether the user should be able to freely resize the window to their needs.
    void setResizable(bool resizable);

    /// @brief Returns the optional minimum width of the window.
    std::optional<int> minWidth() const;
    /// @brief Returns the optional minimum height of the window.
    std::optional<int> minHeight() const;
    /// @brief Returns the optional maximum width of the window.
    std::optional<int> maxWidth() const;
    /// @brief Returns the optional maximum height of the window.
    std::optional<int> maxHeight() const;
    /// @brief Sets all size limits of the window to the given optional values.
    void setSizeLimits(std::optional<int> min_width,
                       std::optional<int> min_height,
                       std::optional<int> max_width,
                       std::optional<int> max_height);
    /// @brief Sets the minimum size of the window to the given optional values.
    void setMinSize(std::optional<int> min_width, std::optional<int> min_height);
    /// @brief Sets the maximum size of the window to the given optional values.
    void setMaxSize(std::optional<int> max_width, std::optional<int> max_height);

    /// @brief Returns the currently set optional width/height ratio to force the window into.
    std::optional<dmath::ivec2> aspectRatio() const;
    /// @brief Sets the optional width/height ratio, which the the window should be forced into.
    void setAspectRatio(std::optional<dmath::ivec2> aspect_ratio);
    /// @brief Sets the width/height ratio of the window to the current framebuffer size, effectively freezing the
    /// current ratio.
    void freezeAspectRatio();

    /// @brief Returns the opcacity of the window.
    float opacity() const;
    /// @brief Sets the opactity of the window.
    void setOpacity(float new_opacity);

    /// @brief Whether the window is currently iconified/minimized.
    bool isIconified() const;
    /// @brief Iconifies/minimizes the window.
    void iconify();
    /// @brief Whether a fullscreen window should iconify/minimize on lost focus.
    bool autoIconify() const;
    /// @brief Sets, whether a fullscreen window should iconify/minimize on lost focus.
    void setAutoIconify(bool auto_iconify);

    /// @brief Whether the window is maximized.
    bool isMaximized() const;
    /// @brief Maximizes the window.
    void maximize();

    /// @brief Restores iconified or maximized windows.
    void restore();

    /// @brief Whether the window is visible.
    bool isVisible() const;
    /// @brief Hides the window, making it invisible.
    void hide();
    /// @brief Shows the window, making it visible again.
    void show();

    /// @brief Whether the window is currently focused.
    bool isFocused() const;
    /// @brief Force the window to focus.
    void focus();
    /// @brief Whether the window should obtain focus when shown.
    bool focusOnShow() const;
    /// @brief Sets, whether the window should obtain focus when shown.
    void setFocusOnShow(bool focus_on_show);

    /// @brief Requests for attention, usually resulting in the window blinking in the taskbar.
    void requestAttention();

    /// @brief Whether the mouse is currently hovering over the content area of the window without being obstructed.
    bool isHovered() const;

    /// @brief Whether the window is decorated with a title bar.
    bool isDecorated() const;
    /// @brief Decorates or undecorates the window with a title bar.
    void setDecorated(bool decorated);

    /// @brief Whether the window will always stay on top.
    bool isFloating() const;
    /// @brief Sets, whether the window should always stay on top.
    void setFloating(bool floating);

    /// @brief Whether the window has a transparent framebuffer.
    bool transparentFramebuffer() const;

    /// @brief Returns the client API, with which the window was created.
    ClientAPI clientAPI() const;
    /// @brief Returns the context API, with which the window was created.
    ContextAPI contextAPI() const;
    /// @brief Returns the OpenGL version, with which the window was created.
    GLVersionFull glVersion() const;
    /// @brief Whether the window was created with OpenGL forward compatibility.
    bool forwardCompatible() const;
    /// @brief Whether the window was created with an OpenGL debug context.
    bool debugContext() const;
    /// @brief Whether the window was created with the core or compatibility profile.
    GLProfile glProfile() const;
    /// @brief Whether the OpenGL pipeline will be flushed before the active window changes.
    ContextReleaseBehavior contextReleaseBehavior() const;
    /// @brief Whether the GLFW should trigger errors or cause undefined behavior instead.
    bool contextNoError() const;
    /// @brief Returns the robustness strategy, which the window was created with.
    ContextRobustness contextRobustness() const;

    /// @brief Returns the current clear mask, which is used at the beginning of a render call.
    dgl::BufferMask clearMask() const;
    /// @brief Setst the clear mask, which is used at the beginning of a render call.
    void setClearMask(dgl::BufferMask mask);

    /// @brief Whether the window should call glFinish after SwapBuffers.
    bool finishAfterSwap() const;
    /// @brief Sets, whether the window should call glFinish after SwapBuffers.
    void setFinishAfterSwap(bool finish_after_swap);

    /// @brief Adjusts the OpenGL viewport to current size of the framebuffer.
    void adjustViewport();
    /// @brief Whether the OpenGL viewport is automatically adjusted, as the window gets resized.
    bool autoAdjustViewport() const;
    /// @brief Sets, whether the OpenGL viewport should be automatically adjusted, as the window is resized.
    void setAutoAdjustViewport(bool auto_adjust_viewport);

    // TODO: C++20 use std::u8string
    /// @brief Returns a string of all typed characters since the last update.
    const std::string& textInput() const;

    /// @brief Whether the given key is currently pressed down.
    /// @remark If sticky keys is active, keys will stay pressed until this function is called on it.
    bool isKeyDown(Key key) const;
    /// @brief Whether the given mouse button is currently pressed down.
    /// @remark If sticky buttons is active, mouse buttons will stay pressed until this function is called on it.
    bool isButtonDown(Button button) const;
    /// @brief Returns the current position of the cursor, using the top left of the window as origin.
    /// @remark If the cursor is disabled, the position is unbounded.
    dmath::dvec2 cursorPos() const;
    /// @brief Sets the cursor position to the given value, using the top left of the window as origin.
    void setCursorPos(dmath::dvec2 cursor_pos);

    /// @brief Whether the cursor is normal, hidden or disabled.
    CursorMode cursorMode() const;
    /// @brief Sets, whether the cursor should be normal, hidden or disabled.
    void setCursorMode(CursorMode cursor_mode);

    /// @brief Whether keys should stay pressed until isKeyDown is called on them.
    bool stickyKeys() const;
    /// @brief Sets, whether keys should stay pressed until isKeyDown is called on them.
    void setStickyKeys(bool sticky_keys);

    /// @brief Whether mouse buttons should stay pressed until isButtonDown is called on them.
    bool stickyButtons() const;
    /// @brief Sets, whether mouse buttons should stay pressed until isButtonDown is called on them.
    void setStickyButtons(bool sticky_buttons);

    /// @brief Whether the state of caps- and scroll-lock should be included in key events.
    bool lockKeyModifiers() const;
    /// @brief Sets, whether the state of caps- and scroll-lock should be included in key events.
    void setLockKeyModifiers(bool lock_key_modifiers);

    /// @brief For a disabled cursor returns, whether the mouse motion should be captured unscaled and unaccelerated.
    bool rawMouseMotion() const;
    /// @brief For a disabled cursor sets, whether the mouse motion should be captured unscaled and unaccelerated.
    void setRawMouseMotion(bool raw_mouse_motion);
    /// @brief Whether capturing raw mouse motion is supported by the system.
    static bool supportsRawMouseMotion();

    /// @brief Activates the OpenGL context of the window.
    void activate();

    /// @brief Activates and updates the window, also triggering the onUpdate event.
    void update();
    /// @brief Activates and renders the window using the default framebuffer, which is first cleared, and then drawn by
    /// the onRender event.
    void render();
    /// @brief Polls window events and clears/updates the text input.
    void pollEvents();

    /// @brief Executes a single update-render-poll step.
    void step();
    /// @brief Runs update-render-poll steps until the window should close.
    void run();

    /// @brief Whether the window should close.
    bool shouldClose() const;

    /// @brief Returns the current delta time to the last call to update.
    float deltaTime() const;
    /// @brief Returns the FPS, which is smoothed out to accommodate for both low and high framerates.
    float fps() const;
    /// @brief Sets the V-Sync mode of the window to the given value.
    void setVSync(VSync vsync);
    /// @brief Activates the window and returns, whether the context supports adaptive V-Sync.
    bool supportsAdaptiveVSync();

    /// @brief Called in the update method.
    WindowEvent onUpdate;
    /// @brief Called by the render method inbetween clear and buffer swapping.
    WindowEvent onRender;

    /// @brief Triggered, when the user attempts to close the window.
    WindowEvent onClose;
    /// @brief Triggered, when the content scale of the window changes.
    WindowEvent onContentScale;
    /// @brief Triggered, when the window receives focus.
    WindowEvent onFocus;
    /// @brief Triggered, when the window loses focus.
    WindowEvent onUnfocus;
    /// @brief Triggered, when the window is iconified/minimized.
    WindowEvent onIconify;
    /// @brief Triggered, when the window is restored from an iconified/minimized state.
    WindowEvent onUniconify;
    /// @brief Triggered, when the window is maximized.
    WindowEvent onMaximize;
    /// @brief Triggered, when the window is restored from being maximized.
    WindowEvent onUnmaximize;
    /// @brief Triggered, when the window is restored from being iconified or maximized.
    WindowEvent onRestore;
    /// @brief Triggered, when the window is moved.
    WindowEvent onMove;
    /// @brief Triggered, when the window is resized.
    WindowEvent onResize;
    /// @brief Triggered, when the window framebuffer is resized.
    WindowEvent onFramebufferResize;

    /// @brief Triggered, when the user types something on the keyboard, which can be queried using the textInput
    /// method.
    WindowEvent onType;
    /// @brief Triggered, when the user presses, holds or releases a key on the keyboard.
    KeyEvent onKey;
    /// @brief Triggered, when the mouse cursor enters the content area of the window.
    WindowEvent onCursorEnter;
    /// @brief Triggered, when the mouse cursor leaves the content area of the window.
    WindowEvent onCursorLeave;
    /// @brief Triggered, when the mouse cursor moves across the content area of the window.
    CursorMoveEvent onCursorMove;
    /// @brief Triggered, when the user drops files on the window.
    DropPathsEvent onDropPaths;
    /// @brief Triggered, when the user presses or releases a mouse button.
    ButtonEvent onButton;
    /// @brief Triggered, when the user scrolls the mouse wheel.
    ScrollEvent onScroll;

private:
    /// @brief Registers all GLFW callbacks.
    void registerCallbacks();

    // GLFW callbacks

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

    /// @brief Updates the current delta time and FPS.
    void updateDeltaTime();
    /// @brief Updates the window size limitations to the stored values.
    void updateSizeLimits();

    GLFWwindow* handle_ = nullptr;

    dgl::Context context_{(activate(), size())};

    // Window-Properties
    std::string title_;
    dmath::ibounds2 size_limits_;
    dmath::ivec2 fullscreen_restore_pos_;
    dmath::ivec2 fullscreen_restore_size_;
    std::optional<dmath::ivec2> aspect_ratio_;

    // Render-Properties
    dgl::BufferMask clear_mask_ = dgl::BufferMask::ALL;
    bool auto_adjust_viewport_ = true;
    bool finish_after_swap_ = true;

    // DeltaTime and FPS
    std::uint64_t last_time_ = 0;
    float delta_time_ = 0;
    float fps_ = 0;

    // Input
    std::string text_input_;
};

} // namespace dang::glfw
