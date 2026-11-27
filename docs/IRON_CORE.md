# Pytyliki IRON CORE v1

> **Architecture contract · 100 engineering rules · reviewed 2026-11-27**
>
> This document defines the target architecture and evidence gates for evolving Pytyliki into a processor-programming and hardware-engineering platform. It is a specification and integration map, not a claim that all listed features are already implemented.

## 1. Mission and boundary

IRON CORE is the Pytyliki workstream for explicit, inspectable and verifiable work across the computing stack:

**bit representation → instruction set → architectural state → microarchitecture → memory system → devices and buses → execution and timing → digital logic → transistor/electrical models → physical-design evidence.**

The scope deliberately distinguishes four different activities:

1. **Programming real hardware** through documented CPU instructions, OS interfaces, drivers or board-specific HALs.
2. **Inspecting hardware** through read-only host and device evidence.
3. **Modeling hardware** in deterministic CPU, device, logic or circuit simulators.
4. **Designing physical hardware** through RTL, synthesis, EDA, process libraries and external sign-off.

A capability in one category does not imply a capability in another. A model of a transistor is not access to the transistors inside a commercial CPU. A simulator passing a test is not hardware-in-the-loop qualification. A valid RTL file is not a fabricated or electrically qualified chip.

## 2. Repository baseline

The audit was performed against `main` at `43dd56fa39b960de129aa98bcca87aec53f7e1cb` on 2026-11-27.

The inspected main branch is a C11 processor-systems workstation with read-only host discovery, bounded local measurements, source diagnostics, memory utilities, numerical metrics and reference/simulated embedded models. It is not yet a complete general-purpose hardware programming language or a physical hardware-control environment.

The status of the 100 rules is deliberately conservative:

| Status | Rules | Meaning |
|---|---:|---|
| Implemented | 0 | Complete in the audited `main` with direct evidence |
| Partial | 17 | A narrower, related capability exists in `main`; the full rule is not complete |
| Open PR only | 10 | Work is proposed in an open PR and is **not** part of `main` |
| Planned | 73 | Defined target with acceptance criteria; implementation is not present |
| Research | 0 | Requires validated models, external libraries or a research/EDA integration |

These are rule counts, not a percentage of code completion. A rule is promoted only after its acceptance evidence exists on the integration branch. An open PR must pass review and CI before changing the baseline.

## 3. Ownership: Pytyliki, Reaper, Duck OS and board backends

| Owner | Canonical responsibility |
|---|---|
| **Pytyliki** | Processor/device profiles, hardware-domain APIs, native adapters, simulators, evidence, diagnostics and qualification workflows |
| **Reaper** | Canonical language syntax, type/effect semantics, `Span`/`SourceMap`, typed IR, compiler passes and shared ABI/compiler contracts |
| **Duck OS** | Privileged process/device services, protected memory mappings, driver lifecycle, interrupt routing and OS-mediated permissions |
| **Board backend** | Exact SoC/board memory maps, startup/linker configuration, vendor HAL, electrical/process libraries and physical-design sign-off |

Pytyliki must not fork Reaper's grammar, type system or canonical IR. Its hardware-specific constructs are introduced through versioned Reaper contracts and target modules. Duck OS is an execution and privilege boundary, not a reason to put kernel privileges into the workstation process.

## 4. Execution and access modes

The default mode is **`inspect`**.

| Mode | Behavior | Capability required |
|---|---|---:|
| `inspect` | Read-only discovery and evidence collection | No |
| `plan` | Produce an inert, bounded plan; perform no mutation | No |
| `simulate` | Execute only against a named deterministic model | No |
| `verify` | Static, formal, differential and conformance checks | No |
| `target` | Named target adapter with bounded non-privileged operations | Yes |
| `privileged` | Narrow OS/driver/board operations mediated by trusted platform code | Yes |
| `physical_design` | External EDA flows; sign-off requires external evidence | Yes |

The contract is deny-by-default. There is no implicit privilege elevation, hidden network activity, automatic driver/firmware installation or physical write access. Unsupported semantics must return an explicit unsupported result rather than silently degrade to a different operation.

## 5. Proposed module map

The following names are stable design identifiers for the workstream, not claims that every library exists in `main`.

