# Pytyliki — NEXUS Diagnostics Profile v1

**Owner:** Pytyliki  
**Status:** additive integration profile  
**Implementation boundary:** native C11 core, portable CLI and Windows workstation.

## Product role

Pytyliki remains the processor-systems workstation. It owns read-only processor/topology discovery, bounded local measurements, memory/metric operations, source diagnostics and its versioned C ABI. NEXUS does not rename the product or turn it into a kernel service.

## NEXUS responsibilities

Pytyliki is a trusted **evidence provider**, not a privileged system controller.

| Capability | Ownership | NEXUS consumer behavior |
|---|---|---|
| CPU topology and usable ISA inventory | Pytyliki | Display provider, timestamp, platform and supported-feature evidence |
| Bounded CPU/memory measurements | Pytyliki | Preserve workload, duration, units and environment with every result |
| Memory arena and metric operations | Pytyliki | Reuse native C11 code through the existing ABI where appropriate |
| Profile analysis and flamegraph evidence | Pytyliki | Read exported artifacts; do not infer live process control |
| C ABI v1 | Pytyliki | Explicit trusted path, version validation and caller-owned buffers |
| Reaper policy example | Reaper + Pytyliki CI | CI-only policy verification; not a runtime dependency |

## Integration rules

- Keep C11 authoritative for shipped processor and metric behavior.
- Reuse `pytyliki.c-abi.v1`; do not create a second ABI for NEXUS naming.
- Keep all discovery and measurements read-only. No clock, voltage, affinity, firmware, MMIO or scheduler changes.
- Keep measurements bounded and include hardware/software context. Never compare scores without matching workload and units.
- Mark unavailable hardware counters as unavailable; do not manufacture estimates.
- Reaper remains an optional CI policy checker where already configured. It is not a mandatory desktop runtime.
- Use stable report envelopes and versioned JSON exports for cross-process exchange.

## New application consumers

- **VectorScope** may visualize Pytyliki measurement reports.
- **Worker Grid** may display recorded partition and capacity analysis as advisory information.
- **ThermalPilot** may correlate exported readings where an actual provider supplies them; it cannot change processor limits or firmware settings.

These application concepts do not authorize new hardware access or alter Pytyliki's UI.

## Acceptance

A NEXUS consumer must pass schema/version checks, reject oversized or malformed reports, preserve source provenance, handle a missing provider and keep the operation read-only. Any new ABI function requires a separate versioned API proposal and C11 tests.
