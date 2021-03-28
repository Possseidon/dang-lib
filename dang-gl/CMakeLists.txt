cmake_minimum_required(VERSION 3.18)
project(dang-gl CXX)

find_package(glad CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)

add_library(${PROJECT_NAME}
    src/Context/Context.cpp
    src/Context/State.cpp
    src/Context/StateTypes.cpp
    src/Image/PNGLoader.cpp
    src/Math/Transform.cpp
    src/Objects/FBO.cpp
    src/Objects/ObjectContext.cpp
    src/Objects/Program.cpp
    src/Objects/RBO.cpp
    src/Objects/VAO.cpp
    src/Rendering/Camera.cpp
    src/Rendering/Renderable.cpp
    src/Texturing/TextureAtlasTiles.cpp
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
    <algorithm>
    <array>
    <cassert>
    <cmath>
    <cstddef>
    <cstdint>
    <filesystem>
    <fstream>
    <functional>
    <initializer_list>
    <iostream>
    <istream>
    <iterator>
    <limits>
    <map>
    <memory>
    <optional>
    <regex>
    <set>
    <sstream>
    <stack>
    <stdexcept>
    <string>
    <tuple>
    <type_traits>
    <utility>
    <vector>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dang-math
    dang-utils
    glad::glad
    png
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include/dang-gl
  PUBLIC
    include
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