| Module | Responsibility | Current status | Owner |
|---|---|---|---|
| `pyty.bit` | Bit vectors, data representation, arithmetic edge semantics. | Planned | reaper |
| `pyty.isa` | ISA descriptors, instruction forms, feature gates and machine-code formats. | Planned | joint |
| `pyty.reg` | Typed architectural register model and access policies. | Planned | joint |
| `pyty.asm` | Assembly parser/adapter contracts, clobbers and machine-code inspection. | Planned | reaper |
| `pyty.cpu` | Host CPU inventory, topology evidence and target CPU models. | Partial | pytyliki |
| `pyty.micro` | Microarchitecture models: pipelines, queues, predictors, caches and execution ports. | Research | pytyliki |
| `pyty.mem` | Bounded memory utilities, address-space types and memory-system analysis. | Partial | pytyliki |
| `pyty.mmu` | Page-table formats, translation and TLB model/adapters. | Planned | joint |
| `pyty.cache` | Cache hierarchy, maintenance and coherence contracts. | Open PR only | joint |
| `pyty.atomic` | Atomics, fences and memory-ordering litmus tests. | Open PR only | joint |
| `pyty.mmio` | Typed MMIO maps and capability-bounded device register access. | Open PR only | duck-os |
| `pyty.bus` | Bus inventory, protocol adapters and transaction models. | Planned | duck-os |
| `pyty.irq` | Interrupt routing, vector and execution-context contracts. | Planned | duck-os |
| `pyty.timer` | Clock sources, cycle counters, watchdog and timer adapters. | Open PR only | joint |
| `pyty.device` | Device discovery, lifecycle, binding and failure management. | Planned | duck-os |
| `pyty.dma` | DMA descriptor and IOMMU ownership/synchronization model. | Planned | duck-os |
| `pyty.exec` | Execution contexts, scheduling models, affinity and recovery policies. | Open PR only | joint |
| `pyty.time` | Clock calibration, latency, jitter and cycle-model interfaces. | Partial | pytyliki |
| `pyty.power` | Power/thermal observation and vendor-specific control adapters. | Planned | duck-os |
| `pyty.logic` | Gate-level and sequential logic simulation. | Research | board-backend |
| `pyty.rtl` | RTL import/export, synthesis adapters and constraints. | Research | board-backend |
| `pyty.transistor` | Transistor and electrical device models. | Research | board-backend |
| `pyty.eda` | Netlist, timing, physical-design and sign-off toolchain integration. | Research | board-backend |
| `pyty.sim` | Instruction, CPU, device and circuit simulation orchestration. | Planned | pytyliki |
| `pyty.verify` | Static checks, formal obligations, conformance and differential verification. | Partial | joint |
| `pyty.trace` | Bounded measurement trace and evidence provenance. | Partial | pytyliki |
| `pyty.compiler-bridge` | Reaper frontend/IR/effect integration and ABI boundary. | Partial | reaper |
| `pyty.toolchain` | Target-specific assembler, object, linker and loader adapters. | Planned | pytyliki |
| `pyty.security` | Capability policy, unsafe contracts, privilege and artifact trust. | Partial | joint |
| `pyty.evidence` | Versioned reports, test provenance and qualification matrix. | Partial | pytyliki |

## 6. Proposed language surface

The visual `BEGIN IRON` notation is retained as the design's entry concept. The following is **illustrative only**; it is not yet a promise of accepted Reaper/Pytyliki syntax:

```text
@use mode=inspect locale=en

iron.begin {
    target = host.cpu
    profile = read_only

    let topology = cpu.topology()
    let features = isa.usable_features()
    let report = evidence.capture(
        topology,
        features,
        mode = live_os_observation
    )

    verify.report(report)
}
```

The same domain call must have an explicit mode and target. For example, `mmio.read` against a simulator is a simulated register read; against a physical device it requires a mapped region capability, exact register descriptor and trusted platform backend.

The language layer must make the following distinctions visible in types and effects:

- `PhysAddr`, `VirtAddr`, `BusAddr`, `IoPort` and `MmioAddr` are distinct address-space types.
- CPU architectural registers are distinct from MMIO device registers.
- TSC ticks, core cycles, device ticks and monotonic time are different units.
- Read-only observations are distinct from write-capable device operations.
- A DMA buffer has direction, ownership, lifetime and mapping state.
- Simulated, fixture, live-OS, hardware-in-loop and physical-sign-off evidence are different evidence modes.
- Unsafe operations declare preconditions, postconditions, register clobbers, memory effects and privilege requirements.

The canonical syntax, type rules, effect rules and typed IR belong to Reaper. Pytyliki supplies target profiles, hardware semantics adapters, test vectors and execution backends under those contracts.

## 7. The 100 engineering rules

Each rule has a normative requirement and a minimum acceptance condition. The machine-readable source of truth is [`iron/iron-core.v1.json`](../iron/iron-core.v1.json); validate it with `python tools/validate_iron_core.py`.

