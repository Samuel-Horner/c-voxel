#ifndef ENGINE
#define ENGINE

#include <stdio.h>
#include <time.h>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef struct VertexArray {
    float *values;
    unsigned int size;
} VertexArray;

typedef struct IndexArray {
    int *values;
    unsigned int size;
} IndexArray;

typedef struct BufferBundle {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int length;
} BufferBundle;

typedef void (*UniformFunction)(unsigned int programID); 

typedef struct UniformBundle {
    unsigned int location;
    UniformFunction func;
} UniformBundle;

typedef struct ProgramBundle {
    unsigned int programID;
    struct {
        UniformBundle *values;
        unsigned int size;
    } uniforms;
} ProgramBundle;

GLFWwindow* initialiseWindow(unsigned int width, unsigned int height) {
    GLFWwindow* window;

    if (!glfwInit()){
        printf("GLFW didn't initialise.\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "C Voxel", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

int setupOpenGL(unsigned int width, unsigned int height) {
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to intialize OpenGL contex.\n");
        return 0;
    }

    printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glViewport(0, 0, width, height);
    glClearColor(1., 0., 1., 1.);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); 

    return 1;
}

int compileShader(unsigned int shader) {
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("ERROR: %s\n", info_log);
    }

    return success;
}

ProgramBundle createProgram(const char *vertex_source, const char *fragment_source) {
    unsigned int vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);

    if (!compileShader(vertex_shader)){ printf("Error compiling the vertex shader!\n"); }
    if (!compileShader(fragment_shader)){ printf("Error compiling the vertex shader!\n"); }

    ProgramBundle bundle;
    bundle.programID = glCreateProgram();

    glAttachShader(bundle.programID, vertex_shader);
    glAttachShader(bundle.programID, fragment_shader);
    glLinkProgram(bundle.programID);

    int success;
    glGetProgramiv(bundle.programID, GL_LINK_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetProgramInfoLog(bundle.programID, 512, NULL, info_log);
        printf("ERROR IN PROGRAM: %s\n", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return bundle;
}

int bindUniforms(ProgramBundle *program, char **uniform_names, UniformFunction *uniform_funcs, unsigned int uniform_count){
    program->uniforms.values = malloc(sizeof(UniformBundle) * uniform_count);
    if (program->uniforms.values == NULL) { return -1; }

    program->uniforms.size = uniform_count;
    for (unsigned int i = 0; i < uniform_count; i++) {
        program->uniforms.values[i].func = uniform_funcs[i];
        program->uniforms.values[i].location = glGetUniformLocation(program->programID, uniform_names[i]);
        printf("Binding uniform: \"%s\" to location %d\n", uniform_names[i], program->uniforms.values[i].location);
    }

    return 0;
}

BufferBundle createVAO(VertexArray vertices, IndexArray indices, unsigned int values_per_vertex, unsigned int value_count, unsigned int *value_split, unsigned int draw_mode) {
    BufferBundle bundle;
    glGenVertexArrays(1, &bundle.VAO);
    glGenBuffers(1, &bundle.VBO);
    glGenBuffers(1, &bundle.EBO);

    glBindVertexArray(bundle.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, bundle.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size, vertices.values, draw_mode);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bundle.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size, indices.values, draw_mode);

    unsigned int stride = values_per_vertex * sizeof(float);
    unsigned int offset = 0;
    for (unsigned int i = 0; i < value_count; i++) {
        glVertexAttribPointer(i, value_split[i], GL_FLOAT, GL_FALSE, stride, (const void *) (offset * sizeof(float)));
        printf("Created vertex attribute pointer {location:%d, stride:%d, offset:%p}\n", i, stride, (const void *) (offset * sizeof(float)));
        glEnableVertexAttribArray(i);
        offset += value_split[i];
    }

    bundle.length = indices.size;

    return bundle;
}

void applyUniforms(ProgramBundle *program) {
    for (unsigned int i = 0; i < program->uniforms.size; i++){
        program->uniforms.values[i].func(program->uniforms.values[i].location);
    }
}

void clearWindow(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render(GLFWwindow *window, ProgramBundle *program, BufferBundle *buffer) {
    glUseProgram(program->programID);
    applyUniforms(program);
    glBindVertexArray(buffer->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);

    glDrawElements(GL_TRIANGLES, buffer->length, GL_UNSIGNED_INT, 0);
}

void finishRender(GLFWwindow *window){
    glfwSwapBuffers(window);
}

#endif
