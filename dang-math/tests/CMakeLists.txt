cmake_minimum_required(VERSION 3.18)
project(dang-math-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME}
  main.cpp
  vector.cpp
)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE 
    dang-math
    Catch2::Catch2
)

catch_discover_tests(${PROJECT_NAME})
