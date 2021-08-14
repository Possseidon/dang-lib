cmake_minimum_required(VERSION 3.18)
project(dang-utils-lua CXX)

add_library(${PROJECT_NAME}-event INTERFACE)

target_link_libraries(${PROJECT_NAME}-event
  INTERFACE
    dang-utils
    dang-lua
)

target_include_directories(${PROJECT_NAME}-event
  INTERFACE
    include
)

target_precompile_headers(${PROJECT_NAME}-event
  INTERFACE
    <array>
    <string>
)
