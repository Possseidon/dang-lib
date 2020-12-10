cmake_minimum_required(VERSION 3.18)
project(dang-gl)

add_library(${PROJECT_NAME}
    src/Context/Context.cpp
    src/Context/State.cpp
    src/Context/StateTypes.cpp
    src/Image/PNGLoader.cpp
    src/Math/Transform.cpp
    src/Objects/FBO.cpp
    src/Objects/ObjectContext.cpp
    src/Objects/Program.cpp
    src/Objects/RBO.cpp
    src/Objects/VAO.cpp
    src/Rendering/Camera.cpp
    src/Rendering/Renderable.cpp
    src/pch.cpp
)

find_package(glad CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dang-math
    dang-utils
    glad::glad
    png
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include/dang-gl
  PUBLIC
    include
)
