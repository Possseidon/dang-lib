cmake_minimum_required(VERSION 3.18)
project(dang-math-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME} test-marchingcubes.cpp test-vector.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE <optional> <cmath>)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-math Catch2::Catch2WithMain)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-math)
