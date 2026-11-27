# Processor Fabric v2 — topology, resource ownership and compiler integration

**Contract status:** architecture and interchange contracts are being established. This document does not claim that all collectors, schedulers, hardware adapters or compiler backends are implemented.

## Ownership across the ecosystem

The processor toolchain is one connected pipeline, but it has explicit owners:

| Layer | Canonical owner | Responsibility |
|---|---|---|
| Host evidence | Pytyliki | Read-only CPU/OS topology, hardware-present and OS-usable ISA facts, cache and NUMA evidence, provenance |
| Processor inventory graph | Pytyliki PPF | Stable hardware/process/resource identifiers, dependencies, resource relationships and advisory placement |
| Hardware source syntax | Pytyliki | Structural Hardware AST, stable construct IDs, exact source offsets and source lengths |
| Typed semantics | Reaper | Name resolution, types, effects, capabilities, ownership/borrowing and canonical Typed IR |
| Processor project and build | Reaper PEP | Target/ABI contracts, build graph, diagnostics, tool orchestration and evidence reconciliation |
| Processor lifecycle/control | Reaper Processor Control Plane | CPU/thread state models, scheduling policy, capability-gated platform adapters and lifecycle admission |
| Target backends | Reaper compiler/toolchain | Instruction selection, assembly, object writing, linking, emulation, RTL and formal verification |

PPF is not a second compiler or operating-system scheduler. Reaper is not a substitute for native host inventory. A graph edge or backend descriptor is not permission to perform an effect.

## 1. Read-only topology and ISA evidence

The current pl_processor_scan() captures CPU identity, architecture, logical processor count, memory totals and a bounded x86 ISA set. Its AVX-family flags already distinguish hardware support from OS-enabled vector state. It does not currently discover package, die, physical-core, processor-group, NUMA or cache-sharing topology.

Pytyliki's existing Processor Control model validates caller-supplied logical CPU/package/core identifiers and produces affinity-constrained advisory worker choices. That validation is not an operating-system topology collector.

The v2 evidence envelope in schemas/pytyliki.processor-topology-evidence.v2.schema.json closes the data-contract gap:

- every logical processor keeps its OS identifier and optional processor-group identifier;
- package, die, core and NUMA identifiers are nullable; null means not observed, not zero;
- cache descriptors preserve level, type, capacity, line size and the observed sharing set;
- ISA reports separate hardware_present from os_usable; unknown is distinct from false;
- every capture carries a monotonic generation, collector identity, timestamp, source and completeness state;
- the collector is read-only; lack of permission or unsupported platform APIs results in partial/unknown evidence rather than inferred topology.

The next implementation must add platform collectors (Windows processor topology APIs and Linux sysfs), reconcile their CPU identifiers with the existing CPU profile, and reject stale/mismatched generations. The resulting evidence can then populate PPF. A logical processor may be represented as an observed root when the OS does not expose its parent package/core; the graph must never invent a parent to satisfy a preferred tree shape.

ISA legality uses two independent facts:

1. hardware_present: the instruction feature is exposed by the processor.
2. os_usable: the process/OS has enabled the required execution state and may safely use it.

A compiler target check must reject unknown for a required feature. A performance preference may rank known usable features but may not silently promote a hardware-present, OS-unusable feature.

## 2. NUMA/cache-aware planning and contention

The current PPF planner is deterministic and constrained by process dependencies, domain masks, feature masks, affinity and a per-worker basis-point capacity. Pytyliki's processor-control planner adds affinity-aware least-load selection. Neither model currently includes NUMA locality, cache-sharing domains or memory-bandwidth pressure.

The extended planner is an advisory placement estimator, not an OS scheduler. It applies hard admission constraints first, then a stable cost function:

- hard constraints: online/available worker, usable ISA, process dependency completion, affinity, execution domain and declared resource ownership;
- locality: preferred NUMA node, local versus remote memory, cache-domain compatibility and data reuse;
- contention: SMT sibling pressure, shared-cache working-set pressure, memory-bandwidth demand versus observed/declared NUMA capacity, and existing worker load;
- deterministic tie-break: lower estimated cost, then logical processor ID, then stable worker ID.

Each assignment must expose component scores and reasons (remote_numa, shared_core, shared_cache, bandwidth_pressure, capacity_pressure). Unknown cache/NUMA evidence contributes an explicit unknown penalty or causes a hard rejection when the task requires locality; it must not be treated as local by default.

The planner must never claim that it reserved a core, bound a thread, pinned memory, changed affinity or controlled the OS scheduler. Actual execution and affinity belong to a separately authorized Reaper/platform adapter and require fresh generation checks.

## 3. Typed registers, address spaces, MMIO and DMA ownership

schemas/pytyliki.hardware-resource-contract.v1.schema.json defines the declarative resource envelope. Its key concepts are:

- address-space kind: virtual, physical, I/O port, device BAR or DMA/I/O virtual address;
- memory-region base/length with checked range arithmetic, alignment, permissions and cache policy;
- register bank and register descriptors with width, access mode, volatility, reset/reserved masks, side-effect class and owning device/core;
- DMA buffer direction, alignment, address space, coherence model, device identity, generation and ownership state;
- explicit ownership transitions with bounded lease/lifetime metadata.

Ownership states are cpu_owned, prepared_for_device, device_owned, completed_for_cpu and released. A shared region must declare the applicable coherence and synchronization contract. A device-owned DMA buffer cannot be read or mutated by CPU-side code until completion/reacquisition is explicitly observed.

