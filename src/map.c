

#include "map.h"

#include <stdio.h>
#include <stdlib.h>

Map create_map(uint32_t r_size, uint32_t c_size) {

    Chunk** new_map;
    new_map = calloc(r_size, sizeof(Chunk*));
    if(new_map == NULL) {
        printf("failed to allocate new map\n");
        return (Map){0};
    }

    for(uint32_t i = 0; i < r_size; i++) {
        new_map[i] = calloc(c_size, sizeof(Chunk));
        if(new_map[i] == NULL) {
            printf("failed to allocate hieght map\n"); 
            for(uint32_t k = 0; k < i; k++) {free(new_map[k]); new_map[k] = NULL;}
            free(new_map); 
            return (Map){0};
        }
    }


    Map m;
    m.chunk_map = new_map;
    m.x_size = r_size;
    m.y_size = c_size;

    return m;
}


