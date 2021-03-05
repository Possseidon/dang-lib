cmake_minimum_required(VERSION 3.18)
project(dang-math-lua CXX)

add_library(${PROJECT_NAME}-vector-matrix STATIC
  src/lua-vector-matrix.cpp
)

target_precompile_headers(${PROJECT_NAME}-vector-matrix
  PRIVATE
    <algorithm>
    <utility>
  PUBLIC
    <optional>
    <string_view>
    <string>
    <tuple>
    <type_traits>
    <variant>
    <vector>
)

target_link_libraries(${PROJECT_NAME}-vector-matrix
  PRIVATE
    dang-utils
  PUBLIC
    dang-lua
    dang-math
) 

target_include_directories(${PROJECT_NAME}-vector-matrix
  PUBLIC
    include
)

add_library(dmath SHARED
  src/dmath.cpp
)

target_link_libraries(dmath
  PRIVATE
    dang-lua
    ${PROJECT_NAME}-vector-matrix
)