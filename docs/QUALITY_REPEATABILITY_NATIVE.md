# Quality, Repeatability and Native Integration

This contract turns three engineering priorities into reviewable acceptance rules.

## Quality
Correctness, failure containment, traceability and release readiness must be supported by source, tests and observable results. Evidence must be updated with the implementation; claims without a repeatable check remain planned.

## Repeatability
Operations that can be repeated must define inputs, outputs, side effects and failure behavior. Prefer deterministic results where the domain permits them, bounded batches over unbounded work, explicit retry and partial-failure semantics, and provenance for generated or measured output. Repeating a read-only operation must not mutate the system.

## Native integration
Use the project's primary language, runtime, platform APIs and data contracts directly where practical. Keep adapters explicit and narrow; preserve native conventions, versioned boundaries and graceful degradation. Avoid wrappers that obscure errors or make the native path less reliable.

## Pytyliki application
Keep the C11 core and processor-facing DSP path direct. Treat memory ownership, bounded measurements, diagnostic provenance, embedded boundaries and platform builds as product-level contracts. Repeated multi-stage work must define batch bounds and per-stage outcomes before being marked complete.

## Acceptance and evidence
Each pillar has at least three criteria in quality-repeatability-native.v1.json. Every criterion names implementation evidence, verification evidence, status and verification intent. The validator checks structure, uniqueness and evidence-path existence; it does **not** claim that file existence proves behavior. Runtime tests, native builds, review and release checks remain authoritative.

Use implemented only when the implementation and verification are present and maintained. Use planned for work that has a documented target but is not complete. Use not_applicable only with an explicit rationale.

Local checks: python tools/validate_quality_repeatability_native.py --self-test and python tools/validate_quality_repeatability_native.py --json.
