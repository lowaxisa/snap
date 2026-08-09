#ifndef CL_PROTOCOL_H
#define CL_PROTOCOL_H

#include "../simpcl/simpcl.h"

#define POOL_SIZE 128

typedef struct cl_input_pool {
    char buffer[POOL_SIZE];
    uint16_t head;
    uint16_t size;
    size_t count;
} cl_input_pool;

typedef struct cl_routines {
    uint16_t pid;
    size_t id;
    char status;
    char *name;
    size_t spawn_time;
} cl_routines;

// composer
typedef struct cl_composer_color {
    uint8_t r, g, b;
} cl_composer_color;

typedef struct cl_composer_pixel {
    uint8_t up;
    uint8_t down;
} cl_composer_pixel;

typedef struct cl_composer_draw {
    cl_composer_color color;
    uint16_t x, y;
} cl_composer_draw;

#endif