| ID | Rule | Required capability | Minimum acceptance condition | Status | Owner |
|---|---|---|---|---|---|
| IRON-001 | Exact representation | Every scalar and aggregate has a defined size, alignment, bit representation, endianness and valid value range. | Compile-time layout reports and golden byte vectors agree with the declared target ABI. | Planned | reaper |
| IRON-002 | Parameterized bit vectors | Provide bounded compile-time bit vectors of widths not limited to 8, 16, 32 or 64 bits. | Widths, slicing, concatenation, shifts and overflow behavior pass exhaustive tests for small widths and property tests for larger widths. | Planned | reaper |
| IRON-003 | Defined integer arithmetic | Signedness, overflow, carry, borrow, saturation, division-by-zero and shift edge cases have explicit semantics. | Boundary tests match the language specification on every supported backend; no host-language undefined behavior leaks through. | Planned | reaper |
| IRON-004 | Floating-point representation | Expose floating-point format, rounding mode, exceptional values and bit-level reinterpretation explicitly. | Tests cover NaN payloads, infinities, signed zero, subnormals and target-specific format constraints. | Planned | reaper |
| IRON-005 | Aggregate and bit-field layout | Specify field ordering, packing, padding, alignment and ABI layout without implicit compiler guesses. | A layout inspector emits offsets and byte images that match ABI conformance fixtures. | Planned | reaper |
| IRON-006 | Aliasing and object lifetime | Define when storage may be viewed through multiple types and how object lifetime begins and ends. | Static analysis rejects invalid aliasing/lifetime transitions or requires an explicit unsafe contract. | Planned | reaper |
| IRON-007 | Endianness as a type property | Make byte order explicit at serialization, MMIO and cross-target boundaries. | Big- and little-endian golden vectors round-trip without host-endian assumptions. | Planned | reaper |
| IRON-008 | Observable effects | Represent memory, register, I/O, timing, synchronization and privilege effects in the type/effect model. | Effect reports identify all externally observable operations for each function and reject undeclared effects. | Planned | reaper |
| IRON-009 | Operation cost evidence | Associate target-specific instruction, memory and latency estimates with operations without presenting estimates as guarantees. | Compiler reports separate static estimates from measured samples and carry target/provenance metadata. | Partial | reaper |
| IRON-010 | Formal semantics for constructs | Specify grammar, type rules, evaluation order, effects, traps and observable behavior for every accepted construct. | Every language construct has normative semantics plus positive, negative and regression conformance tests. | Planned | reaper |
| IRON-011 | Bit-addressable operations | Expose individual-bit read, write, test, set, clear, toggle, mask and field extraction primitives. | Property tests verify exact results for all bit positions and reject out-of-range indexes. | Planned | joint |
| IRON-012 | Condition-code model | Represent target condition flags and their producer/consumer semantics explicitly. | ISA fixtures verify flag writes, preservation and condition evaluation per instruction. | Planned | joint |
| IRON-013 | Typed CPU registers | Model general, vector, floating-point, control, debug and status registers as target-specific typed entities. | Register descriptors validate width, access class, architectural availability and privilege before a read or write plan. | Planned | joint |
| IRON-014 | Privileged register access | Separate user-accessible, privileged, debug-only and unavailable registers; never infer permission from register existence. | An unprivileged target rejects privileged operations before backend dispatch; tests assert no side effects. | Planned | joint |
| IRON-015 | Inline assembly contract | Support assembly blocks with declared inputs, outputs, clobbers, memory effects, control-flow effects and target constraints. | Compiler rejects incomplete clobber/effect declarations and preserves declared register constraints. | Planned | joint |
| IRON-016 | Instruction decoding | Decode supported machine encodings into typed instruction records with operands, length and feature requirements. | Round-trip and malformed-byte fixtures cover each supported encoding family; unknown encodings remain unknown. | Planned | joint |
| IRON-017 | Instruction encoding | Encode typed instructions while validating operand forms, feature gates and target-specific constraints. | Encoded bytes match published architectural fixtures and invalid operands are rejected. | Planned | joint |
| IRON-018 | Feature versus OS-state gating | Distinguish hardware-present ISA features from features enabled and usable under the current OS/runtime state. | Dispatch tests reject AVX/other stateful extensions when required OS-managed register state is unavailable. | Partial | joint |
| IRON-019 | Instruction semantic metadata | Maintain machine-readable instruction semantics including registers read/written, flags, memory, exceptions and serialization. | Each supported instruction descriptor has complete metadata checked against conformance fixtures. | Planned | joint |
| IRON-020 | Machine-code image model | Represent sections, symbols, relocations, entry points, permissions and architecture identity without executing the image. | Image validation rejects invalid bounds, overlaps, unsupported relocations and mismatched architecture tags. | Planned | joint |
| IRON-021 | CPU topology model | Represent packages, dies, clusters, cores, SMT threads, cache sharing and NUMA relations with provenance. | Topology validation rejects duplicate IDs and impossible parent relations; live inventory is distinguished from fixtures. | Partial | pytyliki |
| IRON-022 | Execution-unit models | Model scalar integer, floating-point, vector, cryptographic and specialized execution resources. | A reference CPU model routes supported operations to declared units and reports unsupported operations explicitly. | Planned | pytyliki |
| IRON-023 | Pipeline stages | Describe fetch, decode, rename, schedule, execute, memory and retirement stages with configurable latencies. | Cycle simulation produces deterministic stage traces for a documented microarchitecture fixture. | Planned | pytyliki |
| IRON-024 | Dependency and hazard analysis | Track RAW, WAR and WAW dependencies, memory alias uncertainty and structural hazards. | Hazard tests demonstrate stalls, forwarding or rejection according to the selected model. | Planned | pytyliki |
| IRON-025 | Superscalar issue model | Model issue width, queues, port pressure, reorder capacity and retirement limits. | Instruction traces obey configured throughput and resource constraints without claiming to reproduce undocumented silicon. | Planned | pytyliki |
| IRON-026 | Branch prediction model | Support configurable predictors, history, target buffers, miss penalties and speculation boundaries. | Deterministic branch traces report prediction outcomes and modeled penalties. | Planned | pytyliki |
| IRON-027 | Speculation and side-channel model | Represent speculative execution, transient state and observable timing channels for security analysis. | Security fixtures detect declared leakage patterns; simulation results are labeled model evidence, not proof of silicon behavior. | Planned | pytyliki |
| IRON-028 | Cache hierarchy and protocol model | Describe cache levels, line size, associativity, replacement policy, inclusion and coherence protocol. | Cache-model tests validate hits, misses, evictions and protocol transitions against deterministic traces. | Planned | pytyliki |
| IRON-029 | Performance counter integration | Read supported OS/performance-counter sources only through explicit provider adapters with units and permissions. | Counter records identify provider, event, scope, unit, multiplexing and uncertainty; unsupported events remain unavailable. | Open PR only | pytyliki |
| IRON-030 | Custom CPU and ISA descriptions | Allow versioned processor models with register files, instructions, execution resources, memory model and exceptions. | A custom model passes model-schema validation and executes the shared architecture conformance suite. | Planned | pytyliki |
| IRON-031 | Typed address spaces | Keep virtual, physical, bus, I/O-port and device addresses as distinct non-interchangeable types. | The type checker rejects implicit casts between address spaces; explicit checked conversions require a platform capability. | Planned | pytyliki |
| IRON-032 | Page-table representation | Model page-table formats, levels, entries, permissions, page sizes and reserved bits per architecture. | Fixtures decode valid and invalid page tables and reject reserved-bit or alignment violations. | Planned | pytyliki |
| IRON-033 | MMU translation model | Translate virtual addresses through a declared page-table root and architecture profile. | Translation tests cover permissions, page faults, large pages and address-width boundaries. | Planned | pytyliki |
| IRON-034 | TLB model and invalidation | Represent TLB entries, address-space tags, invalidation scope and required ordering. | Tests distinguish local/global invalidation and reject unsupported invalidation semantics. | Planned | pytyliki |
| IRON-035 | Cache-maintenance correctness | Describe cache clean, invalidate and clean-invalidate operations by architecture and memory region. | A backend must declare exact operation semantics; unsupported operations return unsupported, never a silent no-op. | Open PR only | pytyliki |
| IRON-036 | Coherence and ownership | Model cache-line ownership, sharing, visibility and device/CPU coherency boundaries. | Concurrent CPU/device traces detect stale-data scenarios and require the correct synchronization sequence. | Planned | pytyliki |
| IRON-037 | Memory ordering model | Provide explicit atomic orders and target mappings, including fences and device-ordering distinctions. | Litmus tests pass for declared guarantees on each backend; unsupported mappings are rejected. | Planned | pytyliki |
| IRON-038 | Placement and section control | Place code/data/stack/heap/NOINIT sections into named memory regions with alignment and capacity constraints. | Linker-map checks prove every section fits its declared region and report slack/overflow. | Partial | pytyliki |
| IRON-039 | DMA buffers and IOMMU mappings | Represent buffer direction, ownership, lifetime, pinning, mapping and IOMMU permissions explicitly. | State-machine tests reject use-after-unmap, CPU/device concurrent ownership and out-of-range descriptors. | Planned | pytyliki |
| IRON-040 | Memory-system diagnostics | Inspect allocator bounds, fragmentation, locality, bandwidth, faults and address-translation evidence. | Reports identify sample source, method, target and uncertainty; simulated and live evidence are never mixed. | Partial | pytyliki |
| IRON-041 | Typed MMIO registers | Represent device registers with width, access class, reset value, writable mask, endian and ordering requirements. | Register access validates mapping bounds, alignment, width, permission and reserved bits before touching the backend. | Open PR only | duck-os |
| IRON-042 | Machine-readable register maps | Import documented CMSIS-SVD, device-tree bindings or vendor register descriptions as versioned input. | Import is deterministic, records source/version and rejects overlapping or malformed register definitions. | Planned | duck-os |
| IRON-043 | Bus protocol models | Provide explicit models/adapters for PCIe, USB, I2C, SPI, UART and platform-specific interconnects. | Each adapter declares supported transactions, limits, timeouts and whether it is simulated or backed by a live OS service. | Planned | duck-os |
| IRON-044 | Interrupt-controller integration | Model vector tables, interrupt routing, masking, priorities, nesting and architecture-specific entry constraints. | Target tests validate vector bounds and priority rules; privileged registration is available only via a trusted OS/board backend. | Planned | duck-os |
| IRON-045 | Clock and watchdog controls | Inspect and, where explicitly supported, configure timers, clock sources and watchdogs through a board-specific HAL. | Every write requires an exact board/revision profile, capability, safe-state plan and simulator/target conformance test. | Open PR only | duck-os |
| IRON-046 | Device discovery and identity | Enumerate device IDs, topology, functions, resources and driver ownership from trusted platform interfaces. | Reports preserve provider provenance and distinguish absent, inaccessible and unsupported devices. | Planned | duck-os |
| IRON-047 | Driver lifecycle contracts | Specify probe, bind, start, quiesce, suspend, resume, unbind and failure states for device drivers. | State-machine tests ensure resources are acquired/released once and failed starts roll back safely. | Planned | duck-os |
| IRON-048 | DMA engine descriptors | Build and validate device-specific DMA descriptor rings and queue ownership through target adapters. | Descriptor validation checks alignment, bounds, chain termination, permissions and memory synchronization. | Planned | duck-os |
| IRON-049 | Port-mapped I/O | Support architecture-specific I/O port operations only when the target and privilege model expose them. | Compilation and execution are gated by architecture, OS permission and a dedicated backend; unsupported targets fail closed. | Planned | duck-os |
| IRON-050 | Device failure and recovery | Model bus faults, timeouts, device reset, link retraining and recovery checkpoints. | Fault-injection tests prove bounded retries, idempotency and a documented safe-stop path. | Planned | duck-os |
| IRON-051 | Execution context model | Represent process, thread, interrupt, exception, task and bare-metal contexts with allowed operations. | Context checking rejects blocking, allocation or unsafe calls in restricted ISR/exception contexts. | Planned | joint |
| IRON-052 | CPU affinity constraints | Represent allowed CPU sets, core preferences and reversible thread-placement leases. | Planning is inert by default; live placement is limited to the calling process/thread and OS-granted CPU set. | Open PR only | joint |
| IRON-053 | Scheduler semantics | Separate modeled scheduling policy from real OS scheduling and privileged scheduler control. | Simulation is deterministic; live adapters identify the OS policy and do not claim kernel control without a driver/API. | Open PR only | joint |
| IRON-054 | Interrupt-context restrictions | Define interrupt-safe APIs, stack budgets, reentrancy, lock-free requirements and deferred-work handoff. | Static checks and target tests reject blocking/allocating calls in ISR-restricted functions. | Planned | joint |
| IRON-055 | Typed clock sources | Represent monotonic clocks, architectural counters and device timers with frequency, width, scope and unit. | Conversions require calibrated frequency and uncertainty; TSC ticks are never silently called core cycles. | Open PR only | joint |
| IRON-056 | Cycle-accurate execution | Provide a discrete-event CPU model that advances architectural and microarchitectural state per modeled cycle. | Reference traces reproduce instruction, memory and exception timelines for published model fixtures. | Planned | joint |
| IRON-057 | Latency and jitter measurement | Measure latency distributions with warm-up, sample count, clock source, workload, affinity and environmental metadata. | Reports include percentile, dispersion, outlier policy and evidence mode; they do not imply hard real-time guarantees. | Partial | joint |
| IRON-058 | Concurrent interleaving model | Explore multicore interleavings, synchronization order, cache visibility and data-race scenarios. | Bounded model-checking fixtures reproduce known races and verify declared synchronization eliminates them. | Planned | joint |
| IRON-059 | Deterministic record and replay | Record nondeterministic inputs, event order, clock mapping, device observations and execution checkpoints. | Replay either reproduces the recorded modeled execution or reports the first divergence with provenance. | Open PR only | joint |
| IRON-060 | Power and thermal control | Inspect energy, frequency, thermal and power states; expose writes only through vendor/OS-approved adapters. | Read-only mode is default; any control is board/model-specific, consented, reversible where possible and temperature-guarded. | Planned | joint |
| IRON-061 | Transistor device models | Represent MOSFET and other supported devices through explicit model parameters and validity domains. | The model exports a simulator-compatible netlist and passes reference operating-point fixtures. | Planned | board-backend |
| IRON-062 | Electrical units and quantities | Use dimensioned voltage, current, resistance, capacitance, charge and power values with explicit units. | The type system rejects dimensionally invalid formulas and conversions without unit scaling. | Planned | board-backend |
| IRON-063 | Logic gate simulation | Model primitive gates, propagation delay, drive constraints and unknown/high-impedance states where required. | Truth-table and timing fixtures match declared gate semantics, including X/Z behavior where supported. | Planned | board-backend |
| IRON-064 | Combinational circuit synthesis | Describe adders, muxes, decoders, comparators and arbitrary combinational networks. | Equivalence tests compare synthesized network outputs with a reference function across exhaustive or randomized inputs. | Planned | board-backend |
| IRON-065 | Sequential circuit models | Describe flip-flops, latches, counters, registers, finite-state machines and reset behavior. | Clocked simulation checks edge semantics, reset priority, setup/hold assumptions and state transitions. | Planned | board-backend |
| IRON-066 | Clock-domain crossing | Model asynchronous clock domains, synchronizers, metastability risk and CDC constraints. | CDC analysis flags unsynchronized crossings and records modeled probability/assumptions without claiming physical certainty. | Planned | board-backend |
| IRON-067 | Static timing analysis | Analyze path delay, setup/hold slack, clock uncertainty and timing exceptions for a declared library/model. | Timing reports include constraints, library identity and worst paths; unknown delay data prevents pass certification. | Planned | board-backend |
| IRON-068 | Netlist graph representation | Represent cells, pins, nets, drivers, loads, hierarchy and connectivity as a typed graph. | Graph validation rejects dangling pins, multiple illegal drivers and combinational loops where prohibited. | Planned | board-backend |
| IRON-069 | RTL and synthesis flow | Integrate synthesizable hardware-description subsets and external synthesis tools through explicit adapters. | A reproducible flow compiles RTL, records tool versions and checks synthesized structure against constraints. | Planned | board-backend |
| IRON-070 | Physical design and sign-off | Integrate floorplanning, placement, routing, parasitic extraction, DRC, LVS and PVT-corner analysis as external EDA stages. | A physical sign-off claim requires target process/library data and successful external tool reports; a software model alone is insufficient. | Planned | board-backend |
| IRON-071 | Canonical Reaper frontend | Keep accepted hardware-language syntax and normative grammar under Reaper ownership; Pytyliki supplies domain modules and target profiles. | A pinned Reaper toolchain parses the canonical source and emits versioned diagnostics; Pytyliki does not fork the grammar. | Partial | reaper |
| IRON-072 | Canonical typed IR and source mapping | Use Reaper typed IR, Span and SourceMap as the source of truth for every hardware operation. | Every lowered instruction/effect maps to source Span and passes IR schema plus round-trip fixtures. | Planned | reaper |
| IRON-073 | Effect and capability IR | Carry memory, MMIO, DMA, interrupt, timing, privilege and nondeterminism effects through the IR. | A denied capability or missing effect declaration is rejected before backend code generation. | Planned | reaper |
| IRON-074 | HIR/MIR/LIR lowering | Lower high-level hardware descriptions through stable, inspectable intermediate representations. | Each pass has pre/postconditions, deterministic output and golden IR regression tests. | Planned | reaper |
| IRON-075 | ABI and calling-convention control | Model DABI64 and target ABIs, register assignments, stack layout, aggregate passing and unwind boundaries. | Cross-language fixtures validate parameter/result layout and reject incompatible ABI revisions. | Partial | reaper |
| IRON-076 | Assembler and object writer | Provide assembler, object sections, symbols, relocations and architecture-specific encoding backends. | Object fixtures are accepted by independent inspection tools and match golden bytes/relocations. | Planned | reaper |
| IRON-077 | Linker and loader path | Develop deterministic linking, relocation application, image layout and a constrained loader interface. | Relocation fixtures resolve correctly; loader validates permissions, address ranges and target ABI before mapping. | Planned | reaper |
| IRON-078 | No-runtime and bare-metal profiles | Support no_std/no_alloc, caller-owned storage, deterministic startup and explicit entry points. | Minimal artifacts link without an implicit runtime or allocator and pass target-specific startup tests. | Partial | reaper |
| IRON-079 | Cross-target code generation | Select backend by ISA, ABI, feature set and execution environment without pretending all targets are equivalent. | A target matrix compiles shared conformance programs or emits a precise unsupported-feature diagnostic. | Planned | reaper |
| IRON-080 | Artifact inspection and reproducibility | Inspect generated code, relocations, memory maps, dependency versions and deterministic build inputs. | Repeated builds from pinned inputs produce byte-identical outputs or explain nondeterministic sections. | Open PR only | reaper |
| IRON-081 | Static safety analysis | Analyze types, bounds, lifetimes, register constraints, effects, privileges and concurrency before execution. | Negative fixtures are rejected with stable diagnostics and source spans. | Partial | pytyliki |
| IRON-082 | Proof-carrying compiler passes | Attach machine-checkable proof obligations or validated invariants to semantics-preserving compiler transformations. | Proof checks run in CI for supported passes; unproved experimental passes remain explicitly marked. | Planned | pytyliki |
| IRON-083 | Reference and differential simulator | Compare independent CPU/device models and supported reference implementations using identical input traces. | Differential mismatches are minimized and reported with model versions and deterministic seeds. | Planned | pytyliki |
| IRON-084 | Instruction-level simulator | Interpret supported ISA instructions with architectural state, exceptions and memory effects. | Architecture conformance suites pass for every claimed instruction; missing instructions fail explicitly. | Planned | pytyliki |
| IRON-085 | Cycle-accurate simulator | Simulate pipeline, caches, buses and timing per cycle for a declared model. | Cycle traces match golden model fixtures and never claim cycle fidelity for undocumented physical CPUs. | Planned | pytyliki |
| IRON-086 | Fault injection and trace verification | Inject bounded faults in simulators and validate trace, recovery and safe-stop behavior. | Tests prove bounded execution and recovery decisions; live hardware fault injection requires separate lab authorization. | Planned | pytyliki |
| IRON-087 | Hardware-in-the-loop qualification | Run target tests on named board, silicon revision, toolchain, firmware and test fixture. | Every qualification report records exact hardware identity and operator-approved procedure; host-only results cannot pass this gate. | Planned | pytyliki |
| IRON-088 | Golden conformance corpus | Maintain golden source, IR, assembly, object, memory-map, trace and simulator outputs. | CI detects drift and requires reviewed fixture changes with provenance. | Partial | pytyliki |
| IRON-089 | Evidence provenance and trust | Distinguish live measurement, OS-reported inventory, static analysis, fixture, simulation and physical sign-off evidence. | Every report includes evidence_mode, provider, target, timestamp/build identity and confidence limitations. | Partial | pytyliki |
| IRON-090 | Reproducible experiments | Pin source revisions, tools, parameters, seeds, fixtures and result formats for experiments. | A clean environment can reproduce the declared output or reports the precise missing dependency. | Planned | pytyliki |
| IRON-091 | Capability-secured hardware access | Require narrow, revocable capabilities for each privileged register, memory mapping, DMA queue and device operation. | Denied-by-default tests prove missing, expired or wrong-target capabilities cannot reach a hardware backend. | Partial | joint |
| IRON-092 | Explicit unsafe contracts | Mark unsafe regions and state preconditions, invariants, clobbers, lifetime and failure behavior. | Every unsafe API has reviewed safety documentation and positive/negative compile-time or runtime tests. | Planned | joint |
| IRON-093 | OS and privilege boundary | Keep process-level tools separate from kernel drivers, hypervisors, firmware and board startup code. | Unprivileged builds cannot silently elevate; privileged adapters are separate, signed/reviewed and target-specific. | Planned | joint |
| IRON-094 | Local-first deterministic behavior | Keep discovery and analysis local, bounded and read-only by default; network access is explicit and optional. | Offline tests pass; no hidden telemetry, downloads, tool installation or device modification occurs. | Partial | joint |
| IRON-095 | Versioned architecture adapters | Add ISA, OS, device and board adapters through stable versioned contracts and explicit capability declarations. | A new adapter passes contract tests and cannot broaden another platform's permissions. | Partial | joint |
| IRON-096 | Language bridge registry | Maintain the planned 66-language/toolchain registry with separate statuses for profile, parser, FFI, ABI, compiler and verification support. | Every registry entry has a unique identity and honest support level; a profile is never counted as a working compiler. | Open PR only | joint |
| IRON-097 | Complete specifications and migration policy | Document syntax, semantics, APIs, ABI compatibility, diagnostics, examples and deprecation rules. | Every public contract version has schema, changelog, examples and compatibility tests. | Partial | joint |
| IRON-098 | Supply-chain and toolchain integrity | Pin compiler/tool versions and hashes; validate imported packages, generated artifacts and signed release evidence. | CI verifies pins/digests and records provenance; untrusted binaries are never auto-executed. | Planned | joint |
| IRON-099 | Per-target conformance matrix | Maintain independent compile, simulation and hardware qualification status per ISA/OS/board combination. | Support claims are generated from test evidence and identify unsupported or unqualified combinations. | Partial | joint |
| IRON-100 | Evidence-based lowest-level claim | Publish a level-by-level comparison of direct control, modeling, verification, backend coverage and physical qualification. | No superlative claim is made unless all cited capabilities have reproducible evidence and scope limitations. | Planned | joint |

