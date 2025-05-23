#ifndef PLAYER
#define PLAYER

#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include "engine.c"

#include <math.h>

#define DEFAULT_FOV 90.
#define ZOOMED_FOV 30.

typedef struct Camera {
    mat4 projection;
    vec3 pos;
    vec3 dir;
    vec3 forward;
    vec3 right;
    vec3 up;
    float pitch;
    float yaw;
    float sensitivity;
    float speed;
    float fov;
} Camera;

Camera cam;

void cameraRotate(float yaw, float pitch) {
    cam.yaw += yaw;
    cam.pitch += pitch;

    if (cam.pitch > 89.) { cam.pitch = 89.; }    
    if (cam.pitch < -89.) { cam.pitch = -89.; }    

    // From learn opengl
    cam.dir[0] = cos(glm_rad(cam.yaw)) * cos(glm_rad(cam.pitch));
    cam.dir[1] = sin(glm_rad(cam.pitch));
    cam.dir[2] = sin(glm_rad(cam.yaw)) * cos(glm_rad(cam.pitch));

    glm_vec3_normalize(cam.dir);

    glm_vec3_cross(cam.dir, (vec3) {0., 1., 0.}, cam.right);
    glm_vec3_normalize(cam.right);

    glm_vec3_cross(cam.right, cam.dir, cam.up);
    glm_vec3_normalize(cam.up);

    glm_vec3_cross(cam.right, (vec3) {0., -1., 0.}, cam.forward);
    glm_vec3_normalize(cam.forward);
}

double prev_x = 0;
double prev_y = 0;
void cursorPositionCallback(GLFWwindow *window, double x, double y) {
    cameraRotate((x - prev_x) * cam.sensitivity, -(y - prev_y) * cam.sensitivity);
    prev_x = x;
    prev_y = y;
}

void viewFunction(unsigned int location){
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_look(cam.pos, cam.dir, cam.up, view);
    glBufferSubData(GL_UNIFORM_BUFFER, location, 16 * OPENGL_N_SIZE, view);
}

void projectionFunction(unsigned int location){
    glBufferSubData(GL_UNIFORM_BUFFER, location, 16 * OPENGL_N_SIZE, cam.projection);
}

void calculateProjection(int window_width, int window_height) {
    glm_perspective(cam.fov, (float) window_width / (float) window_height, .1, 1000., cam.projection);
}

uint zoomed_in = 0;
void handleZoom(GLFWwindow *window, uint window_width, uint window_height) {
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (zoomed_in == 0) {
            cam.fov = glm_rad(ZOOMED_FOV);
            calculateProjection(window_width, window_height);
            zoomed_in = 1; 
        }
    } else {
        if (zoomed_in == 1) {
            cam.fov = glm_rad(DEFAULT_FOV);
            calculateProjection(window_width, window_height);
            zoomed_in = 0;
        }
    }
}

void cameraMovement(GLFWwindow *window, float delta_time, uint window_width, uint window_height) {
    handleZoom(window, window_width, window_height);
    vec3 movement = GLM_VEC3_ZERO_INIT;
    float speed = cam.speed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { 
        glm_vec3_add(movement, cam.forward, movement);       
    } 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_sub(movement, cam.forward, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { 
        glm_vec3_add(movement, cam.right, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { 
        glm_vec3_sub(movement , cam.right, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { 
        glm_vec3_add(movement, (vec3) {0., 1., 0.}, movement);       
    } 
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { 
        glm_vec3_sub(movement , (vec3) {0., 1., 0.}, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speed *= 4;
    }

    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, delta_time * speed, movement);
    glm_vec3_add(cam.pos, movement, cam.pos);

}

void initialisePlayerCamera(int window_width, int window_height, vec3 initial_pos) {
    cam.speed = 10.;
    cam.sensitivity = 0.05;
    cam.pitch = 0;
    cam.yaw = 90; // Start looking in pos_z dir
    cam.fov = glm_rad(DEFAULT_FOV);
    
    glm_vec3_copy(initial_pos, cam.pos);
    cameraRotate(0, 0);

    calculateProjection(window_width, window_height);
}

#endif
