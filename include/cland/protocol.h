#ifndef CL_PROTOCOL_H
#define CL_PROTOCOL_H

#include "../simpcl/simpcl.h"

#define POOL_SIZE 128

typedef enum cl_message_t {
    KILL,
    ASK_NAME,
    RESP_NAME,
    ASK_INFO,
    RESP_INFO,
} cl_message_t;

typedef struct cl_pool_t {
    char buffer[POOL_SIZE];
    uint16_t head;
    uint16_t size;
    size_t count;
} cl_pool_t;

typedef struct cl_routine_t {
    uint16_t pid;
    size_t id;
    char *name;
    size_t spawn_time;
} cl_routine_t;

// composer
typedef struct cl_color_t {
    uint8_t r, g, b;
} cl_color_t;

typedef struct cl_pixel_t {
    uint8_t up;
    uint8_t down;
} cl_pixel_t;

typedef struct cl_draw_t {
    cl_color_t color;
    uint16_t x, y;
} cl_draw_t;

// shell
typedef struct cl_request_t {
    uint16_t service;
    uint16_t signal;
    void *source;
} cl_request_t;

typedef struct cl_shell_t {
    cl_routine_t *routines;
    cl_routine_t *proccess;
    uint16_t shell_pid;
} cl_shell_t;

#endif