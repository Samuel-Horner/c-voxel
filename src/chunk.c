#ifndef CHUNK
#define CHUNK

#include "cglm/cglm.h"
#include "engine.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 16
#define VERTS_PER_VOXEL 8
#define VALS_PER_VERT 6
#define TRIS_PER_VOXEL 12

const float VOXEL_VERTICES[][3] = {
    {1., 1., 1.},
    {1., 0., 1.},
    {0., 0., 1.},
    {0., 1., 1.},
    {1., 1., 0.},
    {1., 0., 0.},
    {0., 0., 0.},
    {0., 1., 0.}
};

const int VOXEL_INDICES[][3] = {
    {3, 1, 0}, // Back Face
    {3, 2, 1},
    {4, 5, 7}, // Front Face
    {5, 6, 7},
    {0, 5, 4}, // Right Face
    {0, 1, 5},
    {7, 6, 3}, // Left Face
    {6, 2, 3},
    {0, 4, 7}, // Top Face
    {3, 0, 7},
    {6, 5, 1}, // Bottom Face
    {6, 1, 2}
};

const float VOXEL_COLORS[][3] = {
    {1., 0., 0.},
    {0., 1., 0.},
    {0., 0., 1.},
    {1., 1., 1.},
    {.5, 0., 0.},
    {0., .5, 0.},
    {0., 0., .5},
    {.5, .5, .5}
};

typedef enum Voxel {
    EMPTY,
    OCCUPIED
} Voxel;

typedef struct Chunk {
    ivec3 chunk_pos;
    Voxel voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    vec3 voxel_colors[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    BufferBundle buffer_bundle;
    mat4 model;
} Chunk;

BufferBundle createBuffers(Voxel *voxels, vec3 *voxel_colors, int voxel_count) {
    BufferBundle bundle;

    VertexArray vertices;
    vertices.size = voxel_count * VERTS_PER_VOXEL * VALS_PER_VERT;
    vertices.values = malloc(sizeof(float) * vertices.size);
    if (vertices.values == NULL) { return bundle; }

    IndexArray indices;
    indices.size = voxel_count * TRIS_PER_VOXEL * 3;
    indices.values = malloc(sizeof(float) * indices.size);
    if (indices.values == NULL) { return bundle; }
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int voxel_index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
                // Add vertex positions
                // For now, all voxels are assumed to be occupied.
                vec3 voxel_coords;
                glm_vec3_copy((vec3) {x, y, z}, voxel_coords);
                // printf("Voxel Coords: (%f, %f, %f)\n", voxel_coords[0], voxel_coords[1], voxel_coords[2]);

                for (int i = 0; i < VERTS_PER_VOXEL; i++) {
                    vec3 vert_coords;
                    glm_vec3_add(VOXEL_VERTICES[i], voxel_coords, vert_coords);
                    memcpy(vertices.values + voxel_index * VERTS_PER_VOXEL * 6 + i * 6, vert_coords, sizeof(float) * 3);
    
                    vec3 vert_color;
                    // glm_vec3_copy(voxel_colors[voxel_index], vert_color);
                    glm_vec3_copy(VOXEL_COLORS[i], vert_color);
                    memcpy(vertices.values + voxel_index * VERTS_PER_VOXEL * 6 + i * 6 + 3, vert_color, sizeof(float) * 3);

                    // printf("Adding vert: (%f, %f, %f, %f, %f, %f) to %d\n", vert_coords[0], vert_coords[1], vert_coords[2], vert_color[0], vert_color[1], vert_color[2], voxel_index * VERTS_PER_VOXEL + i * 6);
                }

                for (int i = 0; i < TRIS_PER_VOXEL; i++) {
                    ivec3 tri_indices;
                    glm_ivec3_adds(VOXEL_INDICES[i], voxel_index * VERTS_PER_VOXEL, tri_indices);
                    memcpy(indices.values + voxel_index * TRIS_PER_VOXEL * 3 + i * 3, tri_indices, sizeof(int) * 3);

                    // printf("Adding tri: (%d, %d, %d) to %d\n", tri_indices[0], tri_indices[1], tri_indices[2], voxel_index * TRIS_PER_VOXEL + i * 3);
                }
            }
        }
    }

    // for (int i = 0; i < vertices.size / 6; i++) {
    //     printf("Vert: %d:(%3.3f, %3.3f, %3.3f, %1.3f, %1.3f, %1.3f)\n", i, vertices.values[i * 6], vertices.values[i * 6 + 1], vertices.values[i * 6 + 2], vertices.values[i * 6 + 3], vertices.values[i * 6 + 4], vertices.values[i * 6 + 5]);
    // }
    // 
    // for (int i = 0; i < indices.size / 3; i++) {
    //     printf("Tri: %d:(%d, %d, %d) at %d / %d\n", i, indices.values[i * 3], indices.values[i * 3 + 1], indices.values[i * 3 + 2], i * 3, indices.size);
    // }

    unsigned int vertex_split[2] = {3, 3};
    bundle = createVAO(vertices, indices, 6, 2, vertex_split);
    
    return bundle;
}

Chunk *createChunk(ivec3 chunk_pos) {
    Chunk *chunk = malloc(sizeof(Chunk));
    if (chunk == NULL) { return NULL; }

    glm_ivec3_copy(chunk_pos, chunk->chunk_pos);

    int voxel_count = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    for (int i = 0; i < voxel_count; i++) {
        chunk->voxels[i] = OCCUPIED;
        glm_vec3_dup(GLM_VEC3_ONE, chunk->voxel_colors[i]);
    }

    chunk->buffer_bundle = createBuffers(chunk->voxels, chunk->voxel_colors, voxel_count);

    glm_mat4_dup(GLM_MAT4_IDENTITY, chunk->model);
    vec3 chunk_translation;
    chunk_translation[0] = (float) chunk->chunk_pos[0] * CHUNK_SIZE;
    chunk_translation[1] = (float) chunk->chunk_pos[1] * CHUNK_SIZE;
    chunk_translation[2] = (float) chunk->chunk_pos[2] * CHUNK_SIZE;
    glm_translate(chunk->model, chunk_translation);

    return chunk;
}

#endif
