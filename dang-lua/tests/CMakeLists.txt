cmake_minimum_required(VERSION 3.18)
project(dang-lua-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(
  ${PROJECT_NAME}
  convert/test-Base.cpp
  convert/test-Boolean.cpp
  convert/test-Class.cpp
  convert/test-Enum.cpp
  convert/test-Fail.cpp
  convert/test-Function.cpp
  convert/test-Integer.cpp
  convert/test-Nil.cpp
  convert/test-Number.cpp
  convert/test-Optional.cpp
  convert/test-Reference.cpp
  convert/test-String.cpp
  convert/test-Tuple.cpp
  convert/test-Variant.cpp
  main.cpp
  test-Allocator.cpp
  test-Reference.cpp
  test-State.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-lua dang-utils dang-utils-catch2)

target_include_directories(${PROJECT_NAME} PRIVATE .)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-lua)
