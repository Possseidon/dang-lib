cmake_minimum_required(VERSION 3.18)
project(dang-gl-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME}
  main.cpp
  test-PNGLoader.cpp
)

target_precompile_headers(${PROJECT_NAME}
  PRIVATE
    <filesystem>
    <fstream>
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    dang-gl
    Catch2::Catch2
)

catch_discover_tests(${PROJECT_NAME})

if(NOT EXISTS PngSuite.zip)
  message("-- Downloading PngSuite")
  file(DOWNLOAD http://www.schaik.com/pngsuite/PngSuite-2017jul19.zip PngSuite.zip)
endif()

file(ARCHIVE_EXTRACT
  INPUT PngSuite.zip
  DESTINATION PngSuite
)
