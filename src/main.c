#include <stdio.h>
#include "glad/gl.h"
#include <GLFW/glfw3.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const char *VERTEX_SHADER_SOURCE = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *FRAGMENT_SHADER_SOURCE = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, 1); }
}

GLFWwindow* initialiseWindow() {
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

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "C Voxel", NULL, NULL);
    glfwMakeContextCurrent(window);

    return window;
}

int setupOpenGL() {
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to intialize OpenGL contex.\n");
        return 0;
    }

    printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(1., 0., 1., 1.);
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

unsigned int createProgram(const char *vertex_source, const char *fragment_source) {
    unsigned int vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);

    if (!compileShader(vertex_shader)){ printf("Error compiling the vertex shader!"); }
    if (!compileShader(fragment_shader)){ printf("Error compiling the vertex shader!"); }

    unsigned int shader_program;
    shader_program = glCreateProgram();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        printf("ERROR IN PROGRAM: %s\n", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

unsigned int createVAO(float *vertices) {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0); 

    return VAO;
}

void render(GLFWwindow* window, unsigned int program, unsigned int VAO) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 3);


    glfwSwapBuffers(window);
}

int main() {
    GLFWwindow* window = initialiseWindow();
    if (window == NULL) { return -1; }

    if (!setupOpenGL()) { return -1; }

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    unsigned int VAO = createVAO(vertices);

    unsigned int program = createProgram(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);
    glUseProgram(program); 

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        processInput(window);
        render(window, program, VAO);
    }

    glfwTerminate();

    return 0;
}