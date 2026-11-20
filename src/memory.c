#include "memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int is_power_of_two(size_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static int arena_is_valid(const pl_arena *arena) {
    return arena != NULL && arena->storage != NULL &&
           arena->capacity != 0u && arena->used <= arena->capacity;
}

int pl_arena_init(pl_arena *arena, size_t capacity) {
    if (arena == NULL || capacity == 0u) return -1;
    memset(arena, 0, sizeof(*arena));
    arena->storage = (unsigned char *)malloc(capacity);
    if (arena->storage == NULL) return -1;
    arena->capacity = capacity;
    arena->owns_storage = 1;
    return 0;
}

int pl_arena_init_buffer(pl_arena *arena, void *buffer, size_t capacity) {
    if (arena == NULL || buffer == NULL || capacity == 0u) return -1;
    memset(arena, 0, sizeof(*arena));
    arena->storage = (unsigned char *)buffer;
    arena->capacity = capacity;
    arena->owns_storage = 0;
    return 0;
}

void *pl_arena_alloc(pl_arena *arena, size_t size, size_t alignment) {
    uintptr_t base, current;
    size_t padding, remaining;

    if (!arena_is_valid(arena) || size == 0u || !is_power_of_two(alignment))
        return NULL;
    remaining = arena->capacity - arena->used;
    base = (uintptr_t)arena->storage;
    if (arena->used > (size_t)(UINTPTR_MAX - base)) return NULL;
    current = base + (uintptr_t)arena->used;
    padding = (size_t)((0u - current) & (uintptr_t)(alignment - 1u));
    if (padding > remaining) return NULL;
    remaining -= padding;
    if (size > remaining) return NULL;

    arena->used += padding + size;
    if (arena->used > arena->peak_used) arena->peak_used = arena->used;
    arena->allocation_count++;
    return arena->storage + (arena->used - size);
}

void *pl_arena_calloc(pl_arena *arena, size_t count, size_t element_size, size_t alignment) {
    size_t total;
    void *memory;
    if (count == 0u || element_size == 0u || count > SIZE_MAX / element_size)
        return NULL;
    total = count * element_size;
    memory = pl_arena_alloc(arena, total, alignment);
    if (memory != NULL) memset(memory, 0, total);
    return memory;
}

size_t pl_arena_mark(const pl_arena *arena) {
    return arena_is_valid(arena) ? arena->used : SIZE_MAX;
}

int pl_arena_rewind(pl_arena *arena, size_t mark, int clear_released) {
    if (!arena_is_valid(arena) || mark > arena->used) return -1;
    if (clear_released && mark < arena->used)
        memset(arena->storage + mark, 0, arena->used - mark);
    arena->used = mark;
    arena->allocation_count = 0u;
    return 0;
}

int pl_arena_reset(pl_arena *arena, int clear_memory) {
    if (!arena_is_valid(arena)) return -1;
    if (clear_memory && arena->used != 0u)
        memset(arena->storage, 0, arena->used);
    arena->used = 0u;
    arena->allocation_count = 0u;
    return 0;
}

int pl_arena_get_stats(const pl_arena *arena, pl_arena_stats *out) {
    if (!arena_is_valid(arena) || out == NULL) return -1;
    out->capacity = arena->capacity;
    out->used = arena->used;
    out->available = arena->capacity - arena->used;
    out->peak_used = arena->peak_used;
    out->allocation_count = arena->allocation_count;
    return 0;
}

void pl_arena_destroy(pl_arena *arena) {
    if (arena == NULL) return;
    if (arena->owns_storage) free(arena->storage);
    memset(arena, 0, sizeof(*arena));
}
