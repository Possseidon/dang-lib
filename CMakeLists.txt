cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

project(dang-lib VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
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
include(FindLua)
include(FindVLD)

add_subdirectory(dang-example)
add_subdirectory(dang-gl)
add_subdirectory(dang-glfw)
add_subdirectory(dang-lua)
add_subdirectory(dang-math)
add_subdirectory(dang-utils)

add_subdirectory(doc)
