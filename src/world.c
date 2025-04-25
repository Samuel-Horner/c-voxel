#ifndef WORLD
#define WORLD

#include "cglm/ivec3.h"
#include "chunk.c"
#include "engine.c"
#include "vector.c"
#include "misc.c"

#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdatomic.h>
#include <stdio.h>

typedef struct World {
    Vector chunks;
    int render_distance;
    int world_height;
    int seed;
    ivec2 centre_pos;
} World;

#define worldSize(world) (world.render_distance * 2 * world.render_distance * 2 * world.world_height)
#define getIndexGivenRelativePos(world, rpos) ((rpos)[0] * 2 * (world).render_distance * (world).world_height + (rpos)[1] * 2 * (world).render_distance + (rpos)[2])

World *current_world = NULL;

Voxel getVoxel(ivec3 pos) {
    ivec3 chunk_pos = {divFloor(pos[0], CHUNK_SIZE), divFloor(pos[1], CHUNK_SIZE), divFloor(pos[2], CHUNK_SIZE)};
    ivec3 relative_pos = {current_world->render_distance, 0, current_world->render_distance};
    glm_ivec3_add(chunk_pos, relative_pos, relative_pos);

    if (relative_pos[0] < 0 || relative_pos[0] >= current_world->render_distance * 2 ||
        relative_pos[1] < 0 || relative_pos[1] >= current_world->world_height        ||
        relative_pos[2] < 0 || relative_pos[2] >= current_world->render_distance * 2  ){
        return EMPTY;
    }

    ivec3 pos_in_chunk = {mod(pos[0], CHUNK_SIZE), mod(pos[1], CHUNK_SIZE), mod(pos[2], CHUNK_SIZE)};

    Chunk *chunk = vectorIndex(&current_world->chunks, getIndexGivenRelativePos((*current_world), relative_pos));
    
    Voxel voxel = chunk->voxels[getVoxelIndex(pos_in_chunk[0], pos_in_chunk[1], pos_in_chunk[2])];

    return voxel;
}

void renderWorld(World *world, ProgramBundle *chunk_program, mat4 **model_pointer, GLFWwindow *window) {
    for (int i = 0; i < world->chunks.size; i++) {
        Chunk *chunk = vectorIndex(&world->chunks, i);
        *model_pointer = &(chunk->model);
        renderWithSSBOBundle(window, chunk_program, &(chunk->buffer_bundle), 0, chunk->buffer_bundle.length * VERTS_PER_FACE / VALS_PER_VOXEL);
    }
}

void populateWorld(World *world) {
    // int index = 0;
    for (int x = -world->render_distance; x < world->render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->render_distance; z < world->render_distance; z++) {
                Chunk *new_chunk = createChunk((ivec3) {world->centre_pos[0] + x, y, world->centre_pos[1] + z}, 0, world->seed);
                if (new_chunk == NULL) { printf("Error: NULL chunk at (%d %d %d).\n", x, y, z); continue; }

                vectorPush(&world->chunks, new_chunk);
                
                // ivec3 relative_pos;
                // glm_ivec3_add(new_chunk->chunk_pos, (ivec3) {world->render_distance, 0, world->render_distance}, relative_pos);
                // 
                // printf("Created chunk at: %d %d %d (RP: %d %d %d) I: %d RI: %d\n", 
                //        new_chunk->chunk_pos[0], new_chunk->chunk_pos[1], new_chunk->chunk_pos[2],
                //        relative_pos[0], relative_pos[1], relative_pos[2],
                //        index, getIndexGivenRelativePos((*world), relative_pos)
                // );
                // index++;
                
                free(new_chunk);

            }
        }
    }

    current_world = world;
    for (int i = 0; i < world->chunks.size; i++) { createChunkMesh(vectorIndex(&world->chunks, i), &getVoxel); }
}

World createWorld(int render_distance, int world_height, ivec2 centre_pos, int seed) {
    World world;
    world.render_distance = render_distance;
    world.world_height = world_height;
    world.chunks = vectorInit(sizeof(Chunk), worldSize(world));
    world.seed = seed;
    glm_ivec2_copy(centre_pos, world.centre_pos);

    populateWorld(&world);

    return world;
}

#endif
