#ifndef PYTYLIKI_BUDGET_H
#define PYTYLIKI_BUDGET_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PL_BUDGET_CATEGORY_COUNT 6u
#define PL_BUDGET_MAX_UAH INT64_C(1000000)

typedef struct {
    int64_t limit_uah;
    int64_t allocations_uah[PL_BUDGET_CATEGORY_COUNT];
    int64_t planned_uah;
    int64_t remaining_uah;
    int over_budget;
} pl_budget_plan;

const char *pl_budget_category_name(size_t index);
int pl_budget_init(pl_budget_plan *plan, int64_t limit_uah);
int pl_budget_apply_scenario(pl_budget_plan *plan, const char *scenario);
int pl_budget_set_allocation(pl_budget_plan *plan, size_t category, int64_t amount_uah);
int pl_budget_calculate(pl_budget_plan *plan);
void pl_budget_print(FILE *stream, const pl_budget_plan *plan);

#endif
