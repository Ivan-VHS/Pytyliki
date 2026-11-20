# TRACE — Measurement Provenance & Consistency Engine

**Status:** implemented as a C11 core API (`trace.h`, `trace.c`). TRACE is a read-only measurement record and comparability primitive; it does not certify sensors or declare a source truthful.

## Record model

Each `pl_trace_record` carries:

- normalized metric key and unit;
- bounded context key (test profile / environment identifier);
- opaque 64-character SHA-256 source token;
- numeric value and absolute uncertainty in the same unit;
- exact UTC epoch sampling interval and sample count;
- quality tag: observed, estimated, simulated, unknown or disputed.

The record contains no person name, account identifier, raw device serial or unbounded free-form narrative. A trusted adapter is responsible for generating the source digest and supplying correct timestamps and uncertainty.

## Comparability contract

`pl_trace_compare(left, right, out)` returns:

- `1`: comparable records; `out` contains delta, combined uncertainty and whether the declared uncertainty envelopes are disjoint;
- `0`: both records are valid but incomparable;
- `-1`: invalid input or output pointer.

Comparison requires identical metric key, unit, context, exact sampling window and quality class. Source digests must differ so that a duplicated record is not mistaken for independent evidence. Unknown and disputed quality tags cannot be compared as if they were measurements.

Combined absolute uncertainty is computed as `hypot(u_left, u_right)`. The `uncertainty_envelopes_disjoint` flag is true when `abs(value_right - value_left)` exceeds that combined value. This is a declared-envelope check, not a statistical significance test and not a choice of which source is correct.

## Example

```c
pl_trace_comparison comparison;
int state = pl_trace_compare(&sensor_a, &sensor_b, &comparison);
if (state == 1 && comparison.uncertainty_envelopes_disjoint) {
    /* record a discrepancy for review; do not overwrite either sample */
}
```

## Limits

- SHA-256 source tokens bind records to externally hashed artifacts; they do not authenticate a publisher.
- Absolute uncertainty must be meaningful and expressed in the declared unit; TRACE does not estimate it.
- Context and metric keys are identifiers, not hardware certification.
- No hardware state is changed and no device is controlled.
- Discrepant observations remain separate; the API never picks a winner or averages away a conflict.

## Test

The CTest target `measurement-trace-tests` covers validation, matching context/time/unit, source separation, quality mismatch, finite values and uncertainty-envelope overlap.
