#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

out vec3 VertexColor;

// uniform mat4 projection;
// uniform mat4 view;
layout (std140) uniform CamBlock {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    VertexColor = vec3(aCol);
}
