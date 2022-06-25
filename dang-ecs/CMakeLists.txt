cmake_minimum_required(VERSION 3.18)
project(dang-ecs CXX)

dang_check_dependencies(UTILS)

dang_find_package(Boost REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                     $<INSTALL_INTERFACE:include> ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} INTERFACE dang-utils)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ TYPE INCLUDE)

install(TARGETS ${PROJECT_NAME} EXPORT dang-lib-targets)
