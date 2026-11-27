# Pytyliki Hardware Language Extension 0.4

**Language:** Pytyliki  
**Extension:** `pytyliki.hardware`  
**Revision:** 0.4.0-draft  
**Canonical compiler foundation:** Reaper  
**Current frontend maturity:** structural syntax + metadata registry; full semantic compilation is not yet implemented  
**Review date:** 2026-11-27

## 1. Normative relationship

This is an optional extension of the existing Pytyliki language. It is not a new standalone language and does not replace Pytyliki Core. Ordinary Pytyliki code retains its existing meaning unless a module explicitly enables the hardware extension.

Ownership is divided:

- **Pytyliki Core:** ordinary language grammar and base semantics.
- **Pytyliki Hardware:** target profiles, hardware types and effects, SystemArray, sigil operators, ISA/RTL declarations, device protocols, hardware simulation and verification UX.
- **Reaper:** canonical typed IR, type and compiler foundations, source `Span` and `SourceMap`, shared runtime/ABI contracts such as DABI64.
- **Backend/tool adapter:** actual machine code, object emission, simulator execution, RTL generation or physical platform access for an explicitly named and supported target.

One source language may describe multiple execution domains, but domains may not be silently mixed. Firmware is not RTL. An ISA model is not a microarchitecture. A memory-map declaration does not map physical memory.

## 2. Consolidation of the four source TXT files

The four supplied texts are retained as design history:

1. The two System Processor drafts (the nominal 35,000-character version and the expanded variant) overlap but are not byte-identical. The expanded variant contains additional ALU/register-slice and `tray,overlay` / `overload"load"save` material.
2. The 0.2 hardware extension generalized the processor language design.
3. The 0.3 improved extension supplied the normative architecture: domains, target contracts, typed address spaces, SystemArray, operator semantics, effects, DMA, interrupts, ISA, microarchitecture, RTL, simulation, verification and Reaper IR boundaries.

This 0.4 specification preserves the useful ideas from all four inputs but supersedes inconsistent preliminary pseudo-operations. In particular:

- magic SplitMix constants and hidden writes to R0-R7 are not operator semantics;
- framing punctuation is parser metadata, never a capability or privilege grant;
- aliases denote one SystemArray type and do not select different data transformations;
- software state transactions are not automatically hardware-atomic transactions;
- cache flushing, DMA ownership, clock transitions and MMIO require a target contract;
- pseudocode does not establish that a compiler, simulator or device backend exists.

## 3. Extension activation

~~~pytyliki
@extend pytyliki.hardware version "0.4";
@target riscv.application;
@domain firmware;
@hardware_effects explicit;
@unsafe deny_by_default;

import pytyliki.hardware::{
    target, processor, register, isa, asm,
    bits, memory, cache, atomic, bus, dma, interrupt,
    logic, clock, device, microarch, model,
    simulate, verify, trace, debug
};
~~~

Importing the extension does not grant privileged instructions, physical-device access or write capabilities. Every hardware effect must be represented in the typed program and authorized by the selected execution environment.

## 4. Execution domains

~~~pytyliki
enum HardwareDomain {
    Firmware,
    ArchitecturalModel,
    MicroarchitectureModel,
    StructuralLogic,
    Verification,
    Simulation
}
~~~

- `Firmware` emits code executed by a real or simulated processor.
- `ArchitecturalModel` describes programmer-visible state transitions, instruction semantics, exceptions and memory ordering.
- `MicroarchitectureModel` describes pipeline stages, queues, resources, speculation and implementation-dependent timing.
- `StructuralLogic` describes bit-level combinational/sequential networks and clock/reset domains.
- `Verification` states properties and coverage goals over named models.
- `Simulation` configures execution of a named architectural or microarchitectural model.

Cross-domain conversion requires an explicit lowering or adapter stage. A firmware function cannot be silently reinterpreted as combinational hardware.

## 5. Target profiles and ISA feature closure

CPU marketing names are not target contracts. A target is a versioned profile plus implementation-specific features.

~~~pytyliki
target_profile RiscvApplication {
    architecture = "riscv";
    profile = "RVA23";
    profile_version = "1.0";
    xlen = 64;
    endian = little;
    privilege = [M, S, U];
    required_extensions = profile_required("RVA23");
    optional_extensions = [];
    abi = "lp64d";
    feature_source = signed_manifest("target/riscv/app.json");
}
~~~

