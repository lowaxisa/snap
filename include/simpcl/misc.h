#ifndef SCL_MISC_H
#define SCL_MISC_H

#include <time.h>
#include "types.h"

static size_t scl_ms() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (size_t) (time.tv_sec * 1000ULL + time.tv_nsec / 1000000ULL);
}

#endif