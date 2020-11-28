cmake_minimum_required(VERSION 3.18)
project(dang-gl)

add_library(${PROJECT_NAME}
    src/Camera.cpp
    src/Context.cpp
    src/FBO.cpp
    src/GLFW.cpp
    src/Input.cpp
    src/Monitor.cpp
    src/ObjectContext.cpp
    src/pch.cpp
    src/PNGLoader.cpp
    src/Program.cpp
    src/RBO.cpp
    src/Renderable.cpp
    src/State.cpp
    src/StateTypes.cpp
    src/Transform.cpp
    src/VAO.cpp
    src/Window.cpp
)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    dang-math
    dang-utils
    glad::glad
    glfw
    png
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include/dang-gl
  PUBLIC
    include
)