Target profile resolution must validate extension dependencies, conflicts, architectural versions, ABI requirements, OS-enabled state and hypervisor constraints. Unknown target features remain unknown and block code generation when required. Host CPUID or device-tree discovery is evidence, not permission.

Profiles must retain source and revision provenance. RISC-V is a practical first profile family because ratified application/embedded profiles, a server platform profile and a separately versioned IOMMU specification provide versioned contracts. Other architectures must be modeled with equal rigor.

## 6. Typed architectural values

~~~pytyliki
type Bit<const N: usize>;
type SBit<const N: usize>;
type Logic<const N: usize>;       // 0, 1, X, Z
type LogicBit = Logic<1>;
type Int<const W: usize, Signedness>;
type Vector<const W: usize, const LANES: usize>;
type Address<const W: usize, Space>;
type ByteCount;
type CycleCount<ClockDomain>;
type Register<RegisterClass, const W: usize>;
~~~

Widths are part of type identity. No implicit conversion exists between signed, unsigned, four-state and ordinary integer values. Arithmetic overflow, shifts, rotations, endianness and bit slicing have explicit semantics.

## 7. Address spaces, capabilities and memory ownership

~~~pytyliki
enum AddressSpace {
    HostVirtual,
    GuestVirtual,
    Physical,
    DeviceIOVA<Device>,
    RegisterSpace<Device>,
    InstructionSpace
}

type Capability<Permission, Space, Region>;
~~~

An integer that happens to contain an address is not proof of permission. A capability binds the address range to a region, permissions, lifetime, provenance and address space.

Required permissions are distinct: `MemoryRead`, `MemoryWrite`, `Execute`, `Map`, `DeviceSubmit`, `InterruptControl`, `DebugRead`, `DebugWrite`. They are never inferred from punctuation, numeric addresses, source strings or macro expansion.

Address arithmetic is checked. Wrapping is explicitly unsafe. Region slices retain their owner and bounds. Capability-oriented memory designs inform the model, but support for a CHERI/CHERIoT target is a separate backend deliverable.

## 8. SystemArray

Canonical source and type identity:

~~~pytyliki
type A = system::array<64, 16, RAM>;
let data: SystemArray<64, 16, RAM> =
    system::array<64, 16, RAM>(ram_base, ram_region, {Read, Write})?;
~~~

SystemArray represents a bounded typed collection over one address space. It records base address, length, stride, alignment, endian, permissions, region and provenance.

Construction verifies required storage size, checked stride, checked multiplication, alignment, address overflow and region bounds. An MMIO bulk array is allowed only for homogeneous registers with no read side effects. Read-clear, FIFO, command and status registers require a typed register interface.

The supported aliases normalize to the same AST identity. Canonical emitted spelling is `system::array`. The old bare `system` spelling is accepted in typed source only under `@allow_legacy_system_shorthand`. Framing tokens preserve source compatibility and never grant access.

See `SYSTEM_SIGIL_OPERATORS.md` for the versioned thirteen-token operator table and implementation boundary.

## 9. Hardware effects

Every function and hardware construct declares effects. Representative effect classes:

~~~pytyliki
effect Pure;
effect MemoryRead<Space>;
effect MemoryWrite<Space>;
effect MmioRead<Device>;
effect MmioWrite<Device>;
effect DmaSubmit<Device>;
effect InterruptSafe;
effect MayBlock;
effect Nondeterministic<EventSource>;
effect Privileged<PrivilegeLevel>;
effect Unsafe;
~~~

A compiler must reject an effect not authorized by the function, domain, target or capability set. Interrupt handlers cannot block, allocate without a proven bounded allocator, or perform unbounded work. Simulation-only and report-only operations cannot be mistaken for physical I/O.

## 10. Register files and architectural state

~~~pytyliki
register_bank Gpr64 {
    width = 64;
    count = 32;
    zero_register = x0;
    program_counter = pc;
    flags = target_defined;
}
~~~

Register identifiers, widths, read/write behavior, privilege, reset value, side effects and ABI role come from the selected ISA profile. Registers are not global variables. Register reads/writes lower to explicit architectural operations. Hidden register clobbers are prohibited; every assembly block declares inputs, outputs, clobbers, memory effects and flags.

## 11. Instruction sets and assembly

Every instruction definition specifies encoding, operand constraints, architectural transition, flags, exceptions, privilege, memory effects, ordering and feature requirements.

