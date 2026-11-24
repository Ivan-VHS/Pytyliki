# Reaper policy integration

Pytyliki remains a native C11 processor-systems workstation. Its processor discovery, measurements, resource planning, embedded profiles, MMIO simulation and C ABI stay owned by the native project.

Reaper is adopted as a **CI policy language**, not as an application runtime or replacement for C11. The checked-in example `examples/reaper_resource_policy.rpr` evaluates deterministic, reference-only FLASH/SRAM/stack budget invariants. The values are fixtures; they are not read from a device and must not be reported as live hardware data.

## Verification

The dedicated `.github/workflows/reaper-policy.yml` workflow checks out an immutable Reaper commit, runs Reaper's own static checker and executes the policy program. CI requires the exact `PYTYLIKI_REAPER_POLICY:PASS` result. A failing predicate produces a different output and fails the workflow assertion.

## Boundaries

- Reaper parsing and policy evaluation run in a CI process only. The installed Pytyliki application has no Reaper runtime dependency.
- The workflow does not load hardware drivers, write MMIO, flash firmware, change processor configuration or execute a generated device operation.
- The native C11 implementation remains authoritative for actual processor inventory and all shipped product behavior.
- Reaper's source grammar is not reimplemented in Pytyliki; the upstream checker is the only parser used for the policy file.
- The workflow pins Reaper by commit SHA so a language update is deliberate and reviewable.

This integration adds a second, deterministic policy-verification layer without blurring the product's measurement and safety boundaries.
