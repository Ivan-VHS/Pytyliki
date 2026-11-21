# Quality, repeatability, native integration

| Pillar | Acceptance |
|---|---|
| Quality | correctness, bounds, failure containment, traceability |
| Repeatability | fixed inputs, explicit effects, deterministic runs where feasible |
| Native | narrow adapters, versioned ABI, native error semantics |

The profile `quality-repeatability-native.v1.json` has 3 criteria per pillar. Run `python tools/validate_quality_repeatability_native.py --json`. Evidence-path presence is not proof of behavior; tests remain authoritative.
