#include "memory.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    pl_arena arena = {0};
    pl_arena_stats stats;
    assert(pl_arena_init(&arena, 256u) == 0);
    assert(pl_arena_alloc(&arena, 0u, 8u) == NULL);
    assert(pl_arena_alloc(&arena, 8u, 3u) == NULL);

    unsigned char *first = (unsigned char *)pl_arena_alloc(&arena, 7u, 1u);
    assert(first != NULL);
    memset(first, 0xA5, 7u);
    size_t mark = pl_arena_mark(&arena);
    void *aligned = pl_arena_alloc(&arena, 32u, 32u);
    assert(aligned != NULL);
    assert(((uintptr_t)aligned & 31u) == 0u);

    unsigned *zeroed = (unsigned *)pl_arena_calloc(&arena, 4u, sizeof(unsigned), 16u);
    assert(zeroed != NULL);
    for (size_t i = 0u; i < 4u; ++i) assert(zeroed[i] == 0u);

    assert(pl_arena_get_stats(&arena, &stats) == 0);
    assert(stats.capacity == 256u && stats.used > mark);
    assert(stats.peak_used == stats.used && stats.allocation_count == 3u);
    assert(pl_arena_rewind(&arena, mark, 1) == 0);
    assert(pl_arena_mark(&arena) == mark);
    assert(pl_arena_rewind(&arena, mark + 1u, 0) != 0);
    assert(pl_arena_reset(&arena, 1) == 0);
    assert(pl_arena_get_stats(&arena, &stats) == 0);
    assert(stats.used == 0u && stats.available == stats.capacity);
    assert(stats.peak_used > 0u);
    assert(pl_arena_alloc(&arena, 512u, 8u) == NULL);
    pl_arena_destroy(&arena);
    assert(arena.storage == NULL && arena.capacity == 0u);

    unsigned char external[64];
    memset(external, 0xCC, sizeof(external));
    assert(pl_arena_init_buffer(&arena, external, sizeof(external)) == 0);
    unsigned char *block = (unsigned char *)pl_arena_calloc(&arena, 8u, 1u, 8u);
    assert(block != NULL);
    for (size_t i = 0u; i < 8u; ++i) assert(block[i] == 0u);
    pl_arena_destroy(&arena);
    assert(block[0] == 0u); /* Destroy never frees or mutates caller-owned storage. */

    assert(pl_arena_init_buffer(&arena, external, sizeof(external)) == 0);
    assert(pl_arena_calloc(&arena, SIZE_MAX, 2u, 1u) == NULL);
    pl_arena_destroy(&arena);
    return 0;
}
