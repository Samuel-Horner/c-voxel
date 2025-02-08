cmake_minimum_required(VERSION 3.11)
project(c_voxel)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall")

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  cglm
  GIT_REPOSITORY https://github.com/recp/cglm.git
  GIT_TAG v0.9.4
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(cglm)

add_executable(c_voxel 
    src/main.c
    src/glad/gl.h
    src/gl.c
)

target_link_libraries(c_voxel 
    m
    glfw
    cglm
)
