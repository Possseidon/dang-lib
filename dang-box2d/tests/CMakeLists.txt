cmake_minimum_required(VERSION 3.18)
project(dang-box2d-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME}
  main.cpp
  test-Body.cpp
  test-Contact.cpp
  test-Fixture.cpp
  test-Joint.cpp
  test-Shape.cpp
  test-World.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    dang-box2d
    dang-utils
    dang-utils-catch2
    Catch2::Catch2
)

catch_discover_tests(${PROJECT_NAME}
  PROPERTIES
    LABELS dang-box2d
)
