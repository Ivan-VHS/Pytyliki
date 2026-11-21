#ifndef PYTYLIKI_CONCAT_H
#define PYTYLIKI_CONCAT_H

#include <stddef.h>

/*
 * Bounded byte/string builder for deterministic report and metadata assembly.
 * Zero-initialize before first init. max_length limits payload bytes; the
 * trailing NUL is additional. Appended input must not alias internal storage.
 */
typedef struct {
    unsigned char *data;
    size_t length;
    size_t capacity;
    size_t max_length;
} pl_concat_buffer;

int pl_concat_buffer_init(pl_concat_buffer *buffer, size_t max_length);
int pl_concat_buffer_append(pl_concat_buffer *buffer, const void *data, size_t length);
int pl_concat_buffer_append_text(pl_concat_buffer *buffer, const char *text);
int pl_concat_buffer_append_format(pl_concat_buffer *buffer, const char *format, ...);
const unsigned char *pl_concat_buffer_data(const pl_concat_buffer *buffer);
const char *pl_concat_buffer_cstr(const pl_concat_buffer *buffer);
void pl_concat_buffer_free(pl_concat_buffer *buffer);

#endif
