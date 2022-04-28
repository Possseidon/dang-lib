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

option(WITH_DANG_EXAMPLE "Include examples." ON)
if(${WITH_DANG_EXAMPLE})
  add_subdirectory(dang-example)
endif()

option(WITH_DANG_GL "Include OpenGL library." ON)
if(${WITH_DANG_GL})
  add_subdirectory(dang-gl)
endif()

option(WITH_DANG_GLFW "Include GLFW wrapper library." ON)
if(${WITH_DANG_GLFW})
  add_subdirectory(dang-glfw)
endif()

option(WITH_DANG_LUA "Include Lua wrapper library." ON)
if(${WITH_DANG_LUA})
  add_subdirectory(dang-lua)
endif()

option(WITH_DANG_MATH "Include math library." ON)
if(${WITH_DANG_MATH})
  add_subdirectory(dang-math)
endif()

option(WITH_DANG_UTILS "Include utility library." ON)
if(${WITH_DANG_UTILS})
  add_subdirectory(dang-utils)
endif()

option(WITH_DANG_DOC "Include documentation." ON)
if(${WITH_DANG_DOC})
  add_subdirectory(doc)
endif()
