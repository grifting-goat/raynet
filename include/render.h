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
    uint32_t height;
    uint32_t width;

    uint32_t* colors;
} Texture;


typedef struct {

    double angle;
    double max_distance;
    double pos[3];

    double fov;
    uint32_t rays;
    double step;

    Texture wall_tex[4];

} Camera;




void load_texture(Texture* t, const char* path);

void clear_buf(Texture* t);

void rendercast_auto(Camera* cam, Map* m, Frame* f);

void rendercast(Camera* cam, Map* m, Frame* f);

void init_camera(Camera* cam, double fov, double max_distance, uint32_t rays);




#endif //RENDER_H