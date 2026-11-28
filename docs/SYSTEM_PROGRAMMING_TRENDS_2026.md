# Pytyliki processor and hardware systems programming: engineering trends

**Review date:** 2026-11-27  
**Role:** design input for Pytyliki Hardware Extension 0.4  
**Maturity:** research-informed engineering proposal; not a statement that all listed standards or tools are integrated.

## 1. Direction of the field

Modern processor engineering is moving away from isolated instruction encoders and hand-maintained hardware descriptions toward coordinated hardware/software contracts. A language for processor systems should make the following information first-class:

- exact ISA profile and extension dependencies;
- architectural state and exception/interrupt behavior;
- target-specific feature availability and operating-system enablement;
- address-space, ownership, capability and DMA isolation;
- vector, matrix and accelerator execution domains;
- multi-level compiler IR that preserves both software semantics and hardware structure;
- deterministic simulation, trace and replay;
- assertions, coverage, formal properties and differential testing;
- power, performance, area, thermal and timing evidence;
- chiplet, coherent interconnect, IOMMU and device-management contracts.

These are design priorities, not a claim that a single new language can replace ISA specifications, platform firmware, operating-system drivers, EDA tools or formal verification engines.

## 2. Trend-to-language decisions

### 2.1 Profiles instead of vague CPU names

Ratified ISA profiles and platform profiles increasingly define a portable feature baseline. Pytyliki should model a target as a named, versioned profile plus implementation-specific extensions, rather than inferring support from a marketing CPU name.

A target manifest should include architecture family, XLEN/data model, endian, privilege model, required and optional ISA extensions, vector/matrix parameters, cache-line and atomic constraints, ABI, MMU/IOMMU features, interrupt model, debug features, and provenance. Feature probing must separate CPU support from OS-enabled state and hypervisor availability.

Example:

~~~pytyliki
@target riscv.application {
    profile: "RVA23";
    profile_version: "1.0";
    implementation_extensions: [vendor("example", "v1")];
    privilege: [M, S, U];
    execution: firmware;
}
~~~

This is a target declaration example, not a statement that the host CPU implements that profile.

### 2.2 Capability-oriented memory safety

A raw integer address should not be treated as proof of authority. Separate an address value from an access capability, region provenance, permissions and lifetime. Hardware-enforced capability designs such as CHERI/CHERIoT show why bounds, permissions and compartment boundaries should be representable in the language and compiler contracts.

Pytyliki should provide typed address spaces, region-bounded slices, checked offset arithmetic, explicit unsafe escapes, capability tokens and ownership transfer. A capability must be unforgeable within the runtime model and cannot be created by a sigil, text annotation or user-supplied numeric address.

### 2.3 Device memory and IOMMU isolation

DMA is not a CPU pointer with a different name. Model CPU virtual addresses, guest virtual addresses, physical addresses and device-visible I/O virtual addresses as separate spaces. A DMA mapping is a scoped resource associated with a device context and access direction. Mapping, synchronization, completion and unmapping are distinct states.

Target-aware declarations should model IOMMU translation stages, device/process context, permissions, invalidation queues, ATS/PRI only when supported, coherency and pinned-buffer lifetime. The language may describe these contracts; an OS adapter is responsible for actual mapping and device operations.

### 2.4 Multi-level IR

A single flattened IR cannot preserve every property of firmware, architectural state transitions, microarchitecture and RTL. Pytyliki should use explicit domain IRs:

- Firmware IR: typed operations, memory effects, calls, ABI and exceptions.
- ISA IR: instruction identity, operands, architectural state transition, traps and memory ordering.
- Architectural Model IR: cycles abstracted away; precise architectural state and nondeterministic event points.
- Microarchitecture IR: pipeline stages, resources, queues, speculation and implementation-specific timing.
- RTL/Logic IR: bit widths, four-state values, clocks, reset, edge behavior and structural connections.
- Verification IR: assumptions, assertions, covers, temporal properties and proof obligations.

Every transition must be explicit and source-mapped. Reaper's canonical typed IR is the shared compiler interchange foundation, but hardware-specific metadata must survive as typed attributes and explicit side tables/contracts rather than disappearing in generic optimization.

### 2.5 Deterministic simulation and replay

A useful processor simulator needs controlled nondeterminism: interrupts, external inputs, timers, DMA completion, device events and scheduling points must be recordable. A trace should include instruction count or cycle model, event sequence, target profile, initial memory snapshot, device model versions and replay schema.

Pytyliki should support deterministic seeds, checkpoints, event logs and architectural-state comparison. It should not promise cycle accuracy unless the target model implements cycle-level timing.

### 2.6 Formal and coverage-driven verification

Hardware correctness requires more than source parsing and example tests. Add invariants, temporal assertions, covers, bounded model checks, property-based tests and differential comparison between independent models.

A verification result must identify assumptions, clocks, reset model, bounds, target configuration, tool versions, test seeds, coverage scope and artifacts. A passed parser test is not evidence of ISA correctness or RTL equivalence.

### 2.7 Heterogeneous and vector compute

Model scalar, vector, matrix, DSP and accelerator execution separately, with explicit lane count, element width, alignment, execution domain, memory movement and synchronization. A vector extension being present in a CPU is not enough: the operating system may need to enable the relevant architectural state, and the selected target profile must declare legal use.

### 2.8 Chiplet and coherent interconnect systems

Future processor engineering has to represent die-to-die links, coherent memory, device fabrics, management channels, reset domains and error reporting. Pytyliki should model these as typed interconnect contracts and verifiable link state machines, not hardcode one bus into the language core.

### 2.9 Power, performance, area and thermal evidence

