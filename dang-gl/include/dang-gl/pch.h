#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <algorithm>
#include <array>
#include <cassert>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "png.h"

namespace fs = std::filesystem;

namespace dang::gl {}
namespace dgl = dang::gl;
