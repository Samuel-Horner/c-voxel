#version 330 core

out vec4 FragColor;

in vec4 gl_FragCoord;
in vec3 VertexColor;

uniform vec2 resolution;
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    // FragColor = vec4(uv.xyx, 1.);
    FragColor = vec4(VertexColor.xyz, 1.);
}