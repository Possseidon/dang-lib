cmake_minimum_required(VERSION 3.18)
project(dang-math CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME}
  INTERFACE
    dang-utils
)

target_precompile_headers(${PROJECT_NAME}
  INTERFACE
    <algorithm>
    <array>
    <functional>
    <iostream>
    <numeric>
    <optional>
    <sstream>
    <type_traits>
)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    include
)

add_subdirectory(lua)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
