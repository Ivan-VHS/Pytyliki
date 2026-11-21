#include "division.h"

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

static void check_condition(int condition, const char *expression,
                            const char *file, int line) {
    if (!condition) {
        fprintf(stderr, "CHECK failed at %s:%d: %s\n", file, line, expression);
        exit(EXIT_FAILURE);
    }
}
#define CHECK(condition) check_condition((condition), #condition, __FILE__, __LINE__)

static void expect_division(int64_t a, int64_t b,
                            int64_t quotient, int64_t remainder) {
    pl_division_result result;
    CHECK(pl_division_euclidean(a, b, &result) == 0);
    CHECK(result.quotient == quotient);
    CHECK(result.remainder == remainder);
}

int main(void) {
    pl_partition_plan plan;
    pl_division_result result;
    uint64_t offset, count, cursor = 0u, allocated = 0u, workers = 0u;
    uint64_t shares[3];
    uint32_t weights[3] = {1u, 2u, 3u};
    int exact = 0;

    expect_division(17, 5, 3, 2);
    expect_division(-17, 5, -4, 3);
    expect_division(17, -5, -3, 2);
    expect_division(-17, -5, 4, 3);
    CHECK(pl_division_euclidean(9, 0, &result) != 0);
    CHECK(pl_division_euclidean(INT64_MIN, -1, &result) != 0);
    CHECK(pl_division_is_exact(42, 7, &exact) == 0 && exact == 1);
    CHECK(pl_division_is_exact(43, 7, &exact) == 0 && exact == 0);
    CHECK(pl_division_is_exact(1, 0, &exact) != 0);

    CHECK(pl_partition_plan_create(10u, 3u, &plan) == 0);
    for (uint64_t i = 0u; i < plan.partition_count; ++i) {
        CHECK(pl_partition_plan_range(&plan, i, &offset, &count) == 0);
        CHECK(offset == cursor);
        cursor += count;
        allocated += count;
    }
    CHECK(cursor == 10u && allocated == 10u);
    CHECK(pl_partition_plan_create(2u, 5u, &plan) == 0);
    CHECK(pl_partition_plan_range(&plan, 4u, &offset, &count) == 0);
    CHECK(offset == 2u && count == 0u);
    CHECK(pl_partition_plan_create(1u, 0u, &plan) != 0);
    CHECK(pl_partition_plan_create(1u, PL_DIVISION_MAX_PARTITIONS + 1u, &plan) != 0);

    CHECK(pl_division_recommend_workers(10000u, 16u, 256u, &workers) == 0);
    CHECK(workers == 16u);
    CHECK(pl_division_recommend_workers(100u, 16u, 256u, &workers) == 0);
    CHECK(workers == 1u);
    CHECK(pl_division_recommend_workers(0u, 16u, 256u, &workers) == 0);
    CHECK(workers == 0u);
    CHECK(pl_division_recommend_workers(10u, 0u, 1u, &workers) != 0);

    CHECK(pl_division_weighted_split(10u, weights, 3u, shares) == 0);
    CHECK(shares[0] == 2u && shares[1] == 3u && shares[2] == 5u);
    CHECK(shares[0] + shares[1] + shares[2] == 10u);
    CHECK(pl_division_weighted_split(1u, weights, 0u, shares) != 0);
    CHECK(pl_division_weighted_split((uint64_t)UINT32_MAX + 1u, weights, 3u, shares) != 0);
    {
        const uint32_t zero_weights[2] = {0u, 0u};
        CHECK(pl_division_weighted_split(5u, zero_weights, 2u, shares) != 0);
    }

    puts("Pytyliki division, allocation and partition tests passed.");
    return 0;
}
