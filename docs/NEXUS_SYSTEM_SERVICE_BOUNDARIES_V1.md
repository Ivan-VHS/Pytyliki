# Pytyliki — NEXUS System-Service Boundaries v1

**Owner:** Pytyliki  
**NEXUS role:** processor, memory and metric evidence provider  
**Status:** additive federation contract

Pytyliki remains a native C11 processor-systems workstation with a portable CLI and Windows desktop. NEXUS does not convert it into a DuckOS kernel module, memory manager or privileged hardware controller.

## Service relationships

| DuckOS service / app | Permitted use of Pytyliki | Prohibited inference |
|---|---|---|
| MEMORY_FORGE | Consume bounded processor/memory measurements, arena metrics and exported reports | Pytyliki does not provide page tables, per-process kernel accounting, allocator control, swap mutation or VRAM mapping |
| DATA_ORCHESTRATOR | Display workload and partition evidence as advisory input | A benchmark does not authorize affinity, priority, scheduler, clock or voltage changes |
| KERNEL_SENTINEL | Use build/host inventory and diagnostic metadata as one evidence source | A report is not a kernel measurement root, malware verdict or signature trust decision |
| VectorScope | Visualize report values with source, capture time, units and validity | Missing counters must remain unavailable; do not invent or interpolate measurements |
| ThermalPilot | Correlate only readings supplied by an actual supported provider | No temperature, power or fan control is introduced by this profile |

## Contract rules

1. C11 remains authoritative for shipped processor and metric behavior.
2. Reuse the existing versioned C ABI v1 and report envelope; do not add a NEXUS-specific duplicate ABI.
3. Native library loading requires an explicit trusted path, ABI validation, bounded arguments and caller-owned buffers.
4. Every measurement preserves workload, duration, hardware/software context, unit, timestamp and provider identity.
5. Unsupported counters, permission failures and unavailable devices remain explicit states.
6. Hardware discovery remains read-only. No clock, voltage, affinity, firmware, MMIO, scheduler or device configuration writes.
7. Reaper policy checks remain optional validation examples, not a mandatory desktop runtime dependency.

## Acceptance

- Valid, malformed, oversized, stale and version-mismatched reports have tests.
- Cross-project consumers validate schema, units, provenance and bounds before display.
- Metrics are not compared across incompatible workloads or measurement conditions.
- Any new C ABI function receives an independent versioned proposal and C11 tests.

This profile authorizes evidence exchange only. It does not implement MEMORY_FORGE or grant kernel memory access.
