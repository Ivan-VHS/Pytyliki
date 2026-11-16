#include "mtte.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_mtte_layer *items = NULL, *item = NULL;
    pl_mtte_clock clock;
    size_t count = pl_mtte_catalog(&items);
    assert(count == PL_MTTE_LAYER_COUNT && items != NULL);
    for (size_t i = 0u; i < count; ++i) {
        assert(items[i].id == i + 1u);
        assert(items[i].concept && items[i].concept[0]);
        assert(items[i].function && items[i].function[0]);
        assert(items[i].mechanical_operation && items[i].mechanical_operation[0]);
        assert(items[i].description && items[i].description[0]);
    }
    assert(strcmp(items[0].concept, "Типизация") == 0);
    assert(strcmp(items[0].function, "Типизирующий") == 0);
    assert(strcmp(items[0].mechanical_operation, "Механизирующий") == 0);
    assert(strcmp(items[19].concept, "Метаклассификация") == 0);
    assert(strcmp(items[19].mechanical_operation, "Тактодиагностирующий") == 0);
    assert(pl_mtte_get(1u, &item) == 0 && item == &items[0]);
    assert(pl_mtte_get(20u, &item) == 0 && item == &items[19]);
    assert(pl_mtte_get(0u, &item) == -1);
    assert(pl_mtte_get(21u, &item) == -1);
    assert(pl_mtte_get(1u, NULL) == -1);
    assert(pl_mtte_find("Тактогенерирующий", &item) == 0 && item->id == 3u);
    assert(pl_mtte_find("Кодирование", &item) == 0 && item->id == 9u);
    assert(pl_mtte_find("unknown", &item) == -2);
    assert(pl_mtte_find(NULL, &item) == -1);
    assert(pl_mtte_catalog(NULL) == 0u);

    assert(pl_mtte_clock_init(&clock, 1000u) == 0);
    assert(clock.tick_count == 0u && clock.elapsed_ns == 0u);
    assert(pl_mtte_clock_advance(&clock, 25u) == 0);
    assert(clock.tick_count == 25u && clock.elapsed_ns == 25000u);
    assert(pl_mtte_clock_advance(&clock, 0u) == 0);
    assert(pl_mtte_clock_reset(&clock) == 0 && clock.tick_count == 0u);
    assert(pl_mtte_clock_init(&clock, 0u) == -1);
    assert(pl_mtte_clock_init(&clock, PL_MTTE_MAX_PERIOD_NS + 1u) == -1);
    assert(pl_mtte_clock_init(NULL, 100u) == -1);
    assert(pl_mtte_clock_init(&clock, 1u) == 0);
    clock.tick_count = UINT64_MAX;
    assert(pl_mtte_clock_advance(&clock, 1u) == -2);
    assert(pl_mtte_clock_init(&clock, UINT64_MAX) == -1);
    puts("All MTTE catalogue and logical tick tests passed.");
    return 0;
}
