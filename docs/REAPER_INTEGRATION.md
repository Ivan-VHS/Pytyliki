# Reaper policy integration

Pytyliki remains a native C11 processor-systems workstation. Its processor discovery, measurements, resource planning, embedded profiles, MMIO simulation and C ABI stay owned by the native project.

Reaper is adopted as a **CI policy language**, not as an application runtime or replacement for C11. The checked-in [examples/reaper_resource_policy.rpr](../examples/reaper_resource_policy.rpr) evaluates deterministic FLASH/SRAM/stack budgets and bounded metric-evidence envelopes. Metric fixtures include provider identity, metric ID, unit, range, sample count and workload ID; results from different workloads or incompatible units must not be compared.

The pinned Reaper toolchain also runs its [Processor Reliability Core v1](https://github.com/Ivan-VHS/Reaper/blob/3dd72e3b9aeb32b02fb4bfd0cea688625a55bff9/docs/PROCESSOR_RELIABILITY_CORE_V1.md) conformance fixture. That fixture verifies the 5 × 4 execution matrix, checked RPR-BC decoding, repair-budget transitions, simulated silicon/Cu-Mesh/nanochip models, signed phase correction and deterministic Pytyliki trace replay. Its `pytyliki.processor-reliability.v1` report is model evidence only; Pytyliki does not ingest it as observed hardware telemetry.

## Verification

The dedicated .github/workflows/reaper-policy.yml workflow checks out an immutable Reaper commit, runs Reaper's own static checker and executes the policy program. CI requires the exact PYTYLIKI_REAPER_POLICY:PASS result. A failing predicate produces a different output and fails the workflow assertion.

## Boundaries

- Reaper parsing and policy evaluation run in the existing Reaper CI toolchain only. The installed Pytyliki application has no Reaper runtime dependency.
- The values in the example are deterministic fixtures, not live processor or memory telemetry.
- The workflow does not load hardware drivers, write MMIO, flash firmware, change processor configuration or execute a generated device operation.
- Native C11 remains authoritative for actual processor inventory, measurements, metrics and all shipped product behavior.
- Reaper's source grammar is not reimplemented in Pytyliki; the upstream checker is the only parser used for the policy file.
- The workflow pins Reaper by commit SHA so a language update is deliberate and reviewable.

The Reaper policy layer adds repeatable admission and evidence checks without taking over hardware ownership or the measurement path.
