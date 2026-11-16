#ifndef PYTYLIKI_IMPROVEMENT_H
#define PYTYLIKI_IMPROVEMENT_H

#include <stddef.h>
#include <stdint.h>

#define PL_IMPROVEMENT_CAPABILITY_COUNT 20u
#define PL_IMPROVEMENT_LABEL_MAX 40u

typedef struct {
    unsigned id;
    const char *engineering;
    const char *optimization;
    const char *automation;
} pl_improvement_capability;

typedef enum {
    PL_IMPROVE_OBSERVE = 0,
    PL_IMPROVE_DIAGNOSE,
    PL_IMPROVE_PROPOSE,
    PL_IMPROVE_IMPLEMENT,
    PL_IMPROVE_VERIFY,
    PL_IMPROVE_EVALUATE,
    PL_IMPROVE_REVIEW,
    PL_IMPROVE_INTEGRATE,
    PL_IMPROVE_LEARN,
    PL_IMPROVE_IMPROVE_ENGINE,
    PL_IMPROVE_COMPLETE
} pl_improvement_phase;

enum {
    PL_IMPROVE_EVIDENCE_OBSERVED       = 1u << 0,
    PL_IMPROVE_EVIDENCE_DIAGNOSED      = 1u << 1,
    PL_IMPROVE_EVIDENCE_PROPOSED       = 1u << 2,
    PL_IMPROVE_EVIDENCE_IMPLEMENTED    = 1u << 3,
    PL_IMPROVE_EVIDENCE_TESTED         = 1u << 4,
    PL_IMPROVE_EVIDENCE_EVALUATED      = 1u << 5,
    PL_IMPROVE_EVIDENCE_REVIEWED       = 1u << 6,
    PL_IMPROVE_EVIDENCE_APPROVED       = 1u << 7,
    PL_IMPROVE_EVIDENCE_INTEGRATED     = 1u << 8,
    PL_IMPROVE_EVIDENCE_LEARNED        = 1u << 9,
    PL_IMPROVE_EVIDENCE_META_EVALUATED = 1u << 10
};

typedef struct {
    pl_improvement_phase phase;
    uint32_t evidence;
    uint32_t generation;
} pl_improvement_cycle;

/* Static, deterministic catalogue. Returned strings have static lifetime. */
size_t pl_improvement_catalog(const pl_improvement_capability **out);

/* Start a new bounded improvement cycle. */
int pl_improvement_cycle_init(pl_improvement_cycle *cycle);

/* Record evidence for the current phase; evidence cannot be fabricated by advance. */
int pl_improvement_record(pl_improvement_cycle *cycle, uint32_t evidence);

/* Advance exactly one phase when its evidence gate is satisfied.
 * Integration requires explicit human approval evidence. */
int pl_improvement_advance(pl_improvement_cycle *cycle);

/* Human-readable phase label; returned string has static lifetime. */
const char *pl_improvement_phase_name(pl_improvement_phase phase);

#endif
