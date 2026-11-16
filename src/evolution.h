#ifndef PYTYLIKI_EVOLUTION_H
#define PYTYLIKI_EVOLUTION_H

#include <stdint.h>

#define PL_EVOLUTION_MODULE_COUNT 6u
#define PL_EVOLUTION_MAX_ROUNDS 64u

typedef enum {
    PL_EV_CONCREO = 0,
    PL_EV_PYEMONT,
    PL_EV_BOLLO,
    PL_EV_JJ_HH,
    PL_EV_PENWHITE,
    PL_EV_SERIOUS_SERIOUS
} pl_evolution_module;

typedef struct {
    unsigned source_files;
    unsigned static_findings;
    unsigned benchmark_samples;
    double baseline_iterations_per_ms;
    unsigned memory_mib;
    unsigned architecture_known;
    unsigned tests_passed;
} pl_evolution_input;

typedef struct {
    unsigned rounds_completed;
    uint64_t module_invocations;
    uint64_t pairwise_interactions;
    uint64_t interaction_product_sum;
    uint64_t interaction_counts[PL_EVOLUTION_MODULE_COUNT][PL_EVOLUTION_MODULE_COUNT];
    unsigned module_scores[PL_EVOLUTION_MODULE_COUNT];
    unsigned trace_events;
    unsigned planned_transformations;
    double readiness_score;
    double synergy_score;
    int converged;
    int gate_passed;
} pl_evolution_report;

/* These functions create bounded analysis/planning evidence; they do not rewrite source files. */
int pl_concreo_analyze(const pl_evolution_input *input, unsigned *score);
int pl_pyemont_plan(const pl_evolution_input *input, unsigned *score, unsigned *planned);
int pl_bollo_profile(const pl_evolution_input *input, unsigned *score);
int pl_jjhh_validate(const pl_evolution_input *input, unsigned *score);
int pl_penwhite_trace(unsigned *trace_events);
int pl_serious_gate(const pl_evolution_input *input, int *gate_passed);

/* Executes up to max_rounds of six-module evaluation and all 36 directed pair interactions. */
int pl_evolution_run(const pl_evolution_input *input, unsigned max_rounds,
                     pl_evolution_report *report);

#endif
