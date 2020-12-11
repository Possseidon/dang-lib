cmake_minimum_required(VERSION 3.18)
project(dang-utils-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME}
  event.cpp
  main.cpp
)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE 
    dang-utils
    Catch2::Catch2
)

catch_discover_tests(${PROJECT_NAME})
