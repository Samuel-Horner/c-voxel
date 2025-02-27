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
    ivec2 centre_pos;
} World;

#define worldSize(world) (world.render_distance * world.render_distance)

void renderWorld(World *world, ProgramBundle *chunk_program, mat4 **model_pointer, GLFWwindow *window) {
    for (int i = 0; i < world->chunks.size; i++) {
        Chunk *chunk = vectorIndex(&world->chunks, i);
        *model_pointer = &(chunk->model);
        render(window, chunk_program, &(chunk->buffer_bundle));
    }
}

void populateWorld(World *world) {
    for (int x = 0; x <= world->render_distance * 2; x++) {
        for (int z = 0; z <= world->render_distance * 2; z++) {
            vectorPush(&world->chunks, createChunk((ivec3) {world->centre_pos[0] + x - world->render_distance, 0, world->centre_pos[1] + z - world->render_distance}));
        }
    }
}

World createWorld(int render_distance, ivec2 chunk_pos) {
    World world;
    world.render_distance = render_distance;
    world.chunks = vectorInit(sizeof(Chunk), worldSize(world));
    glm_ivec2_copy(chunk_pos, world.centre_pos);

    populateWorld(&world);

    return world;
}

#endif
