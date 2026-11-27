# Pytyliki Processor IDE

> **Processor IDE Contract v1 · 100 rules · 20 workspaces · 2026-11-28**
>
> This is the architecture and acceptance contract for a unified target-aware processor development IDE. It defines the environment, views, target model, evidence rules and integration requirements; it is not a claim that the full IDE is already shipped.

## 1. Product definition

**Processor IDE** is one integrated engineering environment for processor programming, assembly, microarchitecture, machine-state inspection, debugging, simulation, verification, measurement and hardware qualification.

It is not twenty separate applications. The twenty named systems already registered by `src/ironclad_systems.c` become twenty domain workspaces inside one project/target-aware shell. Each workspace owns a concern and shares the same project identity, target profile, execution mode, evidence context and diagnostics.

The IDE must expose the machine clearly without destroying useful abstraction. The developer can move down through:

**project/source → Reaper typed IR → assembly → instruction bytes → architectural registers → address spaces → microarchitecture model → signal/logic model → qualified physical device evidence.**

No layer is hidden, but higher-level source and HDL support is not banned. Pytyliki's goal is explicit machine transparency, not the artificial restriction of every project to handwritten assembly.

## 2. Repository baseline and honest status

Audited against `main` at `41edd2bd33fbf554c8a8cbdc1ddd97847e1f9a53` on 2026-11-28.

The audited main branch contains a C11 processor workstation, read-only CPU/usable-ISA discovery, bounded benchmarks, a 20-entry enhancement/workspace registry, memory and trace primitives, reference embedded models, and a capability-gated low-level target/memory/RAM-MMIO/clock-power/IRQ contract tested with a fake backend. It does not yet ship a unified Processor IDE with live register panes, assembly editor/disassembler, pipeline/cycle simulator, physical logic-analyzer transport or vendor debug integration.

The rules below have a conservative status:

- `partial`: a supporting primitive exists in `main`, but the integrated Processor IDE behavior is not shipped.
- `planned`: a concrete IDE requirement with an acceptance condition; no implementation is claimed.
- `research`: requires validated microarchitecture, analog, physical-design or external laboratory models.
- `implemented`: reserved for a complete, tested IDE behavior; none is claimed by this contract.

The twenty-entry enhancement registry is implemented, but the twenty interactive IDE workspaces are planned. Existing CPU discovery, bounded benchmarks, trace records, memory primitives, snapshot storage, embedded reference models and the low-level target contract are building blocks—not a finished IDE.

## 3. The twenty workspaces

| Workspace ID | Workspace | Existing domain | Responsibility | Maturity |
|---|---|---|---|---|
| chrome-ironclad | **Chrome Ironclad** | reliability | Project and target invariants, state validation and fail-safe readiness. | implemented; IDE planned |
| velocity-tempest | **Velocity Tempest** | performance | Cycle/latency budgets, reproducible benchmarks and throughput comparisons. | implemented; IDE planned |
| midnight-monolith | **Midnight Monolith** | modularity | Subsystem graph, dependency boundaries, profiles and project structure. | implemented; IDE planned |
| raven-vortex | **Raven Vortex** | diagnostics | Compiler, assembler, simulator, hardware and source-mapped diagnostics. | implemented; IDE planned |
| circuit-cobalt | **Circuit Cobalt** | low-level | Registers, bits, bus, memory maps, privilege and device capability inspection. | implemented; IDE planned |
| thunder-phantom | **Thunder Phantom** | async | Threads, cores, event queues, cancellation, interleavings and bounded jobs. | implemented; IDE planned |
| phantom-radiance | **Phantom Radiance** | api | CLI, native API, Reaper bridge, plugin and target contracts. | implemented; IDE planned |
| velvet-gravitas | **Velvet Gravitas** | usability | Editor layout, command palette, accessibility, onboarding and help. | implemented; IDE planned |
| obsidian-inferno | **Obsidian Inferno** | recovery | Snapshots, replay, fault injection, rollback and safe-stop workflow. | implemented; IDE planned |
| horizon-spectral | **Horizon Spectral** | portability | ISA/ABI/OS/board target matrix and cross-target compatibility. | implemented; IDE planned |
| crimson-overdrive | **Crimson Overdrive** | compilation | Reaper frontend, IR, assembler, linker, image generation and build graph. | implemented; IDE planned |
| voltage-citadel | **Voltage Citadel** | security | Capabilities, privilege, secrets, W^X, constant-time and secure debugging. | implemented; IDE planned |
| shadow-nemesis | **Shadow Nemesis** | testing | Unit, golden, differential, model, HIL and regression tests. | implemented; IDE planned |
| mirage-aether | **Mirage Aether** | distributed | Multi-core, multi-target and remote lab session orchestration with explicit transport. | implemented; IDE planned |
| titanium-riptide | **Titanium Riptide** | memory | Memory regions, cache/TLB, stack/heap, DMA buffers and address inspection. | implemented; IDE planned |
| eclipse-outlaw | **Eclipse Outlaw** | extensibility | Target packs, ISA plugins, HDL/EDA adapters and compatible extensions. | implemented; IDE planned |
| neutron-zenith | **Neutron Zenith** | profiling | Hot-path analysis, cycles, waits, cache events and energy measurements. | implemented; IDE planned |
| scarlet-avalanche | **Scarlet Avalanche** | automation | Build/test pipelines, reproducible artifacts and pull-request gates. | implemented; IDE planned |
| quantum-paradox | **Quantum Paradox** | experimental | Isolated CPU, logic, transistor, quantum and speculative research models. | implemented; IDE planned |
| reactor-afterglow | **Reactor Afterglow** | observability | Live/simulated status, event stream, health and evidence provenance. | implemented; IDE planned |

The existing registry API is the source of truth for the workspace names and domain IDs. Processor IDE must extend or consume it rather than creating a second list with renamed domains.

### The twenty named engineering spaces

These are the twenty perspectives supplied in the Processor IDE brief. They sit inside the shared workspace shell and map to the existing twenty-domain registry; they are not separate applications and do not grant privilege.

