#include "division.h"

#include <limits.h>
#include <stdint.h>

int pl_division_euclidean(int64_t dividend, int64_t divisor,
                          pl_division_result *out) {
    int64_t quotient;
    int64_t remainder;
    if (out == NULL || divisor == 0 ||
        (dividend == INT64_MIN && divisor == -1)) {
        return -1;
    }

    quotient = dividend / divisor;
    remainder = dividend % divisor;
    if (remainder < 0) {
        if (divisor > 0) {
            --quotient;
            remainder += divisor;
        } else {
            ++quotient;
            remainder -= divisor;
        }
    }

    out->quotient = quotient;
    out->remainder = remainder;
    return 0;
}

int pl_division_is_exact(int64_t dividend, int64_t divisor, int *out) {
    pl_division_result result;
    if (out == NULL || pl_division_euclidean(dividend, divisor, &result) != 0)
        return -1;
    *out = result.remainder == 0 ? 1 : 0;
    return 0;
}

int pl_partition_plan_create(uint64_t item_count, uint64_t partition_count,
                             pl_partition_plan *out) {
    if (out == NULL || partition_count == 0u ||
        partition_count > PL_DIVISION_MAX_PARTITIONS) {
        return -1;
    }
    out->item_count = item_count;
    out->partition_count = partition_count;
    out->base_count = item_count / partition_count;
    out->remainder_count = item_count % partition_count;
    return 0;
}

int pl_partition_plan_range(const pl_partition_plan *plan, uint64_t index,
                            uint64_t *offset, uint64_t *count) {
    if (plan == NULL || offset == NULL || count == NULL ||
        plan->partition_count == 0u ||
        plan->partition_count > PL_DIVISION_MAX_PARTITIONS ||
        index >= plan->partition_count) {
        return -1;
    }
    *count = plan->base_count + (index < plan->remainder_count ? 1u : 0u);
    *offset = index * plan->base_count +
        (index < plan->remainder_count ? index : plan->remainder_count);
    return 0;
}

int pl_division_recommend_workers(uint64_t work_items,
                                  uint64_t available_workers,
                                  uint64_t minimum_grain,
                                  uint64_t *recommended) {
    uint64_t candidate;
    if (recommended == NULL || available_workers == 0u || minimum_grain == 0u)
        return -1;
    if (work_items == 0u) {
        *recommended = 0u;
        return 0;
    }
    candidate = work_items / minimum_grain;
    if (candidate == 0u) candidate = 1u;
    if (candidate > available_workers) candidate = available_workers;
    if (candidate > work_items) candidate = work_items;
    if (candidate > PL_DIVISION_MAX_PARTITIONS)
        candidate = PL_DIVISION_MAX_PARTITIONS;
    *recommended = candidate;
    return 0;
}

int pl_division_weighted_split(uint64_t total, const uint32_t *weights,
                               size_t weight_count, uint64_t *allocations) {
    uint64_t remainders[PL_DIVISION_MAX_PARTITIONS];
    uint64_t weight_sum = 0u;
    uint64_t assigned = 0u;
    uint64_t remaining;
    if (weights == NULL || allocations == NULL || weight_count == 0u ||
        weight_count > PL_DIVISION_MAX_PARTITIONS ||
        total > PL_DIVISION_MAX_WEIGHTED_TOTAL) {
        return -1;
    }
    for (size_t i = 0u; i < weight_count; ++i) {
        if (weight_sum > (uint64_t)UINT32_MAX - (uint64_t)weights[i])
            return -1;
        weight_sum += (uint64_t)weights[i];
    }
    if (weight_sum == 0u) return -1;

    for (size_t i = 0u; i < weight_count; ++i) {
        uint64_t product = total * (uint64_t)weights[i];
        allocations[i] = product / weight_sum;
        remainders[i] = product % weight_sum;
        assigned += allocations[i];
    }
    remaining = total - assigned;

    for (uint64_t extra = 0u; extra < remaining; ++extra) {
        size_t best = weight_count;
        uint64_t best_remainder = 0u;
        for (size_t i = 0u; i < weight_count; ++i) {
            if (remainders[i] != UINT64_MAX &&
                (best == weight_count || remainders[i] > best_remainder)) {
                best = i;
                best_remainder = remainders[i];
            }
        }
        if (best == weight_count) return -1;
        ++allocations[best];
        remainders[best] = UINT64_MAX;
    }
    return 0;
}
