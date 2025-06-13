#ifndef WORLD
#define WORLD

#include "cglm/ivec3.h"
#include "cglm/vec3-ext.h"
#include "cglm/vec3.h"
#include "chunk.c"
#include "engine.c"
#include "vector.c"
#include "misc.c"

#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef struct World {
    Vector chunks;
    int render_distance;
    int lod_render_distance;
    int world_height;
    ivec2 centre_pos;
    // Debug
    int chunk_render_count;
} World;

#define worldSize(world) ((world).lod_render_distance * 2 * (world).lod_render_distance * 2 * (world).world_height)
#define getIndexGivenXYZ(world, x, y, z) ((x + (world.lod_render_distance)) * 2 * (world).lod_render_distance * (world).world_height + (y) * 2 * (world).lod_render_distance + (z + (world).lod_render_distance))
#define getIndexGivenRelativePos(world, rpos) ((rpos)[0] * 2 * (world).lod_render_distance * (world).world_height + (rpos)[1] * 2 * (world).lod_render_distance + (rpos)[2])

World *current_world = NULL;

Chunk *getChunk(World *world, ivec3 pos) {
    // Assumes static chunks loaded
    ivec3 relative_pos = {world->lod_render_distance, 0, world->lod_render_distance};
    glm_ivec3_add(pos, relative_pos, relative_pos);

    if (relative_pos[0] < 0 || relative_pos[0] >= world->lod_render_distance * 2 ||
        relative_pos[1] < 0 || relative_pos[1] >= world->world_height            ||
        relative_pos[2] < 0 || relative_pos[2] >= world->lod_render_distance * 2   ){
        return NULL;
    }
    
    return vectorIndex(&world->chunks, getIndexGivenRelativePos((*world), relative_pos));
    //
    // Slow
    // for (int i = 0; i < world->chunks.size; i++) {
    //     Chunk *chunk = vectorIndex(&world->chunks, i);
    //     if (glm_ivec3_eqv(chunk->chunk_pos, pos)) {
    //         return chunk;
    //     }
    // }
    // return NULL;
}

Voxel getVoxel(ivec3 pos) {
    ivec3 chunk_pos = {divFloor(pos[0], CHUNK_SIZE), divFloor(pos[1], CHUNK_SIZE), divFloor(pos[2], CHUNK_SIZE)};
    Chunk *chunk = getChunk(current_world, chunk_pos);
    
    if (chunk == NULL) { return EMPTY; }

    ivec3 pos_in_chunk = {mod(pos[0], CHUNK_SIZE), mod(pos[1], CHUNK_SIZE), mod(pos[2], CHUNK_SIZE)};
    
    int voxel_index = getVoxelIndex(pos_in_chunk[0], pos_in_chunk[1], pos_in_chunk[2]);
    Voxel voxel = chunk->voxels[voxel_index];

    return voxel;
}

#define IN_CHUNK_OFFSET (CHUNK_SIZE / 2.)

#define CHUNK_MIN_CULL_DISTANCE (2 * CHUNK_SIZE) * (2 * CHUNK_SIZE)
void renderWorld(World *world, ProgramBundle *chunk_program, Chunk **current_chunk_pointer, GLFWwindow *window, vec3 view_dir, vec3 cam_pos) {
    world->chunk_render_count = 0;

    for (int i = 0; i < world->chunks.size; i++) {
        Chunk* chunk = vectorIndex(&world->chunks, i);
        // View direction culling
        vec3 real_chunk_pos = {chunk->chunk_pos[0] * CHUNK_SIZE + IN_CHUNK_OFFSET, chunk->chunk_pos[1] * CHUNK_SIZE + IN_CHUNK_OFFSET, chunk->chunk_pos[2] * CHUNK_SIZE + IN_CHUNK_OFFSET};
        vec3 chunk_to_cam; glm_vec3_sub(real_chunk_pos, cam_pos, chunk_to_cam);
        
        float cam_distance = chunk_to_cam[0] * chunk_to_cam[0] + chunk_to_cam[1] * chunk_to_cam[1] + chunk_to_cam[2] * chunk_to_cam[2];
        if (cam_distance > CHUNK_MIN_CULL_DISTANCE) {
            float dot = glm_vec3_dot(view_dir, chunk_to_cam);
            if (dot < 0) { continue; }
        }

        *current_chunk_pointer = chunk;
        renderWithSSBOBundle(window, chunk_program, &(chunk->buffer_bundle), 0, chunk->buffer_bundle.length * VERTS_PER_FACE / VALS_PER_VOXEL);
        world->chunk_render_count++;
    }
}

