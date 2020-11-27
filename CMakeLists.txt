cmake_minimum_required(VERSION 3.18)
project(dang-lib)

set(CMAKE_CXX_STANDARD 17)

include(cmake/FindVLD.cmake)

include(FindLua)

add_subdirectory(dang-example)
add_subdirectory(dang-gl)
add_subdirectory(dang-lua)
add_subdirectory(dang-math)
add_subdirectory(dang-utils)
