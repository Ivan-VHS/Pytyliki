# Security and trust boundaries — Pytyliki

## Scope and current operating model

Pytyliki is a native C11 workstation for read-only processor discovery, bounded local workloads, static source diagnostics and local DSP. Its security posture is deliberately local and non-invasive. This document describes the current product boundary, not a promise of kernel-level isolation.

## Assets and trust boundaries

| Asset / boundary | Input trust | Required handling |
|---|---|---|
| CPU and OS inventory | host-provided observations | read-only queries; report unavailable fields explicitly |
| CLI and configuration | untrusted user input | parse, validate ranges and reject overflow before work begins |
| Workload partitioning | user-supplied counts and limits | checked arithmetic, bounded output count, deterministic ranges |
| Source diagnostics | untrusted source text | static inspection only; never execute inspected files |
| WAV / DSP input | untrusted binary data | validate RIFF/chunk bounds, channels, sample format and buffer size |
| Reports and metrics | derived local data | label estimates vs measured values; avoid secrets and private paths |

## Non-negotiable safety rules

- No writes to BIOS, UEFI, firmware, microcode, voltage, frequency or power-limit settings.
- No network transport, remote execution or hidden telemetry is enabled by this profile.
- Source inspection must not compile, execute or rewrite the inspected source.
- CPU capability detection and workload recommendations are observational/advisory; they do not silently change affinity, scheduling or clock settings.
- All sizes, counts, offsets and arithmetic derived from external input must be checked before allocation or indexing.

## Failure behavior

Invalid or unsupported input must fail closed with a bounded diagnostic. A failed inventory query must not be converted into a fabricated hardware value. DSP import errors must stop processing before out-of-range reads or partial output publication. Resource exhaustion must return a controlled error rather than retry indefinitely.

## Verification evidence

For a change that affects these boundaries, add:
1. unit tests for valid, invalid, minimum, maximum and overflow inputs;
2. regression tests for buffer and parser boundaries;
3. deterministic tests for partition and numerical results;
4. fuzz targets for binary parsers and text frontends where supported;
5. CI evidence and a link to the exact implementation.

Run the AppGuard catalog checks:

```sh
python scripts/validate_beelzebul_appguard.py
python -m unittest discover -s tests -p "test_beelzebul_appguard.py" -v
```

These commands validate the policy catalog and its metadata. They do not prove that every L001–L666 control is enforced.

## AppGuard status vocabulary

- **planned** — catalogued; no executable enforcement claim.
- **partial** — a named subset is implemented and evidenced.
- **implemented** — enforcement code and focused tests exist.
- **verified** — implementation, tests and review evidence are linked.

The current AppGuard change is a catalog/validator/CI addition. Do not upgrade a control's status based on documentation alone.

## Change and release checklist

- Update the relevant feature, architecture and API documentation when behavior changes.
- Record numeric limits beside the parser or API they constrain.
- Keep README screenshots as genuine application captures; do not present mockups as runtime evidence.
- Re-run documented build/test commands and verify relative links.
- State unsupported behavior and platform limitations explicitly.
