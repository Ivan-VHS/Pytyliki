#include "impact.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

int main(void) {
    const pl_impact_profile *items = NULL;
    const pl_impact_profile *item = NULL;
    size_t count = pl_impact_catalog(&items);
    size_t i;

    assert(count == PL_IMPACT_PROFILE_COUNT);
    assert(items != NULL);
    assert(pl_impact_catalog(NULL) == 0u);
    for (i = 0u; i < count; ++i) {
        assert(items[i].id == (unsigned)i + 1u);
        assert(pl_impact_profile_validate(&items[i]) == 0);
        assert(pl_impact_profile_rank(&items[i], PL_IMPACT_ACOUSTIC) >= 1u);
        assert(pl_impact_profile_rank(&items[i], PL_IMPACT_IMPULSE) <= 5u);
    }
    assert(pl_impact_find(1u, &item) == 0);
    assert(strcmp(item->sound, "Грохот") == 0);
    assert(pl_impact_find(20u, &item) == 0);
    assert(strcmp(item->motion_word, "Гиперскоростной") == 0);
    assert(pl_impact_find(0u, &item) != 0);
    assert(pl_impact_find(21u, &item) != 0);
    assert(pl_impact_find_term("Баллистический", &item) == 0);
    assert(item->id == 1u);
    assert(pl_impact_find_term("not-a-profile", &item) != 0);
    assert(strcmp(pl_impact_dimension_name(PL_IMPACT_REACH), "reach") == 0);
    assert(strcmp(pl_impact_dimension_name(99u), "invalid") == 0);
    assert(pl_impact_profile_rank(NULL, PL_IMPACT_SCALE) == 0u);
    return 0;
}
