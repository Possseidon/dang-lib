cmake_minimum_required(VERSION 3.18)
project(dang-lua-stl CXX)

find_package(Lua REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_precompile_headers(${PROJECT_NAME}
  INTERFACE
    <algorithm>
    <string>
)

target_link_libraries(${PROJECT_NAME}
  INTERFACE
    dang-lua
)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    include
)
