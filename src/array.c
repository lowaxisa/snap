#include "../include/simpcl/array.h"

typedef struct scl_array_t {
    void *source;
    size_t length;
    size_t capacity;
    size_t element_size;
} scl_array_t;



// life-cycle
scl_array_t *scl_array_new(size_t element_size) {
    scl_array_t *a = (scl_array_t *) malloc(sizeof(scl_array_t));

    if (!a) return NULL;
    a->length = 0;
    a->capacity = 16;
    a->source = (void *) malloc(16 * element_size);
    a->element_size = element_size;
    if (!a->source) {
        free(a);
        return NULL;
    }

    return a;
}

void scl_array_destroy(scl_array_t *a) {
    if (!a) return;
    free(a->source);
    free(a);
}



// memory
void scl_array_realloc(scl_array_t *a, size_t size) {
    if (size < 16) size = 16;

    a->capacity = size;
    a->source = realloc(a->source, size * a->element_size);
}

void scl_array_reserve(scl_array_t *a, size_t size) {
    if (size < a->capacity) return;
    scl_array_realloc(a, size);
}

void scl_array_shrink(scl_array_t *a) {
    scl_array_realloc(a, a->length);
}

void *scl_array_at(const scl_array_t *a, size_t index) {
    if (index >= a->length) return NULL;
    return (char *) a->source + (a->element_size * index);
}

scl_array_t *scl_array_copy(const scl_array_t *a, void (*callback)(void *a, void *b)) {
    scl_array_t *temp = scl_array_new(a->element_size);
    if (!temp) return NULL;

    temp->length = a->length;
    scl_array_reserve(temp, a->capacity);

    memcpy(temp->source, a->source, a->element_size * a->length);

    if (!callback) return temp;
    for (size_t i = 0; i < a->length; i++) {
        callback(scl_array_at(temp, i), scl_array_at(a, i));
    }
    return temp;
}

void scl_array_grow(scl_array_t *array, size_t needed) {
    while (array->capacity < needed) scl_array_realloc(array, array->capacity * 2);
}

void scl_array_clear(scl_array_t *array) {
    array->length = 0;
}



// control
void scl_array_push(scl_array_t *a, void *source) {
    if (a->length >= a->capacity) {
        scl_array_realloc(a, a->capacity * 2);
    }

    a->length++;
    memcpy(scl_array_at(a, a->length - 1), source, a->element_size);
}

void scl_array_pop(scl_array_t *a, void *dest) {
    if (!a->length) {
        return;
    }

    if (dest) memcpy(dest, scl_array_at(a, a->length - 1), a->element_size);
    a->length--;
}

void scl_array_swap(scl_array_t *a, size_t index, size_t target, void *buffer) {
    memcpy(buffer, scl_array_at(a, index), a->element_size);
    memcpy(scl_array_at(a, index), scl_array_at(a, target), a->element_size);
    memcpy(scl_array_at(a, target), buffer, a->element_size);
}

void scl_array_remove(scl_array_t *a, size_t index, void *dest) {
    if (index == a->length - 1) {scl_array_pop(a, dest); return;}
    if (dest) memcpy(dest, scl_array_at(a, index), a->element_size);

    memmove(scl_array_at(a, index), scl_array_at(a, index + 1), (a->length - index - 1) * a->element_size);
    a->length--;
}

void scl_array_insert(scl_array_t *a, size_t index, void *source) {
    scl_array_grow(a, ++a->length);
    memmove(scl_array_at(a, index + 1), scl_array_at(a, index), ((a->length - 1) - index) * a->element_size);
    memcpy(scl_array_at(a, index), source, a->element_size);
}

void scl_array_foreach(scl_array_t *a, void (*callback)(void *data)) {
    for (size_t i = 0; i < a->length; i++) {
        callback(scl_array_at(a, i));
    }
}

void *scl_array_find(scl_array_t *array, void *context, bool (*callback)(void *element, void *context)) {
    for (size_t i = 0; i < array->length; i++) {
        void *element = scl_array_at(array, i);
        if (callback(element, context)) return element;
    }
    return NULL;
}

void scl_array_append(scl_array_t *array, scl_array_t *source, void (*callback)(void *a, void *b)) {
    size_t array_length = array->length;
    size_t source_length = source->length;

    array->length = array_length + source_length;
    scl_array_grow(array, array->length);

    memcpy(scl_array_at(array, array_length), source->source, source_length * array->element_size);

    if (!callback) return;
    for (size_t i = array_length; i < array->length; i++) {
        callback(scl_array_at(array, i), scl_array_at(source, i - array_length));
    }
}



// logic
bool scl_array_compare(const scl_array_t *a, const scl_array_t *b, bool (*callback)(void *a, void *b)) {
    if (a->length != b->length) return false;

    for (size_t i = 0; i < a->length; i++) {
        if (!callback(scl_array_at(a, i), scl_array_at(b, i))) return false;
    }

    return true;
}

void scl_array_sort(scl_array_t *a, bool (*callback)(void *a, void *b), void *buffer) {
    size_t distance = (size_t) a->length / 2;
    bool swaped = false;

    while (swaped || distance > 1) {
        distance = (size_t) distance / 1.3;

        if (distance < 1) distance = 1;

        swaped = false;

        for (size_t j = 0; j + distance < a->length; j++) {
            if (callback(scl_array_at(a, j), scl_array_at(a, j + distance))) {
                scl_array_swap(a, j, j + distance, buffer);
                swaped = true;
            }
        }
    }
}



// getters
size_t scl_array_length(const scl_array_t *a) {return a->length;}
size_t scl_array_capacity(const scl_array_t *a) {return a->capacity;}



// misc
void scl_array_print(const scl_array_t *a, void (*callback)(void *element)) {
    for (size_t i = 0; i < a->length; i++) {
        callback(scl_array_at(a, i));

        if ((i + 1) % 4 == 0 && i != 0) {
            printf("\n");
        } else if (i + 1 < a->length) {
            printf(", ");
        } else {
            printf("\n");
        }
    }
}