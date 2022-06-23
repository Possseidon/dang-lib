cmake_minimum_required(VERSION 3.18)
project(dang-ui CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE dang-math dang-utils)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                     $<INSTALL_INTERFACE:include>)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ TYPE INCLUDE)

install(TARGETS ${PROJECT_NAME} EXPORT dang-lib-targets)
