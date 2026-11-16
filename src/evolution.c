#include "evolution.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static int valid_input(const pl_evolution_input *input) {
    return input != NULL &&
        input->source_files <= PL_EVOLUTION_MAX_SOURCE_FILES &&
        input->static_findings <= PL_EVOLUTION_MAX_FINDINGS &&
        input->benchmark_samples <= 1000000u &&
        isfinite(input->baseline_iterations_per_ms) &&
        input->baseline_iterations_per_ms >= 0.0 &&
        input->baseline_iterations_per_ms <= 1000000000.0 &&
        input->memory_mib <= PL_EVOLUTION_MAX_MEMORY_MIB &&
        input->architecture_known <= 1u &&
        input->tests_passed <= 1u;
}

static unsigned penalty(unsigned value, unsigned scale) {
    return scale == 0u ? 0u : value / scale;
}

int pl_concreo_analyze(const pl_evolution_input *input, unsigned *score) {
    unsigned p;
    if (!valid_input(input) || score == NULL) return -1;
    p = penalty(input->static_findings, 2u);
    *score = p >= 100u ? 0u : 100u - p;
    if (input->source_files == 0u) *score = *score > 20u ? *score - 20u : 0u;
    return 0;
}

int pl_pyemont_plan(const pl_evolution_input *input, unsigned *score, unsigned *planned) {
    if (!valid_input(input) || score == NULL || planned == NULL) return -1;
    *planned = input->source_files;
    *score = input->architecture_known ? 90u : 55u;
    if (input->source_files == 0u) *score -= 15u;
    return 0;
}

int pl_bollo_profile(const pl_evolution_input *input, unsigned *score) {
    if (!valid_input(input) || score == NULL) return -1;
    *score = input->benchmark_samples ? 85u : 50u;
    if (input->baseline_iterations_per_ms <= 0.0) *score -= 10u;
    if (input->memory_mib == 0u) *score -= 5u;
    return 0;
}

int pl_jjhh_validate(const pl_evolution_input *input, unsigned *score) {
    unsigned deduction;
    if (!valid_input(input) || score == NULL) return -1;
    *score = input->tests_passed ? 95u : 25u;
    deduction = penalty(input->static_findings, 4u);
    *score = deduction >= *score ? 0u : *score - deduction;
    return 0;
}

int pl_penwhite_trace(unsigned *trace_events) {
    if (trace_events == NULL || *trace_events == UINT32_MAX) return -1;
    ++(*trace_events);
    return 0;
}

int pl_serious_gate(const pl_evolution_input *input, int *gate_passed) {
    if (!valid_input(input) || gate_passed == NULL) return -1;
    *gate_passed = (input->tests_passed == 1u && input->architecture_known == 1u);
    return 0;
}

/* Rows and columns are modules; every ordered pair, including self-checks, participates. */
static const unsigned char synergy[PL_EVOLUTION_MODULE_COUNT][PL_EVOLUTION_MODULE_COUNT] = {
    {3, 5, 3, 4, 4, 5},
    {5, 3, 5, 4, 4, 5},
    {3, 5, 3, 5, 3, 4},
    {4, 4, 5, 3, 5, 5},
    {4, 4, 3, 5, 3, 5},
    {5, 5, 4, 5, 5, 3}
};

static int evaluate_modules(const pl_evolution_input *input, pl_evolution_report *report) {
    if (pl_concreo_analyze(input, &report->module_scores[PL_EV_CONCREO]) != 0) return -1;
    if (pl_pyemont_plan(input, &report->module_scores[PL_EV_PYEMONT],
                        &report->planned_transformations) != 0) return -1;
    if (pl_bollo_profile(input, &report->module_scores[PL_EV_BOLLO]) != 0) return -1;
    if (pl_jjhh_validate(input, &report->module_scores[PL_EV_JJ_HH]) != 0) return -1;
    if (pl_penwhite_trace(&report->trace_events) != 0) return -1;
    report->module_scores[PL_EV_PENWHITE] = 95u;
    if (pl_serious_gate(input, &report->gate_passed) != 0) return -1;
    report->module_scores[PL_EV_SERIOUS_SERIOUS] = report->gate_passed ? 100u : 0u;
    report->module_invocations += PL_EVOLUTION_MODULE_COUNT;
    return 0;
}

int pl_evolution_run(const pl_evolution_input *input, unsigned max_rounds,
                     pl_evolution_report *report) {
    double previous_score = -1.0;
    if (!valid_input(input) || report == NULL || max_rounds == 0u ||
        max_rounds > PL_EVOLUTION_MAX_ROUNDS) return -1;

    memset(report, 0, sizeof(*report));
    for (unsigned round = 1u; round <= max_rounds; ++round) {
        unsigned score_sum = 0u, weighted_synergy = 0u;
        double base_score;
        if (evaluate_modules(input, report) != 0) return -1;
        for (size_t i = 0u; i < PL_EVOLUTION_MODULE_COUNT; ++i)
            score_sum += report->module_scores[i];
        base_score = (double)score_sum / (double)PL_EVOLUTION_MODULE_COUNT;
        for (size_t from = 0u; from < PL_EVOLUTION_MODULE_COUNT; ++from) {
            for (size_t to = 0u; to < PL_EVOLUTION_MODULE_COUNT; ++to) {
                uint64_t product = (uint64_t)report->module_scores[from] *
                                   (uint64_t)report->module_scores[to];
                ++report->interaction_counts[from][to];
                ++report->pairwise_interactions;
                report->interaction_product_sum += product;
                weighted_synergy += (unsigned)((product * synergy[from][to]) / 1000u);
            }
        }
        report->rounds_completed = round;
        report->synergy_score = 10.0 * (double)weighted_synergy /
            (double)(PL_EVOLUTION_MODULE_COUNT * PL_EVOLUTION_MODULE_COUNT * 50u);
        report->readiness_score = base_score * 0.90 +
            report->synergy_score * 0.10 * (1.0 - 1.0 / (double)(round + 1u));
        if (round > 1u && (report->readiness_score - previous_score) < 0.01) {
            report->converged = 1;
            break;
        }
        previous_score = report->readiness_score;
    }
    if (!report->gate_passed) report->readiness_score *= 0.5;
    return 0;
}
