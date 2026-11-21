# Pytyliki — Measurement Cadence and Repeatability

## Purpose

The TRACE layer now includes a bounded cadence analyzer for repeated positive measurement intervals. This turns the ideas of quality, cohesion and rhythm into a numeric diagnostic for benchmark runs and other repeated measurements, while keeping provenance and cadence analysis separate.

## API

Use `pl_trace_analyze_cadence(intervals, count, expected_interval, tolerance_ratio, &report)` from `trace.h`.

- Inputs are 2 to 1,000,000 finite intervals in seconds.
- Each interval and the expected interval must be within the documented numeric bounds.
- The report contains the mean, sample standard deviation, coefficient of variation, relative bias, first-half/second-half drift and a 0–100 cadence score.
- The score decreases with jitter, deviation from the expected period and temporal drift.
- Invalid or non-finite inputs are rejected; no input array is modified.

## Interpretation

The score is descriptive, not a hardware quality certification or a compatibility verdict. Compare runs only after using TRACE provenance to establish matching metric, unit, context and sampling window. Keep raw measurements and source digests alongside any derived score.

## Verification

The existing `measurement-trace-tests` CTest target now covers stable intervals, temporal drift, invalid values and bounds. The original TRACE record comparison behavior remains unchanged.