int getChunkLOD(World *world, ivec3 chunk_pos, ivec3 center_pos) {
    float rx = fabs(chunk_pos[0] - center_pos[0] + .5),
          rz = fabs(chunk_pos[2] - center_pos[2] + .5); // Needed to be floats to center world at 0.5 0.5
    
    return max(rx / world->render_distance, rz / world->render_distance);
}

void loadChunk(World *world, ivec3 pos, int lod) {
    Chunk *new_chunk = createChunk((ivec3) {world->centre_pos[0] + pos[0], pos[1], world->centre_pos[1] + pos[2]}, 0, world->world_height, lod);
    if (new_chunk == NULL) { printf("Error: NULL chunk at (%d %d %d).\n", pos[0], pos[1], pos[2]); return; }

    vectorPush(&world->chunks, new_chunk);
    free(new_chunk);
}

ivec3 current_cam_chunk = {0, 0, 0};
void tickWorld(World *world, vec3 cam_pos) {
    current_world = world;
    ivec3 new_cam_chunk= {divFloor(cam_pos[0], CHUNK_SIZE), divFloor(cam_pos[1], CHUNK_SIZE), divFloor(cam_pos[2], CHUNK_SIZE)};

    if (glm_ivec3_eqv(current_cam_chunk, new_cam_chunk)) { return; }
    glm_ivec3_copy(new_cam_chunk, current_cam_chunk);

    for (int x = -world->lod_render_distance; x < world->lod_render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->lod_render_distance; z < world->lod_render_distance; z++) {
                Chunk *chunk = getChunk(world, (ivec3) {x, y, z});

                if (chunk == NULL) { continue; }

                int lod = getChunkLOD(world, chunk->chunk_pos, new_cam_chunk);
                
                if (lod > 4) { continue; } // HOTIFX

                if (lod != chunk->lod) {
                    updateChunkLOD(chunk, lod, &getVoxel);
                }
            }
        }
    }
}

void populateWorld(World *world) {
    int total_world_size = worldSize(*world);

    float start = getTimeStamp();

    for (int x = -world->lod_render_distance; x < world->lod_render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->lod_render_distance; z < world->lod_render_distance; z++) {
                printf("\rCreating chunks: %04.1f", (((float) getIndexGivenXYZ((*world), x, y, z)) / total_world_size) * 100);

                int lod = getChunkLOD(world, (ivec3) {x, y, z}, (ivec3) {0, 0, 0});
                loadChunk(world, (ivec3) {x, y, z}, lod);
            }
        }
    }

    float start_meshing = getTimeStamp();

    printf("\nPopulation took %f seconds\n", start_meshing - start);

    current_world = world;
    for (int i = 0; i < world->chunks.size; i++) {
        printf("\rMeshing Chunks: %04.1f", ((float) i / world->chunks.size) * 100);
        createChunkMesh(vectorIndex(&world->chunks, i), &getVoxel); 
    }

    printf("\nMeshing took %f seconds\n", getTimeStamp() - start_meshing);
}

World createWorld(int render_distance, int world_height, ivec2 centre_pos, int seed) {
    World world;
    world.render_distance = render_distance;
    world.lod_render_distance = render_distance * (log2(CHUNK_SIZE) + 1);
    world.world_height = world_height;
    world.chunks = vectorInit(sizeof(Chunk), worldSize(world));
    glm_ivec2_copy(centre_pos, world.centre_pos);
    // Debug
    world.chunk_render_count = 0;

    populateWorld(&world);
    printf("Finished world population.\n");

    return world;
}

#endif
