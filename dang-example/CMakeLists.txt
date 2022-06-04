cmake_minimum_required(VERSION 3.18)
project(dang-example CXX)

dang_check_dependencies(GL GLFW LUA MATH UTILS)

add_executable(${PROJECT_NAME} src/dang-example.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE <iostream>)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/PngSuite.zip)
  message("-- Downloading PngSuite")
  file(DOWNLOAD http://www.schaik.com/pngsuite/PngSuite-2017jul19.zip ${CMAKE_BINARY_DIR}/PngSuite.zip)
endif()

add_custom_target(
  textures
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/resources/textures
          -DBIN=${CMAKE_CURRENT_BINARY_DIR}/textures -P ${CMAKE_CURRENT_LIST_DIR}/copy_textures.cmake
  COMMENT "Copying textures"
  VERBATIM)

add_custom_target(
  shaders
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders -DBIN=${CMAKE_CURRENT_BINARY_DIR}/shaders
          -P ${CMAKE_CURRENT_LIST_DIR}/copy_shaders.cmake
  COMMENT "Copying shaders"
  VERBATIM)

add_custom_target(
  pngsuite
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_BINARY_DIR} -DBIN=${CMAKE_CURRENT_BINARY_DIR} -P
          ${CMAKE_CURRENT_LIST_DIR}/extract_pngsuite.cmake
  COMMENT "Extracting PngSuite"
  VERBATIM)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE dang-gl
          dang-glfw
          dang-lua
          dang-lua-stl
          dang-math
          # dang-math-lua-enums dang-math-lua-geometry dang-math-lua-vector-matrix
          dang-utils
          # dang-utils-lua-event
)

add_dependencies(${PROJECT_NAME} shaders pngsuite textures)

target_include_directories(${PROJECT_NAME} PRIVATE include)