## 8. Maturity gates

The ten IRON levels are engineering gates, not decorative badges. A level can be declared reached only after its acceptance gate passes on a reviewed branch.

| Level | Name | Scope | Acceptance gate |
|---|---|---|---|
| IRON-0 | Language and contract foundation | Canonical Reaper syntax/IR contract, target descriptors, capability vocabulary and deterministic diagnostics. | Grammar, type/effect and target-profile conformance checks pass. |
| IRON-1 | Bit and machine representation | Bit vectors, exact layout, register model, ASM effects and ISA encoding/decoding. | Golden bit-layout, register and instruction vectors pass. |
| IRON-2 | Processor and microarchitecture | CPU topology plus configurable architectural and microarchitectural models. | Reference CPU model passes architectural fixtures and deterministic traces. |
| IRON-3 | Memory engineering | Address spaces, MMU, page tables, TLB, cache, atomics, DMA ownership. | Translation, memory-ordering and ownership model tests pass. |
| IRON-4 | Device and I/O engineering | Typed MMIO, bus/device models, IRQ and driver lifecycle adapters. | Simulator and target-specific device-contract tests pass. |
| IRON-5 | Execution and timing engineering | Execution contexts, scheduling models, clocks, cycle and latency evidence. | Replayable timing reports and context-safety checks pass. |
| IRON-6 | Digital and analog engineering | Logic, sequential circuits, clock domains, netlists, transistor/electrical models. | Reference circuit and timing analyses pass for declared libraries/models. |
| IRON-7 | Compiler and artifact engineering | Reaper lowering, ABI, assembler, object writer, linker, loader and no-runtime profiles. | Golden IR/object/relocation fixtures and reproducible-build checks pass. |
| IRON-8 | Verification and evidence engineering | Static/formal analysis, simulators, fault injection, hardware-in-loop and provenance. | Every capability status is tied to reproducible evidence and explicit test mode. |
| IRON-9 | Physical hardware engineering | Vendor/process libraries, physical design, DRC/LVS, PVT, fabricated-target qualification. | Named physical implementation passes external EDA sign-off and board/silicon qualification. |

