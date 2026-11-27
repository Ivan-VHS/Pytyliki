# Pytyliki Processor Control Policy

**Status:** Reaper CI policy and C11 read-only processor workstation.  
**Runtime owner:** Pytyliki C11.  
**Policy owner:** Reaper.

## Purpose

This extension formalizes Pytyliki's processor workstation around reproducible evidence and explicit control boundaries. It covers processor snapshots, package/die/core/logical-CPU topology, performance/efficiency core classes, affinity descriptors, bounded parallel work partitioning, measurement provenance, verification evidence, revision records and safe-point admission for policy changes.

The executable fixture is `examples/reaper_processor_policy.rpr`. It uses deterministic fixtures only.

## Ownership

- **Pytyliki C11** remains authoritative for CPUID/inventory collection, operating-system state checks, measurements, profile import, folded-stack analysis, flamegraphs, source diagnostics and native ABI behavior.
- **Reaper** validates descriptors, evidence, compatibility and lifecycle policy in CI. Its parser is not copied into Pytyliki and its policy engine is not an application runtime dependency.
- **DAK** owns future kernel-side scheduling, AP startup, interrupt routing, privileged CPU state and native core lifecycle.
- **The host operating system** continues to own its scheduler and hardware configuration while Pytyliki runs as a normal application.

## Policy-only control model

Allowed workstation operations are bounded inventory, measurements, profile analysis, comparison and report generation. A policy request to change affinity, scheduling, voltage, frequency, boost state, microcode, firmware, interrupt routing, drivers or CPU registers is rejected by this surface. No request is converted into a host command.

Future privileged control, where supported by a target OS and hardware, requires a separate signed native adapter, an explicit capability, user consent, platform-specific validation and rollback. It is not implied by this Reaper contract.

## Evidence model

A processor snapshot records schema, snapshot identity, architecture, vendor, package/die/core/logical counts, online count, topology generation, supported features and source provenance. A measurement binds provider, metric ID, unit, value range, sample count, workload ID, capture time and mode.

Two metrics are comparable only when provider, metric, unit, workload and evidence mode agree. A synthetic fixture must never be mixed with live measurements. Verification evidence binds a candidate to test totals, zero failures, coverage, duration, artifact SHA-256 and verifier identity.

## Acceptance gates

- Snapshot cardinality and online/logical count consistency.
- Unique core identifiers and bounded logical-CPU membership.
- Affinity bounds and duplicate rejection.
- Deterministic work partitioning with conservation of all items.
- Provenance-complete metric envelopes and workload isolation.
- Revision generation, ABI major and evidence checks.
- Safe-point and rollback gates for staged policy changes.
- Explicit rejection of native hardware-control commands.
- Reaper static check and execution in the pinned CI toolchain.

## Run

```sh
PYTHONPATH=_deps/Reaper python -m reaper --check examples/reaper_processor_policy.rpr
PYTHONPATH=_deps/Reaper python -m reaper examples/reaper_processor_policy.rpr
```

Expected output: `PYTYLIKI_PROCESSOR_POLICY:PASS`.

This result proves the policy fixture passed. It does not prove live CPU telemetry, host scheduler control, frequency changes, firmware updates, driver installation, or privileged processor access.
