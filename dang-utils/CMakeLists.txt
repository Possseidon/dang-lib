cmake_minimum_required(VERSION 3.18)
project(dang-utils CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    include
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
