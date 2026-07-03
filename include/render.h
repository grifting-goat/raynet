#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "map.h"
#include "disp.h"
#include <stdint.h>


typedef struct {

    Types list[MAX_DEPTH];
    uint8_t depth;

} HitList;


typedef struct {

    double roll;
    double pitch;
    double max_distance;
    double pos[3];

    double fov;
    uint32_t rays;
    double step;

    Texture wall_tex[4];

} Camera;


void rendercast_auto(Camera* cam, Map* m, Frame* f);

void rendercast(Camera* cam, Map* m, Frame* f);

void init_camera(Camera* cam, double fov, double max_distance, uint32_t rays);




#endif //RENDER_H