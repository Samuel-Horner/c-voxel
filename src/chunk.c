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
// #define FACES_PER_VOXEL 6
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
    uint face_id: 3;
    uint flags : 5;
} VoxelData;

// Face table:
// 0 : x+
// 1 : x-
// 2 : y+
// 3 : y-
// 4 : z+
// 5 : z-

typedef struct Chunk {
    ivec3 chunk_pos;
    Voxel voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    SSBOBundle buffer_bundle;
    mat4 model;
    int lod; // Level of detail, for CHUNK_SIZE 16 we have 0 (16 x 16), 1 (8 x 8), 2 (4 x 4), 3 (2, 2), 4 (1, 1)
    int lod_scale; // LOD scale = pow(2, lod)
} Chunk;

SSBOBundle createBuffers(Vector *voxel_data) {
    return createSSBOBundle(voxel_data->vals, voxel_data->size * voxel_data->item_size, voxel_data->size, 0);
}

#define getVoxelIndex(x, y, z) (x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z)
#define getOffsetIndex(index, x_offset, y_offset, z_offset) (index + (CHUNK_SIZE * CHUNK_SIZE * x_offset) + (CHUNK_SIZE * y_offset) + z_offset)
#define getOffsetIvec3(vec, x_offset, y_offset, z_offset) ((ivec3) {vec[0] + x_offset, vec[1] + y_offset, vec[2] + z_offset})
#define getVoxelPos(x, y, z, chunk_pos) {x + CHUNK_SIZE * chunk_pos[0], y + CHUNK_SIZE * chunk_pos[1], z + CHUNK_SIZE * chunk_pos[2]}

void generateNewChunk(Chunk *chunk, int world_height) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            // vec2 voxel_pos_2d = {x + CHUNK_SIZE * chunk->chunk_pos[0], z + CHUNK_SIZE * chunk->chunk_pos[2]};
            // glm_vec2_adds(voxel_pos_2d, 0.5, voxel_pos_2d);
            // glm_vec2_divs(voxel_pos_2d, CHUNK_SIZE, voxel_pos_2d);
            for (int y = 0; y < CHUNK_SIZE; y++) {
                // float height_map = 0.5 * (float) world_height + 0.5 * (float) world_height * glm_perlin_vec2(voxel_pos_2d);
                vec3 voxel_pos = getVoxelPos(x, y, z, chunk->chunk_pos);
                glm_vec3_adds(voxel_pos, 0.5, voxel_pos);
                glm_vec3_divs(voxel_pos, CHUNK_SIZE, voxel_pos);
                float cut_off = glm_perlin_vec3(voxel_pos);

                int voxel_index = getVoxelIndex(x, y, z);
                if (cut_off > 0) { chunk->voxels[voxel_index] = OCCUPIED; }
                // if (y + CHUNK_SIZE * chunk->chunk_pos[1] <= height_map) { chunk->voxels[voxel_index] = OCCUPIED; }
                else { chunk->voxels[voxel_index] = EMPTY; }
                // chunk->voxels[voxel_index] = OCCUPIED;
            }
        }
    }
}

uint opaqueVoxel(Voxel voxel) {
    return voxel == OCCUPIED;
}

