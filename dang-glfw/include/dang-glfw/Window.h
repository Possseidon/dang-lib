#pragma once

#include "dang-gl/BufferMask.h"
#include "dang-gl/Context.h"

#include "dang-math/bounds.h"
#include "dang-math/vector.h"

#include "dang-utils/event.h"

#include "Input.h"
#include "Monitor.h"

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

enum class GLDebugSource {
    API = GL_DEBUG_SOURCE_API,
    WindowSystem = GL_DEBUG_SOURCE_WINDOW_SYSTEM,
    ShaderCompiler = GL_DEBUG_SOURCE_SHADER_COMPILER,
    ThirdParty = GL_DEBUG_SOURCE_THIRD_PARTY,
    Application = GL_DEBUG_SOURCE_APPLICATION,
    Other = GL_DEBUG_SOURCE_OTHER
};

enum class GLDebugType {
    Error = GL_DEBUG_TYPE_ERROR,
    DeprecatedBehaviour = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
    UndefinedBehaviour = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
    Portability = GL_DEBUG_TYPE_PORTABILITY,
    Performance = GL_DEBUG_TYPE_PERFORMANCE,
    Other = GL_DEBUG_TYPE_OTHER,
    Marker = GL_DEBUG_TYPE_MARKER,
    PushGroup = GL_DEBUG_TYPE_PUSH_GROUP,
    PopGroup = GL_DEBUG_TYPE_POP_GROUP
};

enum class GLDebugSeverity {
    Notification = GL_DEBUG_SEVERITY_NOTIFICATION,
    Low = GL_DEBUG_SEVERITY_LOW,
    Medium = GL_DEBUG_SEVERITY_MEDIUM,
    High = GL_DEBUG_SEVERITY_HIGH
};

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

struct GLDebugMessageInfo : WindowEventInfo {
    GLDebugSource source;
    GLDebugType type;
    GLuint id;
    GLDebugSeverity severity;
    std::string message;
};

using WindowEvent = dutils::Event<Window>;
using CursorMoveEvent = dutils::Event<CursorMoveInfo>;
using ScrollEvent = dutils::Event<ScrollInfo>;
using DropPathsEvent = dutils::Event<DropPathsInfo>;
using KeyEvent = dutils::Event<KeyInfo>;
using ButtonEvent = dutils::Event<ButtonInfo>;
using GLDebugMessageEvent = dutils::Event<GLDebugMessageInfo>;

/// <summary>Wraps the close to full capabilities of GLFW windows.</summary>
class Window {
public:
    /// <summary>Creates a new GLFW window and activates it.</summary>
    Window(const WindowInfo& info = WindowInfo());
    /// <summary>Destroys the GLFW window.</summary>
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    /// <summary>Extracts a window from an existing handle, mainly used in GLFW callbacks.</summary>
    static Window& fromUserPointer(GLFWwindow* window);

    /// <summary>Returns the handle of the GLFW window.</summary>
    GLFWwindow* handle() const;

    /// <summary>Returns the OpenGL context of this window.</summary>
    const dgl::Context& context() const;
    /// <summary>Returns the OpenGL context of this window.</summary>
    dgl::Context& context();

    // TODO: C++20 use std::u8string
    /// <summary>Returns the title of the window.</summary>
    const std::string& title() const;
    /// <summary>Sets the title of the window to the given string.</summary>
    void setTitle(const std::string& title);

    /// <summary>Returns the current position of the window on the virtual screen.</summary>
    dmath::ivec2 pos() const;
    /// <summary>Moves the window to the given position on the virtual screen.</summary>
    void move(dmath::ivec2 new_pos);

    /// <summary>Returns the size of the window.</summary>
    dmath::ivec2 size() const;
    /// <summary>Sets the window size to the given value.</summary>
    void resize(dmath::ivec2 new_size);

    /// <summary>Returns the size of the framebuffer in pixels.</summary>
    dmath::ivec2 framebufferSize() const;
    /// <summary>Returns the aspect (width/height) of the framebuffer.</summary>
    float aspect() const;

    /// <summary>Returns the relative scale of the window, to accommodate for different monitor DPI settings.</summary>
    dmath::vec2 contentScale() const;

