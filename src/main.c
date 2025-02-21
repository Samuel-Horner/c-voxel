#ifndef MAIN
#define MAIN

#include "cglm/cglm.h"
#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "engine.c"
#include "shader.c"
#include "player.c"
#include "chunk.c"
#include "vector.c"
#include "text.c"

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
    updateTextProjection(width, height);
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

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);} 
    else { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

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

    // Initlialise Font Stuff
    initFreeType();
    initFreeTypeFace("./fonts/JetBrainsMonoNerdFont-Regular.ttf", window_width, window_height);
    generateFreeTypeTexture();

    ProgramBundle text_program = createTextProgram(window_width, window_height);
    BufferBundle text_buffer_bundle = createTextBuffer();

    // Fetch vertex and fragment source
    char *chunk_vertex_source = getShaderSource("./src/shaders/basic_vert.glsl");
    if (chunk_vertex_source == NULL) { printf("Error fecthing vertex shader.\n"); return -1; }

    char *chunk_fragment_source = getShaderSource("./src/shaders/basic_frag.glsl");
    if (chunk_fragment_source == NULL) { printf("Error fecthing fragment shader.\n"); return -1; }

    // Create program bundle
    ProgramBundle chunk_program = createProgram(chunk_vertex_source, chunk_fragment_source);

    // Bind uniforms
    #define UNIFORM_COUNT 5
    char *uniform_names[UNIFORM_COUNT] = {"resolution", "time", "model", "view", "projection"};
    UniformFunction uniform_funcs[UNIFORM_COUNT] = {resolutionFunction, timeFunction, modelFunction, viewFunction, projectionFunction};
    bindUniforms(&chunk_program, uniform_names, uniform_funcs, UNIFORM_COUNT);

    // Initlialise Camera
    initialisePlayerCamera(window_width, window_height);

    // Create Chunk
    Vector chunks = vectorInit(sizeof(Chunk), 1);

    // #define CHUNK_COUNT 1
    // for (int x = -CHUNK_COUNT; x <= CHUNK_COUNT; x++) {
    //     for (int z = -CHUNK_COUNT; z <= CHUNK_COUNT; z++) {
    //         vectorPush(&chunks, createChunk((ivec3) {x, 0, z}));
    //         printf("Created chunk at (%d, 0, %d)\n", x, z);
    //     }
    // }

    vectorPush(&chunks, createChunk((ivec3) {0, 0, 0}));

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
        for (int i = 0; i < chunks.size; i++) {
            Chunk *chunk = vectorIndex(&chunks, i);
            model_pointer = &(chunk->model);
            render(window, &chunk_program, &(chunk->buffer_bundle));
        }
        
        char *debug_string;
        if(!asprintf(&debug_string, "FPS: %.3f POS: (%.3f, %.3f, %.3f)", 1. / delta_time, cam.pos[0], cam.pos[1], cam.pos[2])) { printf("ERROR: Error creating fps string!\n"); return -1; }
        renderText(&text_buffer_bundle, &text_program, debug_string, (vec2) {10, 10}, 0.15);
        free(debug_string);

        finishRender(window);
    }

    vectorFree(&chunks);

    glfwTerminate();

    return 0;
}

#endif
