cmake_minimum_required(VERSION 3.18)
project(dang-example CXX)

dang_check_dependencies(GL GLFW LUA MATH UTILS)

add_executable(${PROJECT_NAME} src/dang-example.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE <iostream>)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-gl dang-glfw dang-lua dang-math dang-utils)

target_include_directories(${PROJECT_NAME} PRIVATE include)