    /// <summary>Whether the window is currently in fullscreen mode.</summary>
    bool isFullscreen() const;
    /// <summary>If the window is in fullscreen mode, returns the monitor, which the window is displayed on, otherwise returns null.</summary>
    Monitor fullscreenMonitor() const;
    /// <summary>Puts the window in fullscreen mode with optional supplied resolution and refresh rate.</summary>
    void makeFullscreen(std::optional<dmath::ivec2> size = std::nullopt,
                        std::optional<int> refresh_rate = std::nullopt);
    /// <summary>Puts the window in fullscreen mode on a specific monitor.</summary>
    void makeFullscreen(Monitor monitor,
                        std::optional<dmath::ivec2> size = std::nullopt,
                        std::optional<int> refresh_rate = std::nullopt);
    /// <summary>Restores the window from fullscreen mode, defaulting to the original position and size, before the fullscreen was activated.</summary>
    void restoreFullscreen(std::optional<dmath::ivec2> pos = std::nullopt,
                           std::optional<dmath::ivec2> size = std::nullopt);

    /// <summary>Whether the user can resize the window.</summary>
    bool isResizable() const;
    /// <summary>Sets, whether the user should be able to freely resize the window to their needs.</summary>
    void setResizable(bool resizable);

    /// <summary>Returns the optional minimum width of the window.</summary>
    std::optional<int> minWidth() const;
    /// <summary>Returns the optional minimum height of the window.</summary>
    std::optional<int> minHeight() const;
    /// <summary>Returns the optional maximum width of the window.</summary>
    std::optional<int> maxWidth() const;
    /// <summary>Returns the optional maximum height of the window.</summary>
    std::optional<int> maxHeight() const;
    /// <summary>Sets all size limits of the window to the given optional values.</summary>
    void setSizeLimits(std::optional<int> min_width,
                       std::optional<int> min_height,
                       std::optional<int> max_width,
                       std::optional<int> max_height);
    /// <summary>Sets the minimum size of the window to the given optional values.</summary>
    void setMinSize(std::optional<int> min_width, std::optional<int> min_height);
    /// <summary>Sets the maximum size of the window to the given optional values.</summary>
    void setMaxSize(std::optional<int> max_width, std::optional<int> max_height);

    /// <summary>Returns the currently set optional width/height ratio to force the window into.</summary>
    std::optional<dmath::ivec2> aspectRatio() const;
    /// <summary>Sets the optional width/height ratio, which the the window should be forced into.</summary>
    void setAspectRatio(std::optional<dmath::ivec2> aspect_ratio);
    /// <summary>Sets the width/height ratio of the window to the current framebuffer size, effectively freezing the current ratio.</summary>
    void freezeAspectRatio();

    /// <summary>Returns the opcacity of the window.</summary>
    float opacity() const;
    /// <summary>Sets the opactity of the window.</summary>
    void setOpacity(float new_opacity);

    /// <summary>Whether the window is currently iconified/minimized.</summary>
    bool isIconified() const;
    /// <summary>Iconifies/minimizes the window.</summary>
    void iconify();
    /// <summary>Whether a fullscreen window should iconify/minimize on lost focus.</summary>
    bool autoIconify() const;
    /// <summary>Sets, whether a fullscreen window should iconify/minimize on lost focus.</summary>
    void setAutoIconify(bool auto_iconify);

    /// <summary>Whether the window is maximized.</summary>
    bool isMaximized() const;
    /// <summary>Maximizes the window.</summary>
    void maximize();

    /// <summary>Restores iconified or maximized windows.</summary>
    void restore();

    /// <summary>Whether the window is visible.</summary>
    bool isVisible() const;
    /// <summary>Hides the window, making it invisible.</summary>
    void hide();
    /// <summary>Shows the window, making it visible again.</summary>
    void show();

    /// <summary>Whether the window is currently focused.</summary>
    bool isFocused() const;
    /// <summary>Force the window to focus.</summary>
    void focus();
    /// <summary>Whether the window should obtain focus when shown.</summary>
    bool focusOnShow() const;
    /// <summary>Sets, whether the window should obtain focus when shown.</summary>
    void setFocusOnShow(bool focus_on_show);

    /// <summary>Requests for attention, usually resulting in the window blinking in the taskbar.</summary>
    void requestAttention();

    /// <summary>Whether the mouse is currently hovering over the content area of the window without being obstructed.</summary>
    bool isHovered() const;

    /// <summary>Whether the window is decorated with a title bar.</summary>
    bool isDecorated() const;
    /// <summary>Decorates or undecorates the window with a title bar.</summary>
    void setDecorated(bool decorated);

    /// <summary>Whether the window will always stay on top.</summary>
    bool isFloating() const;
    /// <summary>Sets, whether the window should always stay on top.</summary>
    void setFloating(bool floating);

