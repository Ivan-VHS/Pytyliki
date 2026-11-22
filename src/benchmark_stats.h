#ifndef PYTYLIKI_BENCHMARK_STATS_H
#define PYTYLIKI_BENCHMARK_STATS_H

#include <stddef.h>

#define PL_BENCHMARK_MAX_SAMPLES 100000u

typedef struct {
    size_t sample_count;
    double minimum;
    double maximum;
    double mean;
    double median;
    double p90;
    double p95;
    double p99;
    double sample_standard_deviation;
    double coefficient_of_variation;
    double median_absolute_deviation;
} pl_benchmark_stats;

/*
 * Analyze finite, non-negative timing samples without modifying the input.
 * Percentiles use the nearest-rank definition. The function is bounded to
 * PL_BENCHMARK_MAX_SAMPLES and returns -1 for invalid input or allocation
 * failure. No benchmark workload is started by this routine.
 */
int pl_benchmark_stats_analyze(
    const double *samples,
    size_t count,
    pl_benchmark_stats *out
);

#endif
