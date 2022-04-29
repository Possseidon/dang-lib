cmake_minimum_required(VERSION 3.18)

set(
  CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file"
)

macro(dang_feature_option FEATURE_NAME FEATURE_DESCRIPTION)
  option(WITH_DANG_${FEATURE_NAME} ${FEATURE_DESCRIPTION} ON)
  if (WITH_DANG_${FEATURE_NAME})
    list(APPEND VCPKG_MANIFEST_FEATURES ${ARGN})
  endif()
endmacro()

dang_feature_option(DOC "Include documentation.")
dang_feature_option(EXAMPLE "Include examples.")
dang_feature_option(GL "Include OpenGL library." gl)
dang_feature_option(GLFW "Include GLFW wrapper library." glfw)
dang_feature_option(LUA "Include Lua wrapper library." lua)
dang_feature_option(MATH "Include math library." math)
dang_feature_option(UTILS "Include utility library." utils)

if (BUILD_TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES tests)
endif()

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

include(dang_add_feature_subdirectory)
include(dang_check_dependency)
include(dang_find_package)
include(dang_include_finder)

dang_add_feature_subdirectory(UTILS dang-utils)
dang_add_feature_subdirectory(MATH dang-math UTILS)
dang_add_feature_subdirectory(GL dang-gl UTILS MATH)
dang_add_feature_subdirectory(GLFW dang-glfw UTILS MATH GL)
dang_add_feature_subdirectory(LUA dang-lua UTILS)
dang_add_feature_subdirectory(EXAMPLE dang-example UTILS MATH GL GLFW LUA)
dang_add_feature_subdirectory(DOC doc)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)

string(REPLACE ";" "\n" PACKAGE_FINDERS "${DANG_FINDERS}")
string(REPLACE ";" "\n" PACKAGE_DEPENDENCIES "${DANG_DEPENDENCIES}")
configure_package_config_file(
  cmake/${PROJECT_NAME}-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION "share/${PROJECT_NAME}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/dang-lib-config-version.cmake"
  COMPATIBILITY ExactVersion
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
)
