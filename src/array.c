#include "../include/simpcl/array.h"

typedef struct scl_array_t {
    void *source;
    size_t length;
    size_t capacity;
    size_t element_size;
} scl_array_t;



// life-cycle
scl_array_t *scl_array_new(size_t element_size) {
    scl_array_t *self = (scl_array_t *) malloc(sizeof(scl_array_t));

    if (!self) return NULL;
    self->length = 0;
    self->capacity = 16;
    self->source = (void *) malloc(16 * element_size);
    self->element_size = element_size;
    if (!self->source) {
        free(self);
        return NULL;
    }

    return self;
}

void scl_array_destroy(scl_array_t *self) {
    if (!self) return;
    free(self->source);
    free(self);
}



// memory
void scl_array_realloc(scl_array_t *self, size_t size) {
    if (size < 16) size = 16;

    self->capacity = size;
    self->source = realloc(self->source, size * self->element_size);
}

void scl_array_reserve(scl_array_t *self, size_t size) {
    if (size < self->capacity) return;
    scl_array_realloc(self, size);
}

void scl_array_shrink(scl_array_t *self) {
    scl_array_realloc(self, self->length);
}

void *scl_array_at(const scl_array_t *self, size_t index) {
    if (index >= self->length) return NULL;
    return (char *) self->source + (self->element_size * index);
}

scl_array_t *scl_array_copy(const scl_array_t *self, void (*callback)(void *a, void *b)) {
    scl_array_t *temp = scl_array_new(self->element_size);
    if (!temp) return NULL;

    temp->length = self->length;
    scl_array_reserve(temp, self->capacity);

    memcpy(temp->source, self->source, self->element_size * self->length);

    if (!callback) return temp;
    for (size_t i = 0; i < self->length; i++) {
        callback(scl_array_at(temp, i), scl_array_at(self, i));
    }
    return temp;
}

void scl_array_grow(scl_array_t *self, size_t needed) {
    while (self->capacity < needed) scl_array_realloc(self, self->capacity * 2);
}

void scl_array_clear(scl_array_t *self) {
    self->length = 0;
}

void scl_array_fill(scl_array_t *self, void *source) {
    if (self->length > self->capacity) self->length = self->capacity;

    while (self->length < self->capacity) {
        scl_array_push(self, source);
    }
}



// control
void scl_array_push(scl_array_t *self, void *source) {
    if (self->length >= self->capacity) {
        scl_array_realloc(self, self->capacity * 2);
    }

    self->length++;
    memcpy(scl_array_at(self, self->length - 1), source, self->element_size);
}

void scl_array_pop(scl_array_t *self, void *dest) {
    if (!self->length) {
        return;
    }

    if (dest) memcpy(dest, scl_array_at(self, self->length - 1), self->element_size);
    self->length--;
}

void scl_array_swap(scl_array_t *self, size_t index, size_t target, void *buffer) {
    memcpy(buffer, scl_array_at(self, index), self->element_size);
    memcpy(scl_array_at(self, index), scl_array_at(self, target), self->element_size);
    memcpy(scl_array_at(self, target), buffer, self->element_size);
}

void scl_array_remove(scl_array_t *self, size_t index, void *dest) {
    if (index == self->length - 1) {scl_array_pop(self, dest); return;}
    if (dest) memcpy(dest, scl_array_at(self, index), self->element_size);

    memmove(scl_array_at(self, index), scl_array_at(self, index + 1), (self->length - index - 1) * self->element_size);
    self->length--;
}

void scl_array_insert(scl_array_t *self, size_t index, void *source) {
    scl_array_grow(self, ++self->length);
    memmove(scl_array_at(self, index + 1), scl_array_at(self, index), ((self->length - 1) - index) * self->element_size);
    memcpy(scl_array_at(self, index), source, self->element_size);
}

void scl_array_foreach(scl_array_t *self, void (*callback)(void *data)) {
    for (size_t i = 0; i < self->length; i++) {
        callback(scl_array_at(self, i));
    }
}

void *scl_array_find(scl_array_t *self, void *context, bool (*callback)(void *element, void *context)) {
    for (size_t i = 0; i < self->length; i++) {
        void *element = scl_array_at(self, i);
        if (callback(element, context)) return element;
    }
    return NULL;
}

void scl_array_append(scl_array_t *self, scl_array_t *source, void (*callback)(void *a, void *b)) {
    size_t array_length = self->length;
    size_t source_length = source->length;

    self->length = array_length + source_length;
    scl_array_grow(self, self->length);

    memcpy(scl_array_at(self, array_length), source->source, source_length * self->element_size);

    if (!callback) return;
    for (size_t i = array_length; i < self->length; i++) {
        callback(scl_array_at(self, i), scl_array_at(source, i - array_length));
    }
}



// logic
bool scl_array_compare(const scl_array_t *self, const scl_array_t *other, bool (*callback)(void *a, void *b)) {
    if (self->length != other->length) return false;

    for (size_t i = 0; i < self->length; i++) {
        if (!callback(scl_array_at(self, i), scl_array_at(other, i))) return false;
    }

    return true;
}

void scl_array_sort(scl_array_t *self, bool (*callback)(void *a, void *b), void *buffer) {
    size_t distance = (size_t) self->length / 2;
    bool swapped = false;

    while (swapped || distance > 1) {
        distance = (size_t) distance / 1.3;

        if (distance < 1) distance = 1;

        swapped = false;

        for (size_t j = 0; j + distance < self->length; j++) {
            if (callback(scl_array_at(self, j), scl_array_at(self, j + distance))) {
                scl_array_swap(self, j, j + distance, buffer);
                swapped = true;
            }
        }
    }
}



// getters
size_t scl_array_length(const scl_array_t *self) {return self->length;}
size_t scl_array_capacity(const scl_array_t *self) {return self->capacity;}



// misc
void scl_array_print(const scl_array_t *self, void (*callback)(void *element)) {
    for (size_t i = 0; i < self->length; i++) {
        callback(scl_array_at(self, i));

        if ((i + 1) % 4 == 0 && i != 0) {
            printf("\n");
        } else if (i + 1 < self->length) {
            printf(", ");
        } else {
            printf("\n");
        }
    }
}