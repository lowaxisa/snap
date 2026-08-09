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

#endif