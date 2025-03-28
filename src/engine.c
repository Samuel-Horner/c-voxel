#ifndef ENGINE
#define ENGINE

#include <emmintrin.h>
#include <stdio.h>
#include <time.h>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <unistd.h>

#define OPENGL_N_SIZE 4 // sizeof(float) for the GPU

typedef struct {
    float *values;
    unsigned int size;
} VertexArray;

typedef struct {
    int *values;
    unsigned int size;
} IndexArray;

typedef struct {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int length;
} VertexBufferBundle;

typedef struct {
    unsigned int SSBO;
    unsigned int length;
} SSBOBundle;

typedef void (*UniformFunction)(unsigned int programID); 

typedef struct UniformBundle {
    unsigned int location;
    UniformFunction func;
} UniformBundle;

struct UniformArray {
    UniformBundle *values;
    unsigned int size;
};

typedef struct {
    unsigned int UBO;
    struct UniformArray uniforms;
    unsigned int size;
} UniformBufferBundle;

typedef struct ProgramBundle {
    unsigned int programID;
    struct UniformArray uniforms;
} ProgramBundle;

unsigned int empty_vao;

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
    
    glGenVertexArrays(1, &empty_vao); // Empty VAO needed to avoid GL_INVALID_OPERATION https://www.khronos.org/opengl/wiki/Vertex_Rendering/Rendering_Failure

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

VertexBufferBundle createVertexBufferBundle(VertexArray vertices, IndexArray indices, unsigned int values_per_vertex, unsigned int value_count, unsigned int *value_split, unsigned int draw_mode, int verbose) {
    VertexBufferBundle bundle;
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
        if (verbose) { printf("Created vertex attribute pointer {location:%d, stride:%d, offset:%p}\n", i, stride, (const void *) (offset * sizeof(float))); }
        glEnableVertexAttribArray(i);
        offset += value_split[i];
    }

    bundle.length = indices.size;
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return bundle;
}

void updateBuffer(GLenum buffer_type, unsigned int buffer_id, void *data, size_t data_size, size_t data_length) {
    glBindBuffer(buffer_type, buffer_id);
    glBufferSubData(buffer_type, 0, data_size * data_length, data);
    glBindBuffer(buffer_type, 0);
}

void applyUniforms(ProgramBundle *program) {
    for (unsigned int i = 0; i < program->uniforms.size; i++){
        program->uniforms.values[i].func(program->uniforms.values[i].location);
    }
}

UniformBufferBundle createUniformBufferBundle(UniformFunction *uniform_funcs, unsigned int *uniform_sizes, unsigned int uniform_count, unsigned int bind_point, int verbose) {
    // https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    // See uniform buffers
    UniformBufferBundle bundle;

    bundle.uniforms.values = malloc(sizeof(UniformBundle) * uniform_count);
    if (bundle.uniforms.values == NULL) { return bundle; }

    bundle.uniforms.size = uniform_count;
    int offset = 0;
    for (int i = 0; i < uniform_count; i++) {
        bundle.uniforms.values[i].func = uniform_funcs[i];
        bundle.uniforms.values[i].location = offset;
        offset += uniform_sizes[i];
    }

    glGenBuffers(1, &bundle.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, bundle.UBO);

    bundle.size = 0;
    for (int i = 0; i < uniform_count; i++) {
        bundle.size += uniform_sizes[i];
    }

    glBufferData(GL_UNIFORM_BUFFER, bundle.size, NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, bundle.UBO);

    if (verbose) { 
        printf("Created uniform buffer bundle {location:%d, size:%d, bound:%d, locations:{", bundle.UBO, bundle.size, bind_point);
        for (int i = 0; i < bundle.uniforms.size; i++) {
            printf("%d", bundle.uniforms.values[i].location);
            if (i < bundle.uniforms.size - 1) { printf(", "); }
        }
        printf("}}\n");
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return bundle;
}

// Assign to things shader program
void bindUniformBufferBundle(ProgramBundle *program, UniformBufferBundle *bundle, char *name, unsigned int bind_point) {
    glUniformBlockBinding(program->programID, glGetUniformBlockIndex(program->programID, name), bind_point);
}

// Apply functions to set values
void applyUniformBufferBundle(UniformBufferBundle *bundle) {
    glBindBuffer(GL_UNIFORM_BUFFER, bundle->UBO);
    for (unsigned int i = 0; i < bundle->uniforms.size; i++){
        bundle->uniforms.values[i].func(bundle->uniforms.values[i].location);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

SSBOBundle createSSBOBundle(void *values, size_t data_size, unsigned int length, int verbose) {
    SSBOBundle bundle;

    glGenBuffers(1, &bundle.SSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bundle.SSBO);

    glBufferData(GL_SHADER_STORAGE_BUFFER, data_size, values, GL_DYNAMIC_DRAW);
    if (verbose) { printf("Created new SBBO %d sized %zu B (%d vals).\n", bundle.SSBO, data_size, length); }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    bundle.length = length;

    return bundle;
}

void bindSBBOBundle(SSBOBundle *bundle, unsigned int bind_point) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_point, bundle->SSBO);
}

void clearWindow(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render(GLFWwindow *window, ProgramBundle *program, VertexBufferBundle *buffer) {
    glUseProgram(program->programID);
    applyUniforms(program);
    glBindVertexArray(buffer->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);

    glDrawElements(GL_TRIANGLES, buffer->length, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void renderWithSSBOBundle(GLFWwindow *window, ProgramBundle *program, SSBOBundle *bundle, unsigned int bind_point, unsigned int draw_amount) {
    glUseProgram(program->programID);
    applyUniforms(program);

    glBindVertexArray(empty_vao);
    bindSBBOBundle(bundle, bind_point);
    
    glDrawArrays(GL_TRIANGLES, 0, draw_amount);

    glBindVertexArray(0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void finishRender(GLFWwindow *window) {
    glfwSwapBuffers(window);
}

void freeProgram(ProgramBundle *program) {
    free(program->uniforms.values);
}

void freeUniformBuffer(UniformBufferBundle *bundle) {
    free(bundle->uniforms.values);
}

#endif
