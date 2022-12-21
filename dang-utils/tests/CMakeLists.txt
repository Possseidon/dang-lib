cmake_minimum_required(VERSION 3.18)
project(dang-utils-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME} test-bitset.cpp test-event.cpp test-stub.cpp test-typelist.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE <vector>)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-utils dang-utils-catch2 Catch2::Catch2WithMain)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-utils)
