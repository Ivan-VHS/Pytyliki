#ifndef PYTYLIKI_DIVISION_H
#define PYTYLIKI_DIVISION_H

#include <stddef.h>
#include <stdint.h>

#define PL_DIVISION_MAX_PARTITIONS 4096u
#define PL_DIVISION_MAX_WEIGHTED_TOTAL UINT32_MAX

typedef struct {
    int64_t quotient;
    int64_t remainder;
} pl_division_result;

typedef struct {
    uint64_t item_count;
    uint64_t partition_count;
    uint64_t base_count;
    uint64_t remainder_count;
} pl_partition_plan;

/* Euclidean result: dividend = quotient * divisor + remainder,
 * with 0 <= remainder < abs(divisor). Returns 0 on success. */
int pl_division_euclidean(int64_t dividend, int64_t divisor,
                          pl_division_result *out);
int pl_division_is_exact(int64_t dividend, int64_t divisor, int *out);

/* Balanced contiguous ranges. Earlier ranges receive the extra items. */
int pl_partition_plan_create(uint64_t item_count, uint64_t partition_count,
                             pl_partition_plan *out);
int pl_partition_plan_range(const pl_partition_plan *plan, uint64_t index,
                            uint64_t *offset, uint64_t *count);

/* Bounded, advisory partition recommendation based on work grain. */
int pl_division_recommend_workers(uint64_t work_items,
                                  uint64_t available_workers,
                                  uint64_t minimum_grain,
                                  uint64_t *recommended);

/* Exact largest-remainder apportionment. total and the sum of weights are
 * bounded to UINT32_MAX so every intermediate multiplication is checked. */
int pl_division_weighted_split(uint64_t total, const uint32_t *weights,
                               size_t weight_count, uint64_t *allocations);

#endif
