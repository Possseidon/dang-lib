cmake_minimum_required(VERSION 3.18)
project(dang-example)

add_executable(${PROJECT_NAME}
    src/dang-example.cpp
    src/pch.cpp
)

target_link_libraries(${PROJECT_NAME}
    dang-gl
    dang-glfw
    dang-lua
    dang-math
    dang-utils
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include/dang-example
  PUBLIC
    include
)
