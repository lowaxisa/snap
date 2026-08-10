#ifndef SCL_STRING_H
#define SCL_STRING_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "array.h"

typedef struct scl_string_t {
    scl_array_t *source;
} scl_string_t;

// life-cycle
scl_string_t *scl_string_new();
void scl_string_destroy(scl_string_t *string);

// control
void scl_string_cappend(scl_string_t *string, const char *c_str);
void scl_string_append(scl_string_t *string, scl_string_t *source);

// logic
bool scl_string_compare(scl_string_t *string, scl_string_t *other);
bool scl_string_ccompare(scl_string_t *string, const char *c_str);
size_t scl_string_find(scl_string_t *string, scl_string_t *target, size_t ignore);
size_t scl_string_cfind(scl_string_t *string, const char *c_str, size_t ignore);

// memory
void scl_string_clear(scl_string_t *string);
scl_string_t *scl_string_copy(scl_string_t *string);

// transformation
void scl_string_upper(scl_string_t *string);
void scl_string_lower(scl_string_t *string);

// misc
void scl_string_print(scl_string_t *string);
void scl_string_input(scl_string_t *string);
char *scl_string_cstr(scl_string_t *string);

// instance
scl_string_t *scl_string_from(const char *c_str);
scl_string_t *scl_string_substr(scl_string_t *string, size_t index, size_t length);
scl_string_t *scl_string_csubstr(const char *c_str, size_t index, size_t length);
scl_array_t *scl_string_slice(scl_string_t *string, scl_string_t *target, size_t ignore, size_t limit);
scl_array_t *scl_string_cslice(scl_string_t *string, const char *c_str, size_t ignore, size_t limit);
scl_string_t *scl_string_join(scl_array_t *string_array, scl_string_t *separator);
scl_string_t *scl_string_replace(scl_string_t *string, scl_string_t *target, scl_string_t *source);

#endif