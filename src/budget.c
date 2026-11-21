#include "budget.h"

#include <inttypes.h>
#include <string.h>

static const char *const category_names[PL_BUDGET_CATEGORY_COUNT] = {
    "Продукты", "Транспорт", "Быт и здоровье",
    "Связь и обучение", "Досуг и кофе", "Резерв"
};

static const int64_t weekly[PL_BUDGET_CATEGORY_COUNT] =
    {280, 120, 0, 0, 50, 50};
static const int64_t groceries[PL_BUDGET_CATEGORY_COUNT] =
    {500, 0, 0, 0, 0, 0};
static const int64_t transport[PL_BUDGET_CATEGORY_COUNT] =
    {0, 500, 0, 0, 0, 0};
static const int64_t coffee[PL_BUDGET_CATEGORY_COUNT] =
    {0, 0, 0, 0, 460, 40};
static const int64_t household[PL_BUDGET_CATEGORY_COUNT] =
    {0, 0, 500, 0, 0, 0};
static const int64_t learning[PL_BUDGET_CATEGORY_COUNT] =
    {0, 0, 0, 500, 0, 0};
static const int64_t reserve[PL_BUDGET_CATEGORY_COUNT] =
    {0, 0, 0, 0, 0, 500};

const char *pl_budget_category_name(size_t index) {
    return index < PL_BUDGET_CATEGORY_COUNT ? category_names[index] : "";
}

int pl_budget_calculate(pl_budget_plan *plan) {
    int64_t total = 0;
    size_t i;
    if (plan == NULL || plan->limit_uah < 0 || plan->limit_uah > PL_BUDGET_MAX_UAH)
        return -1;
    for (i = 0; i < PL_BUDGET_CATEGORY_COUNT; ++i) {
        const int64_t value = plan->allocations_uah[i];
        if (value < 0 || value > PL_BUDGET_MAX_UAH || total > PL_BUDGET_MAX_UAH - value)
            return -1;
        total += value;
    }
    plan->planned_uah = total;
    plan->over_budget = total > plan->limit_uah;
    plan->remaining_uah = plan->over_budget ? plan->limit_uah - total : plan->limit_uah - total;
    return 0;
}

int pl_budget_init(pl_budget_plan *plan, int64_t limit_uah) {
    if (plan == NULL) return -1;
    memset(plan, 0, sizeof(*plan));
    plan->limit_uah = limit_uah;
    memcpy(plan->allocations_uah, weekly, sizeof(weekly));
    return pl_budget_calculate(plan);
}

int pl_budget_apply_scenario(pl_budget_plan *plan, const char *scenario) {
    const int64_t *values = NULL;
    if (plan == NULL || scenario == NULL) return -1;
    if (strcmp(scenario, "weekly") == 0) values = weekly;
    else if (strcmp(scenario, "groceries") == 0) values = groceries;
    else if (strcmp(scenario, "transport") == 0) values = transport;
    else if (strcmp(scenario, "coffee") == 0) values = coffee;
    else if (strcmp(scenario, "household") == 0) values = household;
    else if (strcmp(scenario, "learning") == 0) values = learning;
    else if (strcmp(scenario, "reserve") == 0) values = reserve;
    else return -1;
    memcpy(plan->allocations_uah, values, sizeof(plan->allocations_uah));
    return pl_budget_calculate(plan);
}

int pl_budget_set_allocation(pl_budget_plan *plan, size_t category, int64_t amount_uah) {
    if (plan == NULL || category >= PL_BUDGET_CATEGORY_COUNT ||
        amount_uah < 0 || amount_uah > PL_BUDGET_MAX_UAH)
        return -1;
    plan->allocations_uah[category] = amount_uah;
    return pl_budget_calculate(plan);
}

void pl_budget_print(FILE *stream, const pl_budget_plan *plan) {
    size_t i;
    if (stream == NULL || plan == NULL) return;
    fprintf(stream, "Планировщик бюджета | лимит: %" PRId64 " грн\n", plan->limit_uah);
    for (i = 0; i < PL_BUDGET_CATEGORY_COUNT; ++i)
        fprintf(stream, "  %-24s %4" PRId64 " грн\n",
                category_names[i], plan->allocations_uah[i]);
    fprintf(stream, "Запланировано: %" PRId64 " грн\n", plan->planned_uah);
    if (plan->over_budget)
        fprintf(stream, "Превышение: %" PRId64 " грн\n", -plan->remaining_uah);
    else
        fprintf(stream, "Остаток: %" PRId64 " грн\n", plan->remaining_uah);
    fprintf(stream, "Статус: %s\n", plan->over_budget ? "ПРЕВЫШЕН ЛИМИТ" : "В ПРЕДЕЛАХ БЮДЖЕТА");
    fputs("Сценарии: weekly, groceries, transport, coffee, household, learning, reserve\n", stream);
    fputs("Цены из исходного материала — ориентиры, не проверенные котировки магазинов.\n", stream);
}
