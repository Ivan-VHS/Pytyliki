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
    combined = hypot(left->absolute_uncertainty, right->absolute_uncertainty);
    if (!isfinite(delta) || !isfinite(combined)) return -1;

    out->delta = delta;
    out->combined_absolute_uncertainty = combined;
    out->uncertainty_envelopes_disjoint = fabs(delta) > combined ? 1 : 0;
    return 1;
}
