#include "forge66.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    const pl_forge66_descriptor *items = NULL;
    pl_forge66_input input = { 42.5 };
    pl_forge66_result result;
    size_t count = pl_forge66_catalog(&items);
    assert(count == PL_FORGE66_FUNCTION_COUNT && items != NULL);
    for (size_t i = 0u; i < count; ++i) {
        assert(items[i].id == i + 1u);
        assert(items[i].name != NULL && items[i].domain != NULL);
        assert(items[i].purpose != NULL && items[i].function != NULL);
        for (size_t j = i + 1u; j < count; ++j)
            assert(strcmp(items[i].name, items[j].name) != 0);
        assert(items[i].function(&input, &result) == 0);
        assert(result.id == i + 1u && isfinite(result.value));
        assert(fabs(result.value) <= 1000000000.0);
        assert(pl_forge66_apply((unsigned)i + 1u, &input, &result) == 0);
        assert(result.id == i + 1u);
    }
    assert(pl_forge66_apply(0u, &input, &result) != 0);
    assert(pl_forge66_apply(67u, &input, &result) != 0);
    assert(pl_forge66_apply(1u, NULL, &result) != 0);
    assert(pl_forge66_apply(1u, &input, NULL) != 0);
    input.value = NAN;
    assert(pl_forge66_apply(1u, &input, &result) != 0);
    input.value = INFINITY;
    assert(pl_forge66_apply(1u, &input, &result) != 0);
    input.value = 1000000001.0;
    assert(pl_forge66_apply(1u, &input, &result) != 0);
    return 0;
}
