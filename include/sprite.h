#ifndef SPRITE_H
#define SPRITE_H

#include "render.h"
#include "stdbool.h"
#include "stdint.h"

typedef struct {

    bool locked;

    uint32_t tex_cnt;
    Texture* tex;

} Sprite;


Sprite create_sprite(const char* tex_path);

void clear_sprite(Sprite* s);

#endif //SPRITE_H