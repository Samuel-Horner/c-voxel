#ifndef NOISE
#define NOISE

#include "cglm/noise.h"
#include <cglm/cglm.h>

float layered2DNoise(vec2 pos, int octaves, float persistance, float octave_scale) {
    float noise = 0;
    float frequency = 1;
    float factor = 1;

    for (int i = 0; i < octaves; i++) {
        glm_vec2_scale(pos, frequency, pos);
        noise += glm_perlin_vec2(pos) * factor;
        frequency *= octave_scale;
        factor *= persistance;
    }

    return noise;
}

#endif
