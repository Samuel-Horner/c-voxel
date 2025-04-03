#version 460 core

// struct VoxelData {
//     float voxel_pos[3]; // Using arrays instead of vectors to tightly pack data.
//     float voxel_col[3];
// };

#define VERTEX_PULLING_SCALE 36

// struct VoxelData {
//     uint x : 4;
//     uint y : 4;
//     uint z : 4;
//     uint r : 4;
//     uint g : 4;
//     uint b : 4;
//     uint flags : 8;
// };

layout (std430, binding = 0) readonly buffer VoxelSSBO {
    // VoxelData voxels[];
    uint voxels[];
};

const vec3 vert_positions[8] = vec3[8](
    vec3(1., 1., 1.),
    vec3(1., 0., 1.),
    vec3(0., 0., 1.),
    vec3(0., 1., 1.),
    vec3(1., 1., 0.),
    vec3(1., 0., 0.),
    vec3(0., 0., 0.),
    vec3(0., 1., 0.)
);

const int voxel_indices[36] = {
    3, 1, 0,
    3, 2, 1, 
    4, 5, 7, 
    5, 6, 7, 
    0, 5, 4, 
    0, 1, 5, 
    7, 6, 3, 
    6, 2, 3, 
    0, 4, 7, 
    3, 0, 7, 
    6, 5, 1, 
    6, 1, 2 
};

out vec3 VertexColor;

layout (std140) uniform CamBlock {
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

void main(){
    int voxel_index = gl_VertexID / VERTEX_PULLING_SCALE;
    uint data = voxels[voxel_index];
    uint x = (data) & 0xF ;
    uint y = (data >> 4) & 0xF;
    uint z = (data >> 8) & 0xF;
    vec3 pos = vec3(x, y, z);

    uint r = (data >> 12) & 0xF;
    uint g = (data >> 16) & 0xF;
    uint b = (data >> 20) & 0xF;
    vec3 col = vec3(r, g, b) / 16.;

    int vert_offset = gl_VertexID % VERTEX_PULLING_SCALE;
    int index = voxel_indices[vert_offset];
    pos += vert_positions[index];

    gl_Position = projection * view * model * vec4(pos, 1.0);
    VertexColor = col;
}
