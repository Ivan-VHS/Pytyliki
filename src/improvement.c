#include "improvement.h"

static const pl_improvement_capability capabilities[PL_IMPROVEMENT_CAPABILITY_COUNT] = {
    { 1u, "Modernize", "Optimize", "Auto-optimize" },
    { 2u, "Optimize", "Transpile", "Auto-modernize" },
    { 3u, "Refine", "Compile", "Auto-adapt" },
    { 4u, "Refactor", "Reconfigure", "Auto-correct" },
    { 5u, "Transform", "Scale", "Auto-compile" },
    { 6u, "Accelerate", "Synthesize", "Auto-transpile" },
    { 7u, "Adapt", "Decompose", "Auto-diagnose" },
    { 8u, "Scale", "Integrate", "Auto-verify" },
    { 9u, "Automate", "Stabilize", "Auto-calibrate" },
    { 10u, "Integrate", "Unify", "Auto-tune" },
    { 11u, "Stabilize", "Rationalize", "Auto-configure" },
    { 12u, "Unify", "Rebuild", "Auto-automate" },
    { 13u, "Rationalize", "Parallelize", "Auto-integrate" },
    { 14u, "Rebuild", "Verify", "Auto-scale" },
    { 15u, "Redesign", "Diagnose", "Auto-accelerate" },
    { 16u, "Self-learn", "Stabilize", "Auto-recover" },
    { 17u, "Self-tune", "Reconstruct", "Auto-synthesize" },
    { 18u, "Self-recover", "Initialize", "Auto-reconfigure" },
    { 19u, "Self-optimize", "Crystallize", "Auto-learn" },
    { 20u, "Evolve", "Improve", "Self-improve" }
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
    return 0;
}

int pl_improvement_record(pl_improvement_cycle *cycle, uint32_t evidence) {
    const uint32_t known = (1u << 11) - 1u;
    uint32_t allowed;
    if (cycle == NULL || cycle->phase < PL_IMPROVE_OBSERVE || cycle->phase >= PL_IMPROVE_COMPLETE ||
        evidence == 0u || (evidence & ~known) != 0u) return -1;
    allowed = required_evidence[cycle->phase];
    if (cycle->phase == PL_IMPROVE_INTEGRATE)
        allowed |= PL_IMPROVE_EVIDENCE_APPROVED;
    if ((evidence & ~allowed) != 0u) return -1;
    cycle->evidence |= evidence;
    return 0;
}

int pl_improvement_advance(pl_improvement_cycle *cycle) {
    if (cycle == NULL || cycle->phase >= PL_IMPROVE_COMPLETE) return -1;
    if ((cycle->evidence & required_evidence[cycle->phase]) == 0u) return -2;
    if (cycle->phase == PL_IMPROVE_INTEGRATE &&
        (cycle->evidence & PL_IMPROVE_EVIDENCE_APPROVED) == 0u) return -3;
    if (cycle->phase == PL_IMPROVE_IMPROVE_ENGINE &&
        cycle->generation == UINT32_MAX) return -4;
    cycle->phase = (pl_improvement_phase)(cycle->phase + 1);
    if (cycle->phase == PL_IMPROVE_COMPLETE) cycle->generation++;
    return 0;
}

const char *pl_improvement_phase_name(pl_improvement_phase phase) {
    if ((int)phase < (int)PL_IMPROVE_OBSERVE || phase > PL_IMPROVE_COMPLETE) return "invalid";
    return phase_names[(unsigned)phase];
}
