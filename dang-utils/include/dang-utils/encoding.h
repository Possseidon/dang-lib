#pragma once

#include <codecvt>
#include <string>

#include "dang-utils/global.h"

namespace dang::utils {

inline auto charPtrFrom(const std::u8string& string) { return reinterpret_cast<const char*>(string.c_str()); }

inline auto u8stringFrom(char32_t codepoint)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    auto text = converter.to_bytes(static_cast<char32_t>(codepoint));
    auto size = text.size();
    std::u8string result(size, u8'\0');
    std::memcpy(result.data(), text.c_str(), size);
    return result;
}

inline auto u8stringFrom(const char* string)
{
    auto size = std::strlen(string);
    std::u8string result(size, u8'\0');
    std::memcpy(result.data(), string, size);
    return result;
}

} // namespace dang::utils
