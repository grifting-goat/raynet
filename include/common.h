#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define Z_LEVEL 8
#define MAX_DEPTH 8

#define PI 3.14159265359

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))

typedef enum {

    BLOCK_VOID = 0,
    BRICK,
    DIRT,
    OBSIDIAN

} Types;

static inline uint32_t get_color(Types t) {
    switch (t) {
        case BRICK:
            return 0xFFFF0022;
        case DIRT:
            return 0xFF472111;
        case OBSIDIAN:
            return 0xFF15061A;
        default:
            return 0xFFFF00FF;
    }
}

//keep between -180 and 180
static inline double fix_angle(double angle) {

    while (angle > PI) {
        angle -= 2.0 * PI;
    }

    while (angle < -PI) {
        angle += 2.0 * PI;
    }


    return angle;
}

static inline void fix_angle_inplace(double* angle) {

    while (*angle > PI) {
        *angle -= 2.0 * PI;
    }

    while (*angle < -PI) {
        *angle += 2.0 * PI;
    }
}

#endif //COMMON_H