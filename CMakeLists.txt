cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
    CACHE STRING "Vcpkg toolchain file")

# Adds a `WITH_DANG_<FEATURE>` option and, if enabled, adds it to the vcpkg features to get dependencies for.
macro(DANG_FEATURE_OPTION feature_name feature_description)
  option(WITH_DANG_${feature_name} ${feature_description} ON)
  if(WITH_DANG_${feature_name})
    list(APPEND VCPKG_MANIFEST_FEATURES ${ARGN})
  endif()
endmacro()

dang_feature_option(BOX2D "Include Box2D wrapper library." box2d)
dang_feature_option(DOC "Include documentation.")
dang_feature_option(EXAMPLE "Include examples.")
dang_feature_option(GL "Include OpenGL library." gl)
dang_feature_option(GLFW "Include GLFW wrapper library." glfw)
dang_feature_option(LUA "Include Lua wrapper library." lua)
dang_feature_option(MATH "Include math library." math)
dang_feature_option(UTILS "Include utility library." utils)

# Cannot include(CTest) before project, but VCPKG_MANIFEST_FEATURES must be filled before.
option(BUILD_TESTING "" ON)
if(BUILD_TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES tests)
endif()

project(
  dang-lib
  VERSION 0.1.0
  LANGUAGES CXX)

include(CTest)
include(GenerateExportHeader)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FindVLD)

include(dang_add_feature_subdirectory)
include(dang_check_dependencies)
include(dang_find_package)
include(dang_include_finder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(DANG_WERROR "Whether to treat warnings as errors.")

if(MSVC)
  add_compile_options(/W4 /utf-8 /bigobj $<$<BOOL:${DANG_WERROR}>:/WX>)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options("SHELL:-Xclang -Wno-missing-braces" "SHELL:-Xclang -Wno-logical-op-parentheses"
                        "SHELL:-Xclang -fconstexpr-steps -Xclang 2500000")
  else()
    add_compile_options(/constexpr:steps2000000)
  endif()

  # On Windows, .exe and .dll need to be in the same directory.
  if(BUILD_SHARED_LIBS)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/runtime)
  endif()
else()
  add_compile_options(-Wall -Wextra -pedantic $<$<BOOL:${DANG_WERROR}>:-Werror>)
  add_compile_options(-Wno-missing-braces -Wno-logical-op-parentheses)
endif()

dang_add_feature_subdirectory(BOX2D dang-box2d)
dang_add_feature_subdirectory(DOC doc)
dang_add_feature_subdirectory(EXAMPLE dang-example)
dang_add_feature_subdirectory(GL dang-gl)
dang_add_feature_subdirectory(GLFW dang-glfw)
dang_add_feature_subdirectory(LUA dang-lua)
dang_add_feature_subdirectory(MATH dang-math)
dang_add_feature_subdirectory(UTILS dang-utils)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

string(REPLACE ";" "\n" PACKAGE_FINDERS "${_DANG_FINDERS}")
string(REPLACE ";" "\n" PACKAGE_DEPENDENCIES "${_DANG_DEPENDENCIES}")
configure_package_config_file(
  cmake/${PROJECT_NAME}-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION "share/${PROJECT_NAME}")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/dang-lib-config-version.cmake" COMPATIBILITY ExactVersion)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake" DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