| Perspective ID | Display name | Workspace | Focus | Status |
|---|---|---|---|---|
| machine-oriented-studio | Окружение — Машинно-ориентированная — Студия разработки процессоров | `crimson-overdrive` | Target-bound project entry, source/IR/ASM navigation and build session. | planned |
| hardware-engineering-lab | Пространство — Аппаратно-ориентированная — Лаборатория процессорной инженерии | `reactor-afterglow` | Waveforms, bus capture, signal phases and measurement provenance. | planned |
| machine-processor-design-center | Платформа — Машинная — Центр проектирования процессоров | `horizon-spectral` | Target profile, register map, bus map, clock and timing configuration. | planned |
| hardware-technology-workshop | Экосистема — Аппаратная — Мастерская процессорных технологий | `eclipse-outlaw` | Simulator, timing analyzer, compiler and backend adapter integration. | planned |
| native-microprocessor-bureau | Инфраструктура — Нативная — Бюро разработки микропроцессоров | `scarlet-avalanche` | Reproducible native builds, memory images and descriptor generation. | planned |
| assembly-systems-institute | Оболочка — Ассемблерная — Институт процессорных систем | `crimson-overdrive` | Assembly editor, instruction encodings, flags and source provenance. | planned |
| processor-development-complex | Система — Процессорная — Комплекс процессорной разработки | `reactor-afterglow` | Unified simulation timeline for pipeline, caches, memory and interrupts. | planned |
| microarchitecture-atelier | Контекст — Микропроцессорная — Ателье микроархитектуры | `quantum-paradox` | Core/thread context, execution mode, pipeline model and target capabilities. | planned |
| register-design-lab | Прослойка — Регистровая — Лаборатория проектирования CPU | `circuit-cobalt` | Architecture-specific register bank, flags and per-step register diff. | planned |
| bit-construction-center | Интерфейс — Битовая — Центр процессорного конструирования | `titanium-riptide` | Bitfields, shifts, rotates, masks, descriptors and endian-aware views. | planned |
| instruction-engineering-studio | Рабочее пространство — Инструкционная — Студия микроэлектронной инженерии | `crimson-overdrive` | Source-to-IR-to-assembly-to-byte correlation and cost evidence. | planned |
| address-architecture-workshop | Исполнительная площадка — Адресная — Мастерская аппаратных архитектур | `titanium-riptide` | Virtual, physical, bus, I/O and MMIO address spaces with page boundaries. | planned |
| microarchitecture-technology-center | Технологическая база — Микроархитектурная — Центр разработки микропроцессорных систем | `quantum-paradox` | Versioned pipeline, branch, queue, cache and execution-unit models. | planned |
| processor-modeling-lab | Программная платформа — Системная — Лаборатория процессорного моделирования | `shadow-nemesis` | Scenario execution on reference models and emulators before target execution. | planned |
| computer-architecture-bureau | Операционная среда — Фундаментальная — Бюро архитектуры вычислительных систем | `voltage-citadel` | Privilege, reset, interrupt-vector and address-space isolation review. | planned |
| hardware-programming-studio | Инструментальная система — Базовая — Студия аппаратного программирования | `raven-vortex` | Quick tests, register snapshots, trace diff, diagnostics and breakpoints. | planned |
| compute-core-engineering-center | Вычислительное окружение — Железно-ориентированная — Центр инженерии вычислительных ядер | `thunder-phantom` | Core load, locks, synchronization barriers and memory-order events. | planned |
| digital-logic-lab | Функциональное пространство — Машинно-приближённая — Лаборатория цифровой логики | `quantum-paradox` | Boolean logic, sequential logic, FSM reachability and transition verification. | planned |
| cpu-hardware-software-workshop | Аппаратно-программный комплекс — Аппаратно-зависимая — Мастерская разработки CPU | `circuit-cobalt` | Stepping-, board-, bus- and silicon-specific dependency inspection. | planned |
| near-machine-runtime-center | Среда выполнения — Близкая к машинному коду — Инжиниринговый центр процессорных технологий | `crimson-overdrive` | No-alloc/bare-metal profiles, execution traces, artifact inspection and runtime boundary. | planned |

## 4. Unified shell and ten persistent panes

The shell binds all panes to one project and one explicit target. A pane may change abstraction level, but it must not silently change architecture, silicon stepping, board, ABI or evidence mode.

| Pane | Purpose |
|---|---|
| `project_target` | Project, target, ISA/ABI, stepping, board, OS and execution-mode identity. |
| `source_ir` | Source editor, Reaper typed IR, spans, diagnostics and source-to-machine navigation. |
| `assembly_image` | Assembler, disassembler, bytes, symbols, relocations, sections and memory map. |
| `registers_flags` | Target-specific architectural registers, flags, access class and before/after diff. |
| `memory_address` | Virtual/physical/bus/MMIO address spaces, pages, regions, permissions and DMA ownership. |
| `pipeline_microarchitecture` | Model-dependent stages, queues, stalls, hazards, predictor, issue and retirement timeline. |
| `signals_timing` | Clock domains, waveforms, cycle/timing/latency evidence, power and thermal signals. |
| `trace_replay` | Bounded event capture, breakpoints, checkpoints, replay and divergence analysis. |
| `build_test` | Reproducible build graph, target matrix, golden tests, CI and artifact verification. |
| `diagnostics_diff` | Source-mapped diagnostics, register/memory/trace/project diffs and remediation. |

Recommended interaction model: target/project header; central source/IR/assembly editor; dockable register, memory, pipeline and waveform inspectors; trace timeline; diagnostics and artifact/diff views. The target profile remains visible while moving between panes.

## 5. Target model and execution modes

The target profile identifies the architecture, ISA revision, ABI, address/word widths, endianness and execution environment. Optional identity includes CPU vendor/family/model, stepping, board revision, topology, cache geometry, clock domains and debug transport.

Unknown information remains unknown. The IDE must never assume a universal `R0-R7` register bank, `regit` register, 64-byte cache line, Little-Endian target, fixed-width instructions or one privilege model across x86, ARM and RISC-V.

| Mode | Meaning | Mutation |
|---|---|---|
| `inspect` | Read-only host/target metadata and existing evidence. | No |
| `plan` | Inert build, debug, target or lab operation plans. | No |
| `static` | Parsing, type/effect analysis, lint and artifact inspection. | No |
| `simulate` | Execution against a named CPU/device/circuit model. | No |
| `debug` | Debug sessions with explicit halt/read/write side effects and permission. | provider-dependent |
| `hardware_in_loop` | Named physical target tests under a documented lab/board procedure. | capability-gated |
| `privileged` | Narrow OS/driver-mediated access to privileged CPU or device operations. | capability-gated |

