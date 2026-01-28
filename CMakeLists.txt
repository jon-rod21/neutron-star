cmake_minimum_required(VERSION 3.10)
project(neutron-star)
add_definitions(-DSHADER_DIR="${CMAKE_SOURCE_DIR}/src/shaders/")

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(libs/glad/include)

add_executable(neutron
    src/main.cpp
    libs/glad/src/glad.c
)

target_link_libraries(neutron
    glfw
    OpenGL::GL
)
