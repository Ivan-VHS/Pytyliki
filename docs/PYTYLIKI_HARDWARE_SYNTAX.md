# Pytyliki hardware constructs and syntax families

**Specification:** draft 0.2 / integrated extension 0.4  
**Frontend:** native C11 structural parser  
**Canonical language owner:** Pytyliki  
**Compiler foundation:** Reaper  
**Status:** construct catalogue and structural syntax subset implemented; semantic analysis and code generation are not implemented by this frontend.

## 1. Design rule

Pytyliki has one language and one semantic model. Its specialized forms are not twenty independent languages. Every construct is intended to lower into a typed Pytyliki AST and Hardware IR, then pass through the Reaper canonical typed-IR and ABI contracts where applicable.

The five syntax families are:

| Family | Constructs |
|---|---|
| Declarative hardware | \`table\`, \`contract\`, \`matrix\`, \`invariant\`, \`memory_map\` |
| Reactive and event-driven | \`stream\`, \`react\`, \`rule\`, \`device\`, \`probe\` |
| Execution and orchestration | \`stack_proc\`, \`fsm\`, \`schedule\`, \`atomic_tx\`, \`dag\` |
| Machine and compiler | \`.section\`, \`fn\`, \`macro_def\` |
| Runtime instrumentation | \`hot_patch\`, \`trace_script\` |

A construct keyword selects a typed node kind. It does not grant permissions, select a physical device, or execute hardware actions merely by appearing in source.

The hardware construct frontend is complemented by the [SystemArray sigil operator registry](SYSTEM_SIGIL_OPERATORS.md). The registry defines stable semantic IDs, scalar/array type signatures and effect metadata for thirteen suffix operators. It remains metadata-only: it does not parse full expressions, resolve overloads, evaluate plans or touch hardware. The consolidated [Hardware Language Extension 0.4](PYTYLIKI_HARDWARE_LANGUAGE_SPEC_0.4.md) defines how these pieces fit with target profiles and Reaper IR.

## 2. Implemented structural subset

The native C11 frontend currently recognizes one top-level construct per input file.

- Block forms: \`table\`, \`stack_proc\`, \`fsm\`, \`contract\`, \`react\`, \`matrix\`, \`schedule\`, \`rule\`, \`atomic_tx\`, \`dag\`, \`macro_def\`, \`hot_patch\`, \`probe\`, \`device\`, \`invariant\`, \`memory_map\`, and \`trace_script\`.
- Pipeline form: \`stream ... ;\`.
- Function form: \`fn ... { ... }\` or \`fn ... = expression;\`.
- Assembly form: \`.section ...\` followed by an assembly section through end-of-file.

The parser checks the top-level construct keyword, required name/event target, body framing, balanced parentheses/brackets/braces, quoted strings, line comments, block comments, and trailing tokens. Input is bounded to one MiB and delimiter nesting is bounded.

It does **not** yet validate statement grammar inside every body, resolve names, check types, infer effects, validate target registers, prove timing, lower to Hardware IR, assemble instructions, synthesize RTL, or execute code. A structurally valid file is not necessarily a semantically valid Pytyliki program.

## 3. Common source forms

### 3.1 Declarative forms

\`\`\`pytyliki
table irq_dispatch [core = 0..3] {
    | IRQ_ID | PRIORITY | HANDLER           | CONTEXT  |
    | 0x01   | HIGH     | capture_irq_state | irq_safe |
}

contract bus_alignment_rule for AXI_BUS {
    require address % 64 == 0;
    require burst_length in 1..16;
    on_violation { emit_diagnostic("BUS_ALIGNMENT"); reject_transfer(); }
}

matrix trace_mode_matrix {
    modes = [DRY, OVERLAY_PLUS];
    cell (DRY, RING_0) => observe_registers(R0..R3);
}

invariant hardware_sanity {
    always temperature_c < 95.0;
    eventually dma_busy == false within 512_cycles;
}

memory_map cpu_window_layout for target("example-board") {
    region SNAP_STORE {
        base: 0x1000_5000;
        size: 64_KiB;
        permissions: [read, write];
        memory_type: dma_coherent;
    }
}
\`\`\`

### 3.2 Reactive and event forms

\`\`\`pytyliki
stream cpu_pulse from telemetry.ring<RingSample>
    |> filter(sample => sample.temperature_c < 88.0)
    |> map(sample => sample.ipc)
    |> sink(telemetry.output, capacity: 1024, overflow: drop_oldest);

react on cache.L2.evict(line_address, way_index, tag) {
    require capability(cache.events);
    telemetry.ring_push_bounded(TRACE_BUF, line_address)?;
}

rule apocrypha_noise_gate priority 100 {
    when input.kind == NOISE_GLYPH and input.entropy > 0.91
    then { discard_untrusted_input(input); }
}

device dma_ring_worker(channel: DmaChannel) {
    on ready { submit_next_owned_descriptor(channel)?; }
    on complete(descriptor) { complete_and_release_descriptor(descriptor)?; }
}

probe ipc_watch on core[0] window 1024_cycles {
    measure retired = pmu::retired_instructions();
    measure cycles = pmu::core_cycles();
    derive ipc = if cycles == 0 { none } else { retired / cycles };
}
\`\`\`

### 3.3 Execution and orchestration forms

\`\`\`pytyliki
stack_proc boot_guard(seed: u64) {
    .stack capacity = 8;
    .clobbers = [R0, R1];
    PUSH_IMM 1
    LOAD_REG R0
    RET_TOP
}

fsm core_power_state initial IDLE {
    state IDLE:
        on tick when load > 0.85 -> goto PERFORMANCE;
    state PERFORMANCE:
        on tick when temperature_c >= 90.0 -> goto THROTTLED;
    state THROTTLED:
        enter { request_clock_throttle(step: 2); }
        on cooled -> goto IDLE;
}

schedule quantum_wheel every 250_cycles {
    core[0]: slice(40_cycles) -> run_ingest();
}

atomic_tx reg_bank_swap(bank_a: RegSet, bank_b: RegSet) {
    let checkpoint = save_checkpoint(R0..R7)?;
    stage_bank_swap(bank_a, bank_b)?;
    verify_register_invariants(bank_b)?;
    commit_state(checkpoint)?;
}

dag pipeline_graph {
    node load_stage : load_region(INPUT_REGION);
    node verify_stage : verify_input();
    node save_stage : commit_snapshot(OUTPUT_REGION);
    load_stage -> verify_stage;
    verify_stage -> save_stage;
}
\`\`\`

### 3.4 Machine and compiler forms

\`\`\`pytyliki
.section .pytyliki.text, "ax"
.global micro_scenario_step

micro_scenario_step:
    LD R0, [R4 + 0]
    ADD R3, R0, R1
    RET

fn fold_address_tree(region: ReadRegion<u64>, start: usize, count: usize)
    -> Result<u64, MemoryError>
{
    require count <= region.length - start;
    return fold_bounded(region, start, count);
}

macro_def vector_lane_shift(lane: bits<64>, count: u32) {
    require count < 64;
    emit(rotate_left(lane, count));
}
\`\`\`

### 3.5 Runtime instrumentation forms

\`\`\`pytyliki
hot_patch overlay_swap {
    let replacement = prepare_overlay(new_overlay)?;
    quiesce_event_source(old_overlay)?;
    drain_inflight_events(timeout: 64_cycles)?;
    atomically_publish(replacement)?;
    retire_after_grace_period(old_overlay);
}

trace_script step_debugger target core[0] {
    mode: read_only;
    attach_debug_session()?;
    set_breakpoint(0x8000_0120)?;
    continue_until_breakpoint()?;
    print_registers(R0, R1, R2);
    dump_memory(authorized_region, words: 4);
}
\`\`\`

## 4. Construct contracts and corrections

### 4.1 \`table\` — interrupt and dispatch tables

A table is declarative data. An interrupt table may select registered handlers, but handler execution has its own rules.

Required future checks: target IRQ range, duplicate IDs, priority ordering, handler signature, interrupt-safe effects, bounded execution, no blocking operations, and explicit deferred-work handoff. A table entry must not silently execute arbitrary allocation, logging, DMA submission, or privilege changes in interrupt context.

### 4.2 \`stream\` — typed pipelines

Every stage needs an input type, output type, effect set, capacity/backpressure policy, cancellation rule, and error policy. \`tap\` is an effectful observation stage and must have an explicit bounded telemetry sink. A stream is not automatically lock-free or real-time.

### 4.3 \`stack_proc\` — operand-stack instruction sequences

The target profile must define the instruction vocabulary, stack effects, maximum depth, underflow/overflow behavior, register clobbers, flags, calling convention and return contract. Pseudo-instructions must be declared and lowered; they must not be mistaken for native opcodes.

### 4.4 \`fsm\` — finite-state machines

States and transitions form a typed state machine. Entry/exit actions have effect contracts. Power-state requests are not equivalent to direct voltage writes: clock, voltage and thermal changes need target capabilities, validated operating ranges, safety envelopes and platform authorization. The default profile cannot raise voltage or alter firmware settings.

### 4.5 \`contract\` — hardware constraints

\`require\` declares a predicate. \`on_violation\` reports or rejects the invalid operation. It must not silently rewrite an address with \`align_down\` and continue: changing the address can redirect access to a different object or device register. Any repair must be a separately typed operation with a validated region and explicit caller consent.

### 4.6 \`.section\` — assembly sections

Section name, flags, alignment, relocations, symbol visibility and instruction encoding are target-specific. Names such as \`SYSCTL OVERLOAD_LOAD_SAVE\` are pseudo-instructions until a target assembler contract defines their expansion. Register names and instruction mnemonics must be validated against the selected ISA profile.

### 4.7 \`react\` — event handlers

Event paths such as \`cache.L2.evict\` require a declared event capability. The event contract defines payload types, execution context, delivery guarantees, reentrancy, queue bounds and dropped-event behavior. Cache names alone do not prove that a hardware event exists.

### 4.8 \`matrix\` — finite transition and mapping matrices

Rows, columns and cells need declared enum domains and one output type. Duplicate cells, missing required cells, unreachable states and incompatible effects are semantic errors. A matrix is not an implicit imperative loop.

### 4.9 \`fn\` — functions

Functions use the common Pytyliki type, ownership, effect and ABI systems. Recursive functions require a termination argument or explicit recursion/stack budget in bounded environments. Address traversal must use typed memory regions, not unchecked physical-address arithmetic.

### 4.10 \`schedule\` — time/core schedules

A schedule declares periods, deadlines, budgets, affinities and resource constraints. A numeric suffix such as \`250_cycles\` is a unit, not a hard real-time guarantee. A real-time claim requires a clock model, interrupt latency, worst-case execution-time bounds, scheduler assumptions and a schedulability analysis.

### 4.11 \`rule\` — prioritized transformation rules

Rules need deterministic priority/tie-breaking, typed pattern bindings, effect declarations, termination limits, conflict detection and an audit trail. Input text or malformed glyphs must remain data; never parse arbitrary noise as executable machine instructions.

### 4.12 \`atomic_tx\` — transactional state changes

The construct describes a transaction protocol, not a magical hardware transaction. The implementation must state whether it provides software rollback, interrupt exclusion, a lock, hardware transactional memory, or an atomic commit primitive. Snapshot restoration alone cannot guarantee isolation from concurrent observers.

### 4.13 \`dag\` — dependency graphs

Edges declare dependencies. Parallel scheduling is legal only after read/write sets, effects, memory ordering, resources and failure dependencies have been checked. A syntactic DAG does not automatically make its nodes safe to run concurrently.

### 4.14 \`macro_def\` — macros

Macros must be hygienic, typed, bounded in expansion depth and preserve source mappings. \`emit\` may only emit nodes allowed by the macro's declared output category. Macro expansion cannot manufacture hardware capabilities or bypass type/effect checks.

### 4.15 \`hot_patch\` — live instrumentation updates

An overlay replacement requires source quiescence, an in-flight event policy, lifetime tracking, atomic publication where supported, rollback and a grace period before old state is reclaimed. This syntax does not authorize executable-code patching or firmware modification.

### 4.16 \`probe\` — performance probes

Counters must be discovered from the target profile and OS privilege context. Multiplexing, counter width/wrap, frequency scaling, zero-cycle intervals and instrumentation overhead must be represented. IPC is undefined when the cycle denominator is zero; return an optional/error result rather than divide by zero.

### 4.17 \`device\` — device state machines

DMA descriptors and buffers must have explicit ownership and lifetimes. Device submission requires pin/map/sync-for-device; completion requires sync-for-CPU, acknowledgement and safe unmap/release. Cache maintenance is architecture- and coherency-model-dependent, not a universal invalidate call.

### 4.18 \`invariant\` — temporal properties

\`always\`, \`eventually\` and \`within\` need a clock domain, sampling semantics, bounded/unbounded interpretation and a verification engine. A property declaration does not itself execute a corrective action. Safety controllers are separate effectful components.

### 4.19 \`memory_map\` — target memory layout

The target owns the address map. Regions must be checked for arithmetic overflow, alignment, overlap, permissions, memory type, cacheability, DMA reachability and reserved areas. A declaration describes a layout; it does not map physical memory or grant access.

### 4.20 \`trace_script\` — debugger scripts

Read-only is the default. Debug attachment, register access, breakpoints and memory dumps require explicit target selection and access permissions. Write, reset, patch and device-control commands must be separate privileged operations and must be disabled in the read-only profile.

## 5. Canonical operational scopes

The older specimen spellings remain recognizable design vocabulary:

- \`& system = tray,overlay+\` describes the intended telemetry/instrumentation scope.
- \`& system = overload"load"save+\` describes the intended load/overload/save workflow.
- \`system(array)\` is a separate array-system directive and does not imply hardware access.

The preferred future block forms are:

\`\`\`pytyliki
with system(tray, overlay_plus) {
    observe bounded_telemetry;
}

with pipeline(load, overload, save_plus) {
    run_bounded_workload();
    verify_snapshot();
}
\`\`\`

These \`with\` forms are design targets. The current structural frontend described here does not parse or execute them. The punctuation framing markers accepted by the separate \`system(array)\` normalizer are syntax metadata, not operators or privilege tokens.

## 6. Structural parser CLI

\`\`\`sh
pytyliki-hw-syntax --version
pytyliki-hw-syntax --catalog
pytyliki-hw-syntax --catalog --json
pytyliki-hw-syntax --check examples/hardware-constructs/01_irq_dispatch.syntax
pytyliki-hw-syntax --ast-json examples/hardware-constructs/01_irq_dispatch.syntax
\`\`\`

The JSON AST schema is \`pytyliki.hardware-construct-ast.v1\`. The catalogue schema is \`pytyliki.hardware-construct-catalog.v1\`. Both explicitly report that source execution was not performed.

## 7. Build and tests

The C11 core exports the structural parser through \`hardware_syntax.h\`. CMake builds \`pytyliki-hw-syntax\`, and CTest covers all twenty-one construct examples plus malformed delimiters, comments, strings, nesting, bounds and AST serialization.

The frontend is intentionally syntax-only. Do not advertise any of the twenty-one constructs as compiled, simulated, synthesized, privileged, real-time, or physically connected until the corresponding semantic analysis, target contract, backend, and integration tests exist.


## 8. Scientific and physical models

The additive **physical_model** construct declares a computational model with explicit quantities, dimensions, parameters, assumptions, uncertainty treatment and provenance.

~~~pytyliki
physical_model radioactive_decay {
    input initial_atoms: Quantity<Count>;
    input elapsed: Quantity<Time>;
    input half_life: Quantity<Time>;

    require initial_atoms >= 0_count;
    require elapsed >= 0_s;
    require half_life > 0_s;

    derive decay_constant = ln(2.0) / half_life;
    derive expected_atoms = initial_atoms * exp(-decay_constant * elapsed);

    uncertainty propagate_poisson_and_parameter_uncertainty;
    invariant expected_atoms >= 0_count;
}
~~~

This construct is a source-language declaration. The current frontend verifies only its outer structure. It does not yet resolve quantity dimensions, evaluate the model, run Monte Carlo samples, access detectors or control physical equipment. Dimensional analysis, numerical semantics, validated datasets and execution backends remain explicit implementation gates.