This contract only validates declarations and state transitions. It does not dereference physical addresses, map MMIO, read/write registers, allocate DMA memory, install an IOMMU mapping, flush caches or submit a DMA descriptor. Those operations require a target-specific privileged adapter and an explicit capability. Reaper's type/effect checker must enforce these rules before code generation.

## 4. Hardware AST → PPF process graph → Reaper Canonical Typed IR

pl_hw_syntax_parse_one() currently produces a structural node with construct kind, body kind, name, source offset and source/body lengths. It does not resolve names, validate types/effects, or perform lowering.

The v2 bridge contract in schemas/pytyliki.hardware-ir-bridge.v1.schema.json defines the hand-off without moving compiler ownership:

1. Pytyliki assigns a stable construct identity and preserves exact AST source spans.
2. Pytyliki links the construct to a PPF process/resource graph using stable node IDs and explicit dependency/resource edges.
3. Pytyliki exports an IR request envelope containing AST kind, source/body spans, declared target hints, resource references and unresolved effect requirements.
4. Reaper resolves names, types, layouts, ownership/borrows, effects and capabilities; then emits its canonical versioned Typed IR with Span/SourceMap.
5. Reaper verifies the typed graph before HIR→MIR→LIR lowering. A process node remains declared or blocked until its semantic and effect requirements are verified.

The bridge must not fabricate Typed IR from a structural AST, erase source provenance, infer a hardware capability from a keyword, or mark a task ready merely because parsing succeeded. The request schema explicitly reports canonical_ir_emitted: false until Reaper has actually produced it.

## 5. Distinct compiler and verification backends

The backend registry schema separates five backend contracts:

| Backend | Input contract | Output contract | Current maturity |
|---|---|---|---|
| Native assembler | Verified target-specific LIR/instruction selection + ABI | Assembly/object input with relocations and source provenance | Contract only |
| Native linker | Object files, ABI/layout and linker policy | Executable/object image plus symbol/relocation/map reports | Contract only |
| Emulator | Verified executable/ISA model + bounded machine state | Deterministic execution trace, state snapshots and fault reports | Contract only |
| RTL | Verified microarchitecture/logic IR + timing/reset/clock contracts | RTL source and synthesis/structural reports | Contract only |
| Formal verification | Typed transition system/property set + assumptions | Proof/unknown/counterexample report with assumptions and tool provenance | Contract only |

Each backend descriptor must identify its ABI, target triple, supported ISA/features, accepted input schema, emitted artifact kinds, determinism, resource bounds, required tools and maturity (specified, prototype, implemented, qualified, disabled). The registry is not proof that an executable tool exists.

No backend is allowed to pass an unqualified artifact to the next stage. Linking requires compatible object ABI and target metadata. Emulator output is simulation evidence, not hardware qualification. RTL emission is not silicon fabrication. Formal unknown is not proved. Native host scalar AOT remains a distinct host-only capability and must not be renamed as a target-specific processor backend.

## Implemented runtime slices in this branch

- Read-only collector source is present for Windows processor groups and Linux sysfs, including package/die/core/thread, NUMA and cache-sharing records with explicit unknowns.
- x86 ISA scan records CPUID hardware feature bits separately from OS-usable AVX-family state.
- The placement implementation computes deterministic component costs for load, NUMA, cache working set/sharing, SMT sibling contention and NUMA bandwidth; it emits advisory assignments only.
- The native C11 resource model validates devices, address spaces, regions, register banks, register ranges, overlapping regions and DMA ownership/generation transitions.

These implementations are marked partial until the platform matrix and CTest workflows pass. They do not perform host affinity changes, MMIO mapping, register access or DMA submissions.

## Sequenced implementation slices

Keep these as separate reviewable changes rather than one unbounded feature branch:

1. **Topology evidence collector:** Windows/Linux read-only collectors, stable CPU IDs, ISA hardware/usable distinction, cache and NUMA evidence, provenance, fixtures and platform CI.
2. **PPF placement v2:** NUMA/cache-aware estimator, contention report, deterministic property tests and unknown-evidence policy.
3. **Hardware resource model:** C11 validators/state machine, schema tests, overflow/alignment/ownership/aliasing negatives and Reaper type/effect fixtures.
4. **AST/IR bridge:** stable semantic IDs, source-map round trips, PPF graph links, Reaper Typed IR adapter, malformed/stale-generation rejection.
5. **Backend contracts and first real backend:** registry/schema first; then assembler, linker, emulator, RTL and formal verification as independently implemented and qualified adapters.

## Acceptance gates

- No fabricated topology IDs; every fact has source, generation and confidence.
- Hardware-present and OS-usable ISA facts remain distinct.
- NUMA/cache placement is deterministic and reports uncertainty and contention.
- Address/range arithmetic is overflow-safe; ownership violations fail closed.
- AST spans round-trip unchanged into Reaper diagnostics and Typed IR.
- Backend maturity is evidence-based; a declared descriptor is not an implementation.
- Tests include malformed, stale, unknown, aliasing, overflow, ownership and negative capability cases.
- No host affinity, register, MMIO, DMA, clock, voltage, firmware, driver or device operation occurs in inventory or planning paths.

## Current implementation boundary

This change records the cross-repository v2 contracts and the required implementation order. It does not add live Windows/Linux topology collectors, NUMA/cache placement code, MMIO/DMA access, a Reaper canonical-IR lowering pass or executable assembler/linker/emulator/RTL/formal backends. Those remain explicit implementation work and must not be presented as complete.
