#ifndef PYTYLIKI_CONCAT_H
#define PYTYLIKI_CONCAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Concatenate byte spans in order.
 * Returns 0 on success, -1 for invalid arguments/overflow, and -2 when
 * the destination is too small. On failure, *written is set to zero.
 * Input spans must not overlap the destination buffer.
 */
int pl_concat_bytes(const void *const *parts, const size_t *lengths,
                    size_t count, void *out, size_t capacity,
                    size_t *written);

/* Concatenate NUL-terminated UTF-8/text byte strings; output is NUL-terminated. */
int pl_concat_text(const char *const *parts, size_t count,
                   char *out, size_t capacity, size_t *written);

#ifdef __cplusplus
}
#endif

#endif
