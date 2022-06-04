#include "dang-glfw/Input.h"

#include "dang-utils/encoding.h"

namespace dang::glfw {

KeyData::KeyData(Key key, int scancode)
    : key_(key)
    , scancode_(scancode)
{}

Key KeyData::key() const { return key_; }

int KeyData::scancode() { return scancode_ ? scancode_ : scancode_ = glfwGetKeyScancode(static_cast<int>(key_)); }

std::u8string KeyData::name() const
{
    const char* key_name = glfwGetKeyName(static_cast<int>(key_), scancode_);
    return key_name ? dutils::u8stringFrom(key_name) : std::u8string();
}

KeyData::operator Key() const { return key_; }

} // namespace dang::glfw
