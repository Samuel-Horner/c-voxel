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

find_package(Freetype REQUIRED)

add_executable(c_voxel 
    src/main.c
    src/glad/gl.h
    src/gl.c
)

target_link_libraries(c_voxel 
    m
    glfw
    cglm
    ${FREETYPE_LIBRARIES}
)

target_include_directories(c_voxel PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
)

# Testing
set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1 --errors-for-leak-kinds=definite --tool=memcheck --show-leak-kinds=definite")
include(CTest)
add_test(NAME c_voxel COMMAND c_voxel 
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
