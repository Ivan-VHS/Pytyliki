#include "concat.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int pl_concat_bytes(const void *const *parts, const size_t *lengths,
                    size_t count, void *out, size_t capacity,
                    size_t *written) {
    size_t total = 0u;
    size_t offset = 0u;

    if (written == NULL) return -1;
    *written = 0u;
    if (count != 0u && (parts == NULL || lengths == NULL)) return -1;

    for (size_t i = 0u; i < count; ++i) {
        if (lengths[i] != 0u && parts[i] == NULL) return -1;
        if (lengths[i] > SIZE_MAX - total) return -1;
        total += lengths[i];
    }

    if (total > capacity) return -2;
    if (total != 0u && out == NULL) return -1;

    for (size_t i = 0u; i < count; ++i) {
        if (lengths[i] != 0u) {
            memcpy((unsigned char *)out + offset, parts[i], lengths[i]);
            offset += lengths[i];
        }
    }

    *written = total;
    return 0;
}

int pl_concat_text(const char *const *parts, size_t count,
                   char *out, size_t capacity, size_t *written) {
    size_t *lengths = NULL;
    const void **spans = NULL;
    size_t total = 0u;
    int result;

    if (written == NULL) return -1;
    *written = 0u;
    if (out == NULL || capacity == 0u || (count != 0u && parts == NULL)) return -1;

    if (count > SIZE_MAX / sizeof(*lengths) || count > SIZE_MAX / sizeof(*spans)) return -1;
    if (count != 0u) {
        /* Keep temporary metadata bounded by the caller-provided part count. */
        lengths = (size_t *)malloc(count * sizeof(*lengths));
        spans = (const void **)malloc(count * sizeof(*spans));
        if (lengths == NULL || spans == NULL) {
            free(lengths);
            free(spans);
            return -1;
        }
    }

    for (size_t i = 0u; i < count; ++i) {
        if (parts[i] == NULL) {
            free(lengths);
            free(spans);
            return -1;
        }
        lengths[i] = strlen(parts[i]);
        spans[i] = parts[i];
        if (lengths[i] > SIZE_MAX - total) {
            free(lengths);
            free(spans);
            return -1;
        }
        total += lengths[i];
    }

    if (total >= capacity) {
        free(lengths);
        free(spans);
        return -2;
    }
    result = pl_concat_bytes(spans, lengths, count, out, capacity - 1u, written);
    free(lengths);
    free(spans);
    if (result != 0) return result;
    ((char *)out)[*written] = '\0';
    return 0;
}