Default mode is `inspect`. Target writes, debug register writes, power controls and physical fault injection require explicit target identity, narrowly scoped capability and visible operator action. The existing `pytyliki.low-level-rules.v1` / `pl_ll_*` API is the canonical target/memory/RAM-MMIO/clock-power/IRQ contract; Processor IDE must not build a competing hardware-access layer.

## 6. Evidence, timing and instrumentation

The waveform/electrical view uses a signal-specific legend, independent from the global IDE theme:

| State | Default color | Pattern |
|---|---|---|
| VCC / logic high | Red `#E53935` | Solid |
| GND / logic low | Black `#070707` | Solid |
| Floating / high impedance | Blue `#2878D0` | Dashed |
| Contended | Yellow `#FFD400` | Crosshatch |
| Unknown / indeterminate | Gray `#8B8B8B` | Dotted |

Every state also has a text label and a non-color pattern. The palette can be changed without changing signal semantics.

All time, power and signal data carries provenance. The UI distinguishes live OS observation, bounded host measurement, static analysis, reference fixture, deterministic or probabilistic simulation, hardware-in-the-loop and physical sign-off.

A value includes target, provider, evidence mode, clock source, unit, sampling window, sample count, uncertainty, dropped samples, profiler/probe overhead and limitations. TSC ticks, core cycles, device ticks and wall time are not interchangeable without a calibrated mapping.

**No universal 1 ns UI promise.** A physical bus event can be shorter than the UI refresh period. A supported instrument may capture it at its own sample rate; the IDE displays a buffered, timestamped representation and reports capture/render latency. Real-time guarantees require target-specific hardware, scheduler, interrupt and measurement evidence.

## 7. Rules 001–050 — IDE core

