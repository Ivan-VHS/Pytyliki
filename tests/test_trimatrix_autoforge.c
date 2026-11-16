#include "trimatrix_autoforge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_tma_cell *catalog = NULL;
    pl_tma_plan plan;
    pl_tma_step step;
    assert(pl_tma_catalog(&catalog) == PL_TMA_CELL_COUNT);
    assert(catalog != NULL);
    for (unsigned i = 0u; i < PL_TMA_CELL_COUNT; ++i) {
        assert(catalog[i].id == i + 1u);
        assert(catalog[i].name != NULL && catalog[i].name[0] != '\0');
        assert((unsigned)catalog[i].line < PL_TMA_LINE_COUNT);
        assert((unsigned)catalog[i].matrix < PL_TMA_MATRIX_COUNT);
        assert((unsigned)catalog[i].stage < PL_TMA_STAGE_COUNT);
        for (unsigned j = i + 1u; j < PL_TMA_CELL_COUNT; ++j)
            assert(strcmp(catalog[i].name, catalog[j].name) != 0);
    }
    assert(pl_tma_catalog(NULL) == 0u);
    assert(strcmp(pl_tma_line_name(PL_TMA_LINE_BUILD), "build") == 0);
    assert(strcmp(pl_tma_matrix_name(PL_TMA_MATRIX_META_AUTOMATION), "meta-automation") == 0);
    assert(strcmp(pl_tma_stage_name(PL_TMA_STAGE_GENERATE), "generate") == 0);
    assert(strcmp(pl_tma_line_name((pl_tma_line)-1), "invalid") == 0);

    assert(pl_tma_plan_init(NULL, 1u) == -1);
    assert(pl_tma_plan_init(&plan, 0u) == -1);
    assert(pl_tma_plan_init(&plan, PL_TMA_MAX_DEPTH + 1u) == -1);
    assert(pl_tma_plan_init(&plan, 100u) == 0);
    assert(pl_tma_plan_total(&plan) == 2700u);

    for (uint32_t depth = 1u; depth <= PL_TMA_MAX_DEPTH; ++depth) {
        for (uint32_t cell = 0u; cell < PL_TMA_CELL_COUNT; ++cell) {
            assert(pl_tma_plan_next(&plan, &step) == 1);
            assert(step.depth == depth);
            assert(step.step == (depth - 1u) * PL_TMA_CELL_COUNT + cell + 1u);
            assert(step.cell.id == cell + 1u);
        }
    }
    assert(pl_tma_plan_next(&plan, &step) == -1);
    assert(plan.emitted_steps == PL_TMA_MAX_STEPS);
    assert(plan.current_depth == PL_TMA_MAX_DEPTH + 1u);

    assert(pl_tma_plan_init(&plan, 1u) == 0);
    assert(pl_tma_plan_total(&plan) == 27u);
    for (unsigned i = 0u; i < 27u; ++i) assert(pl_tma_plan_next(&plan, &step) == 1);
    assert(plan.current_depth == 2u);
    assert(pl_tma_plan_next(&plan, &step) == -1);
    assert(pl_tma_plan_total(NULL) == 0u);
    puts("TriMatrix AutoForge 100 tests passed.");
    return 0;
}
