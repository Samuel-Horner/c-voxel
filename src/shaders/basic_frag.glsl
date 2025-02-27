#version 330 core

out vec4 FragColor;

in vec4 gl_FragCoord;
in vec3 VertexColor;

void main() {
    FragColor = vec4(VertexColor.xyz, 1.);
}
