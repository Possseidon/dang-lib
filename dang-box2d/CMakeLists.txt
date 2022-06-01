cmake_minimum_required(VERSION 3.18)
project(dang-box2d CXX)

dang_check_dependencies(MATH UTILS)

dang_find_package(box2d CONFIG REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                     $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME} INTERFACE dang-math dang-utils box2d::box2d)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(DIRECTORY "include/" TYPE INCLUDE)

install(TARGETS ${PROJECT_NAME} EXPORT dang-lib-targets)
