#ifndef PYTYLIKI_EMBEDDED_H
#define PYTYLIKI_EMBEDDED_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PL_EMBEDDED_MAX_SOURCE_BYTES (1024u * 1024u)
enum {
    PL_EMBEDDED_CAP_FREESTANDING = 1u << 0,
    PL_EMBEDDED_CAP_HARVARD = 1u << 1,
    PL_EMBEDDED_CAP_RTOS_COMMON = 1u << 2,
    PL_EMBEDDED_CAP_SANDBOXED = 1u << 3
};
enum {
    PL_EMBEDDED_WARN_DYNAMIC_ALLOCATION = 1u << 0,
    PL_EMBEDDED_WARN_BLOCKING_CALLS = 1u << 1,
    PL_EMBEDDED_WARN_REGISTER_ACCESS = 1u << 2,
    PL_EMBEDDED_WARN_HOST_IO = 1u << 3
};
typedef struct {
    const char *id;
    const char *architecture;
    const char *abi;
    const char *compiler;
    const char *description;
    uint32_t word_bits;
    uint32_t capabilities;
} pl_embedded_target;
typedef struct {
    size_t source_bytes;
    size_t line_count;
    size_t include_count;
    size_t heap_calls;
    size_t blocking_calls;
    size_t register_markers;
    size_t host_io_calls;
    uint32_t warnings;
} pl_embedded_source_report;
/* Read-only target metadata. Returned profile pointers have static lifetime. */
size_t pl_embedded_target_count(void);
int pl_embedded_target_at(size_t index, const pl_embedded_target **out);
int pl_embedded_target_find(const char *id, const pl_embedded_target **out);
/* Bounded lexical heuristics only: source is never compiled or executed. */
int pl_embedded_inspect_source(const char *source, size_t length, pl_embedded_source_report *out);
#ifdef __cplusplus
}
#endif
#endif
