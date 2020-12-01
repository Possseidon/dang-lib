#include "pch.h"

#include "Input.h"

namespace dang::glfw {

KeyData::KeyData(Key key, int scancode)
    : key_(key)
    , scancode_(scancode)
{}

Key KeyData::key() const { return key_; }

int KeyData::scancode() { return scancode_ ? scancode_ : scancode_ = glfwGetKeyScancode(static_cast<int>(key_)); }

std::string KeyData::name() const
{
    const char* result = glfwGetKeyName(static_cast<int>(key_), scancode_);
    return result ? result : std::string();
}

KeyData::operator Key() const { return key_; }

} // namespace dang::glfw
