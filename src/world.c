#ifndef WORLD
#define WORLD

#include "cglm/ivec3.h"
#include "cglm/vec3.h"
#include "chunk.c"
#include "engine.c"
#include "vector.c"
#include "misc.c"

#include "cglm/cglm.h"
#include <GLFW/glfw3.h>

#include <stdatomic.h>
#include <stdio.h>
#include <math.h>

typedef struct World {
    Vector chunks;
    int render_distance;
    int lod_render_distance;
    int world_height;
    int seed;
    ivec2 centre_pos;
} World;

#define worldSize(world) ((world).lod_render_distance * 2 * (world).lod_render_distance * 2 * (world).world_height)
#define getIndexGivenXYZ(world, x, y, z) ((x + (world.lod_render_distance)) * 2 * (world).lod_render_distance * (world).world_height + (y) * 2 * (world).lod_render_distance + (z + (world).lod_render_distance))
#define getIndexGivenRelativePos(world, rpos) ((rpos)[0] * 2 * (world).lod_render_distance * (world).world_height + (rpos)[1] * 2 * (world).lod_render_distance + (rpos)[2])

World *current_world = NULL;

Voxel getVoxel(ivec3 pos) {
    ivec3 chunk_pos = {divFloor(pos[0], CHUNK_SIZE), divFloor(pos[1], CHUNK_SIZE), divFloor(pos[2], CHUNK_SIZE)};
    ivec3 relative_pos = {current_world->lod_render_distance, 0, current_world->lod_render_distance};
    glm_ivec3_add(chunk_pos, relative_pos, relative_pos);

    if (relative_pos[0] < 0 || relative_pos[0] >= current_world->lod_render_distance * 2 ||
        relative_pos[1] < 0 || relative_pos[1] >= current_world->world_height            ||
        relative_pos[2] < 0 || relative_pos[2] >= current_world->lod_render_distance * 2   ){
        return EMPTY;
    }

    Chunk *chunk = vectorIndex(&current_world->chunks, getIndexGivenRelativePos((*current_world), relative_pos));
    
    ivec3 pos_in_chunk = {mod(pos[0], CHUNK_SIZE), mod(pos[1], CHUNK_SIZE), mod(pos[2], CHUNK_SIZE)};
    
    Voxel voxel = chunk->voxels[getVoxelIndex(pos_in_chunk[0], pos_in_chunk[1], pos_in_chunk[2])];

    return voxel;
}

Voxel getLodVoxel(ivec3 pos) {
    ivec3 chunk_pos = {divFloor(pos[0], CHUNK_SIZE), divFloor(pos[1], CHUNK_SIZE), divFloor(pos[2], CHUNK_SIZE)};
    ivec3 relative_pos = {current_world->lod_render_distance, 0, current_world->lod_render_distance};
    glm_ivec3_add(chunk_pos, relative_pos, relative_pos);

    if (relative_pos[0] < 0 || relative_pos[0] >= current_world->lod_render_distance * 2 ||
        relative_pos[1] < 0 || relative_pos[1] >= current_world->world_height            ||
        relative_pos[2] < 0 || relative_pos[2] >= current_world->lod_render_distance * 2   ){
        return EMPTY;
    }

    Chunk *chunk = vectorIndex(&current_world->chunks, getIndexGivenRelativePos((*current_world), relative_pos));
    
    ivec3 pos_in_chunk = {mod(pos[0], CHUNK_SIZE), mod(pos[1], CHUNK_SIZE), mod(pos[2], CHUNK_SIZE)};
    glm_ivec3_divs(pos_in_chunk, chunk->lod_scale, pos_in_chunk);
    glm_ivec3_scale(pos_in_chunk, chunk->lod_scale, pos_in_chunk);
    
    Voxel voxel = chunk->voxels[getVoxelIndex(pos_in_chunk[0], pos_in_chunk[1], pos_in_chunk[2])];

    return voxel;
}

void renderWorld(World *world, ProgramBundle *chunk_program, Chunk **current_chunk_pointer, GLFWwindow *window, vec3 view_dir, vec3 cam_pos) {
    for (int i = 0; i < world->chunks.size; i++) {
        Chunk* chunk = vectorIndex(&world->chunks, i);
        // View direction culling
        //  Getting weird behaviour, related to camera position.
        // vec3 real_chunk_pos = {chunk->chunk_pos[0] * CHUNK_SIZE, chunk->chunk_pos[1] * CHUNK_SIZE, chunk->chunk_pos[2] * CHUNK_SIZE};
        // vec3 chunk_to_cam; glm_vec3_sub(real_chunk_pos, cam_pos, chunk_to_cam); glm_vec3_norm(chunk_to_cam);
        // float dot = glm_vec3_dot(view_dir, chunk_to_cam);
        // 
        // printf("CP: %f %f %f, VD: %f %f %f, RCP %f %f %f, CTC: %f %f %f, DOT: %f\n",
        //     cam_pos[0], cam_pos[1], cam_pos[2],
        //     view_dir[0], view_dir[1], view_dir[2],
        //     real_chunk_pos[0], real_chunk_pos[1], real_chunk_pos[2],
        //     chunk_to_cam[0], chunk_to_cam[1], chunk_to_cam[2],
        //     dot
        // );
        // 
        // if (dot < 0) { return; }

        *current_chunk_pointer = chunk;
        renderWithSSBOBundle(window, chunk_program, &(chunk->buffer_bundle), 0, chunk->buffer_bundle.length * VERTS_PER_FACE / VALS_PER_VOXEL);
    }
}

void populateWorld(World *world) {
    int total_world_size = worldSize(*world);

    float start = getTimeStamp();

    printf("\n");

    for (int x = -world->lod_render_distance; x < world->lod_render_distance; x++) {
        for (int y = 0; y < world->world_height; y++) {
            for (int z = -world->lod_render_distance; z < world->lod_render_distance; z++) {
                printf("\rCreating chunks: %04.1f", (((float) getIndexGivenXYZ((*world), x, y, z)) / total_world_size) * 100);
                float rx = fabs(x + .5),
                      rz = fabs(z + .5); // Needed to be floats to center world at 0.5 0.5

                int lod = max(rx / world->render_distance, rz / world->render_distance);

                Chunk *new_chunk = createChunk((ivec3) {world->centre_pos[0] + x, y, world->centre_pos[1] + z}, 0, world->world_height, lod);
                if (new_chunk == NULL) { printf("Error: NULL chunk at (%d %d %d).\n", x, y, z); continue; }

                vectorPush(&world->chunks, new_chunk);
                free(new_chunk);
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
    world.seed = seed;
    glm_ivec2_copy(centre_pos, world.centre_pos);

    populateWorld(&world);
    printf("Finished world population.\n");

    return world;
}

#endif
