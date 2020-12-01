cmake_minimum_required(VERSION 3.18)
project(dang-lua)

add_library(${PROJECT_NAME} INTERFACE)

find_package(Lua REQUIRED)

target_link_libraries(${PROJECT_NAME} 
  INTERFACE
    dang-utils
    ${LUA_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
  INTERFACE
    include
    ${LUA_INCLUDE_DIR}
)
