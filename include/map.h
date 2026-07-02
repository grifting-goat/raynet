#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include "common.h"



typedef struct {

    uint8_t blocks[Z_LEVEL];

} Chunk;

typedef struct {

    Chunk** chunk_map;
    uint32_t x_size;
    uint32_t y_size;

} Map;

Map create_map(uint32_t r_size, uint32_t c_size);



#endif  //MAP_H