# Processor Fabric implementation v2 — host evidence, placement and resources

**Maturity:** implementation slice added on a feature branch; Windows/Linux CI and full CTest validation are still required. This page distinguishes source implementation from tested platform behavior.

## Read-only CPU topology collector

The native API is in `src/processor_topology.h` and `src/processor_topology.c`.

- Linux reads `/sys/devices/system/cpu`, per-CPU topology files, node `cpulist` and `meminfo`, plus cache index metadata and `shared_cpu_list`. Linux logical IDs preserve OS CPU numbers; processor-group IDs remain unknown because Linux does not expose Windows-style processor groups.
- Windows reads active processor groups and `GetLogicalProcessorInformationEx(RelationAll)`, including package, die, core, NUMA-node and cache relationships. Per-node available memory is queried separately; total NUMA memory remains unknown in this slice, so that evidence stays partial.
- CPU identity, architecture, capture timestamp, provider/interface, a generation counter, online state and source evidence are retained.
- ISA facts separate hardware presence from OS usability. AVX/AVX2/AVX-512/FMA require the OS-managed vector state; unknown is a tri-state value, not false.
- Bounds are explicit: 256 logical processors, 128 cache records, 32 NUMA nodes. A platform exceeding the representable topology returns a limit error rather than silently truncating the graph.
- Missing attributes stay unknown; the collector does not infer a package, die, core, NUMA ID or processor group.

CLI:

```sh
./build/pytyliki-topology --json
```

The report uses `pytyliki.processor-topology-evidence.v2`. Collection is local and read-only.

## PPF topology graph bridge

`pl_processor_fabric_from_topology()` converts one validated topology snapshot to bounded PPF nodes and edges. It creates only parent nodes backed by observed identifiers; a logical processor whose parent is unknown remains a root. NUMA and processor-group associations are explicit `maps-to` edges. Cache-sharing is represented by `shares-cache` edges.

PPF v1 still uses a 64-bit worker-affinity mask. Therefore the graph bridge rejects logical IDs >=64 rather than dropping workers or aliasing their affinity bits. The topology collector itself can retain IDs up to 255. A future PPF ABI generation must widen the worker mask before the full 256-CPU graph can be planned.

## NUMA/cache-aware Placement v2

`src/processor_placement.c` provides deterministic advisory placement for up to 256 tasks. It applies online/ISA/affinity/locality constraints before ranking candidates. Its cost breakdown includes:

- CPU load and remaining capacity;
- preferred NUMA locality and explicit unknown-locality penalty;
- largest known shared data/unified cache working-set pressure;
- sibling-thread pressure for observed package/die/core identities;
- declared NUMA memory-bandwidth demand versus caller-provided capacity/usage.

Tasks are ordered by descending priority and then ascending task ID. Ties between CPUs resolve by ascending logical processor ID. Assignment results carry component scores and reason flags. A required local NUMA node that is not observed yields `required-locality-unknown`; it is never silently interpreted as local.

The PPF entry point `pl_processor_fabric_place_v2()` requires descriptors to reference READY process nodes. ISA, affinity, load, priority and dependency state are taken from the graph; workload size, bandwidth demand and preferred NUMA are caller-supplied. An incomplete dependency becomes `waiting-dependency`.

CLI for a bounded advisory cost report:

```sh
./build/pytyliki-placement --task 1,10,0,1200,1048576,25000000,1,0x4,1
```

Repeat `--task` to add tasks. The fields are task ID, priority, preferred NUMA node (or `unknown`), requested load in basis points, working set in bytes, bandwidth demand in bytes/second, ISA feature mask, first 64-bit affinity mask (zero means unrestricted), and require-local-NUMA (0/1).

The CLI starts with a zero pressure baseline because it has no live per-core utilization or memory-controller telemetry source yet. The report explicitly says whether caller pressure input was present. This is a reproducible estimate, not a measurement or OS scheduling decision. It never pins threads, changes affinity, reserves CPUs, launches processes or alters scheduling policy.

Output schema: `pytyliki.processor-placement-report.v2`.

## Typed hardware resources and DMA ownership

The C11 validator is in `src/hardware_resources.h` and `src/hardware_resources.c`. The caller-owned model bounds address spaces, devices, regions, register banks, registers and DMA buffers. It checks:

- device ID/generation references;
- address width and overflow-safe base/length ranges;
- power-of-two alignment, permissions and cache-policy enums;
- region overlap, with explicit same-owner alias-group declarations;
- register width, alignment, region bounds and read/write permission compatibility;
- register overlap within a bank;
- DMA address-space/device binding, generation and owner/state agreement.

DMA transitions follow the explicit sequence:

`cpu_owned → prepared_for_device → device_owned → completed_for_cpu → cpu_owned`

A CPU may cancel an unsubmitted prepared buffer or release a CPU-owned/completed buffer. Adapter transitions require a granted capability; the adapter must affirm submission acceptance or completion observation. Stale generations and illegal transitions fail closed.

This library validates descriptors and changes only the caller-owned in-memory model. It does not map MMIO, read/write registers, allocate DMA memory, program an IOMMU, flush caches, acknowledge interrupts or submit hardware descriptors. Those remain separate privileged platform effects.

## Current status and next gates

| Area | Source added in this slice | Still required |
|---|---|---|
| Windows/Linux topology | Read-only collectors and JSON serializer | CI build on both OSes, fixture comparisons and Windows SDK compatibility |
| PPF topology bridge | Graph builder and sparse-ID tests | Wider PPF ABI for IDs >=64 |
| Placement v2 | NUMA/cache/SMT/bandwidth cost model and CLI | Calibrated pressure providers, negative/edge-case CI and schema validation |
| Hardware resources | Native descriptor validator and DMA state transitions | Extended fuzz/property tests, schema-to-C conformance, richer device and IOMMU models |
| Reaper PPF importer | No native importer in this repository | Reaper local JSON importer, digest/generation/reference checks and Canonical Typed IR adapter |
| Backend implementations | No native target backend in this repository | Reaper-owned assembler/linker/emulator/RTL/formal implementations and qualification |

Source presence is not a green build. Promotion from `partial` requires Windows and Linux CI, Release and sanitizer CTest runs, schema validation and documented compatibility limits.