| ID | Workspace | Rule | Requirement | Acceptance | Status |
|---|---|---|---|---|---|
| PIDE-001 | Chrome Ironclad | **Target-bound project entry** | Open a project with an explicit target profile, ISA, ABI, silicon/board revision when known, OS/runtime context and selected execution mode. | A project cannot start a build or target session with an unresolved target; unknown fields remain unknown and are never guessed. | Partial |
| PIDE-002 | Reactor Afterglow | **Signal and phase workspace** | Display signal transitions, trace timestamps, sample cadence, measured latency and uncertainty for the selected model or instrumented target. | Each plotted series identifies live, simulated, estimated or unknown provenance, sampling method, clock source and dropped-sample count. | Partial |
| PIDE-003 | Horizon Spectral | **Versioned machine platform description** | Keep register maps, memory regions, buses, interrupt vectors, clock domains, timing constraints and feature gates in one versioned target description. | Schema validation rejects overlaps, invalid widths, impossible alignments and incompatible target revisions. | Partial |
| PIDE-004 | Midnight Monolith | **Direct tool integration without hidden layers** | Connect assembler, compiler, simulator, trace analyzer and code generator through explicit versioned contracts and visible data transformations. | Every adapter declares input/output schema, version, failure mode, provenance and resource limits; no hidden background operation occurs. | Partial |
| PIDE-005 | Scarlet Avalanche | **Reproducible native build infrastructure** | Build target images, symbol tables, descriptor tables and map files from pinned, inspectable inputs. | Repeated builds from pinned inputs are byte-identical except for documented non-deterministic fields; output includes toolchain and target metadata. | Partial |
| PIDE-006 | Crimson Overdrive | **Assembly-first machine view** | Allow direct inspection and editing of target assembly/machine code while preserving source, IR, symbol, Span and ABI context. | A selected instruction can be traced to its source/IR origin when available and to its address/encoding in the target image. | Partial |
| PIDE-007 | Reactor Afterglow | **Unified processor execution timeline** | Synchronize simulator state, pipeline, cache/TLB, memory, interrupts and events into a single logical timeline. | A simulation step updates all modeled components atomically; live hardware streams use timestamp correlation and declare skew rather than pretending all devices share one clock. | Partial |
| PIDE-008 | Circuit Cobalt | **Microprocessor execution context** | Show active target mode, privilege, execution context, core/thread, clock domain, memory permissions and capabilities at the selected node. | The context panel shows unknown/unavailable separately from false/disabled and displays the provider of every detected field. | Partial |
| PIDE-009 | Circuit Cobalt | **Live and simulated register inspector** | Display the target's documented architectural register set, aliases and flags, including R0–R7 when defined, PC, FLAGS and the project-defined regit view when explicitly declared. | Register names, widths, access classes and read/write effects derive from the selected ISA/target descriptor. regit is shown only as a declared project/debugger view, never fabricated as a universal architectural register; changed values are diffed per trace event. | Planned |
| PIDE-010 | Titanium Riptide | **Bit-level construction interface** | Provide exact bit/field views for masks, slices, shifts, rotates, sign extension, overflow and descriptor layouts. | Round-trip tests prove bit offsets, widths and endian interpretation against target fixtures; dangerous truncation is surfaced before emission. | Planned |
| PIDE-011 | Crimson Overdrive | **Instruction workbench** | Correlate source line, typed IR operation, assembly mnemonic, encoded bytes, decoded operands, target feature requirements and available cost evidence. | A selected instruction shows origin, exact encoding and whether cost is static estimate, simulator result or live measurement. | Planned |
| PIDE-012 | Titanium Riptide | **Typed address execution map** | Visualize virtual, physical, bus, I/O-port and MMIO addresses with page boundaries, permissions, alignment and mappings. | Address-space types cannot be mixed implicitly; every live physical/device map comes from a capability-gated target provider. | Partial |
| PIDE-013 | Horizon Spectral | **Microarchitecture profile library** | Store versioned reference profiles for pipeline stages, branch prediction, queues, execution units, cache hierarchy and retirement policy. | Every profile identifies source/revision, scope, assumptions and modeled features; custom models pass schema and conformance tests. | Planned |
| PIDE-014 | Shadow Nemesis | **Scenario execution platform** | Run programs against reference CPU/device models, instruction emulators and declared hardware fixtures before any target session. | The result includes simulator version, model identity, seed, supported-instruction coverage and divergence from expected fixtures. | Partial |
| PIDE-015 | Voltage Citadel | **Privilege and reset verification** | Check boot/reset entry, privilege transitions, interrupt vectors, protected regions and OS-mediated capabilities against the selected target. | Negative tests prove unprivileged actions cannot reach privileged callbacks; reset/boot claims require a board or emulator backend. | Partial |
| PIDE-016 | Obsidian Inferno | **Snapshot and before/after comparison** | Capture and diff register state, flags, memory ranges, pipeline model state and relevant device-model state before and after an operation. | Snapshots identify captured fields and omitted state; diffs are deterministic and respect permissions and sensitive-memory policies. | Partial |
| PIDE-017 | Thunder Phantom | **Core activity and synchronization monitor** | Inspect core/thread load, queue occupancy, locks, barriers, waits and memory-order events where providers expose them. | Every metric identifies sample interval and scope; simulated and OS-observed data are never mixed. | Partial |
| PIDE-018 | Quantum Paradox | **Digital logic and FSM workbench** | Model Boolean logic, sequential elements, finite-state machines, truth tables, state reachability and transition conditions. | Exhaustive small-state tests or bounded model checks identify unreachable states, dead ends and invalid transitions. | Research |
| PIDE-019 | Eclipse Outlaw | **Hardware dependency map** | Mark instructions, registers, timing assumptions, MMIO layouts and bus behavior tied to specific silicon, stepping or board revisions. | A target incompatibility is reported before build/launch; portable paths and unsupported target-specific paths are visibly separated. | Partial |
| PIDE-020 | Crimson Overdrive | **No-hidden-runtime execution profile** | Provide explicit no_alloc, no_std/bare-metal and bounded-runtime build profiles for critical code and target artifacts. | The build report lists runtime, allocator, TLS, exception, dynamic-loader and background-service dependencies; each profile fails closed on disallowed dependencies. | Partial |
| PIDE-021 | Velocity Tempest | **Measured and estimated operation cost** | Show cycles, latency, memory traffic and energy costs before simulation where static target data exists, then compare against execution evidence. | Each value is labeled measured, estimated, simulated or unknown and includes units, target, method and uncertainty. | Partial |
| PIDE-022 | Raven Vortex | **Explicit narrowing and signedness** | Require visible casts for narrowing, signedness changes, pointer-width conversions and truncating bitfields. | Static diagnostics flag potential value loss and provide boundary tests for accepted conversions. | Planned |
| PIDE-023 | Titanium Riptide | **Alignment checking** | Validate alignment for buffers, page tables, instruction fetch, descriptor rings and target structures. | The diagnostic states required and actual alignment and distinguishes architecturally faulting accesses from merely slower ones. | Partial |
| PIDE-024 | Shadow Nemesis | **FSM reachability and deadlock checks** | Analyze state-machine reachability, terminal states, deadlocks and event-triggered transition completeness. | A bounded or formal check produces a counterexample trace for each detected deadlock or unreachable state. | Research |
| PIDE-025 | Quantum Paradox | **Clock-domain crossing contracts** | Require explicit synchronizers, async FIFOs or documented CDC bridges between unrelated clock domains. | CDC lint rejects unbridged crossings and records synchronizer depth, reset behavior and metastability assumptions. | Research |
| PIDE-026 | Velocity Tempest | **Energy and time windows** | Validate declared timing and energy budgets such as within_window<N> and energy_cap<E> against measured or modeled data. | The run fails or reports indeterminate when a required budget is exceeded or the evidence is insufficient; units and tolerances are explicit. | Partial |
| PIDE-027 | Voltage Citadel | **Constant-time security inspection** | Warn about secret-dependent branches, memory addresses, lookup tables and variable-latency operations inside constant-time regions. | Static findings and target-specific timing tests are reported as risk evidence, not as a proof of universal side-channel absence. | Planned |
| PIDE-028 | Raven Vortex | **Stable diagnostic status vocabulary** | Use stable machine-readable codes for parse/type/ABI/permission/timeout/overflow/device/simulation failures. | Every error has a code, severity, source span or target location, remediation and stable serialization. | Partial |
| PIDE-029 | Crimson Overdrive | **Named constants and physical units** | Require named constants for masks, slot addresses, thresholds and timing/power/energy quantities with explicit units. | Lint identifies raw numeric literals in policy-sensitive contexts; values use unit-safe types or documented suffixes such as _cyc, _bytes and _pj. | Planned |
| PIDE-030 | Velvet Gravitas | **Readable control-flow assistance** | Offer refactor suggestions for excessive nesting, duplicated predicates and unclear early-exit structure. | Suggestions preserve semantics, show a diff and require user acceptance; no silent source rewrite occurs. | Planned |
| PIDE-031 | Obsidian Inferno | **Deterministic crash reproduction** | Record input, instruction position, target/model revision, register checkpoint and event ordering for replayable failures. | Replay reproduces the same modeled state or identifies the first divergence; live-hardware nondeterminism is explicitly captured as evidence rather than claimed deterministic. | Planned |
| PIDE-032 | Obsidian Inferno | **Precise before/after diff** | Compare register, memory, flags, device model, build artifacts and pull-request changes by named ranges and provenance. | Diffs identify changed fields and omitted/unreadable regions; secret data is redacted by policy. | Partial |
| PIDE-033 | Raven Vortex | **Dead-code and dead-state analysis** | Detect unreachable labels, unused registers, unreferenced symbols, dead flags and unreachable basic blocks where target semantics permit. | Findings are source-mapped and architecture-aware; special-register side effects and inline assembly clobbers prevent unsafe false positives. | Planned |
| PIDE-034 | Obsidian Inferno | **Safe rollback and recovery** | Record failure points and restore only supported logical snapshots, simulator state or version-controlled artifacts. | Rollback reports restored and non-restorable state; it never implies reversal of external device or physical events. | Partial |
| PIDE-035 | Voltage Citadel | **No hidden IDE side effects** | Autocomplete, navigation, analysis and suggested actions must remain observational until the user explicitly runs a command. | Tests prove editor suggestions do not launch tools, change devices, write MMIO, install drivers or alter target state. | Planned |
| PIDE-036 | Phantom Radiance | **Canonical operation forms** | Prefer one canonical spelling/API per operation while keeping explicit aliases where compatibility or ISA syntax requires them. | Aliases normalize to one stable semantic ID and preserve original source spans for diagnostics. | Partial |
| PIDE-037 | Crimson Overdrive | **Positive state names** | Prefer positive names such as is_ready, is_locked and is_valid in new APIs and generated examples. | Lint detects confusing double negatives in new declarations without renaming externally specified ISA symbols. | Planned |
| PIDE-038 | Velvet Gravitas | **Intent-first comments** | Explain why an implementation choice exists and which hardware/toolchain limitation it respects. | Documentation review flags comments that merely restate code; public hardware assumptions are documented. | Planned |
| PIDE-039 | Velvet Gravitas | **Novice-readable onboarding** | Provide guided target setup, project tour, register/trace explanations and working examples for the main IDE screens. | A new user can open a sample target project, run a safe simulation and inspect a trace using only in-product guidance. | Planned |
| PIDE-040 | Scarlet Avalanche | **Fast feedback loop** | Keep edit, syntax/type checks, contract validation, incremental build and trace inspection responsive through caching and incremental work. | CI records timings for each stage; local performance budgets are target-class-specific rather than a universal seconds guarantee. | Planned |
| PIDE-041 | Scarlet Avalanche | **Small, reviewable PR flow** | Generate focused change sets with a clear acceptance criterion and independent tests for each subsystem. | A PR checklist identifies affected contracts, target profiles, tests, docs, migrations and downstream repositories. | Planned |
| PIDE-042 | Scarlet Avalanche | **Sequential integration into main** | Rebase/update each next integration PR against the latest accepted mainline and honor branch protection/rulesets. | Merge automation refuses stale base commits, failed required checks, unresolved conflicts and dependency-order violations. | Planned |
| PIDE-043 | Scarlet Avalanche | **Automatic validation pipeline** | Run format/lint, unit tests, schema validation, simulator golden tests, sanitizers and target-specific checks on relevant changes. | CI reports each gate separately and cannot pass a hardware gate using only a host fixture. | Partial |
| PIDE-044 | Neutron Zenith | **Unified engineering metrics** | Normalize cycles, instruction count, code size, cache events, stalls, tests, coverage, energy and thermal measurements across modules. | Metric records include unit, target, evidence mode, provider, context and uncertainty; incompatible samples cannot be compared. | Partial |
| PIDE-045 | Eclipse Outlaw | **External toolchain interoperability** | Integrate C/ASM, linker/debugger, HDL, waveform, EDA and vendor tools through declared adapters and versioned artifacts. | Tool support matrix distinguishes installed, detected, validated, invoked and fully integrated states. | Partial |
| PIDE-046 | Midnight Monolith | **Compact core and opt-in extensions** | Keep the IDE shell and core contracts small; load experimental target packs, EDA tools and debug adapters explicitly. | Extension manifests declare permissions, ABI, target support and dependencies; unknown extensions are rejected safely. | Partial |
| PIDE-047 | Velvet Gravitas | **Fast navigation and keyboard workflow** | Provide command palette, symbol search, register search, go-to-definition, history and configurable hotkeys. | UI interaction latency is benchmarked on named reference machines and large projects; no instant-response guarantee is assumed. | Planned |
| PIDE-048 | Neutron Zenith | **Hot-path and resource profiler** | Rank expensive instructions, pipeline waits, memory stalls, cache events and power/thermal costs from available evidence. | Profiles distinguish actual counters, simulator estimates and sampling bias, and identify profiler overhead. | Partial |
| PIDE-049 | Midnight Monolith | **Workspace lifecycle and deprecation** | Retire obsolete panels, rules and adapters through versioned migration notes while preserving old project readability. | Deprecated features carry removal versions and migration paths; no accepted project is silently reinterpreted. | Planned |
| PIDE-050 | Eclipse Outlaw | **Backward-compatible evolution** | Version project formats, target descriptions, plugins, AST/IR interchange and trace formats with explicit migration policy. | Golden fixtures prove supported prior versions remain readable or receive an explicit migration diagnostic. | Partial |

