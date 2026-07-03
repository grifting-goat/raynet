#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


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

typedef struct {
    uint32_t height;
    uint32_t width;

    uint32_t* colors;
} Texture;



static uint32_t shade_u32(uint32_t color, uint8_t k) {
    uint32_t a = color & 0xFF000000;
    uint32_t r = ((color >> 16) & 0xFF) * k / 255;
    uint32_t g = ((color >> 8) & 0xFF) * k / 255;
    uint32_t b = (color & 0xFF) * k / 255;
    return a | (r << 16) | (g << 8) | b;
}

static uint32_t lerp_u32(uint32_t a, uint32_t b, uint8_t k) {
    uint32_t aa = a & 0xFF000000;

    uint32_t ar = (a >> 16) & 0xFF;
    uint32_t ag = (a >> 8) & 0xFF;
    uint32_t ab = a & 0xFF;

    uint32_t br = (b >> 16) & 0xFF;
    uint32_t bg = (b >> 8) & 0xFF;
    uint32_t bb = b & 0xFF;

    uint32_t r = (ar * (255u - k) + br * k) / 255u;
    uint32_t g = (ag * (255u - k) + bg * k) / 255u;
    uint32_t bl = (ab * (255u - k) + bb * k) / 255u;

    return aa | (r << 16) | (g << 8) | bl;
}


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


static void load_texture(Texture* t, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "could not open %s\n", path); exit(1); }

    char magic[3] = {0};
    int width, height, maxval;
    fscanf(f, "%2s", magic);
    fscanf(f, "%d %d %d", &width, &height, &maxval);
    fgetc(f);                        

    t->width = (uint32_t)width;
    t->height = (uint32_t)height;
    t->colors = malloc((size_t)width * height * sizeof(uint32_t));

    uint8_t* rgb = malloc((size_t)width * height * 3);
    fread(rgb, (size_t)width * height * 3, 1, f);
    fclose(f);
    
    for (int i = 0; i < width * height; i++) {
        uint8_t r = rgb[i*3+0], g = rgb[i*3+1], b = rgb[i*3+2];
        t->colors[i] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    free(rgb);

    printf("loaded: %s", path);
}

static void clear_buf(Texture* t) {
    t->height = 0;
    t->width = 0;
    
    free(t->colors);
    t->colors = NULL;
}



typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

} Color;

static Color parse_color(uint32_t color) {
    Color c;
    c.red = (uint8_t)((color >> 16) & 0xFF);
    c.green = (uint8_t)((color >> 8) & 0xFF);
    c.blue = (uint8_t)(color & 0xFF);

    return c;
}

static uint32_t pack_color(Color c) {
    uint32_t color = 0xFF000000;
    color |= ((uint32_t)c.red << 16);
    color |= ((uint32_t)c.green << 8);
    color |= (uint32_t)c.blue;

    return color;
}

static Color lerp_color(Color a, Color b, double alpha) {
    Color out;

    if (alpha < 0.0) { alpha = 0.0; }
    if (alpha > 1.0) { alpha = 1.0; }

    out.red = (uint8_t)((double)a.red + ((double)b.red - (double)a.red) * alpha);
    out.green = (uint8_t)((double)a.green + ((double)b.green - (double)a.green) * alpha);
    out.blue = (uint8_t)((double)a.blue + ((double)b.blue - (double)a.blue) * alpha);

    return out;

}

//for lighting 
static Color shade_color(Color c, double intensity) {
    if(intensity > 1.0) {intensity = 1.0;}
    if(intensity < 0.0) {intensity = 0.0;}

    c.red = (uint8_t)((double)c.red * intensity);
    c.green = (uint8_t)((double)c.green * intensity);
    c.blue = (uint8_t)((double)c.blue * intensity);

    return c;
}

#endif //COMMON_H