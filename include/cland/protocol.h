#ifndef snp_PROTOCOL_H
#define snp_PROTOCOL_H

#include "../simpcl/simpcl.h"

#define POOL_SIZE 128

typedef enum snp_message_t {
    KILL,
    ASK_NAME,
    RESP_NAME,
    ASK_INFO,
    RESP_INFO,
} snp_message_t;

typedef struct snp_pool_t {
    char buffer[POOL_SIZE];
    uint16_t head;
    uint16_t size;
    size_t count;
} snp_pool_t;

typedef struct snp_routine_t {
    uint16_t pid;
    size_t id;
    char *name;
    size_t spawn_time;
} snp_routine_t;

// composer
typedef struct snp_color_t {
    uint8_t r, g, b;
} snp_color_t;

typedef struct snp_pixel_t {
    uint8_t up;
    uint8_t down;
} snp_pixel_t;

typedef struct snp_draw_t {
    snp_color_t color;
    uint16_t x, y;
} snp_draw_t;

// shell
typedef struct snp_request_t {
    uint16_t service;
    uint16_t signal;
    void *source;
} snp_request_t;

typedef struct snp_shell_t {
    snp_routine_t *routines;
    snp_routine_t *process;
    uint16_t shell_pid;
} snp_shell_t;

#endif