~~~pytyliki
isa_instruction ADD {
    encoding = target.encoding("ADD");
    operands = [rd: Gpr, rs1: Gpr, rs2: Gpr];
    transition { rd <- rs1 + rs2; }
    effects = [ArchitecturalStateWrite];
    exceptions = [];
}
~~~

Inline assembly is target-bound and must declare constraints. Unknown mnemonics or pseudo-instructions are errors unless a named assembler contract explicitly defines them. Assembly sections specify alignment, relocation, visibility, permissions and ABI.

## 12. Memory model, atomics and cache

The hardware extension must distinguish compiler ordering, CPU architectural ordering, interconnect ordering and device ordering. Fences are typed by scope and domain. Atomics use an explicit memory-order vocabulary and legal target lowering; the compiler cannot strengthen/relax ordering without a proven equivalence.

Cache operations are target-specific. A cache maintenance operation must state line size, address range, direction, ordering, privilege and whether the line is coherent with devices. A flush call is never a generic substitute for DMA synchronization.

## 13. Bus, DMA and IOMMU

~~~pytyliki
dma_buffer FrameBuffer = dma::allocate(
    device = target.device("net0"),
    length = 4096,
    direction = ToDevice,
    alignment = 64
)?;

let mapping = iommu::map(
    device = target.device("net0"),
    buffer = FrameBuffer,
    permissions = {DeviceRead}
)?;
let ticket = dma::submit(mapping, descriptors)?;
dma::await_completion(ticket)?;
dma::sync_for_cpu(FrameBuffer)?;
iommu::unmap(mapping)?;
~~~

The semantic model must represent CPU address, device-visible IOVA, IOMMU context, access direction, cache coherency, descriptor ownership, completion, invalidation and lifetime. A DMA buffer has explicit state: CPU-owned, prepared-for-device, device-owned, completion-pending, synchronized-for-CPU, released. Illegal transitions are rejected.

The IOMMU model should represent supported translation stages, context tables, queues, fault reporting, invalidation and optional ATS/PRI behavior only where the target profile supports them. Declarations do not execute a real mapping.

## 14. Interrupts and privilege

Interrupt handlers declare source, priority, nesting, register-save contract, execution context, stack budget, worst-case work bound and effects. Deferred work uses explicit queues or task handoff. Privilege transitions name the target exception/return model. A symbol named `IRQ` is not a hardware interrupt unless resolved by target metadata.

## 15. Logic, clocks and resets

~~~pytyliki
logic_module RegisterSlice64 {
    input clk: Clock;
    input reset_n: Reset<ActiveLow>;
    input enable: LogicBit;
    input data_in: Logic<64>;
    output data_out: Logic<64>;

    sequential on rising_edge(clk) {
        if reset_n == 0 { data_out <= 0; }
        else if enable == 1 { data_out <= data_in; }
    }
}
~~~

Structural logic uses four-state values and explicit drivers. Multiple drivers, unresolved widths, illegal clock crossings and incomplete resets are diagnostics. Clock-domain crossings must use recognized synchronizers, async FIFOs or a formally specified protocol. A cycle is meaningful only relative to a declared clock.

## 16. Microarchitecture

~~~pytyliki
microarchitecture CorePipeline {
    stages = [Fetch, Decode, Rename, Dispatch, Execute, Memory, Commit];
    resources = {
        issue_width: 4,
        reorder_entries: 128,
        load_queue_entries: 48,
        store_queue_entries: 32
    };
    speculation = {
        branch_predictor: target_defined;
        recovery: precise_commit;
    };
}
~~~

Microarchitectural state is not architectural state. It may be nondeterministic across implementations while preserving ISA behavior. Timing claims require explicit latency/throughput assumptions, cache model, memory model, queue bounds and clock domain.

## 17. Device and interconnect models

Device models are state machines with typed registers, reset values, event semantics, queue sizes, error reporting and power/clock dependencies. Interconnects specify address windows, transaction IDs, ordering, burst limits, flow control, error response, reset and link state.

Future profiles may describe PCIe/CXL/UCIe and chiplet links, but those are independent protocol contracts and do not become generic CPU opcodes.

## 18. Simulation and deterministic replay