Digital logic, transistor modeling and physical design are different disciplines from privileged CPU execution. They can progress in parallel, but one cannot be used as evidence for another.

## 9. Existing work that must be integrated, not duplicated

The repository audit found these open workstreams. Their state was checked on 2026-11-27; **none is counted as merged into `main` here**.

| PR | Work | Branch relationship | State | Integration boundary |
|---|---|---|---|---|
| [#280](https://github.com/Ivan-VHS/Pytyliki/pull/280) | Deterministic Reaper processor control plane | `feat/processor-state-kernel` → `main` | open | Reaper-owned processor state model, logical scheduling, replay order and report contracts. |
| [#282](https://github.com/Ivan-VHS/Pytyliki/pull/282) | Processor policy and native C11 engine | `feat/processor-control-plane-v1` → `main` | open | Read-only topology/evidence, worker selection, partitioning and deny-by-default operation admission. |
| [#285](https://github.com/Ivan-VHS/Pytyliki/pull/285) | Explicit silicon runtime primitives | `feat/processor-silicon-runtime-2026-11-27` → `main` | open | Architecture barriers, cycle sources, bounded MMIO region capability, image-layout validation and trace ring. |
| [#286](https://github.com/Ivan-VHS/Pytyliki/pull/286) | Bounded thread fault recovery | `feat/processor-thread-fault-recovery-v1` → `main` | open | Bounded worker-fault correlation and recovery planning; all actions remain plans. |
| [#287](https://github.com/Ivan-VHS/Pytyliki/pull/287) | Embedded silicon runtime | `feat/embedded-silicon-runtime-v1` → `main` | open | Versioned board HAL callbacks, cycle sources, MMIO allowlists and deterministic embedded runtime model. |
| [#290](https://github.com/Ivan-VHS/Pytyliki/pull/290) | Processor platform evidence adapters | `feat/processor-platform-adapters-v1` → `feat/processor-thread-fault-recovery-v1` | open | Read-only WHEA/EDAC evidence, evidence chain and reversible calling-thread affinity lease; stacked on PR 286. |
| [#294](https://github.com/Ivan-VHS/Pytyliki/pull/294) | Processor engineering report v1 | `feat/processor-engineering-report-v1` → `main` | open | Versioned JSON Schema and semantic validation for processor profile/benchmark evidence. |
| [#297](https://github.com/Ivan-VHS/Pytyliki/pull/297) | PTFR polyglot platform adapters | `feat/ptfr-platform-adapters-polyglot-v1` → `feat/processor-thread-fault-recovery-v1` | open-draft | Stacked Rust/Swift/TypeScript adapters for fault-evidence contracts; draft and not part of main. |
| [#298](https://github.com/Ivan-VHS/Pytyliki/pull/298) | Processor language workbench with 66 profiles | `feat/processor-language-workbench-66` → `main` | open-draft | 66-profile registry, structural inspection, target validation and inert build planning; not 66 bundled compilers. |

Integration notes:

- PRs #285 and #287 both touch the silicon/runtime boundary and require an explicit overlap review before any consolidation.
- PR #282 is a processor-policy/evidence engine, not a global scheduler or privileged CPU-control service.
- PR #280 models deterministic state and logical scheduling; do not describe it as kernel scheduler control.
- PR #290 is stacked on #286. Its WHEA/EDAC data is platform evidence, not automatic attribution to a specific worker or core.
- PR #297 is also stacked on #286 and is a draft; it is not an independent mainline capability.
- PR #298 is a 66-profile workbench, not 66 bundled compilers or 66 complete language frontends.
- PR #294 formalizes an evidence/report envelope; a valid JSON report is not proof of physical hardware behavior.

Before implementing a new subsystem, compare it with these workstreams and reuse the reviewed contract/API rather than creating duplicate registries, counters, evidence formats or control planes.

## 10. Technical rules for the most sensitive subsystems

### CPU and ISA

Keep compile-target architecture identity separate from live host CPU identity. Keep hardware-present ISA features separate from OS-enabled and actually usable features. Instruction support requires an encoding/decoder/semantic descriptor and a conformance fixture. A catalogue entry alone is not support.

### Memory, cache and DMA

Use distinct address spaces and overflow-checked ranges. MMIO must be represented by typed, bounded capabilities with width, alignment, allowed access, reserved bits and ordering rules. DMA must specify buffer lifetime, device/CPU ownership, IOMMU permissions and cache synchronization. Do not equate C `volatile` with atomicity or memory ordering.

### IRQ and execution context

Interrupt handlers have explicit context types, stack and latency budgets, reentrancy constraints and a restricted API set. Blocking operations and unbounded allocation must be rejected where the target context cannot permit them. Real vector registration and interrupt routing belong behind Duck OS or a board-specific trusted backend.

### Timing and power

Every clock source declares scope, width, frequency provenance and unit. A TSC reading is a TSC tick count unless calibrated; it is not automatically a core-cycle count. Latency and jitter reports include warm-up, sample count, workload, affinity, environment and uncertainty. CPU frequency, voltage and power-state writes remain unavailable unless a specific vendor/OS-supported adapter and hardware qualification exist.

### Digital and physical modeling

Use dimensional units for voltage, current, resistance, capacitance and power. Gate-level simulation must define X/Z and timing semantics where supported. RTL synthesis, static timing, CDC, DRC, LVS, PVT and physical sign-off are separate external-tool stages with pinned libraries and reports. Transistor-level simulation is not direct access to a fabricated CPU's internals.

### Experimental domains

Quantum, neuromorphic, liquid-memory and bio-signal concepts belong in isolated experimental models or external device adapters. They must carry explicit assumptions and evidence modes. They must not be represented as available physical hardware features without a real device and validated backend.

## 11. Required report provenance

Every result that can influence a support claim should report at least:

- contract and tool versions;
- source revision and target identity;
- architecture, ABI, board and silicon revision where applicable;
- evidence mode and provider;
- clock source, unit and calibration/uncertainty;
- model/library/toolchain versions;
- test command, seed and fixture identity;
- pass/fail/unsupported result and limitations.

The accepted evidence modes are `live_os_observation`, `bounded_host_measurement`, `static_analysis`, `reference_fixture`, `deterministic_simulation`, `hardware_in_loop` and `physical_signoff`. Reports must not silently mix them.

## 12. Implementation sequence

1. **Contract and baseline:** land this architecture contract, schema and validator first.
2. **Silicon/runtime foundations:** review and integrate existing MMIO, barrier and counter work only after target/API overlap is resolved.
3. **Processor policy and evidence:** integrate topology, safe admission, affinity leases, fault evidence and reports as separate reviewed increments.
4. **ISA and register layer:** add target descriptors, typed register metadata, decoder/encoder fixtures and explicit ASM effect contracts.
5. **Memory and device layer:** add address-space types, MMU/TLB/cache models, MMIO register maps, DMA ownership, IRQ and driver lifecycle adapters.
6. **Execution and timing:** add context checking, deterministic CPU models, trace/replay and measured latency with provenance.
7. **Simulation and circuit engineering:** implement instruction/CPU simulation, then logic/RTL/timing analysis, then transistor/electrical integrations.
8. **Compiler integration:** add hardware effects and target lowering through Reaper's canonical IR; then assembler/object/linker/loader work.
9. **Qualification:** build per-ISA, per-OS, per-board test matrices and hardware-in-loop evidence. Physical design requires independent EDA sign-off.
10. **Claims and release:** derive support claims from contract status and evidence. Do not claim a level or a record before its gates pass.

Each implementation increment should be a separate reviewable PR with focused tests, architecture notes, CI and a clear statement of what is still unsupported. Protected main is not bypassed.

## 13. Validation

Run:

```sh
python tools/validate_iron_core.py
python -m unittest tests/test_iron_core_contract.py
```

The validator checks rule cardinality and IDs, references, evidence paths, owner/status vocabulary, PR tracking, maturity gates and the deny-by-default safety invariants. The JSON Schema defines the exchange format; the semantic validator checks cross-record consistency.

## 14. Claim policy

Pytyliki's ambition is to provide unusually broad, explicit and verifiable coverage from bit representation through processor architecture and physical hardware engineering. "The lowest-level language in history" is a research/engineering goal, not a fact established by this specification.

That claim can be evaluated only with a public capability matrix, comparable independent systems, working implementations, target-specific tests, simulator evidence and physical qualification where claimed. Breadth of vocabulary, number of module names or access to privileged instructions alone does not establish it.

## 15. Omega-Null research profile

Omega-Null is an experimental physics-inspired profile within IRON CORE, not a replacement for Reaper and not a claim of control beneath physical law. It preserves 20 resonance markers but normalizes them into typed operations with explicit targets, modes, effects and evidence. Its four framework concepts are mapped to process/model isolation, deterministic checkpoints and replay, materials/CAD/fabrication workflows, and consent-based BCI adapters.

The profile defines 12 research/planned operations and explicitly excludes physical reality mutation, forced quantum outcomes, spacetime manipulation, physical time reversal, matter-from-thought and thought-reading claims. The complete mapping, operation catalogue and acceptance boundaries are in [Omega-Null](OMEGA_NULL.md), with the source contract at [omega-null/omega-null.v0.json](../omega-null/omega-null.v0.json).
