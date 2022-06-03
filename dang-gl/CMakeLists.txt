cmake_minimum_required(VERSION 3.18)
project(dang-gl CXX)

dang_check_dependencies(MATH UTILS)

dang_find_package(glad CONFIG REQUIRED)
dang_find_package(libpng CONFIG REQUIRED)

add_library(
  ${PROJECT_NAME} STATIC
  src/Context/Context.cpp
  src/Context/State.cpp
  src/Context/StateTypes.cpp
  src/General/GLConstants.cpp
  src/Image/BorderedImage.cpp
  src/Image/Image.cpp
  src/Image/ImageBorder.cpp
  src/Image/Pixel.cpp
  src/Image/PixelFormat.cpp
  src/Image/PixelInternalFormat.cpp
  src/Image/PixelType.cpp
  src/Image/PNGLoader.cpp
  src/Math/MathConstants.cpp
  src/Math/MathTypes.cpp
  src/Math/Transform.cpp
  src/Objects/Buffer.cpp
  src/Objects/BufferContext.cpp
  src/Objects/BufferMask.cpp
  src/Objects/DataTypes.cpp
  src/Objects/FBO.cpp
  src/Objects/FramebufferContext.cpp
  src/Objects/Object.cpp
  src/Objects/ObjectContext.cpp
  src/Objects/ObjectHandle.cpp
  src/Objects/ObjectType.cpp
  src/Objects/ObjectWrapper.cpp
  src/Objects/Program.cpp
  src/Objects/ProgramContext.cpp
  src/Objects/RBO.cpp
  src/Objects/RenderbufferContext.cpp
  src/Objects/Texture.cpp
  src/Objects/TextureContext.cpp
  src/Objects/UniformWrapper.cpp
  src/Objects/VAO.cpp
  src/Objects/VBO.cpp
  src/Objects/VertexArrayContext.cpp
  src/Rendering/Camera.cpp
  src/Rendering/Renderable.cpp
  src/Texturing/MultiTextureAtlas.cpp
  src/Texturing/TextureAtlas.cpp
  src/Texturing/TextureAtlasBase.cpp
  src/Texturing/TextureAtlasTiles.cpp
  src/Texturing/TextureAtlasUtils.cpp)

target_precompile_headers(
  ${PROJECT_NAME}
  PUBLIC
  <algorithm>
  <array>
  <cassert>
  <cmath>
  <cstddef>
  <cstdint>
  <cstring>
  <filesystem>
  <fstream>
  <functional>
  <initializer_list>
  <iostream>
  <istream>
  <iterator>
  <limits>
  <map>
  <memory>
  <optional>
  <regex>
  <set>
  <sstream>
  <stack>
  <stdexcept>
  <string>
  <tuple>
  <type_traits>
  <unordered_map>
  <utility>
  <variant>
  <vector>)

target_link_libraries(${PROJECT_NAME} PUBLIC dang-math dang-utils glad::glad png)

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                  $<INSTALL_INTERFACE:include>)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(DIRECTORY "include/" TYPE INCLUDE)

install(TARGETS ${PROJECT_NAME} EXPORT dang-lib-targets)