    /// <summary>Whether the window has a transparent framebuffer.</summary>
    bool transparentFramebuffer() const;

    /// <summary>Returns the client API, with which the window was created.</summary>
    ClientAPI clientAPI() const;
    /// <summary>Returns the context API, with which the window was created.</summary>
    ContextAPI contextAPI() const;
    /// <summary>Returns the OpenGL version, with which the window was created.</summary>
    GLVersionFull glVersion() const;
    /// <summary>Whether the window was created with OpenGL forward compatibility.</summary>
    bool forwardCompatible() const;
    /// <summary>Whether the window was created with an OpenGL debug context.</summary>
    bool debugContext() const;
    /// <summary>Whether the window was created with the core or compatibility profile.</summary>
    GLProfile glProfile() const;
    /// <summary>Whether the OpenGL pipeline will be flushed before the active window changes.</summary>
    ContextReleaseBehavior contextReleaseBehavior() const;
    /// <summary>Whether the GLFW should trigger errors or cause undefined behavior instead.</summary>
    bool contextNoError() const;
    /// <summary>Returns the robustness strategy, which the window was created with.</summary>
    ContextRobustness contextRobustness() const;

    /// <summary>Returns the current clear mask, which is used at the beginning of a render call.</summary>
    dgl::BufferMask clearMask() const;
    /// <summary>Setst the clear mask, which is used at the beginning of a render call.</summary>
    void setClearMask(dgl::BufferMask mask);

    /// <summary>Whether the window should call glFinish after SwapBuffers.</summary>
    bool finishAfterSwap() const;
    /// <summary>Sets, whether the window should call glFinish after SwapBuffers.</summary>
    void setFinishAfterSwap(bool finish_after_swap);

    /// <summary>Adjusts the OpenGL viewport to current size of the framebuffer.</summary>
    void adjustViewport();
    /// <summary>Whether the OpenGL viewport is automatically adjusted, as the window gets resized.</summary>
    bool autoAdjustViewport() const;
    /// <summary>Sets, whether the OpenGL viewport should be automatically adjusted, as the window is resized.</summary>
    void setAutoAdjustViewport(bool auto_adjust_viewport);

    // TODO: C++20 use std::u8string
    /// <summary>Returns a string of all typed characters since the last update.</summary>
    const std::string& textInput() const;

    /// <summary>Whether the given key is currently pressed down.</summary>
    /// <remarks>If sticky keys is active, keys will stay pressed until this function is called on it.</remarks>
    bool isKeyDown(Key key) const;
    /// <summary>Whether the given mouse button is currently pressed down.</summary>
    /// <remarks>If sticky buttons is active, mouse buttons will stay pressed until this function is called on it.</remarks>
    bool isButtonDown(Button button) const;
    /// <summary>Returns the current position of the cursor, using the top left of the window as origin.</summary>
    /// <remarks>If the cursor is disabled, the position is unbounded.</remarks>
    dmath::dvec2 cursorPos() const;
    /// <summary>Sets the cursor position to the given value, using the top left of the window as origin.</summary>
    void setCursorPos(dmath::dvec2 cursor_pos);

    /// <summary>Whether the cursor is normal, hidden or disabled.</summary>
    CursorMode cursorMode() const;
    /// <summary>Sets, whether the cursor should be normal, hidden or disabled.</summary>
    void setCursorMode(CursorMode cursor_mode);

    /// <summary>Whether keys should stay pressed until isKeyDown is called on them.</summary>
    bool stickyKeys() const;
    /// <summary>Sets, whether keys should stay pressed until isKeyDown is called on them.</summary>
    void setStickyKeys(bool sticky_keys);

    /// <summary>Whether mouse buttons should stay pressed until isButtonDown is called on them.</summary>
    bool stickyButtons() const;
    /// <summary>Sets, whether mouse buttons should stay pressed until isButtonDown is called on them.</summary>
    void setStickyButtons(bool sticky_buttons);

    /// <summary>Whether the state of caps- and scroll-lock should be included in key events.</summary>
    bool lockKeyModifiers() const;
    /// <summary>Sets, whether the state of caps- and scroll-lock should be included in key events.</summary>
    void setLockKeyModifiers(bool lock_key_modifiers);

    /// <summary>For a disabled cursor returns, whether the mouse motion should be captured unscaled and unaccelerated.</summary>
    bool rawMouseMotion() const;
    /// <summary>For a disabled cursor sets, whether the mouse motion should be captured unscaled and unaccelerated.</summary>
    void setRawMouseMotion(bool raw_mouse_motion);
    /// <summary>Whether capturing raw mouse motion is supported by the system.</summary>
    static bool supportsRawMouseMotion();

