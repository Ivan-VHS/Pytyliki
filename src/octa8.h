#ifndef PYTYLIKI_OCTA8_H
#define PYTYLIKI_OCTA8_H

/* OCTA-8 C11 value-view contract.
 * Wire serialization is defined by schemas/octa8.v1.schema.json.
 * This header validates a bounded native view; it is not a JSON parser.
 */
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PYTYLIKI_OCTA8_ABI_VERSION 1u
#define PYTYLIKI_OCTA8_KIND_COUNT 8u
#define PYTYLIKI_OCTA8_MAX_TEXT_BYTES 1048576u
#define PYTYLIKI_OCTA8_MAX_BINARY_BYTES 16777216u

typedef enum pytyliki_octa8_kind {
    PYTYLIKI_OCTA8_NULL = 0,
    PYTYLIKI_OCTA8_BOOLEAN = 1,
    PYTYLIKI_OCTA8_INTEGER = 2,
    PYTYLIKI_OCTA8_NUMBER = 3,
    PYTYLIKI_OCTA8_TEXT = 4,
    PYTYLIKI_OCTA8_BYTES = 5,
    PYTYLIKI_OCTA8_SEQUENCE = 6,
    PYTYLIKI_OCTA8_RECORD = 7
} pytyliki_octa8_kind;

typedef struct pytyliki_octa8_view {
    uint32_t version;
    uint32_t kind;
    const void *data;
    uint64_t size_bytes;
} pytyliki_octa8_view;

static inline const char *pytyliki_octa8_kind_name(uint32_t kind) {
    static const char *const names[PYTYLIKI_OCTA8_KIND_COUNT] = {
        "null", "boolean", "integer", "number",
        "text", "bytes", "sequence", "record"
    };
    return kind < PYTYLIKI_OCTA8_KIND_COUNT ? names[kind] : (const char *)0;
}

static inline int pytyliki_octa8_view_valid(const pytyliki_octa8_view *view) {
    if (view == (const pytyliki_octa8_view *)0 ||
        view->version != PYTYLIKI_OCTA8_ABI_VERSION ||
        view->kind >= PYTYLIKI_OCTA8_KIND_COUNT ||
        (view->size_bytes > 0u && view->data == (const void *)0)) {
        return 0;
    }
    switch ((pytyliki_octa8_kind)view->kind) {
        case PYTYLIKI_OCTA8_NULL:
            return view->data == (const void *)0 && view->size_bytes == 0u;
        case PYTYLIKI_OCTA8_BOOLEAN:
            return view->data != (const void *)0 &&
                   view->size_bytes == sizeof(uint8_t) &&
                   (*(const uint8_t *)view->data <= 1u);
        case PYTYLIKI_OCTA8_INTEGER:
            return view->data != (const void *)0 && view->size_bytes == sizeof(int64_t);
        case PYTYLIKI_OCTA8_NUMBER: {
            double number;
            if (view->data == (const void *)0 || view->size_bytes != sizeof(double)) {
                return 0;
            }
            memcpy(&number, view->data, sizeof(number));
            return isfinite(number) ? 1 : 0;
        }
        case PYTYLIKI_OCTA8_TEXT:
            return view->size_bytes <= PYTYLIKI_OCTA8_MAX_TEXT_BYTES;
        case PYTYLIKI_OCTA8_BYTES:
            return view->size_bytes <= PYTYLIKI_OCTA8_MAX_BINARY_BYTES;
        case PYTYLIKI_OCTA8_SEQUENCE:
        case PYTYLIKI_OCTA8_RECORD:
            return view->size_bytes <= PYTYLIKI_OCTA8_MAX_BINARY_BYTES;
        default:
            return 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* PYTYLIKI_OCTA8_H */
