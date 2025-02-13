#include "cglm/cglm.h"

#include "engine.c"
#include "shader.c"
#include "player.c"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

unsigned int window_width = 500;
unsigned int window_height = 500;

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
mat4 model = GLM_MAT4_IDENTITY_INIT;
void modelFunction(unsigned int location){
    glUniformMatrix4fv(location, 1, GL_FALSE, (float *) model);
}

// Projection Stuff
mat4 projection = GLM_MAT4_IDENTITY_INIT;
void projectionFunction(unsigned int location){
    glUniformMatrix4fv(location, 1, GL_FALSE, (float *) projection);
}

// Call Backs
void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
    glm_perspective(glm_rad(45.), (float) window_width / (float) window_height, .1, 100., projection);
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


// NEVER FREE!
const float VERTICES[] = {
    1., 1., 1.,  1., 0., 0.,
    1.,-1., 1.,  0., 1., 0.,
   -1.,-1., 1.,  0., 0., 1.,
   -1., 1., 1.,  1., 1., 1.,
    1., 1.,-1.,  .5, 0., 0.,
    1.,-1.,-1.,  0., .5, 0.,
   -1.,-1.,-1.,  0., 0., .5,
   -1., 1.,-1.,  .5, .5, .5
};

VertexArray vertices = {
    .values = VERTICES,
    .size = 48
};

const unsigned int INDICES[] = {
    0, 1, 3, // Back Face
    1, 2, 3,
    4, 5, 7, // Front Face
    5, 6, 7,
    4, 5, 0, // Right Face
    5, 1, 0,
    7, 6, 3, // Left Face
    6, 2, 3,
    0, 4, 7, // Top Face
    3, 0, 7,
    1, 5, 6, // Bottom Face
    2, 1, 6
};

IndexArray indices = {
    .values = INDICES,
    .size = 36
};

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

    // Create buffer bundle
    unsigned int vertex_split[2] = {3, 3};
    BufferBundle buffer_bundle = createVAO(vertices, indices, 6, 2, vertex_split);

    // Create program bundle
    ProgramBundle program_bundle = createProgram(vertex_source, fragment_source);

    // Bind uniforms
    #define UNIFORM_COUNT 5
    char *uniform_names[UNIFORM_COUNT] = {"resolution", "time", "model", "view", "projection"};
    UniformFunction uniform_funcs[UNIFORM_COUNT] = {resolutionFunction, timeFunction, modelFunction, viewFunction, projectionFunction};
    bindUniforms(&program_bundle, uniform_names, uniform_funcs, UNIFORM_COUNT);

    glm_perspective(glm_rad(45.), (float) window_width / (float) window_height, .1, 100., projection);
    
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
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {printf("w");}

        // Apply uniforms and render
        //glm_rotate(model, delta_time, (vec3) {.5, 1., 0});
        applyUniforms(&program_bundle);
        render(window, program_bundle.programID, &buffer_bundle);
    }

    glfwTerminate();

    return 0;
}
