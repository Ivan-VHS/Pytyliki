#include "trace.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static int pl_trace_key_valid(const char *value, size_t capacity) {
    size_t i;
    if (value == NULL || capacity < 2u) return 0;
    for (i = 0u; i < capacity; ++i) {
        unsigned char ch = (unsigned char)value[i];
        if (ch == '\0') return i > 0u;
        if (!((ch >= 'a' && ch <= 'z') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '.' || ch == '_' || ch == '-' || ch == ':' || ch == '/'))
            return 0;
    }
    return 0;
}

static int pl_trace_digest_valid(const char digest[PL_TRACE_SHA256_BUFFER_SIZE]) {
    size_t i;
    if (digest == NULL) return 0;
    for (i = 0u; i < PL_TRACE_SHA256_HEX_LENGTH; ++i) {
        char ch = digest[i];
        if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))) return 0;
    }
    return digest[PL_TRACE_SHA256_HEX_LENGTH] == '\0';
}

int pl_trace_validate(const pl_trace_record *record) {
    if (record == NULL) return -1;
    if (!pl_trace_key_valid(record->metric_key, sizeof(record->metric_key)) ||
        !pl_trace_key_valid(record->unit, sizeof(record->unit)) ||
        !pl_trace_key_valid(record->context_key, sizeof(record->context_key)) ||
        !pl_trace_digest_valid(record->source_sha256))
        return -1;
    if (!isfinite(record->value) ||
        !isfinite(record->absolute_uncertainty) ||
        record->absolute_uncertainty < 0.0)
        return -1;
    if (record->window_start_epoch >= record->window_end_epoch ||
        record->sample_count == 0u ||
        record->sample_count > PL_TRACE_MAX_SAMPLES)
        return -1;
    if (record->quality < PL_TRACE_OBSERVED || record->quality > PL_TRACE_DISPUTED)
        return -1;
    return 0;
}

int pl_trace_compare(
    const pl_trace_record *left,
    const pl_trace_record *right,
    pl_trace_comparison *out
) {
    double combined;
    double delta;

    if (out == NULL || pl_trace_validate(left) != 0 || pl_trace_validate(right) != 0)
        return -1;

    memset(out, 0, sizeof(*out));
    if (strcmp(left->metric_key, right->metric_key) != 0 ||
        strcmp(left->unit, right->unit) != 0 ||
        strcmp(left->context_key, right->context_key) != 0 ||
        left->window_start_epoch != right->window_start_epoch ||
        left->window_end_epoch != right->window_end_epoch ||
        left->quality != right->quality ||
        left->quality == PL_TRACE_UNKNOWN ||
        left->quality == PL_TRACE_DISPUTED ||
        strcmp(left->source_sha256, right->source_sha256) == 0)
        return 0;

    delta = right->value - left->value;
    combined = left->absolute_uncertainty + right->absolute_uncertainty;
    if (!isfinite(delta) || !isfinite(combined)) return -1;

    out->delta = delta;
    out->combined_absolute_uncertainty = combined;
    out->uncertainty_envelopes_disjoint = fabs(delta) > combined ? 1 : 0;
    return 1;
}

int pl_trace_analyze_cadence(
    const double *intervals,
    size_t count,
    double expected_interval,
    double tolerance_ratio,
    pl_trace_cadence *out
) {
    size_t i;
    size_t split;
    double mean = 0.0;
    double m2 = 0.0;
    double first_sum = 0.0;
    double second_sum = 0.0;
    double first_mean;
    double second_mean;
    double standard_deviation;
    double coefficient_of_variation;
    double relative_bias;
    double relative_drift;
    double penalty;
    double score;

    if (out == NULL || intervals == NULL ||
        count < 2u || count > PL_TRACE_MAX_CADENCE_SAMPLES ||
        !isfinite(expected_interval) || expected_interval < 1e-9 ||
        expected_interval > 1e12 ||
        !isfinite(tolerance_ratio) || tolerance_ratio <= 0.0 ||
        tolerance_ratio > 1.0)
        return -1;

    split = count / 2u;
    for (i = 0u; i < count; ++i) {
        const double value = intervals[i];
        const double delta = value - mean;
        double delta_after;
        if (!isfinite(value) || value < 1e-9 || value > 1e12) return -1;
        mean += delta / (double)(i + 1u);
        delta_after = value - mean;
        m2 += delta * delta_after;
        if (i < split) first_sum += value;
        else second_sum += value;
        if (!isfinite(mean) || !isfinite(m2) ||
            !isfinite(first_sum) || !isfinite(second_sum))
            return -1;
    }

    first_mean = first_sum / (double)split;
    second_mean = second_sum / (double)(count - split);
    standard_deviation = sqrt(m2 / (double)(count - 1u));
    coefficient_of_variation = standard_deviation / mean;
    relative_bias = (mean - expected_interval) / expected_interval;
    relative_drift = (second_mean - first_mean) / expected_interval;
    penalty = coefficient_of_variation + fabs(relative_bias) + 0.5 * fabs(relative_drift);
    if (!isfinite(standard_deviation) || !isfinite(coefficient_of_variation) ||
        !isfinite(relative_bias) || !isfinite(relative_drift) || !isfinite(penalty))
        return -1;

    score = 100.0 * (1.0 - fmin(1.0, penalty / tolerance_ratio));
    memset(out, 0, sizeof(*out));
    out->sample_count = count;
    out->expected_interval = expected_interval;
    out->mean_interval = mean;
    out->sample_standard_deviation = standard_deviation;
    out->coefficient_of_variation = coefficient_of_variation;
    out->relative_bias = relative_bias;
    out->relative_drift = relative_drift;
    out->cadence_score = score;
    return 0;
}
