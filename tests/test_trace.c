#include "trace.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <string.h>

static pl_trace_record sample_record(
    const char *source, double value, double uncertainty
) {
    pl_trace_record record;
    memset(&record, 0, sizeof(record));
    memcpy(record.metric_key, "cpu.package-power", sizeof("cpu.package-power"));
    memcpy(record.unit, "watt", sizeof("watt"));
    memcpy(record.context_key, "profile:idle", sizeof("profile:idle"));
    memcpy(record.source_sha256, source, PL_TRACE_SHA256_BUFFER_SIZE);
    record.value = value;
    record.absolute_uncertainty = uncertainty;
    record.window_start_epoch = UINT64_C(1794000000);
    record.window_end_epoch = UINT64_C(1794000060);
    record.sample_count = 60u;
    record.quality = PL_TRACE_OBSERVED;
    return record;
}

int main(void) {
    pl_trace_record left = sample_record(
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        100.0, 2.0
    );
    pl_trace_record right = sample_record(
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        105.0, 2.0
    );
    pl_trace_comparison result;

    assert(pl_trace_validate(&left) == 0);
    assert(pl_trace_validate(&right) == 0);
    assert(pl_trace_compare(&left, &right, &result) == 1);
    assert(fabs(result.delta - 5.0) < 1e-12);
    assert(fabs(result.combined_absolute_uncertainty - 4.0) < 1e-12);
    assert(result.uncertainty_envelopes_disjoint == 1);

    right.value = 101.0;
    right.absolute_uncertainty = 1.0;
    left.absolute_uncertainty = 1.0;
    assert(pl_trace_compare(&left, &right, &result) == 1);
    assert(result.uncertainty_envelopes_disjoint == 0);

    right.window_start_epoch++;
    right.window_end_epoch++;
    assert(pl_trace_compare(&left, &right, &result) == 0);

    right = sample_record(
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        101.0, 1.0
    );
    assert(pl_trace_compare(&left, &right, &result) == 0);

    right = sample_record(
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        101.0, 1.0
    );
    right.quality = PL_TRACE_SIMULATED;
    assert(pl_trace_compare(&left, &right, &result) == 0);

    right.quality = PL_TRACE_OBSERVED;
    memcpy(right.unit, "kilowatt", sizeof("kilowatt"));
    assert(pl_trace_compare(&left, &right, &result) == 0);

    memcpy(right.unit, "watt", sizeof("watt"));
    right.absolute_uncertainty = -1.0;
    assert(pl_trace_validate(&right) != 0);
    right.absolute_uncertainty = 1.0;
    right.value = NAN;
    assert(pl_trace_validate(&right) != 0);
    right.value = 1.0;
    right.sample_count = 0u;
    assert(pl_trace_validate(&right) != 0);
    right.sample_count = 1u;
    right.source_sha256[0] = 'z';
    assert(pl_trace_validate(&right) != 0);
    assert(pl_trace_compare(&left, &right, &result) == -1);
    assert(pl_trace_compare(&left, &right, NULL) == -1);
    assert(pl_trace_compare(NULL, &right, &result) == -1);

    return 0;
}
