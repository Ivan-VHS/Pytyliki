#ifndef PYTYLIKI_MEMORY_H
#define PYTYLIKI_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char *storage;
    size_t capacity;
    size_t used;
    size_t peak_used;
    size_t allocation_count;
    int owns_storage;
} pl_arena;

typedef struct {
    size_t capacity;
    size_t used;
    size_t available;
    size_t peak_used;
    size_t allocation_count;
} pl_arena_stats;

/* Single-owner, non-thread-safe bump allocator. Alignment is a power of two.
 * Zero-byte requests are rejected. External buffers remain caller-owned.
 */
int pl_arena_init(pl_arena *arena, size_t capacity);
int pl_arena_init_buffer(pl_arena *arena, void *buffer, size_t capacity);
void *pl_arena_alloc(pl_arena *arena, size_t size, size_t alignment);
void *pl_arena_calloc(pl_arena *arena, size_t count, size_t element_size, size_t alignment);
size_t pl_arena_mark(const pl_arena *arena);
int pl_arena_rewind(pl_arena *arena, size_t mark, int clear_released);
int pl_arena_reset(pl_arena *arena, int clear_memory);
int pl_arena_get_stats(const pl_arena *arena, pl_arena_stats *out);
void pl_arena_destroy(pl_arena *arena);

#ifdef __cplusplus
}
#endif

#endif
