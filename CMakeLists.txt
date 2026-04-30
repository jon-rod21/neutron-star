cmake_minimum_required(VERSION 3.14)
project(neutron-star)
add_definitions(-DSHADER_DIR="${CMAKE_SOURCE_DIR}/src/shaders/")

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

# GLM (header-only)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

include_directories(libs/glad/include)

add_executable(neutron
    src/main.cpp
    libs/glad/src/glad.c
)

target_link_libraries(neutron
    glfw
    glm::glm
    OpenGL::GL
)
