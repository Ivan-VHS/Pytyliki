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
    return sorted[middle - 1u] / 2.0 + sorted[middle] / 2.0;
}

int pl_benchmark_stats_analyze(
    const double *samples,
    size_t count,
    pl_benchmark_stats *out
) {
    double *sorted;
    double *deviations;
    double mean = 0.0;
    double sum_squares = 0.0;
    size_t index;

    if (samples == NULL || out == NULL || count == 0u ||
        count > PL_BENCHMARK_MAX_SAMPLES) {
        return -1;
    }

    for (index = 0u; index < count; ++index) {
        if (!isfinite(samples[index]) || samples[index] < 0.0) return -1;
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

    for (index = 0u; index < count; ++index) {
        const double delta = samples[index] - mean;
        mean += delta / (double)(index + 1u);
        sum_squares += delta * (samples[index] - mean);
    }

    memset(out, 0, sizeof(*out));
    out->sample_count = count;
    out->minimum = sorted[0];
    out->maximum = sorted[count - 1u];
    out->mean = mean;
    out->median = median_of_sorted(sorted, count);
    out->p90 = percentile_nearest_rank(sorted, count, 0.90);
    out->p95 = percentile_nearest_rank(sorted, count, 0.95);
    out->p99 = percentile_nearest_rank(sorted, count, 0.99);
    out->sample_standard_deviation = count > 1u
        ? sqrt(sum_squares / (double)(count - 1u)) : 0.0;
    out->coefficient_of_variation = mean > 0.0
        ? out->sample_standard_deviation / mean : 0.0;

    for (index = 0u; index < count; ++index) {
        deviations[index] = fabs(samples[index] - out->median);
    }
    qsort(deviations, count, sizeof(*deviations), compare_double);
    out->median_absolute_deviation = median_of_sorted(deviations, count);

    free(deviations);
    free(sorted);
    return isfinite(out->mean) && isfinite(out->sample_standard_deviation) &&
        isfinite(out->coefficient_of_variation) ? 0 : -1;
}
