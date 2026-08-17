#ifndef SCL_ARRAY_H
#define SCL_ARRAY_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

typedef struct scl_array_t scl_array_t;

// life-cycle
scl_array_t *scl_array_new(size_t element_size);
void scl_array_destroy(scl_array_t *self);

// memory
void scl_array_realloc(scl_array_t *self, size_t size);
void scl_array_reserve(scl_array_t *self, size_t size);
void scl_array_shrink(scl_array_t *self);
void *scl_array_at(const scl_array_t *self, size_t index);
scl_array_t *scl_array_copy(const scl_array_t *self, void (*callback)(void *a, void *b));
void scl_array_grow(scl_array_t *self, size_t needed);
void scl_array_clear(scl_array_t *self);
void scl_array_fill(scl_array_t *self, void *source);

// control
void scl_array_push(scl_array_t *self, void *source);
void scl_array_pop(scl_array_t *self, void *dest);
void scl_array_swap(scl_array_t *self, size_t index, size_t target, void *buffer);
void scl_array_remove(scl_array_t *self, size_t index, void *dest);
void scl_array_insert(scl_array_t *self, size_t index, void *source);
void scl_array_foreach(scl_array_t *self, void (*callback)(void *data));
void *scl_array_find(scl_array_t *self, void *context, bool (*callback)(void *element, void *context));
void scl_array_append(scl_array_t *self, scl_array_t *source, void (*callback)(void *a, void *b));

// logic
bool scl_array_compare(const scl_array_t *self, const scl_array_t *other, bool (*callback)(void *a, void *b));
void scl_array_sort(scl_array_t *self, bool (*callback)(void *a, void *b), void *buffer);

// getters
size_t scl_array_length(const scl_array_t *self);
size_t scl_array_capacity(const scl_array_t *self);

// misc
void scl_array_print(const scl_array_t *self, void (*callback)(void *element));

#endif