## 8. Rules 051–100 — silicon depth and physical engineering

The second rule group preserves the user's absolute-low-level ambitions while making architectural facts target-specific and physically meaningful. These corrections prevent an IDE from generating unsafe or invalid code from universal assumptions.

| ID | Workspace | Rule | Requirement | Acceptance | Status |
|---|---|---|---|---|---|
| PIDE-051 | Crimson Overdrive | **Assembly-first, not abstraction-blind** | The default machine view leads with decoded instructions and registers, while retaining source, Reaper IR, C, HDL and model layers as explicit navigable levels. | Every supported source layer has a visible lowering path; target assembly and machine bytes are inspectable for built artifacts. | Planned |
| PIDE-052 | Reactor Afterglow | **Instrumented bus timeline** | Display sampled bus transactions and logic signals from a simulator or explicitly instrumented target with timestamps and capture metadata. | The UI declares sample rate, buffer depth, trigger, drop count, transport delay and probe intrusion; no universal 1 ns UI deadline is claimed. | Partial |
| PIDE-053 | Velvet Gravitas | **Voltage and signal-state palette** | Provide a signal-specific legend for VCC/high, GND/low, floating, contended, unknown and high-impedance states. | Color is redundant with labels/patterns and meets contrast/accessibility checks; palette is configurable per project. | Research |
| PIDE-054 | Eclipse Outlaw | **Versioned physical layout artifacts** | Treat floorplan, placement, routing, pin map, constraints and cell/library references as versioned project artifacts. | Every layout artifact references process/library version, die revision, units, source RTL/netlist digest and tool provenance. | Research |
| PIDE-055 | Neutron Zenith | **Area and utilization analysis** | Measure cell area, macro area, density, congestion, whitespace, timing and power for a declared process/library. | Reports distinguish utilization and routability constraints; empty area is not filled with arbitrary structures. | Research |
| PIDE-056 | Velocity Tempest | **Thermal and power prediction** | Estimate thermal hotspots from a validated power map, package/cooling model and boundary conditions; compare with available sensors. | Predictions include model, calibration, time horizon, uncertainty and safety margin; target writes remain capability-gated. | Research |
| PIDE-057 | Quantum Paradox | **Clocked and asynchronous domains** | Model synchronous domains by declared clock edges and asynchronous crossings through explicit CDC primitives. | CDC checks validate synchronizers, reset crossings and handshake/FIFO protocols; asynchronous subsystems are not forced into one global clock. | Research |
| PIDE-058 | Neutron Zenith | **Pipeline hazard and stall accounting** | Represent stalls, bubbles, replay, flushes and hazard reasons in the selected pipeline model and trace. | Every modeled stall has an explicit cause and cycle interval; optimizer changes preserve architectural behavior and the model's constraints. | Planned |
| PIDE-059 | Horizon Spectral | **Target-specific branch prediction policy** | Describe predictor profile, branch hints and history behavior per ISA/microarchitecture. | Static hints are explicit and benchmarked; unsupported hints are rejected or ignored only under documented target rules. | Planned |
| PIDE-060 | Horizon Spectral | **Silicon stepping compatibility** | Bind images and target operations to compatible silicon stepping, board revision, errata and feature sets. | An incompatible revision is blocked; compatible revisions follow an explicit tested compatibility matrix. | Planned |
| PIDE-061 | Obsidian Inferno | **Glitch and fault artifact capture** | Capture glitches, malformed waveforms, transient model states and fault-injection cases as reproducible test artifacts. | Artifacts include model/target, trigger, capture window, seed, expected/observed state and recovery outcome. | Planned |
| PIDE-062 | Raven Vortex | **Failure-analysis export** | Export crash bundles, traces, waveform windows, register snapshots, netlists and metadata for external failure-analysis tools/labs. | Exports use documented formats and redact secrets; microscope/FA equipment is never automatically controlled by a crash handler. | Planned |
| PIDE-063 | Crimson Overdrive | **Source as canonical; binary as derived artifact** | Keep human-readable source, declarative hardware models and versioned binary outputs linked through digests and reproducible build metadata. | A clean checkout can rebuild artifacts and verify source-to-binary provenance. | Planned |
| PIDE-064 | Crimson Overdrive | **Static or monolithic image profiles** | Offer fully static/monolithic bare-metal images where target requirements demand them while allowing dynamic linking when ABI and runtime policy permit. | Build report lists every linked module, relocation, runtime dependency, load region and reproducibility input. | Planned |
| PIDE-065 | Voltage Citadel | **Hardware-backed key and entropy providers** | Use approved entropy sources and platform key providers such as TPM/secure elements where available; keep keys opaque to the IDE. | Randomness health tests and key-provider provenance are explicit; secrets are not derived from an assumed CPU noise source. | Planned |
| PIDE-066 | Crimson Overdrive | **Canonical ISA mnemonic registry** | Expose each target ISA's canonical instruction names, encoding, aliases and disassembly spelling in a versioned registry. | Aliases normalize to canonical semantic IDs while preserving source spelling and assembler compatibility. | Planned |
| PIDE-067 | Circuit Cobalt | **Register change and dirty-state visualization** | Highlight registers changed by a selected instruction/event and distinguish dirty compiler values, architectural writes and debugger modifications. | Diff markers are tied to trace sequence and access provenance; animation can be disabled and is not the only indicator. | Planned |
| PIDE-068 | Circuit Cobalt | **Typed condition flags** | Model target flags such as zero, carry, overflow, sign and interrupt state as typed architecture-specific state. | Flag reads/writes and preservation are validated from instruction metadata and calling-convention rules. | Planned |
| PIDE-069 | Thunder Phantom | **Interrupt priority and affinity policy** | Model interrupt routing, priority, affinity, masking, nesting and coalescing according to the target interrupt controller. | No interrupt is silently dropped; coalescing is allowed only for declared edge/level semantics and documented loss policy. | Partial |
| PIDE-070 | Circuit Cobalt | **Atomic operation semantics** | Describe atomicity, width, memory order, lock-free guarantees and bus/device scope per ISA and target. | Litmus tests and target fixtures verify compare-exchange/RMW ordering; unavailable widths or semantics fail explicitly. | Planned |
| PIDE-071 | Thunder Phantom | **Inter-core coherency and communication** | Model cache coherence, shared memory ordering, message passing, barriers and synchronization channels across cores. | Concurrency tests validate visibility and race freedom under the selected memory model; coherence capabilities are target-specific. | Planned |
| PIDE-072 | Circuit Cobalt | **Micro-operation and microcode inspection** | Expose uop decomposition and microcode metadata only when documented or provided by a supported softcore/vendor tool. | A view records source/provenance and separates observed uops from inferred decomposition. | Planned |
| PIDE-073 | Titanium Riptide | **Cache-line and transfer geometry** | Use target-described cache-line size, burst length, sector size and memory transfer granularity for visualization and warnings. | Alignment and efficiency warnings derive from target profile and measured data; byte-granular valid operations remain valid. | Planned |
| PIDE-074 | Titanium Riptide | **TLB invalidation and shootdown model** | Describe address-space invalidation scope, required barriers, interprocessor notifications and completion acknowledgements per architecture/OS. | Model or target adapter verifies required ordering and completion before affected mappings are reused. | Planned |
| PIDE-075 | Circuit Cobalt | **Architectural register names and semantic aliases** | Retain canonical ISA register names while allowing contextual aliases such as ACCUMULATOR or INDEX_PTR in source/debug views. | Aliases resolve to a canonical register identity and never obscure width, role or ABI constraints. | Planned |
| PIDE-076 | Horizon Spectral | **Context save/restore and ABI volatility** | Generate save/restore sets from ABI caller/callee-saved rules, interrupt frames, vector state and target context requirements. | Conformance tests verify preserved state across calls, interrupts and context switches for each target ABI. | Planned |
| PIDE-077 | Voltage Citadel | **Privileged register access policy** | Gate MSR/control/debug register access by ISA privilege state, OS permission, target capability and backend support. | Denied accesses fail before backend invocation; policy accounts for x86 rings, ARM exception levels, RISC-V privilege modes and other targets. | Partial |
| PIDE-078 | Circuit Cobalt | **Bit-shift and carry semantics** | Expose whether shifts/rotates produce carry/overflow flags and how lost bits are represented according to the target ISA. | Instruction-level tests cover each shift width, count boundary and target flag result. | Planned |
| PIDE-079 | Velocity Tempest | **Masking and branch trade-off analysis** | Offer bitmask lowering as an optimization candidate when it is semantically correct and improves the target cost/security profile. | Differential tests prove equivalence; benchmark evidence includes branch predictability, vectorization and side-channel constraints. | Planned |
| PIDE-080 | Horizon Spectral | **Endianness as target property** | Honor the declared target, ABI, device and file-format byte order, with explicit conversion at boundaries. | Big- and little-endian fixtures decode correctly; mixed-endian device fields are explicitly documented. | Partial |
| PIDE-081 | Crimson Overdrive | **Variable-length instruction decoding** | Decode instruction boundaries using the target ISA specification and machine-mode state. | Golden byte streams validate length, alignment, prefixes and malformed encoding behavior. | Planned |
| PIDE-082 | Crimson Overdrive | **NOP and alignment planning** | Insert NOP/alignment padding only when required by ISA, ABI, errata, timing or explicit layout constraints. | The map file records every inserted NOP and its reason; generated output passes functional and timing tests. | Planned |
| PIDE-083 | Raven Vortex | **Undefined and reserved instruction handling** | Represent invalid, reserved, unsupported and implementation-defined encodings distinctly in assemblers, decoders and simulators. | Negative fixtures verify architectural exception/diagnostic behavior without silently assigning invented semantics. | Planned |
| PIDE-084 | Titanium Riptide | **Target-width address model** | Derive virtual/physical address widths, canonical forms and sign-extension rules from the selected ISA/MMU profile. | Boundary fixtures reject noncanonical/out-of-range addresses and never silently ignore high bits. | Partial |
| PIDE-085 | Voltage Citadel | **Page permission and W^X policy** | Represent read/write/execute and device permissions per page/region and enforce W^X where supported by target policy. | Negative tests prove disallowed W+X mappings are rejected; deliberate exceptions require an explicit reviewed policy. | Partial |
| PIDE-086 | Titanium Riptide | **DMA and IOMMU ownership** | Track device ownership, DMA direction, mapping, pinning, lifetime, IOMMU domain and cache synchronization. | State-machine tests reject use-after-unmap, out-of-range descriptor and CPU/device concurrent ownership violations. | Planned |
| PIDE-087 | Quantum Paradox | **Out-of-order and retirement model** | Model OoO issue, reorder buffers, speculation, precise retirement and architectural program-order visibility when the selected core supports them. | Deterministic traces distinguish issue/execute/retire order and validate architectural state against a reference model. | Planned |
| PIDE-088 | Voltage Citadel | **Speculative execution and side-channel analysis** | Analyze speculative/transient execution and observable cache/timing channels with target-specific mitigations and test cases. | Reports identify known attack surface and mitigation status; no blanket guarantee of eliminating all side channels is made. | Research |
| PIDE-089 | Voltage Citadel | **Power gating and idle-state model** | Describe supported clock/power domains, idle states, gating dependencies and approved control hooks for the target. | A model validates sequencing and safety interlocks; physical control requires an exact board/vendor backend and capability. | Partial |
| PIDE-090 | Shadow Nemesis | **Cycle-accurate simulator qualification** | Provide cycle-accurate simulation only for fully declared microarchitecture models with validated timing inputs. | Golden traces match the declared model and reference tests; real silicon correlation reports measured deviation and uncertainty. | Planned |
| PIDE-091 | Obsidian Inferno | **Bounded instruction trace recording** | Record instruction/event traces into a bounded ring buffer with trigger, wrap, loss, sampling and privacy policy. | Overflow and dropped events are explicit; trace overhead and perturbation are measured. | Partial |
| PIDE-092 | Shadow Nemesis | **Fault injection and safety modes** | Inject bit flips, timing faults and device errors in simulation by default; physical fault injection is a separately approved lab mode. | Every injection is reproducible and bounded; live injection requires isolation, operator approval and safe-stop procedure. | Planned |
| PIDE-093 | Midnight Monolith | **Kernel and driver separation** | Model kernel responsibilities, user-mode services, driver privilege and hardware mediation according to the selected OS. | Architecture docs and tests show which components execute in user/kernel/firmware context; no IDE process silently elevates. | Partial |
| PIDE-094 | Velocity Tempest | **Hard real-time evidence gate** | Support deadline analysis, WCET budgets, interrupt load, jitter, scheduling and platform qualification for hard real-time targets. | A guarantee is issued only for a named platform/workload with schedulability analysis and HIL evidence; otherwise label estimate or best effort. | Partial |
| PIDE-095 | Mirage Aether | **Virtualization and zero-copy observability** | Expose hardware-assisted virtualization, mappings, exits, IOMMU and data-copy paths for supported hosts/targets. | Metrics show actual copies, mappings and exits; overhead is benchmarked, not assumed zero. | Planned |
| PIDE-096 | Raven Vortex | **Hardware breakpoint management** | Manage target-supported hardware breakpoint/watchpoint slots, scope, alignment and debugger side effects. | UI reports slot availability and impact; breakpoints beyond capacity fail with a target-specific diagnostic. | Planned |
| PIDE-097 | Reactor Afterglow | **Logic analyzer and waveform integration** | Show simulator waveforms and optionally ingest physical logic-analyzer/oscilloscope streams through named adapters. | Signal names, probe points, sample rates, trigger configuration and calibration/provenance are retained. | Planned |
| PIDE-098 | Circuit Cobalt | **Live register watch and debug transport** | Support target-appropriate JTAG/SWD/debug providers with explicit halt, sampling, side effects and access permissions. | The IDE warns when observation halts or perturbs the target and records transport/session state. | Planned |
| PIDE-099 | Obsidian Inferno | **Hardware versus model discrepancy workflow** | Compare simulator, architectural specification, errata and live hardware observations and record reproducible divergence. | A mismatch is triaged with exact silicon stepping, test, trace and source; observed behavior is not automatically treated as intended architectural behavior. | Partial |
| PIDE-100 | Chrome Ironclad | **Begin Iron session bootstrap** | Initialize a project session with target profile, toolchain, safety mode, resource budget and provenance before any command runs. | A session is ready only after required target/toolchain validation; default operations are inspect/plan/simulate and physical actions remain capability-gated. | Partial |

