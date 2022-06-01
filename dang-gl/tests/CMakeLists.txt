cmake_minimum_required(VERSION 3.18)
project(dang-gl-tests CXX)

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

add_executable(${PROJECT_NAME} Image/test-PNGLoader.cpp main.cpp Texturing/test-TextureAtlasBase.cpp
                               Texturing/test-TextureAtlasTiles.cpp)

target_precompile_headers(
  ${PROJECT_NAME}
  PRIVATE
  <filesystem>
  <fstream>
  <set>
  <stdexcept>
  <string>
  <tuple>
  <type_traits>
  <utility>)

target_link_libraries(${PROJECT_NAME} PRIVATE dang-gl dang-utils dang-utils-catch2 Catch2::Catch2)

catch_discover_tests(${PROJECT_NAME} PROPERTIES LABELS dang-gl WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/PngSuite.zip)
  message("-- Downloading PngSuite")
  file(DOWNLOAD http://www.schaik.com/pngsuite/PngSuite-2017jul19.zip ${CMAKE_BINARY_DIR}/PngSuite.zip)
endif()

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/PngSuite
  COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${PROJECT_NAME}>/PngSuite ${CMAKE_COMMAND} -E tar x
          ${CMAKE_BINARY_DIR}/PngSuite.zip
  COMMENT "-- Extracting PngSuite")

# OpenGL Tests require an OpenGL context using an invisible GLFW window.
if(WITH_DANG_GLFW)

  add_executable(${PROJECT_NAME}-opengl main.cpp Texturing/test-MultiTextureAtlas.cpp Texturing/test-TextureAtlas.cpp
                                        Texturing/test-TextureAtlasUtils.cpp)

  target_precompile_headers(${PROJECT_NAME}-opengl PRIVATE <stdexcept>)

  target_link_libraries(${PROJECT_NAME}-opengl PRIVATE dang-gl dang-glfw Catch2::Catch2)

  catch_discover_tests(${PROJECT_NAME}-opengl PROPERTIES LABELS opengl dang-gl)

endif()
