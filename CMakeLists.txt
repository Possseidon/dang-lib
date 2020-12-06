cmake_minimum_required(VERSION 3.18)
project(dang-lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

include(FindLua)
include(FindVLD)

add_subdirectory(dang-example)
add_subdirectory(dang-gl)
add_subdirectory(dang-glfw)
add_subdirectory(dang-lua)
add_subdirectory(dang-math)
add_subdirectory(dang-utils)
