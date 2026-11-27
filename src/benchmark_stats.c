#include "benchmark_stats.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static int compare_double(const void *left, const void *right) {
    const double a = *(const double *)left;
    const double b = *(const double *)right;
    return (a > b) - (a < b);
}

static double percentile_nearest_rank(const double *sorted, size_t count, double percentile) {
    size_t rank = (size_t)ceil(percentile * (double)count);
    if (rank == 0u) rank = 1u;
    if (rank > count) rank = count;
    return sorted[rank - 1u];
}

static double median_of_sorted(const double *sorted, size_t count) {
    const size_t middle = count / 2u;
    if ((count & 1u) != 0u) return sorted[middle];

    /*
     * Inputs are non-negative and sorted, so the difference cannot overflow.
     * This form preserves subnormal precision that can be lost by halving
     * both endpoints independently.
     */
    return sorted[middle - 1u] +
        (sorted[middle] - sorted[middle - 1u]) / 2.0;
}

int pl_benchmark_stats_analyze(
    const double *samples,
    size_t count,
    pl_benchmark_stats *out
) {
    double *sorted;
    double *deviations;
    double scale = 0.0;
    double normalized_mean = 0.0;
    double normalized_sum_squares = 0.0;
    pl_benchmark_stats result;
    size_t index;

    if (samples == NULL || out == NULL || count == 0u ||
        count > PL_BENCHMARK_MAX_SAMPLES) {
        return -1;
    }

    for (index = 0u; index < count; ++index) {
        if (!isfinite(samples[index]) || samples[index] < 0.0) return -1;
        if (samples[index] > scale) scale = samples[index];
    }

    sorted = (double *)malloc(count * sizeof(*sorted));
    deviations = (double *)malloc(count * sizeof(*deviations));
    if (sorted == NULL || deviations == NULL) {
        free(sorted);
        free(deviations);
        return -1;
    }

    memcpy(sorted, samples, count * sizeof(*sorted));
    qsort(sorted, count, sizeof(*sorted), compare_double);

    /*
     * Work in [0, 1] before calculating the mean and variance. Direct
     * Welford accumulation can still overflow its squared-difference term
     * for valid finite inputs close to DBL_MAX.
     */
    if (scale > 0.0) {
        for (index = 0u; index < count; ++index) {
            const double normalized = samples[index] / scale;
            const double delta = normalized - normalized_mean;
            normalized_mean += delta / (double)(index + 1u);
            normalized_sum_squares += delta * (normalized - normalized_mean);
        }
    }

    memset(&result, 0, sizeof(result));
    result.sample_count = count;
    result.minimum = sorted[0];
    result.maximum = sorted[count - 1u];
    result.mean = normalized_mean * scale;
    result.median = median_of_sorted(sorted, count);

    result.p90 = percentile_nearest_rank(sorted, count, 0.90);
    result.p95 = percentile_nearest_rank(sorted, count, 0.95);
    result.p99 = percentile_nearest_rank(sorted, count, 0.99);

    if (count > 1u) {
        const double normalized_deviation =
            sqrt(normalized_sum_squares / (double)(count - 1u));
        result.sample_standard_deviation = normalized_deviation * scale;
        result.coefficient_of_variation = normalized_mean > 0.0
            ? normalized_deviation / normalized_mean : 0.0;
    }

    for (index = 0u; index < count; ++index) {
        deviations[index] = fabs(samples[index] - result.median);
    }
    qsort(deviations, count, sizeof(*deviations), compare_double);
    result.median_absolute_deviation = median_of_sorted(deviations, count);

    free(deviations);
    free(sorted);

    /*
     * Publish the result only after every derived value passes validation.
     * Callers never receive a partially populated structure on failure.
     */
    if (!isfinite(result.mean) ||
        !isfinite(result.sample_standard_deviation) ||
        !isfinite(result.coefficient_of_variation) ||
        !isfinite(result.median_absolute_deviation)) {
        return -1;
    }

    *out = result;
    return 0;
}
