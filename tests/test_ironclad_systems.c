#include "ironclad_systems.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_system_enhancement *items = NULL;
    assert(pl_system_enhancements(&items) == PL_SYSTEM_ENHANCEMENT_COUNT);
    assert(items != NULL);
    for (size_t i = 0u; i < PL_SYSTEM_ENHANCEMENT_COUNT; ++i) {
        assert(items[i].id == i + 1u);
        assert(pl_system_enhancement_validate(&items[i]) == 0);
        assert(strcmp(pl_system_domain_name(items[i].domain), "invalid") != 0);
        for (size_t j = i + 1u; j < PL_SYSTEM_ENHANCEMENT_COUNT; ++j)
            assert(strcmp(items[i].name, items[j].name) != 0);
    }
    assert(pl_system_enhancements(NULL) == 0u);
    assert(pl_system_enhancement_validate(NULL) == -1);
    assert(strcmp(pl_system_domain_name((pl_system_domain)99), "invalid") == 0);
    puts("All 20 system enhancement registry tests passed.");
    return 0;
}