Performance claims must be target- and workload-scoped. Add measurement provenance, warmup policy, sample distribution, frequency/power state, temperature, counter multiplexing, clock source, WCET assumptions and confidence intervals. Use WCET or schedulability analysis for real-time claims; average benchmark time is not a deadline proof.

## 3. Proposed language architecture

~~~text
Pytyliki Core
  └─ optional pytyliki.hardware extension
       ├─ target profiles and feature manifests
       ├─ type / address-space / capability model
       ├─ SystemArray + reserved sigil operator registry
       ├─ firmware and inline-assembly declarations
       ├─ ISA and architectural state model
       ├─ microarchitecture and timing model
       ├─ RTL / digital logic / clock-domain model
       ├─ device, bus, DMA and interrupt protocols
       ├─ simulation, trace and deterministic replay
       └─ verification contracts and evidence
             ↓
       Hardware AST + domain-specific IR
             ↓
       Reaper canonical typed IR + Span + SourceMap + DABI64 contracts
             ↓
       explicitly selected backend:
       native target / emulator / RTL toolchain / report-only
~~~

Domain is explicit. Firmware does not silently become RTL. An architectural model does not imply cycle timing. A memory map declaration does not map real memory. A simulator backend does not grant host privileges.

## 4. Target manifest contract

A target profile should be versioned and reproducible:

- target ID, vendor/device/board revision and profile version;
- ISA base, profile, extensions and extension dependencies;
- pointer width, address width, endian, ABI and calling convention;
- privilege levels, page table format and exception model;
- register file, flags, CSR/system register set and feature gating;
- vector/matrix lane shape and legal state save/restore requirements;
- cache hierarchy, coherency and memory ordering;
- interrupt controller, timer and debug model;
- IOMMU and device address translation capabilities;
- clock domains, reset sequencing, timing units and unknowns;
- compiler/backend and external tool versions;
- provenance, license and trust source for the manifest.

Unknown or unavailable fields remain unknown. Do not fill them with fabricated defaults. Unsupported required features are compile errors, not silent fallbacks.

## 5. Required type-system principles

- Distinct `Bit<N>`, `SBit<N>`, `Logic<N>`, `Int<W,Signedness>`, `Vector<W,L>`, `Address<W,Space>` and `Capability<Permission,Space>`.
- Static bit-width and lane-count checks where possible.
- Checked address arithmetic and explicit unsafe wrapping operations.
- Region/lifetime-aware memory views; no implicit conversion between CPU and device addresses.
- Separate RAM, MMIO, register-bank, instruction-memory and DMA access types.
- Explicit `Read`, `Write`, `Execute`, `Map`, `DeviceSubmit`, `InterruptControl` and `DebugWrite` capabilities.
- Effect sets on functions and constructs, including interrupt-safe, may-block, allocation, I/O, privileged, nondeterministic and unsafe effects.
- Ownership states for DMA buffers and descriptors.
- No privilege derivation from spelling, macro expansion or runtime string values.

## 6. Verification contract

For every test or proof result, retain:

- source hash and SourceMap;
- language/extension version;
- target profile and required features;
- compiler/backend and tool versions;
- assumptions and excluded behavior;
- deterministic seed and event log;
- expected result and actual result;
- property/cover identifiers and proof bounds;
- coverage summary and failing trace;
- reproducible invocation and artifact hash.

Required layers: lexer/parser tests, typed-AST tests, type/effect negative tests, ISA golden vectors, architectural differential tests, fuzzing, sanitizer runs, RTL lint, assertion checks, coverage, timing checks and target-board integration tests when hardware is actually available.

## 7. Implementation sequence

1. **Syntax registry:** keep current twenty construct structural parser and introduce stable metadata registries for the thirteen SystemArray sigils.
2. **Typed expression frontend:** full lexer, maximal-munch operators, precedence, AST, Span and SourceMap.
3. **Semantic checker:** names, widths, types, address spaces, capabilities, effects and target-profile compatibility.
4. **Hardware IR:** preserve domain, memory space, privilege, side effects, clock, timing and provenance.
5. **ISA/architectural model:** instruction decode, exact state transitions, exceptions and memory ordering.
6. **Deterministic simulator:** memory model, device contracts, interrupts, trace and record/replay.
7. **Microarchitecture and RTL:** cycle/resource modeling, logic IR, clock domains, synthesis/verification adapters.
8. **Physical adapters:** only per supported platform, explicit opt-in, least privilege, bounded operations and hardware-in-the-loop evidence.

No stage should be advertised as completed merely because a specification or structural parser exists.

## 8. Source references

- RISC-V International, Ratified Specifications Library: https://docs.riscv.org/
- RISC-V ISA manual: https://docs.riscv.org/reference/isa/
- RISC-V IOMMU specification: https://docs.riscv.org/reference/iommu/
- RISC-V Annual Report 2025, security and CHERI/CHERIoT progress: https://riscv.org/wp-content/uploads/2026/01/RISC-V-Annual-Report-2025.pdf
- CHERIoT project and specifications: https://cheriot.org/
- MLIR language reference: https://mlir.llvm.org/docs/LangRef/
- CIRCT hardware compiler infrastructure: https://circt.llvm.org/
- QEMU emulation documentation: https://www.qemu.org/docs/master/about/emulation.html
- QEMU deterministic record/replay: https://www.qemu.org/docs/master/system/replay.html
- Verilator documentation: https://verilator.org/guide/latest/
- cocotb documentation: https://docs.cocotb.org/
- UCIe Consortium specifications: https://www.uciexpress.org/specifications

Review date is 2026-11-27. Standards, profile versions, tool support and silicon availability must be rechecked for each release. The links inform the design; this repository does not claim to implement or certify those external specifications.
