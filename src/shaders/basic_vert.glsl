#version 460 core

struct VoxelData {
    float voxel_pos[3]; // Using arrays instead of vectors to tightly pack data.
    float voxel_col[3];
};

#define VERTEX_PULLING_SCALE 36

layout (std430, binding = 0) readonly buffer VoxelSSBO {
    VoxelData voxels[];
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
    vec3 pos = vec3(voxels[voxel_index].voxel_pos[0], voxels[voxel_index].voxel_pos[1], voxels[voxel_index].voxel_pos[2]);

    int vert_offset = gl_VertexID % VERTEX_PULLING_SCALE;
    int index = voxel_indices[vert_offset];
    pos += vert_positions[index];

    gl_Position = projection * view * model * vec4(pos, 1.0);
    VertexColor = vec3(voxels[voxel_index].voxel_col[0], voxels[voxel_index].voxel_col[1], voxels[voxel_index].voxel_col[2]);
}
