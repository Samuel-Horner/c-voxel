#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <math.h>

typedef struct Camera {
    vec3 pos;
    vec3 dir;
    float pitch;
    float yaw;
    vec3 right;
    vec3 up;
    float sensitivity;
    float speed;
} Camera;

// Player Logic
Camera cam = {(vec3) {0., 0., -10.}, (vec3) {0., 0., 1.}, 0, 0, (vec3) {1., 0., 0.,}, (vec3) {0., 1., 0.}, 0.05, 10.};

void camera_rotate(float yaw, float pitch) {
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
}

double prev_x = 0;
double prev_y = 0;
void cursorPositionCallback(GLFWwindow *window, double x, double y) {
    camera_rotate((x - prev_x) * cam.sensitivity, -(y - prev_y) * cam.sensitivity);
    prev_x = x;
    prev_y = y;
    printf("cam dir: (%3.3f, %3.3f, %3.3f)\n", cam.dir[0], cam.dir[1], cam.dir[2]);
}

void viewFunction(unsigned int location){
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_look(cam.pos, cam.dir, cam.up, view);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float *) view);
}

void cameraMovement(GLFWwindow *window, float delta_time) {
    vec3 movement = GLM_VEC3_ZERO_INIT;
    vec3 inverse_movement = GLM_VEC3_ZERO_INIT;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { 
        glm_vec3_add(movement, cam.dir, movement);       
    } 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_sub(movement, cam.dir, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { 
        glm_vec3_add(movement, cam.right, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { 
        glm_vec3_sub(movement , cam.right, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { 
        glm_vec3_add(movement, cam.up, movement);       
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { 
        glm_vec3_sub(movement , cam.up, movement);       
    }

    glm_vec3_scale(movement, delta_time * cam.speed, movement);
    glm_vec3_add(cam.pos, movement, cam.pos);
}