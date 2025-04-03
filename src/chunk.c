#ifndef CHUNK
#define CHUNK

#include "engine.c"
#include "vector.c"

#include "cglm/cglm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 16
// Max 16 since we then only need 4 bits per axis to represent position
#define VALS_PER_VOXEL 1
#define FACES_PER_VOXEL 6
#define VERTS_PER_FACE 6

typedef enum Voxel {
    OCCUPIED,
    EMPTY
} Voxel;

typedef struct VoxelData {
    uint x : 4;
    uint y : 4;
    uint z : 4;
    uint r : 4;
    uint g : 4;
    uint b : 4;
    uint flags : 8;
} VoxelData;

typedef struct Chunk {
    ivec3 chunk_pos;
    Voxel voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    SSBOBundle buffer_bundle;
    mat4 model;
} Chunk;

SSBOBundle createBuffers(Vector *voxel_data) {
    return createSSBOBundle(voxel_data->vals, voxel_data->size * voxel_data->item_size, voxel_data->size, 0);
}

#define getVoxelIndex(x, y, z) (x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z)
#define getOffsetIndex(index, x_offset, y_offset, z_offset) (index + (CHUNK_SIZE * CHUNK_SIZE * x_offset) + (CHUNK_SIZE * y_offset) + z_offset)
#define getOffsetIvec3(vec, x_offset, y_offset, z_offset) ((ivec3) {vec[0] + x_offset, vec[1] + y_offset, vec[2] + z_offset})

void printBinaryInt(int x) {
    for (int i = sizeof(x) * 8 - 1; i >= 0; i--) {
        putc(((x >> i) & 1) ? '1' : '0', stdout);       
    }
}

void createChunkMesh(Chunk *chunk, Voxel (*getVoxel)(ivec3 pos)) {
    Vector voxel_data = vectorInit(sizeof(VoxelData), VALS_PER_VOXEL);

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int voxel_index = getVoxelIndex(x, y, z);
                if (chunk->voxels[voxel_index] != OCCUPIED) { continue; }

                if (x != CHUNK_SIZE - 1 && y != CHUNK_SIZE - 1 && z != CHUNK_SIZE - 1 && x != 0 && y != 0 && z != 0) {
                    Voxel neighbours[6] = {
                        chunk->voxels[getOffsetIndex(voxel_index, 1, 0, 0)],
                        chunk->voxels[getOffsetIndex(voxel_index,-1, 0, 0)],
                        chunk->voxels[getOffsetIndex(voxel_index, 0, 1, 0)],
                        chunk->voxels[getOffsetIndex(voxel_index, 0,-1, 0)],
                        chunk->voxels[getOffsetIndex(voxel_index, 0, 0, 1)],
                        chunk->voxels[getOffsetIndex(voxel_index, 0, 0,-1)]
                    };

                    if (neighbours[0] == OCCUPIED &&
                        neighbours[1] == OCCUPIED &&
                        neighbours[2] == OCCUPIED &&
                        neighbours[3] == OCCUPIED &&
                        neighbours[4] == OCCUPIED &&
                        neighbours[5] == OCCUPIED) {
                        continue;
                    }

                } else {
                    ivec3 voxel_pos;
                    glm_ivec3_scale(chunk->chunk_pos, 16, voxel_pos);
                    glm_ivec3_add(voxel_pos, (ivec3) {x, y, z}, voxel_pos);

                    Voxel neighbours[6] = {
                        getVoxel(getOffsetIvec3(voxel_pos, 1, 0, 0)),
                        getVoxel(getOffsetIvec3(voxel_pos,-1, 0, 0)),
                        getVoxel(getOffsetIvec3(voxel_pos, 0, 1, 0)),
                        getVoxel(getOffsetIvec3(voxel_pos, 0,-1, 0)),
                        getVoxel(getOffsetIvec3(voxel_pos, 0, 0, 1)),
                        getVoxel(getOffsetIvec3(voxel_pos, 0, 0,-1))
                    };

                    if (neighbours[0] == OCCUPIED &&
                        neighbours[1] == OCCUPIED &&
                        neighbours[2] == OCCUPIED &&
                        neighbours[3] == OCCUPIED &&
                        neighbours[4] == OCCUPIED &&
                        neighbours[5] == OCCUPIED) {
                        continue;
                    }
                }

                // float voxel_coords[3] = {x, y, z};
                // vectorPushArray(&voxel_data, voxel_coords, 3);
                // 
                // float vert_color[3] = {(float) x / CHUNK_SIZE, (float) y / CHUNK_SIZE, (float) z / CHUNK_SIZE};
                // vectorPushArray(&voxel_data, vert_color, 3);

                // int color_r = x;
                // int color_g = y;
                // int color_b = z;

                // Voxel data Layout: LSB -> MSB
                // 4 bits: x in chunk
                // 4 bits: y in chunk
                // 4 bits: z in chunk
                // 4 bits: r * 16
                // 4 bits: g * 16
                // 4 bits: b * 16
                // uint32_t data =     (x & 0xF) | 
                //                     ((y & 0xF) << 4) | 
                //                     ((z & 0xF) << 8) | 
                //                     ((color_r & 0xF) << 12) | 
                //                     ((color_g & 0xF) << 16) | 
                //                     ((color_b & 0xF) << 20);


                // printf("x: %2d, y: %2d, z: %2d | ", x, y, z);
                // printBinaryInt(data);
                // printf(" \n");
                
                VoxelData data = {x, y, z, x, y, z, 0};
                vectorPush(&voxel_data, &data);
            }
        }
    }

    chunk->buffer_bundle = createBuffers(&voxel_data);

    freeVector(&voxel_data);
}

Chunk *createChunk(ivec3 chunk_pos, int verbose) {
    Chunk *chunk = malloc(sizeof(Chunk));
    if (chunk == NULL) { return NULL; }

    glm_ivec3_copy(chunk_pos, chunk->chunk_pos);

    int voxel_count = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    for (int i = 0; i < voxel_count; i++) {
        // if (i % 2 == 0) { chunk->voxels[i] = OCCUPIED; }
        // else { chunk->voxels[i] = EMPTY; }
        chunk->voxels[i] = OCCUPIED;
    }

    glm_mat4_dup(GLM_MAT4_IDENTITY, chunk->model);
    vec3 chunk_translation;
    chunk_translation[0] = (float) chunk->chunk_pos[0] * CHUNK_SIZE;
    chunk_translation[1] = (float) chunk->chunk_pos[1] * CHUNK_SIZE;
    chunk_translation[2] = (float) chunk->chunk_pos[2] * CHUNK_SIZE;
    glm_translate(chunk->model, chunk_translation);

    if (verbose) { printf("Created a chunk at (%d, %d, %d).\n", chunk->chunk_pos[0], chunk->chunk_pos[1], chunk->chunk_pos[2]); }

    return chunk;
}

#endif
