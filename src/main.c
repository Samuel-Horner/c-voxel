#ifndef MAIN
#define MAIN

#include "cglm/cglm.h"
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "engine.c"
#include "shader.c"
#include "player.c"
#include "chunk.c"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>


unsigned int window_width = 1280;
unsigned int window_height = 800;

void makeWindowFullscreen(GLFWwindow *window) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor); 

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

void makeWindowWindowed(GLFWwindow *window) {
    glfwSetWindowMonitor(window, NULL, 0, 0, window_width, window_height, 0);
}

float getTimeStamp() {
    struct timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec) != 0) {
        int errsv = errno;
        printf("ERROR: in clock_gettime. errno: %d\n", errno);
        return -1.;
    }

    return (float) spec.tv_sec + spec.tv_nsec / 1.0e9;
}

void resolutionFunction(unsigned int location){
    glUniform2f(location, (float) window_width, (float) window_height);
}

float start_time;
void timeFunction(unsigned int location){
    glUniform1f(location, getTimeStamp() - start_time);
}

// Model Stuff
mat4 *model_pointer = NULL;
void modelFunction(unsigned int location){
    if (model_pointer == NULL) { printf("ERROR: Attempted to render with NULL model pointer\n"); return; }
    glUniformMatrix4fv(location, 1, GL_FALSE, (float *) *model_pointer);
}

// Call Backs
void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
    calculateProjection(width, height);
    printf("Window resized to: %dx%d\n", width, height);
}

unsigned int f_11_down = 0; // Needed for only down press functionality
void processInput(GLFWwindow *window, float delta_time) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, 1); }

    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) { 
        if (f_11_down) { return; }
        GLFWmonitor *monitor = glfwGetWindowMonitor(window);
        if (monitor == NULL) { makeWindowFullscreen(window); }
        else { makeWindowWindowed(window); }
        f_11_down = 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_F11) != GLFW_PRESS) { f_11_down = 0; }

    cameraMovement(window, delta_time);
}

int main() {
    // Check CWD!
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) { printf("Current working dir: %s\n", cwd); }
    else { printf("Couldnt fetch CWD."); return -1; }

    // Initialise window and glsl
    GLFWwindow* window = initialiseWindow(window_width, window_height);
    if (window == NULL) { return -1; }
    if (!setupOpenGL(window_width, window_height)) { return -1; }
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // Fetch vertex and fragment source
    char *vertex_source = getShaderSource("./src/shaders/basic.vert");
    if (vertex_source == NULL) { printf("Error fecthing vertex shader.\n"); return -1; }

    char *fragment_source = getShaderSource("./src/shaders/basic.frag");
    if (fragment_source == NULL) { printf("Error fecthing fragment shader.\n"); return -1; }

    // Create program bundle
    ProgramBundle program_bundle = createProgram(vertex_source, fragment_source);

    // Bind uniforms
    #define UNIFORM_COUNT 5
    char *uniform_names[UNIFORM_COUNT] = {"resolution", "time", "model", "view", "projection"};
    UniformFunction uniform_funcs[UNIFORM_COUNT] = {resolutionFunction, timeFunction, modelFunction, viewFunction, projectionFunction};
    bindUniforms(&program_bundle, uniform_names, uniform_funcs, UNIFORM_COUNT);

 
    // Initlialise Camera
    initialisePlayerCamera(window_width, window_height);

    // Create Chunk
    #define CHUNK_COUNT 5
    Chunk *chunks[] = {createChunk((ivec3) {0, 0, 0}), createChunk((ivec3) {0, 0, 2}), createChunk((ivec3) {0, 0, -2}), createChunk((ivec3) {2, 0, 0}), createChunk((ivec3) {-2, 0, 0})};

    float last = getTimeStamp();
    start_time = last;

    while(!glfwWindowShouldClose(window)){
        // Calculate delta time
        float current = getTimeStamp();
        float delta_time = current - last;
        last = current;

        // Process events
        glfwPollEvents();
        processInput(window, delta_time);

        clearWindow(window);

        // Apply uniforms and render
        for (int i = 0; i < CHUNK_COUNT; i++) {
            model_pointer = &(chunks[i]->model);
            applyUniforms(&program_bundle);
            render(window, program_bundle.programID, &(chunks[i]->buffer_bundle));
        }
        
        finishRender(window);
    }


    for (int i = 0; i < CHUNK_COUNT; i++) { free(chunks[i]); }

    glfwTerminate();

    return 0;
}

#endif