    /// <summary>Activates the OpenGL context of the window.</summary>
    void activate();

    /// <summary>Activates and updates the window, also triggering the onUpdate event.</summary>
    void update();
    /// <summary>Activates and renders the window using the default framebuffer, which is first cleared, and then drawn by the onRender event.</summary>
    void render();
    /// <summary>Polls window events and clears/updates the text input.</summary>
    void pollEvents();

    /// <summary>Executes a single update-render-poll step.</summary>
    void step();
    /// <summary>Runs update-render-poll steps until the window should close.</summary>
    void run();

    /// <summary>Whether the window should close.</summary>
    bool shouldClose() const;

    /// <summary>Returns the current delta time to the last call to update.</summary>
    float deltaTime() const;
    /// <summary>Returns the FPS, which is smoothed out to accommodate for both low and high framerates.</summary>
    float fps() const;
    /// <summary>Sets the V-Sync mode of the window to the given value.</summary>
    void setVSync(VSync vsync);
    /// <summary>Activates the window and returns, whether the context supports adaptive V-Sync.</summary>
    bool supportsAdaptiveVSync();

    /// <summary>Called in the update method.</summary>
    WindowEvent onUpdate;
    /// <summary>Called by the render method inbetween clear and buffer swapping.</summary>
    WindowEvent onRender;

    /// <summary>Triggered, when the user attempts to close the window.</summary>
    WindowEvent onClose;
    /// <summary>Triggered, when the content scale of the window changes.</summary>
    WindowEvent onContentScale;
    /// <summary>Triggered, when the window receives focus.</summary>
    WindowEvent onFocus;
    /// <summary>Triggered, when the window loses focus.</summary>
    WindowEvent onUnfocus;
    /// <summary>Triggered, when the window is iconified/minimized.</summary>
    WindowEvent onIconify;
    /// <summary>Triggered, when the window is restored from an iconified/minimized state.</summary>
    WindowEvent onUniconify;
    /// <summary>Triggered, when the window is maximized.</summary>
    WindowEvent onMaximize;
    /// <summary>Triggered, when the window is restored from being maximized.</summary>
    WindowEvent onUnmaximize;
    /// <summary>Triggered, when the window is restored from being iconified or maximized.</summary>
    WindowEvent onRestore;
    /// <summary>Triggered, when the window is moved.</summary>
    WindowEvent onMove;
    /// <summary>Triggered, when the window is resized.</summary>
    WindowEvent onResize;
    /// <summary>Triggered, when the window framebuffer is resized.</summary>
    WindowEvent onFramebufferResize;

    /// <summary>Triggered, when the user types something on the keyboard, which can be queried using the textInput method.</summary>
    WindowEvent onType;
    /// <summary>Triggered, when the user presses, holds or releases a key on the keyboard.</summary>
    KeyEvent onKey;
    /// <summary>Triggered, when the mouse cursor enters the content area of the window.</summary>
    WindowEvent onCursorEnter;
    /// <summary>Triggered, when the mouse cursor leaves the content area of the window.</summary>
    WindowEvent onCursorLeave;
    /// <summary>Triggered, when the mouse cursor moves across the content area of the window.</summary>
    CursorMoveEvent onCursorMove;
    /// <summary>Triggered, when the user drops files on the window.</summary>
    DropPathsEvent onDropPaths;
    /// <summary>Triggered, when the user presses or releases a mouse button.</summary>
    ButtonEvent onButton;
    /// <summary>Triggered, when the user scrolls the mouse wheel.</summary>
    ScrollEvent onScroll;

    /// <summary>Triggered, if OpenGL debug output is enabled in the state.</summary>
    /// <remarks>Enabling synchronous debug output is very useful for debugging.</remarks>
    GLDebugMessageEvent onGLDebugMessage;

private:
    /// <summary>Registers all GLFW callbacks.</summary>
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

    // OpenGL callbacks

    static void APIENTRY debugMessageCallback(GLenum source,
                                              GLenum type,
                                              GLuint id,
                                              GLenum severity,
                                              GLsizei length,
                                              const GLchar* message,
                                              const void* user_param);

    /// <summary>Updates the current delta time and FPS.</summary>
    void updateDeltaTime();
    /// <summary>Updates the window size limitations to the stored values.</summary>
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
