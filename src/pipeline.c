#include "pipeline.h"

const char *pl_pipeline_target_name(pl_pipeline_target target) {
    switch (target) {
        case PL_TARGET_C11: return "ISO C11";
        case PL_TARGET_C23: return "ISO C23";
        default: return "unknown";
    }
}

const char *pl_pipeline_stage_name(pl_pipeline_stage stage) {
    switch (stage) {
        case PL_STAGE_PARSE: return "lexical and syntax analysis";
        case PL_STAGE_TYPE_CHECK: return "type and dependency validation";
        case PL_STAGE_INTERMEDIATE_REPRESENTATION: return "intermediate representation (IR)";
        case PL_STAGE_C_CODE_GENERATION: return "C source generation";
        case PL_STAGE_COMPILE: return "compiler validation";
        case PL_STAGE_TEST: return "semantic and regression tests";
        default: return "unknown";
    }
}

int pl_pipeline_build(pl_pipeline_target target, pl_pipeline_plan *out) {
    static const pl_pipeline_stage ordered_stages[PL_STAGE_COUNT] = {
        PL_STAGE_PARSE,
        PL_STAGE_TYPE_CHECK,
        PL_STAGE_INTERMEDIATE_REPRESENTATION,
        PL_STAGE_C_CODE_GENERATION,
        PL_STAGE_COMPILE,
        PL_STAGE_TEST
    };
    size_t i;

    if (out == NULL || (target != PL_TARGET_C11 && target != PL_TARGET_C23)) return -1;

    out->target = target;
    out->stage_count = PL_STAGE_COUNT;
    out->deterministic = 1;
    out->semantic_equivalence_required = 1;
    out->human_review_required = 1;
    out->is_reference_plan = 1;

    for (i = 0; i < PL_STAGE_COUNT; ++i) out->stages[i] = ordered_stages[i];
    return 0;
}
