cmake_minimum_required(VERSION 3.18)
project(dang-math-lua CXX)

add_library(${PROJECT_NAME}-vector-matrix INTERFACE)

target_link_libraries(${PROJECT_NAME}-vector-matrix
  INTERFACE
    dang-lua
    dang-math
) 

target_include_directories(${PROJECT_NAME}-vector-matrix 
  INTERFACE
    include
)
