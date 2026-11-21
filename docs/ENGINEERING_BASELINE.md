# Engineering Baseline

**Status:** normative  
**Product:** Pytyliki processor systems workstation

## Product boundary

Pytyliki provides local processor discovery, bounded measurements, numerical operations, DSP, memory primitives, source diagnostics and explicitly selected native interoperability. It is not a firmware tool, driver manager, unrestricted hardware-control plane or general-purpose distributed runtime.

## Required behavior

- Processor discovery and source inspection MUST remain read-only. Source inspection MUST NOT compile or execute the inspected program.
- BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, driver and device-register writes MUST NOT be performed by the documented workflows.
- Numerical operations MUST validate dimensions, ranges, units, finite values and overflow before producing a result.
- Workloads, allocations, input files and queues MUST have explicit bounds. Unsupported capabilities MUST be reported rather than silently substituted.
- DSP operations MUST identify their input/output formats and process only the buffers supplied by the caller.
- Optional native interoperability MUST use a versioned ABI and an explicitly selected trusted library. Loading native code executes it in-process.

## Evidence and data handling

- Measurement records MUST preserve metric, unit, context, sampling window, sample count and declared quality information.
- Comparisons MUST reject incompatible contexts. Missing telemetry MUST remain missing; it MUST NOT be fabricated or silently treated as a passing result.
- Device or source digests establish byte-level identity only. They do not prove publisher authenticity, safety or semantic equivalence.
- Consent for export, sharing and telemetry MUST remain distinct from analysis permission.

## Integration contract

Cross-project consumers MUST use versioned data formats or the published C ABI. They MUST NOT depend on private object layouts, unversioned internal modules or implicit library loading. Each integration MUST define ownership, validation, failure behavior and compatibility expectations.

## Verification

CMake/CTest results and the checked-in implementation determine shipped capability. Documentation MUST label reference profiles, simulations, advisory scores and planned work. Changes to public behavior require regression tests and an updated contract.
