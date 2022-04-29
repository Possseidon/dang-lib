cmake_minimum_required(VERSION 3.18)
project(dang-utils CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(catch2)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
  DIRECTORY "include/"
  TYPE INCLUDE
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT dang-lib-targets
)
