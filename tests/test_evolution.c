#include "evolution.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    pl_evolution_input input = {12u, 4u, 3u, 1250.0, 8192u, 1u, 1u};
    pl_evolution_report report;
    assert(pl_evolution_run(&input, 8u, &report) == 0);
    assert(report.rounds_completed >= 1u && report.rounds_completed <= 8u);
    assert(report.module_invocations == (uint64_t)report.rounds_completed * 6u);
    assert(report.pairwise_interactions == (uint64_t)report.rounds_completed * 36u);
    assert(report.interaction_product_sum > 0u);
    assert(report.planned_transformations == input.source_files);
    assert(report.gate_passed == 1);
    assert(report.readiness_score > 0.0 && report.readiness_score <= 100.0);
    assert(report.trace_events == report.rounds_completed);
    for (unsigned i = 0u; i < PL_EVOLUTION_MODULE_COUNT; ++i)
        for (unsigned j = 0u; j < PL_EVOLUTION_MODULE_COUNT; ++j)
            assert(report.interaction_counts[i][j] == report.rounds_completed);

    input.tests_passed = 0u;
    assert(pl_evolution_run(&input, 4u, &report) == 0);
    assert(report.gate_passed == 0);
    assert(report.readiness_score < 50.0);
    assert(pl_evolution_run(&input, 0u, &report) != 0);
    assert(pl_evolution_run(&input, PL_EVOLUTION_MAX_ROUNDS + 1u, &report) != 0);
    assert(pl_concreo_analyze(NULL, &report.module_scores[0]) != 0);
    assert(pl_penwhite_trace(NULL) != 0);

    input.baseline_iterations_per_ms = NAN;
    assert(pl_evolution_run(&input, 2u, &report) != 0);
    input.baseline_iterations_per_ms = 10.0;
    input.architecture_known = 2u;
    assert(pl_evolution_run(&input, 2u, &report) != 0);
    input.architecture_known = 1u;
    input.source_files = PL_EVOLUTION_MAX_SOURCE_FILES + 1u;
    assert(pl_evolution_run(&input, 2u, &report) != 0);
    return 0;
}
