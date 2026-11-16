#include "improvement.h"

static const pl_improvement_capability capabilities[PL_IMPROVEMENT_CAPABILITY_COUNT] = {
    { 1u, "Modernizing", "Modernizing", "Auto-optimizing" },
    { 2u, "Optimizing", "Optimizing", "Auto-modernizing" },
    { 3u, "Refining", "Transpiling", "Auto-adapting" },
    { 4u, "Refactoring", "Compiling", "Auto-correcting" },
    { 5u, "Transforming", "Reconfiguring", "Auto-compiling" },
    { 6u, "Accelerating", "Scaling", "Auto-transpiling" },
    { 7u, "Adapting", "Accelerating", "Auto-diagnosing" },
    { 8u, "Scaling", "Synthesizing", "Auto-verifying" },
    { 9u, "Automating", "Decomposing", "Auto-calibrating" },
    { 10u, "Integrating", "Integrating", "Auto-tuning" },
    { 11u, "Stabilizing", "Adapting", "Auto-configuring" },
    { 12u, "Unifying", "Automating", "Auto-automating" },
    { 13u, "Rationalizing", "Parallelizing", "Auto-integrating" },
    { 14u, "Rebuilding", "Verifying", "Auto-scaling" },
    { 15u, "Redesigning", "Diagnosing", "Auto-accelerating" },
    { 16u, "Self-learning", "Stabilizing", "Auto-recovering" },
    { 17u, "Self-tuning", "Reconstructing", "Auto-synthesizing" },
    { 18u, "Self-recovering", "Initializing", "Auto-reconfiguring" },
    { 19u, "Self-optimizing", "Crystallizing", "Auto-learning" },
    { 20u, "Evolving", "Perfecting", "Auto-perfecting" }
};

static const uint32_t required_evidence[] = {
    PL_IMPROVE_EVIDENCE_OBSERVED,
    PL_IMPROVE_EVIDENCE_DIAGNOSED,
    PL_IMPROVE_EVIDENCE_PROPOSED,
    PL_IMPROVE_EVIDENCE_IMPLEMENTED,
    PL_IMPROVE_EVIDENCE_TESTED,
    PL_IMPROVE_EVIDENCE_EVALUATED,
    PL_IMPROVE_EVIDENCE_REVIEWED,
    PL_IMPROVE_EVIDENCE_INTEGRATED,
    PL_IMPROVE_EVIDENCE_LEARNED,
    PL_IMPROVE_EVIDENCE_META_EVALUATED
};

static const char *const phase_names[] = {
    "observe", "diagnose", "propose", "implement", "verify",
    "evaluate", "review", "integrate", "learn", "improve-engine", "complete"
};

size_t pl_improvement_catalog(const pl_improvement_capability **out) {
    if (out == NULL) return 0u;
    *out = capabilities;
    return PL_IMPROVEMENT_CAPABILITY_COUNT;
}

int pl_improvement_cycle_init(pl_improvement_cycle *cycle) {
    if (cycle == NULL) return -1;
    cycle->phase = PL_IMPROVE_OBSERVE;
    cycle->evidence = 0u;
    cycle->generation = 1u;
    cycle->completed_iterations = 0u;
    cycle->target_iterations = PL_IMPROVEMENT_DEFAULT_ITERATIONS;
    return 0;
}

int pl_improvement_cycle_set_target(pl_improvement_cycle *cycle, uint64_t target) {
    if (cycle == NULL || target == 0u || target > PL_IMPROVEMENT_MAX_ITERATIONS) return -1;
    if (cycle->phase != PL_IMPROVE_OBSERVE || cycle->evidence != 0u ||
        cycle->completed_iterations != 0u) return -2;
    cycle->target_iterations = target;
    return 0;
}

int pl_improvement_record(pl_improvement_cycle *cycle, uint32_t evidence) {
    const uint32_t known = (1u << 11) - 1u;
    uint32_t allowed;
    if (cycle == NULL || cycle->phase < PL_IMPROVE_OBSERVE || cycle->phase >= PL_IMPROVE_COMPLETE ||
        cycle->target_iterations == 0u ||
        cycle->target_iterations > PL_IMPROVEMENT_MAX_ITERATIONS ||
        evidence == 0u || (evidence & ~known) != 0u) return -1;
    allowed = required_evidence[cycle->phase];
    if (cycle->phase == PL_IMPROVE_INTEGRATE)
        allowed |= PL_IMPROVE_EVIDENCE_APPROVED;
    if ((evidence & ~allowed) != 0u) return -1;
    cycle->evidence |= evidence;
    return 0;
}

int pl_improvement_advance(pl_improvement_cycle *cycle) {
    if (cycle == NULL || (int)cycle->phase < (int)PL_IMPROVE_OBSERVE ||
        cycle->phase >= PL_IMPROVE_COMPLETE) return -1;
    if (cycle->target_iterations == 0u ||
        cycle->target_iterations > PL_IMPROVEMENT_MAX_ITERATIONS) return -1;
    if ((cycle->evidence & required_evidence[cycle->phase]) == 0u) return -2;
    if (cycle->phase == PL_IMPROVE_INTEGRATE &&
        (cycle->evidence & PL_IMPROVE_EVIDENCE_APPROVED) == 0u) return -3;

    if (cycle->phase == PL_IMPROVE_IMPROVE_ENGINE) {
        if (cycle->generation == UINT32_MAX ||
            cycle->completed_iterations >= cycle->target_iterations) return -4;
        cycle->completed_iterations++;
        cycle->generation++;
        cycle->evidence = 0u;
        cycle->phase = cycle->completed_iterations == cycle->target_iterations
            ? PL_IMPROVE_COMPLETE : PL_IMPROVE_OBSERVE;
        return 0;
    }

    cycle->phase = (pl_improvement_phase)(cycle->phase + 1);
    return 0;
}

const char *pl_improvement_phase_name(pl_improvement_phase phase) {
    if ((int)phase < (int)PL_IMPROVE_OBSERVE || phase > PL_IMPROVE_COMPLETE) return "invalid";
    return phase_names[(unsigned)phase];
}