// Issues:
// - Doesnt account for scaling up, aka lod 1 -> lod 2 can see a voxel when it is not rendered - Shouldnt be a problem though since player will never see this face
// Still strugling with some down-scaling issues (see rd 1, wh 1, 44, -33), only happens in -x downscaling direction (+x quads)
void checkVoxelNeighbours(Chunk* chunk, Voxel (*getVoxel)(ivec3 pos), int x, int y, int z, uint voxel_index, ivec3 voxel_pos, uint *neighbours) {
    if (chunk->lod_scale == 0) {
        if (x + chunk->lod_scale < CHUNK_SIZE) { neighbours[0] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, chunk->lod_scale, 0, 0)]); }
        else                                   { neighbours[0] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,        chunk->lod_scale, 0, 0))); }
        if (x - chunk->lod_scale >= 0)         { neighbours[1] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index,-chunk->lod_scale, 0, 0)]); }
        else                                   { neighbours[1] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,       -chunk->lod_scale, 0, 0))); }
        if (y + chunk->lod_scale < CHUNK_SIZE) { neighbours[2] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, chunk->lod_scale, 0)]); }
        else                                   { neighbours[2] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,        0, chunk->lod_scale, 0))); }
        if (y - chunk->lod_scale >= 0)         { neighbours[3] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0,-chunk->lod_scale, 0)]); }
        else                                   { neighbours[3] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,        0,-chunk->lod_scale, 0))); }
        if (z + chunk->lod_scale < CHUNK_SIZE) { neighbours[4] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, 0, chunk->lod_scale)]); }
        else                                   { neighbours[4] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,        0, 0, chunk->lod_scale))); }
        if (z - chunk->lod_scale >= 0)         { neighbours[5] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, 0,-chunk->lod_scale)]); }
        else                                   { neighbours[5] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,        0, 0,-chunk->lod_scale))); }
    } else {
        // To account for scaling down, we sample 4 times per face.
        // We dont need to account for upscaling, since players will never see those faces
        int next_lod = chunk->lod_scale / 2;

        if (x + chunk->lod_scale < CHUNK_SIZE) { neighbours[0] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, chunk->lod_scale, 0, 0)]); }
        else { 
            neighbours[0] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, chunk->lod_scale, 0,        0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, chunk->lod_scale, next_lod, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, chunk->lod_scale, 0,        next_lod))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, chunk->lod_scale, next_lod, next_lod))); 
        }
        if (x - chunk->lod_scale >= 0)         { neighbours[1] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index,-chunk->lod_scale, 0, 0)]); }
        else { 
            neighbours[1] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,-chunk->lod_scale, 0,        0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,-chunk->lod_scale, next_lod, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,-chunk->lod_scale, 0,        next_lod))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos,-chunk->lod_scale, next_lod, next_lod))); 
        }
        if (y + chunk->lod_scale < CHUNK_SIZE) { neighbours[2] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, chunk->lod_scale, 0)]); }
        else { 
            neighbours[2] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        chunk->lod_scale, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, chunk->lod_scale, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        chunk->lod_scale, next_lod))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, chunk->lod_scale, next_lod))); 
        }
        if (y - chunk->lod_scale >= 0)         { neighbours[3] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0,-chunk->lod_scale, 0)]); }
        else { 
            neighbours[3] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,       -chunk->lod_scale, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod,-chunk->lod_scale, 0       ))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,       -chunk->lod_scale, next_lod))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod,-chunk->lod_scale, next_lod))); 
        }
        if (z + chunk->lod_scale < CHUNK_SIZE) { neighbours[4] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, 0, chunk->lod_scale)]); }
        else { 
            neighbours[4] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        0       , chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, 0       , chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        next_lod, chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, next_lod, chunk->lod_scale))); 
        }
        if (z - chunk->lod_scale >= 0)         { neighbours[5] = opaqueVoxel(chunk->voxels[getOffsetIndex(voxel_index, 0, 0,-chunk->lod_scale)]); }
        else { 
            neighbours[5] = opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        0       ,-chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, 0       ,-chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, 0,        next_lod,-chunk->lod_scale))) &&
                            opaqueVoxel(getVoxel(getOffsetIvec3(voxel_pos, next_lod, next_lod,-chunk->lod_scale))); 
        }
    }
}

void createChunkMesh(Chunk *chunk, Voxel (*getVoxel)(ivec3 pos)) {
    Vector voxel_data = vectorInit(sizeof(VoxelData), VALS_PER_VOXEL);
    int voxels_per_lod_block = (chunk->lod_scale * chunk->lod_scale * chunk->lod_scale);


    for (int x = 0; x < CHUNK_SIZE; x += chunk->lod_scale) {
        for (int y = 0; y < CHUNK_SIZE; y += chunk->lod_scale) {
            for (int z = 0; z < CHUNK_SIZE; z += chunk->lod_scale) {
                int voxel_index = getVoxelIndex(x, y, z);
                
                if (chunk->voxels[voxel_index] != OCCUPIED) { continue; }

                ivec3 voxel_pos = getVoxelPos(x, y, z, chunk->chunk_pos);
                
                uint neighbours[6];
                checkVoxelNeighbours(chunk, getVoxel, x, y, z, voxel_index, voxel_pos, neighbours);
                
                if (neighbours[0] &&
                    neighbours[1] &&
                    neighbours[2] &&
                    neighbours[3] &&
                    neighbours[4] &&
                    neighbours[5]   ) {
                    continue;
                } 
                
                ivec3 voxel_color = {x, y, z};
                glm_ivec3_adds(voxel_color, chunk->lod_scale / 2, voxel_color);

                for (int face = 0; face < 6; face++) {
                    if (neighbours[face]) { continue; } 
                    
                    VoxelData data = {x, y, z, voxel_color[0], voxel_color[1], voxel_color[2], face, 0};
                    vectorPush(&voxel_data, &data);
                }
            }
        }
    }

    chunk->buffer_bundle = createBuffers(&voxel_data);

    freeVector(&voxel_data);
}

Chunk *createChunk(ivec3 chunk_pos, int verbose, int world_height, int lod) {
    Chunk *chunk = malloc(sizeof(Chunk));
    if (chunk == NULL) { return NULL; }

    glm_ivec3_copy(chunk_pos, chunk->chunk_pos);
    chunk->lod = lod;
    chunk->lod_scale = pow(2, lod);

    generateNewChunk(chunk, world_height);

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
