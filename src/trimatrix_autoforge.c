#include "trimatrix_autoforge.h"

static const pl_tma_cell cells[PL_TMA_CELL_COUNT] = {
    {  1u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DISCOVER, "build.function.discover" },
    {  2u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DESIGN,   "build.function.design" },
    {  3u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_GENERATE, "build.function.generate" },
    {  4u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DISCOVER, "build.automation.discover" },
    {  5u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DESIGN,   "build.automation.design" },
    {  6u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_GENERATE, "build.automation.generate" },
    {  7u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DISCOVER, "build.meta.discover" },
    {  8u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DESIGN,   "build.meta.design" },
    {  9u, PL_TMA_LINE_BUILD,  PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_GENERATE, "build.meta.generate" },
    { 10u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DISCOVER, "evolve.function.discover" },
    { 11u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DESIGN,   "evolve.function.design" },
    { 12u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_GENERATE, "evolve.function.generate" },
    { 13u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DISCOVER, "evolve.automation.discover" },
    { 14u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DESIGN,   "evolve.automation.design" },
    { 15u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_GENERATE, "evolve.automation.generate" },
    { 16u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DISCOVER, "evolve.meta.discover" },
    { 17u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DESIGN,   "evolve.meta.design" },
    { 18u, PL_TMA_LINE_EVOLVE, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_GENERATE, "evolve.meta.generate" },
    { 19u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DISCOVER, "verify.function.discover" },
    { 20u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_DESIGN,   "verify.function.design" },
    { 21u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_FUNCTION,       PL_TMA_STAGE_GENERATE, "verify.function.generate" },
    { 22u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DISCOVER, "verify.automation.discover" },
    { 23u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_DESIGN,   "verify.automation.design" },
    { 24u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_AUTOMATION,     PL_TMA_STAGE_GENERATE, "verify.automation.generate" },
    { 25u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DISCOVER, "verify.meta.discover" },
    { 26u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_DESIGN,   "verify.meta.design" },
    { 27u, PL_TMA_LINE_VERIFY, PL_TMA_MATRIX_META_AUTOMATION,PL_TMA_STAGE_GENERATE, "verify.meta.generate" }
};

static const char *const line_names[] = { "build", "evolve", "verify" };
static const char *const matrix_names[] = { "function", "automation", "meta-automation" };
static const char *const stage_names[] = { "discover", "design", "generate" };

size_t pl_tma_catalog(const pl_tma_cell **out) {
    if (out == NULL) return 0u;
    *out = cells;
    return PL_TMA_CELL_COUNT;
}

int pl_tma_plan_init(pl_tma_plan *plan, uint32_t depth) {
    if (plan == NULL || depth == 0u || depth > PL_TMA_MAX_DEPTH) return -1;
    plan->target_depth = depth;
    plan->current_depth = 1u;
    plan->next_cell = 0u;
    plan->emitted_steps = 0u;
    return 0;
}

uint32_t pl_tma_plan_total(const pl_tma_plan *plan) {
    if (plan == NULL || plan->target_depth == 0u ||
        plan->target_depth > PL_TMA_MAX_DEPTH) return 0u;
    return plan->target_depth * PL_TMA_CELL_COUNT;
}

int pl_tma_plan_next(pl_tma_plan *plan, pl_tma_step *out) {
    uint32_t total;
    uint32_t expected_depth;
    uint32_t expected_cell;
    if (plan == NULL || out == NULL || plan->target_depth == 0u ||
        plan->target_depth > PL_TMA_MAX_DEPTH) return -1;
    total = pl_tma_plan_total(plan);
    if (plan->emitted_steps > total) return -1;
    expected_depth = plan->emitted_steps / PL_TMA_CELL_COUNT + 1u;
    expected_cell = plan->emitted_steps % PL_TMA_CELL_COUNT;
    if (plan->current_depth != expected_depth || plan->next_cell != expected_cell)
        return -1;
    if (plan->emitted_steps == total) return 0;

    out->depth = plan->current_depth;
    out->step = plan->emitted_steps + 1u;
    out->cell = cells[plan->next_cell];
    plan->emitted_steps++;
    plan->next_cell++;
    if (plan->next_cell == PL_TMA_CELL_COUNT) {
        plan->next_cell = 0u;
        plan->current_depth++;
    }
    return 1;
}

const char *pl_tma_line_name(pl_tma_line line) {
    if ((int)line < 0 || line > PL_TMA_LINE_VERIFY) return "invalid";
    return line_names[(unsigned)line];
}

const char *pl_tma_matrix_name(pl_tma_matrix matrix) {
    if ((int)matrix < 0 || matrix > PL_TMA_MATRIX_META_AUTOMATION) return "invalid";
    return matrix_names[(unsigned)matrix];
}

const char *pl_tma_stage_name(pl_tma_stage stage) {
    if ((int)stage < 0 || stage > PL_TMA_STAGE_GENERATE) return "invalid";
    return stage_names[(unsigned)stage];
}