## 9. Mandatory interpretations and corrections

| Original absolute | Processor IDE rule |
|---|---|
| Only ASM; all higher-level languages forbidden | Assembly-first visibility, with explicit source/IR/HDL layers and navigable lowering |
| Direct, unbuffered bus view with 1 ns UI response | Instrumented capture with timestamps, bounded buffers and measured capture/render latency |
| All targets Little-Endian, 64-bit address, 64-byte cache line | Endian, address width, cache geometry and register map come from the named target |
| Every instruction fixed-width and all stalls removable | Decode according to ISA; preserve required stalls, hazards and synchronization |
| Branch predictor always taken | Model predictor behavior and hints per target microarchitecture |
| Any RMW must be one cycle or lock bus | Validate atomicity/ordering according to ISA and memory model |
| Arbitrary CPU microcode editing | Inspection where documented; editing only on programmable softcores/vendor-authorized platforms |
| All physical glitches accepted as useful production state | Keep fault/glitch captures as isolated test artifacts; never accept corrupted state as normal output |
| Binary-only source and chip-noise-generated encryption | Preserve reviewable source and reproducible artifacts; use validated entropy and trusted key providers |
| Zero-overhead virtualization and guaranteed RT | Measure overhead; require WCET, schedulability and hardware qualification for RT claims |
| Every real CPU can be cycle-accurately cloned | Cycle-accurate simulation is limited to a declared model and validated timing evidence |

