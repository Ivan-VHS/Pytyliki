#include "concat.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int reserve_payload(pl_concat_buffer *buffer, size_t required) {
    size_t next;
    unsigned char *grown;

    if (buffer == NULL || buffer->max_length == 0u ||
        required > buffer->max_length || required == SIZE_MAX) return -1;
    if (required <= buffer->capacity) return 0;

    next = buffer->capacity ? buffer->capacity : 64u;
    if (next > buffer->max_length) next = buffer->max_length;
    while (next < required) {
        if (next > buffer->max_length / 2u) {
            next = buffer->max_length;
            break;
        }
        next *= 2u;
    }
    if (next < required || next == SIZE_MAX) return -1;

    grown = (unsigned char *)realloc(buffer->data, next + 1u);
    if (grown == NULL) return -1;
    buffer->data = grown;
    buffer->capacity = next;
    buffer->data[buffer->length] = '\0';
    return 0;
}

int pl_concat_buffer_init(pl_concat_buffer *buffer, size_t max_length) {
    if (buffer == NULL || max_length == 0u || max_length == SIZE_MAX) return -1;
    buffer->data = NULL;
    buffer->length = 0u;
    buffer->capacity = 0u;
    buffer->max_length = max_length;
    return 0;
}

int pl_concat_buffer_append(pl_concat_buffer *buffer, const void *data, size_t length) {
    size_t required;
    if (buffer == NULL || buffer->max_length == 0u || (length != 0u && data == NULL)) return -1;
    if (buffer->length > buffer->max_length ||
        length > buffer->max_length - buffer->length) return -1;
    required = buffer->length + length;
    if (length == 0u) return 0;
    if (reserve_payload(buffer, required) != 0) return -1;

    memcpy(buffer->data + buffer->length, data, length);
    buffer->length = required;
    buffer->data[buffer->length] = '\0';
    return 0;
}

int pl_concat_buffer_append_text(pl_concat_buffer *buffer, const char *text) {
    if (text == NULL) return -1;
    return pl_concat_buffer_append(buffer, text, strlen(text));
}

int pl_concat_buffer_append_format(pl_concat_buffer *buffer, const char *format, ...) {
    va_list args, measure;
    int needed, written;
    size_t required, original_length;

    if (buffer == NULL || buffer->max_length == 0u || format == NULL) return -1;
    if (buffer->length > buffer->max_length) return -1;

    va_start(args, format);
    va_copy(measure, args);
    needed = vsnprintf(NULL, 0u, format, measure);
    va_end(measure);
    if (needed < 0) {
        va_end(args);
        return -1;
    }
    if (needed == 0) {
        va_end(args);
        return 0;
    }
    if ((size_t)needed > buffer->max_length - buffer->length) {
        va_end(args);
        return -1;
    }

    original_length = buffer->length;
    required = original_length + (size_t)needed;
    if (reserve_payload(buffer, required) != 0) {
        va_end(args);
        return -1;
    }
    written = vsnprintf((char *)buffer->data + original_length,
                        buffer->capacity - original_length + 1u, format, args);
    va_end(args);
    if (written != needed) {
        buffer->data[original_length] = '\0';
        return -1;
    }
    buffer->length = required;
    buffer->data[buffer->length] = '\0';
    return 0;
}

const unsigned char *pl_concat_buffer_data(const pl_concat_buffer *buffer) {
    return buffer ? buffer->data : NULL;
}

const char *pl_concat_buffer_cstr(const pl_concat_buffer *buffer) {
    static const char empty[] = "";
    return (buffer && buffer->data) ? (const char *)buffer->data : empty;
}

void pl_concat_buffer_free(pl_concat_buffer *buffer) {
    if (buffer == NULL) return;
    free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0u;
    buffer->capacity = 0u;
    buffer->max_length = 0u;
}
