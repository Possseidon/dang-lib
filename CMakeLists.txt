cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

project(dang-lib VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

option(DANG_WERROR "Whether to treat warnings as errors.")

if(MSVC)
  add_compile_options(/W4 /utf-8 /bigobj $<$<BOOL:${DANG_WERROR}>:/WX>)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(
      "SHELL:-Xclang -Wno-missing-braces"
      "SHELL:-Xclang -Wno-logical-op-parentheses"
    )
  endif()
  add_compile_definitions("DANG_DLLEXPORT=__declspec(dllexport)")
else()
  add_compile_options(-Wall -Wextra -pedantic $<$<BOOL:${DANG_WERROR}>:-Werror>)
  add_compile_options(
    -Wno-missing-braces
    -Wno-logical-op-parentheses
  )
  add_compile_definitions("DANG_DLLEXPORT=__attribute__((visibility(\"default\")))")
endif()

include(CTest)
include(FindVLD)
include(dang_add_feature)

dang_add_feature(
  UTILS dang-utils
  "Include utility library."
)

dang_add_feature(
  MATH dang-math
  "Include math library."
  UTILS
)

dang_add_feature(
  GL dang-gl
  "Include OpenGL library."
  UTILS MATH
)

dang_add_feature(
  GLFW dang-glfw
  "Include GLFW wrapper library."
  UTILS MATH GL
)

dang_add_feature(
  LUA dang-lua
  "Include Lua wrapper library."
  UTILS
)

dang_add_feature(
  EXAMPLE dang-example
  "Include examples."
  UTILS MATH GL GLFW LUA
)

dang_add_feature(
  DOC doc
  "Include documentation."
)

include(GNUInstallDirs)
install(
  EXPORT dang-lib-targets
  FILE dang-lib-config.cmake
  NAMESPACE dang-lib::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dang-lib"
)
