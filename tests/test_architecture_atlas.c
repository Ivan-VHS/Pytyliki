#include "architecture_atlas.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_architecture_profile *items = NULL;
    pl_architecture_profile found;
    char current[32];
    size_t count = pl_architecture_atlas_catalog(&items);

    assert(count == PL_ARCH_ATLAS_COUNT);
    assert(items != NULL);
    for (size_t i = 0u; i < count; ++i) {
        assert(items[i].id == i + 1u);
        assert(items[i].name && items[i].name[0]);
        assert(items[i].family && items[i].family[0]);
        assert(items[i].word_bits == 8u || items[i].word_bits == 32u || items[i].word_bits == 64u);
        assert(items[i].endianness && items[i].endianness[0]);
        for (size_t j = i + 1u; j < count; ++j)
            assert(strcmp(items[i].name, items[j].name) != 0);
    }
    assert(pl_architecture_atlas_find("AARCH64", &found) == 0);
    assert(strcmp(found.name, "aarch64") == 0);
    assert(pl_architecture_atlas_find("Power ISA", &found) == 0);
    assert(strcmp(found.name, "ppc64le") == 0);
    assert(pl_architecture_atlas_find("imaginary-isa", &found) == -2);
    assert(pl_architecture_atlas_find(NULL, &found) == -1);
    assert(pl_architecture_atlas_catalog(NULL) == 0u);
    assert(pl_architecture_atlas_current(current, sizeof(current)) == 0);
    assert(current[0] != '\0');
    puts("All architecture atlas tests passed.");
    return 0;
}
