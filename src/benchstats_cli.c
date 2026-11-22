#include "benchmark_stats.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLI_MAX_SAMPLES 256u

static int parse_samples(const char *text, double *samples, size_t *count) {
    const char *cursor = text;
    size_t used = 0u;

    if (text == NULL || text[0] == '\0' || samples == NULL || count == NULL) return -1;
    while (*cursor != '\0') {
        char *end = NULL;
        double value;
        if (used == CLI_MAX_SAMPLES) return -1;
        errno = 0;
        value = strtod(cursor, &end);
        if (end == cursor || errno == ERANGE || value < 0.0) return -1;
        samples[used++] = value;
        if (*end == '\0') break;
        if (*end != ',') return -1;
        cursor = end + 1;
        if (*cursor == '\0') return -1;
    }
    *count = used;
    return used > 0u ? 0 : -1;
}

int main(int argc, char **argv) {
    double samples[CLI_MAX_SAMPLES];
    size_t count = 0u;
    pl_benchmark_stats stats;

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        puts("Pytyliki Benchmark Statistics\nUsage: pytyliki-benchstats MS,MS,...\n"
             "Summarizes existing non-negative timing samples; it does not run a workload.");
        return argc == 2 ? 0 : 2;
    }
    if (parse_samples(argv[1], samples, &count) != 0 ||
        pl_benchmark_stats_analyze(samples, count, &stats) != 0) {
        fputs("Expected 1..256 finite non-negative comma-separated timing samples in milliseconds.\n", stderr);
        return 2;
    }

    printf("{\"schema\":\"pytyliki.benchmark-statistics.v1\","
           "\"sample_count\":%zu,\"unit\":\"ms\","
           "\"minimum\":%.9g,\"maximum\":%.9g,\"mean\":%.9g,"
           "\"median\":%.9g,\"p90\":%.9g,\"p95\":%.9g,\"p99\":%.9g,"
           "\"sample_standard_deviation\":%.9g,"
           "\"coefficient_of_variation\":%.9g,"
           "\"median_absolute_deviation\":%.9g,"
           "\"execution\":\"statistics_only\"}\n",
           stats.sample_count, stats.minimum, stats.maximum, stats.mean,
           stats.median, stats.p90, stats.p95, stats.p99,
           stats.sample_standard_deviation, stats.coefficient_of_variation,
           stats.median_absolute_deviation);
    return 0;
}