These are not reductions in ambition. They are guardrails that make the results technically trustworthy.

## 10. Existing integration points

| Existing component/workstream | Processor IDE integration |
|---|---|
| `src/ironclad_systems.h/.c` | Canonical 20-workspace domain registry |
| `src/processor.c/.h` | Read-only host/usable-ISA discovery and bounded benchmark foundation |
| `src/low_level_contract.h/.c` | Target descriptor, memory map, RAM/MMIO capabilities, clock/power requests and IRQ registry |
| `src/trace.h/.c` | Measured/simulated trace records with units, uncertainty and provenance |
| `src/memory.c` | Checked memory views, arenas and pools |
| `src/snapshot_store.c` | Durable logical snapshots and recovery |
| Reaper canonical frontend/IR | Source parsing, types/effects, spans, lowering and ABI contracts |
| PR #280 / #281 / #282 | Reaper processor state/policy and evidence integration |
| PR #285 / #287 | Silicon runtime/cycle/MMIO adapter overlap to reconcile |
| PR #286 / #290 | Bounded fault recovery and read-only platform evidence |
| PR #294 / #298 | Processor report schema and language profile workbench |

Current status is not overstated: register editing, machine-code disassembly, pipeline simulation, live JTAG/SWD sessions, hardware logic-analyzer transport, physical design and cycle-accurate real-silicon equivalence are not shipped by this contract.

