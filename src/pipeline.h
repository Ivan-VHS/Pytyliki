#ifndef PYTYLIKI_PIPELINE_H
#define PYTYLIKI_PIPELINE_H

#include <stddef.h>

typedef enum {
    PL_TARGET_C11 = 0,
    PL_TARGET_C23
} pl_pipeline_target;

typedef enum {
    PL_STAGE_PARSE = 0,
    PL_STAGE_TYPE_CHECK,
    PL_STAGE_INTERMEDIATE_REPRESENTATION,
    PL_STAGE_C_CODE_GENERATION,
    PL_STAGE_COMPILE,
    PL_STAGE_TEST,
    PL_STAGE_COUNT
} pl_pipeline_stage;

typedef struct {
    pl_pipeline_target target;
    pl_pipeline_stage stages[PL_STAGE_COUNT];
    size_t stage_count;
    int deterministic;
    int semantic_equivalence_required;
    int human_review_required;
    int is_reference_plan;
} pl_pipeline_plan;

/* Builds a deterministic reference plan. This function does not transpile code. */
int pl_pipeline_build(pl_pipeline_target target, pl_pipeline_plan *out);
const char *pl_pipeline_target_name(pl_pipeline_target target);
const char *pl_pipeline_stage_name(pl_pipeline_stage stage);

#endif
