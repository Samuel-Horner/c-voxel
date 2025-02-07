#include "engine.c"
#include "shader.c"

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

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, 1); }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
    printf("Window resized to: %dx%d\n", width, height);
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

// NEVER FREE!
const float VERTICES[] = {
   -1.,-1., 0.,  0., 0., 0.,
    1.,-1., 0.,  1., 0., 0.,
   -1., 1., 0.,  0., 1., 0.,
    1., 1., 0.,  1., 1., 0.,
};

VertexArray vertices = {
    .values = VERTICES,
    .size = 24
};

const unsigned int INDICES[] = {
    0, 1, 2,
    3, 2, 1
};

IndexArray indices = {
    .values = INDICES,
    .size = 6
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

    // Fetch vertex and fragment source
    char *vertex_source = getShaderSource("./src/shaders/basic.vert");
    if (vertex_source == NULL) { printf("Error fecthing vertex shader.\n"); return -1; }

    char *fragment_source = getShaderSource("./src/shaders/basic.frag");
    if (fragment_source == NULL) { printf("Error fecthing fragment shader.\n"); return -1; }

    // Create buffer bundle
    unsigned int vertex_split[2] = {3, 3};
    BufferBundle bufferBundle = createVAO(vertices, indices, 6, 2, vertex_split);

    // Create program bundle
    ProgramBundle programBundle = createProgram(vertex_source, fragment_source);

    // Bind uniforms
    char *uniform_names[1] = {"resolution"};
    UniformFunction uniform_funcs[1] = {resolutionFunction};
    bindUniforms(&programBundle, uniform_names, uniform_funcs, 1);

    float last = getTimeStamp();

    while(!glfwWindowShouldClose(window)){
        // Calculate delta time
        float current = getTimeStamp();
        float delta_time = current - last;
        last = current;

        // Process events
        glfwPollEvents();
        processInput(window);

        // Apply uniforms and render
        applyUniforms(&programBundle);
        render(window, programBundle.programID, &bufferBundle);
    }

    glfwTerminate();

    return 0;
}
