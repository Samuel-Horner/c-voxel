#ifndef WORLD
#define WORLD

#include "cglm/ivec3.h"
#include "chunk.c"
#include "engine.c"
#include "vector.c"
#include "misc.c"

#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

typedef struct World {
    Vector chunks;
    int render_distance;
    int world_height;
    ivec2 centre_pos;
} World;

#define worldSize(world) (world.render_distance * 2 * world.render_distance * 2 * world.world_height)
#define getIndexGivenRelativePos(world, rpos) ((rpos)[0] * 2 * (world).render_distance * (world).world_height + (rpos)[1] * 2 * (world).render_distance + (rpos)[2])

World *current_world = NULL;

Voxel getVoxel(ivec3 pos) {
    if (current_world == NULL) { printf("ERROR: Attempting to read voxel without setting current world.\n"); return EMPTY; }

    ivec3 chunk_pos;
    glm_ivec3_copy((ivec3) {divFloor(pos[0], CHUNK_SIZE), divFloor(pos[1], CHUNK_SIZE), divFloor(pos[2], CHUNK_SIZE)}, chunk_pos);

    // Chunk *chunk = NULL;
    // Chunk *current = current_world->chunks.vals;

    // // TODO: OPTIMISE THIS! like seriously it is not good, maybe hashmap? or a lookup table?
    // // Very costly here since we are not using a hashmap for the world, but a vector
    // for (size_t i = 0; i < current_world->chunks.size; i++) {
    //     if (glm_ivec3_eqv(current->chunk_pos, chunk_pos)) { chunk = current; break; }
    //     current++;
    // }
    
    // Solution assuming chunk pos is 0, 0, 0 and chunk vector is not edited.
    // This will need to be replaced at some point
    ivec3 relative_pos;
    glm_ivec3_add(chunk_pos, (ivec3) {current_world->render_distance, 0, current_world->render_distance}, relative_pos);
    if (relative_pos[0] < 0 || relative_pos[0] >= current_world->render_distance * 2 ||
        relative_pos[1] < 0 || relative_pos[1] >= current_world->world_height  ||
        relative_pos[2] < 0 || relative_pos[2] >= current_world->render_distance * 2) {
        return EMPTY;
    }
    
    int index = getIndexGivenRelativePos((*current_world), relative_pos);

    Chunk *chunk = vectorIndex(&current_world->chunks, index);
    if (!glm_ivec3_eqv(chunk_pos, chunk->chunk_pos)) { printf("(%d): (%d, %d, %d) [%d, %d, %d] -> (%d, %d, %d)\n", index, chunk_pos[0], chunk_pos[1], chunk_pos[2], relative_pos[0], relative_pos[1], relative_pos[2], chunk->chunk_pos[0], chunk->chunk_pos[1], chunk->chunk_pos[2]); }

    if (chunk == NULL) { return EMPTY; }

    return chunk->voxels[getVoxelIndex(relative_pos[0], relative_pos[1], relative_pos[2])];
}

void renderWorld(World *world, ProgramBundle *chunk_program, mat4 **model_pointer, GLFWwindow *window) {
    for (int i = 0; i < world->chunks.size; i++) {
        Chunk *chunk = vectorIndex(&world->chunks, i);
        *model_pointer = &(chunk->model);
        renderWithSSBOBundle(window, chunk_program, &(chunk->buffer_bundle), 0, chunk->buffer_bundle.length * VERTS_PER_FACE / VALS_PER_VOXEL);
    }
}

void populateWorld(World *world) {
    for (int x = -world->render_distance; x < world->render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->render_distance; z < world->render_distance; z++) {
                Chunk *new_chunk = createChunk((ivec3) {world->centre_pos[0] + x, y, world->centre_pos[1] + z}, 0);
                if (new_chunk == NULL) { printf("Error: NULL chunk at (%d %d %d).\n", x, y, z); continue; }

                vectorPush(&world->chunks, new_chunk);
                free(new_chunk);
            }
        }
    }

    current_world = world;
    for (int i = 0; i < world->chunks.size; i++) { createChunkMesh(vectorIndex(&world->chunks, i), &getVoxel); }
}

World createWorld(int render_distance, int world_height, ivec2 centre_pos) {
    World world;
    world.render_distance = render_distance;
    world.world_height = world_height;
    world.chunks = vectorInit(sizeof(Chunk), worldSize(world));
    glm_ivec2_copy(centre_pos, world.centre_pos);

    populateWorld(&world);

    return world;
}

#endif
