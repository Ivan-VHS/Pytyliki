#include "syntax_pie.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_syntax_pie_layer *items = NULL;
    const pl_syntax_pie_layer *item = NULL;
    size_t count = pl_syntax_pie_catalog(&items);
    assert(count == PL_SYNTAX_PIE_LAYER_COUNT);
    assert(items != NULL);

    for (size_t i = 0u; i < count; ++i) {
        assert(items[i].id == i + 1u);
        assert(items[i].function_name != NULL && items[i].function_name[0] != '\0');
        assert(items[i].room_name != NULL && items[i].room_name[0] != '\0');
        assert(items[i].level_name != NULL && items[i].level_name[0] != '\0');
        assert(items[i].operation != NULL && items[i].operation[0] != '\0');
        assert((unsigned)items[i].level == i);
        assert(strcmp(pl_syntax_pie_level_name(items[i].level), items[i].level_name) == 0);
    }

    assert(strcmp(items[0].function_name, "Зал") == 0);
    assert(strcmp(items[0].room_name, "Комната") == 0);
    assert(strcmp(items[0].level_name, "Лексема") == 0);
    assert(strcmp(items[13].room_name, "Спальня") == 0);
    assert(strcmp(items[14].level_name, "Абстрактное синтаксическое дерево") == 0);
    assert(strcmp(items[19].room_name, "Анфилада") == 0);

    assert(pl_syntax_pie_get(1u, &item) == 0 && item == &items[0]);
    assert(pl_syntax_pie_get(20u, &item) == 0 && item == &items[19]);
    assert(pl_syntax_pie_get(0u, &item) == -1);
    assert(pl_syntax_pie_get(21u, &item) == -1);
    assert(pl_syntax_pie_get(1u, NULL) == -1);

    assert(pl_syntax_pie_find_function("Залатать", &item) == 0);
    assert(item->id == 4u);
    assert(pl_syntax_pie_find_room("Анфилада", &item) == 0 && item->id == 20u);
    assert(pl_syntax_pie_find_level("Синтагма", &item) == 0 && item->id == 5u);
    assert(pl_syntax_pie_find_level("", &item) == -1);
    assert(pl_syntax_pie_find_room("несуществующая", &item) == -2);
    assert(pl_syntax_pie_catalog(NULL) == 0u);
    assert(strcmp(pl_syntax_pie_level_name((pl_syntax_level)99), "invalid") == 0);

    puts("All syntax-pie function-layer tests passed.");
    return 0;
}
