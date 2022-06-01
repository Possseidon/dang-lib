cmake_minimum_required(VERSION 3.18)
project(dang-utils-catch2 CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE dang-utils)

target_include_directories(${PROJECT_NAME} INTERFACE include)
