# Native hardware intelligence v1

This implementation adds a native C11 contract for the host collector, static PPF placement and resource lifecycle validation. It is deliberately separate from benchmark execution and does not change scheduler affinity or device configuration.

## Collector

```c
pl_ni_status pl_ni_collect(pl_ni_snapshot *out);
```

The normalized snapshot contains processor-group and group-relative CPU identities, package/die/core/thread relations, NUMA membership, cache-sharing CPU masks, memory totals, ISA masks and per-field provenance states.

- Windows uses GetLogicalProcessorInformationEx(RelationAll), including variable-size processor/core/package/die, NUMA and cache group masks, active processor groups and GetProcessGroupAffinity/GetProcessAffinityMask where the process is confined to one group. Variable-sized records are size-checked before their group arrays are read.
- Linux reads /sys/devices/system/cpu, /sys/devices/system/node (including sparse node IDs, per-node memory and distance rows) and the kernel cache topology attributes; sched_getaffinity reports the calling thread's effective CPU set. Its exact source and availability are recorded separately in provenance.
- /proc/cpuinfo is used for per-logical-CPU kernel-enabled ISA flags on Linux. A CPUID/XGETBV observation on the executing collector CPU is never promoted to a per-core guarantee.
- Missing values remain unknown. If a Windows process spans several processor groups and an exact per-CPU allowed mask cannot be established through the current read-only provider, all affected allowed facts stay unknown and PPF may reject them under require_known_allowed. The provenance record also identifies whether the allowed-set observation is known or unavailable and names its API source.

pl_ni_snapshot_fingerprint is a deterministic, non-cryptographic FNV-1a fingerprint of normalized topology and resource data. It is a replay/cache identity, not an authenticity signature or security digest. The generation field is currently derived from that fingerprint.

## PPF Placement v2

pl_ppf_place is a pure function. It does not call SetThreadGroupAffinity, sched_setaffinity, or any scheduler mutation API.

Hard constraints are evaluated first: online state, process-allowed state, known-allowed state, ISA support with appropriate provenance scope, explicit occupied CPU mask and exclusive-core policy. Accepted candidates are scored with integer-only weighted cost components for NUMA locality, shared-cache topology, SMT contention, optional bandwidth telemetry and migration cost.

Static-only values carry the ESTIMATED evidence flag. Telemetry is optional and is accepted only when the caller marks the corresponding field valid. No hardware counter is fabricated when a source is unavailable. Equal costs are resolved by the lower normalized CPU index.

The output is a pl_ppf_report with all candidates, rejection reasons, component costs, policy fingerprint and topology generation/fingerprint. It is a placement proposal, not an affinity change.

## Native resource validation

The validator supports typed address-space tags, checked half-open ranges, register width/alignment/access/mask checks and generation-checked DMA ownership transitions. It never dereferences an address or invokes a bus callback.

DMA transitions implemented in v1:

- CPU-owned -> device-owned
- device-owned -> completed or faulted
- completed -> CPU-owned
- CPU-owned/completed -> unmapped

Each successful transition increments the generation. CPU access is denied while the buffer is device-owned or unmapped. A real DMA adapter must additionally implement platform-specific map/unmap, direction, cache maintenance, IOMMU and fence rules before it can be qualified.

## Verification

tests/test_native_intelligence.c runs deterministic fixture tests plus a read-only probe of the current host. CTest runs the same test executable on the available CI runners. A CI runner confirms the APIs and topology visible inside that runner; it is not a substitute for qualification on the user's physical PC, and virtualized topology may differ from bare metal.

## Current boundaries

- The Linux collector is implemented against the kernel sysfs topology and /proc/cpuinfo interfaces.
- The Windows collector parses processor-group/core/package/die/cache and NUMA group relations. Exact per-CPU process eligibility is marked unknown for a multi-group process until a provider can query effective CPU Sets without changing affinity.
- Windows processor-specific ISA capability mapping is not yet per logical CPU; ISA observed on the collector CPU is provenance-labelled and cannot satisfy a per-CPU PPF ISA constraint.
- Bandwidth and dynamic cache pressure are caller-supplied optional telemetry. No unprivileged universal hardware counter source exists across all supported platforms.
- The snapshot does not claim physical device, firmware, voltage, clock-control, driver, MMIO or DMA access.
