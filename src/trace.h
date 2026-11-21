#ifndef PYTYLIKI_TRACE_H
#define PYTYLIKI_TRACE_H

#include <stddef.h>
#include <stdint.h>

#define PL_TRACE_METRIC_MAX 64u
#define PL_TRACE_UNIT_MAX 32u
#define PL_TRACE_CONTEXT_MAX 64u
#define PL_TRACE_SHA256_HEX_LENGTH 64u
#define PL_TRACE_SHA256_BUFFER_SIZE 65u
#define PL_TRACE_MAX_SAMPLES UINT64_C(1000000000)
#define PL_TRACE_MAX_CADENCE_SAMPLES 1000000u

typedef enum {
    PL_TRACE_OBSERVED = 1,
    PL_TRACE_ESTIMATED = 2,
    PL_TRACE_SIMULATED = 3,
    PL_TRACE_UNKNOWN = 4,
    PL_TRACE_DISPUTED = 5
} pl_trace_quality;

typedef struct {
    char metric_key[PL_TRACE_METRIC_MAX];
    char unit[PL_TRACE_UNIT_MAX];
    char context_key[PL_TRACE_CONTEXT_MAX];
    /* Opaque SHA-256 identifier supplied by a trusted source adapter.
       Never place a person's name, account ID or raw device ID here. */
    char source_sha256[PL_TRACE_SHA256_BUFFER_SIZE];
    double value;
    double absolute_uncertainty;
    uint64_t window_start_epoch;
    uint64_t window_end_epoch;
    uint64_t sample_count;
    pl_trace_quality quality;
} pl_trace_record;

typedef struct {
    double delta;
    double combined_absolute_uncertainty;
    int uncertainty_envelopes_disjoint;
} pl_trace_comparison;

/* Descriptive cadence report for repeated positive measurement intervals. */
typedef struct {
    size_t sample_count;
    double expected_interval;
    double mean_interval;
    double sample_standard_deviation;
    double coefficient_of_variation;
    double relative_bias;
    double relative_drift;
    double cadence_score;
} pl_trace_cadence;

/* Return 0 when valid; -1 for invalid bounds, identifiers or values. */
int pl_trace_validate(const pl_trace_record *record);

/* Return 1 when comparable, 0 when valid but not comparable, -1 on invalid
   arguments. Comparison requires equal metric, unit, context, exact time
   window and quality, plus distinct source digests. A conflict means only
   that the two declared absolute-uncertainty envelopes do not overlap. */
int pl_trace_compare(
    const pl_trace_record *left,
    const pl_trace_record *right,
    pl_trace_comparison *out
);

/* Analyze 2..1,000,000 positive finite intervals (seconds). The score is a
   bounded descriptive consistency score based on coefficient of variation,
   mean-period bias and first-half/second-half drift. It is not a pass/fail
   hardware certification or a substitute for measurement provenance. */
int pl_trace_analyze_cadence(
    const double *intervals,
    size_t count,
    double expected_interval,
    double tolerance_ratio,
    pl_trace_cadence *out
);

#endif
