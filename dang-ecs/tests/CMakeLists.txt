cmake_minimum_required(VERSION 3.18)
project(dang-ecs-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME} test-World.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-ecs Catch2::Catch2WithMain)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-ecs)
