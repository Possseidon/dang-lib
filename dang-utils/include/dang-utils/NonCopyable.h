#pragma once

#include "utils.h"

namespace dang::utils
{

/// <summary>A simple type, which can be used as a base-class to prevent copying, but allow moving.</summary>
struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;

    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&&) = default;
};

}
