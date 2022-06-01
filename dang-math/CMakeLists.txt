cmake_minimum_required(VERSION 3.18)
project(dang-math CXX)

dang_check_dependencies(UTILS)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE dang-utils)

target_precompile_headers(
  ${PROJECT_NAME}
  INTERFACE
  <algorithm>
  <array>
  <cmath>
  <functional>
  <iostream>
  <numeric>
  <optional>
  <sstream>
  <type_traits>)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                     $<INSTALL_INTERFACE:include>)

add_subdirectory(lua)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(DIRECTORY "include/" TYPE INCLUDE)

install(TARGETS ${PROJECT_NAME} EXPORT dang-lib-targets)
