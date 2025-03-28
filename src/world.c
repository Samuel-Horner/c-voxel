#ifndef WORLD
#define WORLD

#include "chunk.c"
#include "engine.c"
#include "vector.c"

#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

typedef struct World {
    Vector chunks;
    int render_distance;
    int world_height;
    ivec2 centre_pos;
} World;

#define worldSize(world) (world.render_distance * world.render_distance * world.world_height)

void renderWorld(World *world, ProgramBundle *chunk_program, mat4 **model_pointer, GLFWwindow *window) {
    for (int i = 0; i < world->chunks.size; i++) {
        Chunk *chunk = vectorIndex(&world->chunks, i);
        *model_pointer = &(chunk->model);
        renderWithSSBOBundle(window, chunk_program, &(chunk->buffer_bundle), 0, chunk->buffer_bundle.length * FACES_PER_VOXEL * VERTS_PER_FACE / VALS_PER_VOXEL);
    }
}

void populateWorld(World *world) {
    for (int x = -world->render_distance; x < world->render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->render_distance; z < world->render_distance; z++) {
                Chunk *new_chunk = createChunk((ivec3) {world->centre_pos[0] + x, y, world->centre_pos[1] + z});
                if (new_chunk == NULL) { printf("Error: NULL chunk at (%d %d %d).\n", x, y, z); continue; }

                vectorPush(&world->chunks, new_chunk);

                free(new_chunk);
            }
        }
    }
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
