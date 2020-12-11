cmake_minimum_required(VERSION 3.18)
project(dang-glfw)

find_package(glfw3 CONFIG REQUIRED)

add_library(${PROJECT_NAME}
    src/GLFW.cpp
    src/Input.cpp
    src/Monitor.cpp
    src/Window.cpp
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
    <algorithm>
    <cmath>
    <codecvt>
    <cstdint>
    <cstdlib>
    <filesystem>
    <iostream>
    <locale>
    <optional>
    <sstream>
    <stdexcept>
    <string>
    <tuple>
    <vector>
)

if(MSVC)
  target_compile_definitions(${PROJECT_NAME} 
    PUBLIC
      _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
  )
endif()

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dang-gl
    glfw
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include/dang-glfw
  PUBLIC
    include
)
