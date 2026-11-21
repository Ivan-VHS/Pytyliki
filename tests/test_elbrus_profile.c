#include "architecture_atlas.h"
#include "processor.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_architecture_profile profile;
    pl_processor_snapshot snapshot;

    assert(pl_architecture_atlas_find("e2k", &profile) == 0);
    assert(strcmp(profile.family, "Elbrus") == 0);
    assert(profile.word_bits == 64u);
    assert(strcmp(profile.endianness, "little") == 0);
    assert(strstr(profile.toolchain_hint, "LCC") != NULL);

    assert(pl_processor_scan(&snapshot) == 0);
#if defined(__e2k__)
    assert(snapshot.is_e2k_target == 1u);
    assert(strcmp(snapshot.architecture, "e2k") == 0);
    assert(snapshot.mcst_lcc == 1u);
    assert(strcmp(snapshot.base.vendor, "MCST") == 0);
#else
    assert(snapshot.is_e2k_target == 0u);
#endif
    puts("Elbrus E2K architecture and processor-profile tests passed.");
    return 0;
}
