#include "division.h"

#include <limits.h>
#include <math.h>
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

int pl_partition_plan_analyze(const pl_partition_plan *plan,
                              uint64_t alignment_items,
                              double serial_fraction,
                              pl_partition_stats *out) {
    uint64_t minimum = UINT64_MAX;
    uint64_t maximum = 0u;
    uint64_t unaligned = 0u;

    if (plan == NULL || out == NULL || alignment_items == 0u ||
        plan->partition_count == 0u ||
        plan->partition_count > PL_DIVISION_MAX_PARTITIONS ||
        !isfinite(serial_fraction) || serial_fraction < 0.0 ||
        serial_fraction > 1.0) {
        return -1;
    }

    for (uint64_t index = 0u; index < plan->partition_count; ++index) {
        uint64_t offset = 0u;
        uint64_t count = 0u;
        if (pl_partition_plan_range(plan, index, &offset, &count) != 0) {
            return -1;
        }
        if (count < minimum) minimum = count;
        if (count > maximum) maximum = count;
        if (offset % alignment_items != 0u) ++unaligned;
    }

    out->minimum_count = minimum == UINT64_MAX ? 0u : minimum;
    out->maximum_count = maximum;
    out->unaligned_partitions = unaligned;
    if (plan->item_count == 0u) {
        out->load_imbalance_ratio = 0.0;
        out->tail_fraction = 0.0;
        out->amdahl_speedup_bound = 0.0;
        return 0;
    }

    {
        const double mean = (double)plan->item_count /
                            (double)plan->partition_count;
        const double denominator = serial_fraction +
            (1.0 - serial_fraction) / (double)plan->partition_count;
        out->load_imbalance_ratio = mean > 0.0
            ? (double)maximum / mean : 0.0;
        out->tail_fraction = (double)plan->remainder_count /
                             (double)plan->item_count;
        out->amdahl_speedup_bound = denominator > 0.0
            ? 1.0 / denominator : 0.0;
    }
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
