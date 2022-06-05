#pragma once

#include "dang-glfw/global.h"
#include "dang-utils/enum.h"

namespace dang::glfw {

/// @brief Whether a mouse button has been pressed or released.
enum class ButtonAction : int { Release = GLFW_RELEASE, Press = GLFW_PRESS, COUNT };

/// @brief A list of possible mouse buttons.
enum class Button : int {
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE,

    Button1 = GLFW_MOUSE_BUTTON_1,
    Button2 = GLFW_MOUSE_BUTTON_2,
    Button3 = GLFW_MOUSE_BUTTON_3,
    Button4 = GLFW_MOUSE_BUTTON_4,
    Button5 = GLFW_MOUSE_BUTTON_5,
    Button6 = GLFW_MOUSE_BUTTON_6,
    Button7 = GLFW_MOUSE_BUTTON_7,
    Button8 = GLFW_MOUSE_BUTTON_8,

    COUNT
};

/// @brief Whether a keyboard key has been pressed, released or is held down, causing it to repeat in quick succession.
enum class KeyAction : int { Release = GLFW_RELEASE, Press = GLFW_PRESS, Repeat = GLFW_REPEAT, COUNT };

/// @brief Modifier keys, which can be held down in key-combinations.
enum class ModifierKey : int { Shift, Control, Alt, Super, CapsLock, NumLock, COUNT };

} // namespace dang::glfw

namespace dang::utils {

template <>
struct enum_count<dang::glfw::ButtonAction> : default_enum_count<dang::glfw::ButtonAction> {};

template <>
struct enum_count<dang::glfw::Button> : default_enum_count<dang::glfw::Button> {};

template <>
struct enum_count<dang::glfw::KeyAction> : default_enum_count<dang::glfw::KeyAction> {};

template <>
struct enum_count<dang::glfw::ModifierKey> : default_enum_count<dang::glfw::ModifierKey> {};

} // namespace dang::utils

namespace dang::glfw {

using ModifierKeys = dutils::EnumSet<ModifierKey>;

static_assert(ModifierKeys::fromBits(GLFW_MOD_SHIFT) == ModifierKey::Shift);
static_assert(ModifierKeys::fromBits(GLFW_MOD_CONTROL) == ModifierKey::Control);
static_assert(ModifierKeys::fromBits(GLFW_MOD_ALT) == ModifierKey::Alt);
static_assert(ModifierKeys::fromBits(GLFW_MOD_SUPER) == ModifierKey::Super);
static_assert(ModifierKeys::fromBits(GLFW_MOD_CAPS_LOCK) == ModifierKey::CapsLock);
static_assert(ModifierKeys::fromBits(GLFW_MOD_NUM_LOCK) == ModifierKey::NumLock);

/// @brief A list of all possible keyboard keys.
enum class Key : int {
    Unknown = GLFW_KEY_UNKNOWN,
    Space = GLFW_KEY_SPACE,
    Apostrophe = GLFW_KEY_APOSTROPHE,
    Comma = GLFW_KEY_COMMA,
    Minus = GLFW_KEY_MINUS,
    Period = GLFW_KEY_PERIOD,
    Slash = GLFW_KEY_SLASH,

    Num0 = GLFW_KEY_0,
    Num1 = GLFW_KEY_1,
    Num2 = GLFW_KEY_2,
    Num3 = GLFW_KEY_3,
    Num4 = GLFW_KEY_4,
    Num5 = GLFW_KEY_5,
    Num6 = GLFW_KEY_6,
    Num7 = GLFW_KEY_7,
    Num8 = GLFW_KEY_8,
    Num9 = GLFW_KEY_9,

    Semicolon = GLFW_KEY_SEMICOLON,
    Equal = GLFW_KEY_EQUAL,

    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,

    LeftBracket = GLFW_KEY_LEFT_BRACKET,
    Backslash = GLFW_KEY_BACKSLASH,
    RightBracket = GLFW_KEY_RIGHT_BRACKET,
    GraveAccent = GLFW_KEY_GRAVE_ACCENT,
    World1 = GLFW_KEY_WORLD_1,
    World2 = GLFW_KEY_WORLD_2,

    Escape = GLFW_KEY_ESCAPE,
    Enter = GLFW_KEY_ENTER,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,

    Right = GLFW_KEY_RIGHT,
    Left = GLFW_KEY_LEFT,
    Down = GLFW_KEY_DOWN,
    Up = GLFW_KEY_UP,

    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN,
    Home = GLFW_KEY_HOME,
    End = GLFW_KEY_END,
    CapsLock = GLFW_KEY_CAPS_LOCK,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    NumLock = GLFW_KEY_NUM_LOCK,
    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause = GLFW_KEY_PAUSE,

    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13,
    F14 = GLFW_KEY_F14,
    F15 = GLFW_KEY_F15,
    F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17,
    F18 = GLFW_KEY_F18,
    F19 = GLFW_KEY_F19,
    F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21,
    F22 = GLFW_KEY_F22,
    F23 = GLFW_KEY_F23,
    F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25,

    Kp0 = GLFW_KEY_KP_0,
    Kp1 = GLFW_KEY_KP_1,
    Kp2 = GLFW_KEY_KP_2,
    Kp3 = GLFW_KEY_KP_3,
    Kp4 = GLFW_KEY_KP_4,
    Kp5 = GLFW_KEY_KP_5,
    Kp6 = GLFW_KEY_KP_6,
    Kp7 = GLFW_KEY_KP_7,
    Kp8 = GLFW_KEY_KP_8,
    Kp9 = GLFW_KEY_KP_9,

    KpDecimal = GLFW_KEY_KP_DECIMAL,
    KpDivide = GLFW_KEY_KP_DIVIDE,
    KpMultiply = GLFW_KEY_KP_MULTIPLY,
    KpSubtract = GLFW_KEY_KP_SUBTRACT,
    KpAdd = GLFW_KEY_KP_ADD,
    KpEnter = GLFW_KEY_KP_ENTER,
    KpEqual = GLFW_KEY_KP_EQUAL,

    LeftShift = GLFW_KEY_LEFT_SHIFT,
    LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt = GLFW_KEY_LEFT_ALT,
    LeftSuper = GLFW_KEY_LEFT_SUPER,

    RightShift = GLFW_KEY_RIGHT_SHIFT,
    RightControl = GLFW_KEY_RIGHT_CONTROL,
    RightAlt = GLFW_KEY_RIGHT_ALT,
    RightSuper = GLFW_KEY_RIGHT_SUPER,

    Menu = GLFW_KEY_MENU
};

/// @brief Wraps a key and an optional scancode.
class KeyData {
public:
    /// @brief Initializes the key data with the given key and optional scancode, if it is already known.
    KeyData(Key key, int scancode = 0);

    /// @brief Returns the keyboard key.
    Key key() const;
    /// @brief Returns the scancode of the key attempting to query it, if it has not been supplied in the constructor.
    int scancode();
    /// @brief Returns a displayable name for the key or an empty string.
    std::u8string name() const;

    /// @brief Allows for implicit conversion to the key enum.
    operator Key() const;

private:
    Key key_;
    int scancode_;
};

} // namespace dang::glfw
