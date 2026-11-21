#include "budget.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    pl_budget_plan plan;
    assert(pl_budget_init(&plan, 500) == 0);
    assert(plan.planned_uah == 500);
    assert(plan.remaining_uah == 0);
    assert(!plan.over_budget);
    assert(pl_budget_apply_scenario(&plan, "coffee") == 0);
    assert(plan.planned_uah == 500);
    assert(plan.allocations_uah[4] == 460);
    assert(plan.allocations_uah[5] == 40);
    assert(pl_budget_set_allocation(&plan, 4, 0) == 0);
    assert(pl_budget_set_allocation(&plan, 5, 0) == 0);
    assert(pl_budget_set_allocation(&plan, 0, 550) == 0);
    assert(plan.over_budget);
    assert(plan.remaining_uah == -50);
    assert(pl_budget_set_allocation(&plan, 0, -1) != 0);
    assert(pl_budget_set_allocation(&plan, PL_BUDGET_CATEGORY_COUNT, 1) != 0);
    assert(pl_budget_apply_scenario(&plan, "unknown") != 0);
    assert(pl_budget_init(&plan, PL_BUDGET_MAX_UAH + 1) != 0);
    puts("Pytyliki budget planner tests: PASS");
    return 0;
}
