cmake_minimum_required(VERSION 3.18)
project(dang-lua-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME} main.cpp test-Convert.cpp #[[test-State.cpp]])

target_link_libraries(${PROJECT_NAME} PRIVATE dang-lua)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-lua)
