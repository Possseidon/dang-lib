#pragma once

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <locale>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "dang-gl/global.h"

#include "dang-math/global.h"

#include "dang-utils/global.h"

#include "GLFW/glfw3.h"

namespace fs = std::filesystem;

namespace dang::glfw {

namespace dgl = dang::gl;
namespace dmath = dang::math;
namespace dutils = dang::utils;

} // namespace dang::glfw
