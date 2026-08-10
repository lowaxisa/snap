#include "../include/simpcl/string.h"

// life-cycle
scl_string_t *scl_string_new() {
    scl_string_t *string = malloc(sizeof(scl_string_t));

    if (!string) return NULL;
    string->source = scl_array_new(sizeof(char));

    if (!string->source) {
        free(string);
        return NULL;
    }

    return string;
}

void scl_string_destroy(scl_string_t *string) {
    scl_array_destroy(string->source);
    free(string);
}



// control
void scl_string_cappend(scl_string_t *string, const char *c_str) {
    size_t c_str_length = strlen(c_str);

    for (size_t i = 0; i < c_str_length; i++) {
        scl_array_push(string->source, (void *) &c_str[i]);
    }
}

void scl_string_append(scl_string_t *string, scl_string_t *source) {
    scl_array_append(string->source, source->source, NULL);
}



// memory
void scl_string_clear(scl_string_t *string) {
    scl_array_clear(string->source);
}

scl_string_t *scl_string_copy(scl_string_t *string) {
    scl_string_t *temp = scl_string_new();
    scl_string_append(temp, string);
    return temp;
}



// logic
static bool string_compare_callback(void *a, void *b) {
    return *(char *) a == *(char *) b;
}

bool scl_string_compare(scl_string_t *string, scl_string_t *other) {
    return scl_array_compare(string->source, other->source, string_compare_callback);
}

bool scl_string_ccompare(scl_string_t *string, const char *c_str) {
    scl_string_t *temp = scl_string_new();

    scl_string_cappend(temp, c_str);
    bool result = scl_string_compare(string, temp);
    scl_string_destroy(temp);

    return result;
}

size_t scl_string_find(scl_string_t *string, scl_string_t *target, size_t ignore) {
    size_t index = 0;
    size_t counter = 0; // occurrence count
    size_t target_length = scl_array_length(target->source);

    while (index + target_length - 1 < scl_array_length(string->source)) {
        scl_string_t *temp = scl_string_substr(string, index, target_length);

        if (scl_string_compare(target, temp)) {
            scl_string_destroy(temp);
            temp = NULL;

            if (counter < ignore) {
                counter++;
                index += target_length;
            } else {
                return index;
            }

            continue;
        }

        if (temp) scl_string_destroy(temp);
        index++;
    }

    return (size_t) -1;
}

size_t scl_string_cfind(scl_string_t *string, const char *c_str, size_t ignore) {
    scl_string_t *temp = scl_string_new();
    scl_string_cappend(temp, c_str);

    size_t result = scl_string_find(string, temp, ignore);
    scl_string_destroy(temp);

    return result;
}



// transformation
void scl_string_upper(scl_string_t *string) {
    size_t string_length = scl_array_length(string->source);

    for (size_t i = 0; i < string_length; i++) {
        char *c = (char *) scl_array_at(string->source, i);
        if (*c >= 'a' && *c <= 'z') {
            *c = *c - 32;
        }
    }
}

void scl_string_lower(scl_string_t *string) {
    size_t string_length = scl_array_length(string->source);

    for (size_t i = 0; i < string_length; i++) {
        char *c = (char *) scl_array_at(string->source, i);
        if (*c >= 'A' && *c <= 'Z') {
            *c = *c + 32;
        }
    }
}



// misc
void scl_string_print(scl_string_t *string) {
    write(1, scl_array_at(string->source, 0), scl_array_length(string->source));
}

void scl_string_input(scl_string_t *string) {
    char c;

    while ((c = getchar()) != EOF && c != '\n') {
        scl_array_push(string->source, (void *) &c);
    }
}

char *scl_string_cstr(scl_string_t *string) {
    size_t size = scl_array_length(string->source) + 1;
    char *c_str = (char *) malloc(size);

    c_str[size - 1] = '\0';
    memcpy(c_str, scl_array_at(string->source, 0), size - 1);
    return c_str;
}

// instance
scl_string_t *scl_string_from(const char *c_str) {
    scl_string_t *temp = scl_string_new();
    scl_string_cappend(temp, c_str);
    return temp;
}

scl_string_t *scl_string_substr(scl_string_t *string, size_t index, size_t length) {
    scl_string_t *temp = scl_string_new();

    for (size_t i = index; i < (index + length); i++) {
        scl_array_push(temp->source, scl_array_at(string->source, i)); 
    }

    return temp;
}

scl_string_t *scl_string_csubstr(const char *c_str, size_t index, size_t length) {
    scl_string_t *temp = scl_string_new();
    scl_string_cappend(temp, c_str);

    scl_string_t *result = scl_string_substr(temp, index, length);
    scl_string_destroy(temp);

    return result;
}

scl_array_t *scl_string_slice(scl_string_t *string, scl_string_t *target, size_t ignore, size_t limit) {
    scl_array_t *array = scl_array_new(sizeof(scl_string_t *));
    size_t target_length = scl_array_length(target->source);

    size_t counter = ignore;
    size_t index = 0;
    size_t last_pos = 0;
    while ((index = scl_string_find(string, target, counter)) != (size_t) -1 && counter < limit) {
        scl_string_t *part = scl_string_substr(string, last_pos, index - last_pos);
        scl_array_push(array, (void *) &part);

        last_pos = index + target_length;
        counter++;
    }
    scl_string_t *part = scl_string_substr(string, last_pos, scl_array_length(string->source) - last_pos);
    scl_array_push(array, (void *) &part);

    return array;
}

scl_array_t *scl_string_cslice(scl_string_t *string, const char *target, size_t ignore, size_t limit) {
    scl_string_t *str_target = scl_string_from(target);
    scl_array_t *array = scl_string_slice(string, str_target, ignore, limit);
    scl_string_destroy(str_target);
    return array;
}

scl_string_t *scl_string_join(scl_array_t *string_array, scl_string_t *separator) {
    scl_string_t *temp = scl_string_new();
    size_t array_length = scl_array_length(string_array);

    for (size_t i = 0; i < array_length; i++) {
        scl_string_t *string = *(scl_string_t **) scl_array_at(string_array, i);
        scl_string_append(temp, string);

        if (i + 1 < array_length) scl_string_append(temp, separator);
    }

    return temp;
}

scl_string_t *scl_string_replace(scl_string_t *string, scl_string_t *target, scl_string_t *source) {
    scl_array_t *slice = scl_string_slice(string, target, 0, (size_t) -1);
    scl_string_t *temp = scl_string_join(slice, source);

    for (size_t i = 0; i < scl_array_length(slice); i++) {
        scl_string_destroy(*(scl_string_t **) scl_array_at(slice, i));
    }
    scl_array_destroy(slice);

    return temp;
}