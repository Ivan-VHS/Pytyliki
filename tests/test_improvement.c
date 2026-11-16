#include "improvement.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

int main(void) {
    const pl_improvement_capability *catalog = NULL;
    pl_improvement_cycle cycle;
    assert(pl_improvement_catalog(&catalog) == PL_IMPROVEMENT_CAPABILITY_COUNT);
    assert(catalog != NULL);
    for (unsigned i = 0u; i < PL_IMPROVEMENT_CAPABILITY_COUNT; ++i) {
        assert(catalog[i].id == i + 1u);
        assert(catalog[i].engineering != NULL);
        assert(catalog[i].optimization != NULL);
        assert(catalog[i].automation != NULL);
    }
    assert(pl_improvement_catalog(NULL) == 0u);
    assert(pl_improvement_cycle_init(&cycle) == 0);
    assert(cycle.phase == PL_IMPROVE_OBSERVE);
    assert(pl_improvement_advance(&cycle) == -2);
    assert(pl_improvement_record(&cycle, PL_IMPROVE_EVIDENCE_DIAGNOSED) == -1);

    const uint32_t evidence[] = {
        PL_IMPROVE_EVIDENCE_OBSERVED,
        PL_IMPROVE_EVIDENCE_DIAGNOSED,
        PL_IMPROVE_EVIDENCE_PROPOSED,
        PL_IMPROVE_EVIDENCE_IMPLEMENTED,
        PL_IMPROVE_EVIDENCE_TESTED,
        PL_IMPROVE_EVIDENCE_EVALUATED,
        PL_IMPROVE_EVIDENCE_REVIEWED,
        PL_IMPROVE_EVIDENCE_APPROVED,
        PL_IMPROVE_EVIDENCE_INTEGRATED,
        PL_IMPROVE_EVIDENCE_LEARNED,
        PL_IMPROVE_EVIDENCE_META_EVALUATED
    };
    for (unsigned i = 0u; i < sizeof(evidence) / sizeof(evidence[0]); ++i) {
        assert(pl_improvement_record(&cycle, evidence[i]) == 0);
        assert(pl_improvement_advance(&cycle) == 0);
    }
    assert(cycle.phase == PL_IMPROVE_COMPLETE);
    assert(cycle.generation == 2u);
    assert(pl_improvement_advance(&cycle) == -1);
    assert(pl_improvement_phase_name(PL_IMPROVE_COMPLETE)[0] == 'c');
    assert(pl_improvement_phase_name((pl_improvement_phase)99)[0] == 'i');
    puts("All recursive improvement engine tests passed.");
    return 0;
}
