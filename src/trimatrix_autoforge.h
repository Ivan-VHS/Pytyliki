#ifndef PYTYLIKI_TRIMATRIX_AUTOFORGE_H
#define PYTYLIKI_TRIMATRIX_AUTOFORGE_H

#include <stddef.h>
#include <stdint.h>

#define PL_TMA_LINE_COUNT 3u
#define PL_TMA_MATRIX_COUNT 3u
#define PL_TMA_STAGE_COUNT 3u
#define PL_TMA_CELL_COUNT 27u
#define PL_TMA_MAX_DEPTH 100u
#define PL_TMA_MAX_STEPS (PL_TMA_CELL_COUNT * PL_TMA_MAX_DEPTH)

typedef enum {
    PL_TMA_LINE_BUILD = 0,
    PL_TMA_LINE_EVOLVE,
    PL_TMA_LINE_VERIFY
} pl_tma_line;

typedef enum {
    PL_TMA_MATRIX_FUNCTION = 0,
    PL_TMA_MATRIX_AUTOMATION,
    PL_TMA_MATRIX_META_AUTOMATION
} pl_tma_matrix;

typedef enum {
    PL_TMA_STAGE_DISCOVER = 0,
    PL_TMA_STAGE_DESIGN,
    PL_TMA_STAGE_GENERATE
} pl_tma_stage;

typedef struct {
    unsigned id;
    pl_tma_line line;
    pl_tma_matrix matrix;
    pl_tma_stage stage;
    const char *name;
} pl_tma_cell;

typedef struct {
    uint32_t target_depth;
    uint32_t current_depth;
    uint32_t next_cell;
    uint32_t emitted_steps;
} pl_tma_plan;

typedef struct {
    uint32_t depth;
    uint32_t step;
    pl_tma_cell cell;
} pl_tma_step;

/* Returns the immutable 3 x 3 x 3 catalogue. */
size_t pl_tma_catalog(const pl_tma_cell **out);

/* Initializes a bounded plan for 1..100 logical generations. */
int pl_tma_plan_init(pl_tma_plan *plan, uint32_t depth);

/* Emits the next deterministic planning step: 1=step, 0=complete, -1=invalid. */
int pl_tma_plan_next(pl_tma_plan *plan, pl_tma_step *out);

/* Returns total planned steps, or 0 for an invalid plan. */
uint32_t pl_tma_plan_total(const pl_tma_plan *plan);

/* Human-readable labels with static lifetime. */
const char *pl_tma_line_name(pl_tma_line line);
const char *pl_tma_matrix_name(pl_tma_matrix matrix);
const char *pl_tma_stage_name(pl_tma_stage stage);

#endif