### First native session foundation in this change

This PR also adds a small C11 API:

- `src/processor_ide.h/.c` — `pl_ide_session_init`, `pl_ide_session_validate`, status strings and workspace lookup.
- `pl_ide_session` binds project/session IDs, one of the twenty existing workspace IDs, a validated `pl_ll_target` and an explicit mode.
- `PL_IDE_MODE_DEFAULT` resolves to `PL_IDE_MODE_INSPECT`.
- Inspect, plan, static and simulation sessions are metadata-only. Debug, hardware-in-loop and privileged modes require declared target-provider, authorization and operator-approval preconditions; debug additionally requires a debug provider. These session flags are not security tokens or hardware grants.
- The session validator performs no device access, tool invocation, MMIO operation or hardware mutation. Every actual operation must still pass `pl_ll_bus` capability checks and trusted Duck OS/board-backend authorization.
- `tests/test_processor_ide.c` and CTest target `processor-ide-session-tests` cover workspace reuse, mode admission, invalid identifiers, target validation and denied-by-default behavior.

This is the first native core slice, not a graphical IDE. The UI, assembler/disassembler, live register/debug providers, pipeline simulator and waveform integration remain separate implementation gates.

## 11. Implementation layers

| Layer | Scope | Acceptance gate |
|---|---|---|
| IDE-0 — Target/session foundation | A project opens with a validated target identity, mode and evidence context. |
| IDE-1 — Source, IR and machine inspection | Source/IR/ASM/bytes/register navigation preserves symbol and source provenance. |
| IDE-2 — Memory, device and privilege | Address, permission, MMIO and device state are explicit and capability-gated. |
| IDE-3 — Simulator and microarchitecture | Named reference CPU models execute deterministic fixtures and emit synchronized traces. |
| IDE-4 — Debug, trace and replay | Bounded snapshots, breakpoints, before/after diffs and reproducible simulator replay pass tests. |
| IDE-5 — Timing, power and signal labs | Measured/estimated/simulated evidence has units, provenance and uncertainty. |
| IDE-6 — HDL, physical design and lab integration | RTL/EDA/lab adapters pass external tool and physical qualification; research claims remain scoped. |
| IDE-7 — Release and ecosystem maturity | Windows/Linux builds, target matrix, compatibility migrations and independent qualification pass. |

Implementation proceeds as separate reviewable increments: target/session model; source/IR/ASM navigation; registers and memory inspection; instruction decoding; deterministic reference CPU simulator; bounded trace/diff/replay; waveform and EDA adapters; qualified hardware debug; then compatibility and ecosystem maturity.

## 12. Verification

Run:

```sh
python tools/validate_processor_ide.py
python -m unittest discover -s tests -p "test_processor_ide_contract.py" -v
```

The validator checks all 100 ordered rule IDs, the two groups of 50, the exact twenty existing workspace domains, source evidence paths, target modes, panes, safety invariants and integration references. The dedicated GitHub Actions workflow is the authoritative CI result.

## 13. Claim policy

The current result is an architecture/requirements contract plus a mapping to the existing Pytyliki registry and APIs. It is not a finished graphical IDE. A capability is promoted from planned/partial only when its panel or backend exists, its target-specific behavior is tested, its evidence provenance is recorded and its relevant CI gates pass.

Processor IDE's defining promise is **machine transparency with explicit boundaries**: the engineer can inspect every supported layer, understand what is measured or modeled, and know exactly which operations could affect a real target.