~~~pytyliki
simulation CpuBootTrace {
    target = target_profile.RiscvApplication;
    model = ArchitecturalModel;
    deterministic_seed = 0x5EED;
    record_events = [Interrupt, Timer, DmaCompletion, ExternalInput];
    checkpoint_every = 100000_instructions;
    compare_architectural_state = true;
}
~~~

Record the inputs that make execution nondeterministic. A replay artifact includes target profile, initial state, event log, model version and source/build hashes. Cycle accuracy is claimed only for a cycle-level model.

## 19. Verification and evidence

~~~pytyliki
verify RegisterSlice64 {
    clock = clk;
    reset = reset_n;
    assert reset_behavior;
    assert output_holds_when_disabled;
    cover at_least_one_write;
    prove no_unknown_output_after_reset;
}
~~~

Verification can include unit/golden tests, property-based tests, fuzzing, differential ISA execution, formal properties, assertions, cover points, bounded model checking, RTL lint and hardware-in-loop runs. Every result must state assumptions, target/tool versions, bounds, seeds, coverage and artifact hashes.

## 20. Performance and observability

Measurements record CPU model, ISA/profile, OS state, power/frequency policy when known, thermal state, benchmark inputs, compiler flags, warmup, number of samples, distribution and counter provenance. Do not turn a local benchmark into a universal ranking. WCET and hard-real-time claims require a declared timing model and schedulability proof.

## 21. Compiler pipeline and Reaper integration

~~~text
source
 -> lexer (maximal-munch, source spans)
 -> parser (Pytyliki Core + hardware grammar)
 -> typed Hardware AST
 -> name and target resolution
 -> type, width, capability and effect checks
 -> domain-specific Hardware IR
 -> Reaper Canonical Typed IR + Span + SourceMap
 -> target-specific backend
 -> diagnostics, object/RTL/simulator artifacts
~~~

Hardware metadata must survive lowering: address space, width, signedness, clock domain, memory order, privilege, effects, ownership, target feature and source origin. Generic optimizations cannot erase MMIO volatility, DMA synchronization, interrupt constraints or verification assertions.

Reaper owns the shared canonical typed IR and ABI foundation. Pytyliki owns the processor-specific language experience and engineering contracts. An adapter must be versioned and tested; no undocumented struct layout or implicit ABI assumption is allowed.

## 22. Current repository implementation and maturity

At review date, repository state and the open hardware-syntax PR support the following precise claims:

| Layer | Current state |
|---|---|
| C11 processor workstation / local discovery / bounded workloads | existing repository capability |
| SystemArray alias parser | syntax-only frontend; normalizes aliases |
| Twenty hardware construct forms | structural parser and metadata catalogue in PR #299 |
| Thirteen sigil semantic definitions | specified by this extension |
| Thirteen sigil metadata registry/parser | added in the PR #299 feature branch |
| Typed overload resolution | not implemented |
| Full expression parser and type/effect checker | not implemented |
| Hardware IR lowering to Reaper | contract/specification only |
| ISA machine-code backend and linker | not implemented by this change |
| Deterministic emulator | not implemented by this change |
| RTL synthesis / FPGA flow | not implemented by this change |
| Physical MMIO/DMA / device driver | not implemented by this change |

The SystemArray registry parses one directive and exposes metadata. It does not evaluate an operator or touch hardware. The twenty construct parser checks structure, not every statement inside each body.

## 23. Roadmap

1. **Syntax metadata (this PR):** preserve 20 construct forms and add the 13-entry SystemArray operator registry.
2. **Expression frontend:** lexer, precedence parser, multi-expression modules, spans and diagnostics.
3. **Semantic phase:** overload resolution, types, widths, address spaces, capabilities, effects and target closure.
4. **Hardware IR:** domain-specific IR and lossless Reaper IR adapter.
5. **ISA model:** canonical instruction schema, decoder and architectural state transition tests.
6. **Emulator:** deterministic architectural execution and record/replay.
7. **Microarchitecture / RTL:** timing/resource models, clock-domain checks, RTL adapter and formal validation.
8. **Physical platform:** individually reviewed board/device adapters, least privilege and hardware-in-loop evidence.

Each step is a separate acceptance gate. No design document alone moves a capability to implemented.

## 24. External engineering references

The trend choices in this revision are grounded in versioned ISA/platform profiles, capability-based memory protection, IOMMU device isolation, multi-level IR, deterministic replay and hardware verification tooling. See `SYSTEM_PROGRAMMING_TRENDS_2026.md` for the reviewed sources and rationale.
