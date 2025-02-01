cmake_minimum_required(VERSION 3.10)
project(c_voxel)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall")

add_executable(c_voxel 
    src/main.c
    src/glad/gl.h
    src/gl.c
)

target_link_libraries(c_voxel 
    glfw
)