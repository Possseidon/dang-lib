cmake_minimum_required(VERSION 3.18)
project(dang-math-lua CXX)

add_library(${PROJECT_NAME}-enums INTERFACE)

target_link_libraries(${PROJECT_NAME}-enums INTERFACE dang-lua dang-math)

target_include_directories(${PROJECT_NAME}-enums INTERFACE include)

add_library(${PROJECT_NAME}-vector-matrix STATIC EXCLUDE_FROM_ALL src/lua-vector-matrix.cpp)

target_precompile_headers(
  ${PROJECT_NAME}-vector-matrix
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
  <vector>)

target_link_libraries(
  ${PROJECT_NAME}-vector-matrix
  PRIVATE dang-utils ${PROJECT_NAME}-enums
  PUBLIC dang-lua dang-math)

target_include_directories(${PROJECT_NAME}-vector-matrix PUBLIC include)

set_property(TARGET ${PROJECT_NAME}-vector-matrix PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME}-geometry STATIC EXCLUDE_FROM_ALL src/lua-geometry.cpp)

target_precompile_headers(
  ${PROJECT_NAME}-geometry
  PRIVATE
  <tuple>
  <type_traits>
  <utility>
  PUBLIC
  <string>
  <vector>)

target_link_libraries(
  ${PROJECT_NAME}-geometry
  PRIVATE dang-utils
  PUBLIC dang-lua dang-math ${PROJECT_NAME}-vector-matrix)

target_include_directories(${PROJECT_NAME}-geometry PUBLIC include)

set_property(TARGET ${PROJECT_NAME}-geometry PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(dmath SHARED EXCLUDE_FROM_ALL src/dmath.cpp)

target_link_libraries(dmath PRIVATE dang-lua ${PROJECT_NAME}-vector-matrix ${PROJECT_NAME}-geometry)

set_target_properties(${PROJECT_NAME}-vector-matrix ${PROJECT_NAME}-geometry dmath #
                      PROPERTIES POSITION_INDEPENDENT_CODE ON PREFIX "")
