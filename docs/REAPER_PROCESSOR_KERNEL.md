# Reaper Processor Control Kernel

Pytyliki now contains a Reaper 0.2 control-plane implementation in
`automation/reaper/processor/`. It is deliberately separate from the existing
C11 discovery, measurement and memory primitives.

## Implemented Reaper modules

| Module | Responsibility |
| --- | --- |
| `state.rpr` | Finite-state machine, immutable snapshots, generations and action ledger |
| `scheduler.rpr` | Bounded balanced work allocation and deterministic logical-lane replay |
| `memory.rpr` | Memory resize proposals, budget checks and generation planning |
| `hardware.rpr` | Read-only processor evidence validation |
| `verification.rpr` | Transition audit, invariant checks and simplified state graph |
| `tray.rpr` | Versioned system-tray status feed for a separate native GUI adapter |
| `processor_kernel.rpr` | End-to-end verification and versioned report export |

The runner exposes operation, state-event, thread-lane, memory-plan and tray-status tables.
The report contract is `pytyliki.processor-kernel-report.v1`; its JSON Schema is
`schemas/processor-kernel-report.v1.schema.json`.

## Run with Reaper 0.2

From the Pytyliki repository root, with the matching Reaper checkout available:

```sh
PYTHONPATH=/absolute/path/to/Reaper python -m reaper --check automation/reaper/processor_kernel.rpr
PYTHONPATH=/absolute/path/to/Reaper python -m reaper --engine vm automation/reaper/processor_kernel.rpr
```

The source modules are imported relative to the runner. The program writes
`build/processor-kernel-report.json` and asserts its key invariants before
export. It uses the verified Reaper stack VM; the existing C11 ABI benchmark
workflow remains independent.

## State, operations and determinism

The state machine has six named phases: `cold`, `ready`, `running`,
`blocked`, `stopped` and `fault`. Every accepted or rejected request gets
a monotonically increasing action sequence and generation. Successful state
changes advance `state_version`; rejected requests preserve the phase and are
still counted in the audit ledger.

The trace verifier replays the transition table, reconciles generations and
counters, and rejects both illegal accepted transitions and state mutation by
rejected operations. The graph exporter collapses repeated edges while
preserving their occurrence counts and self-loops.

Running the same action sequence twice must produce equal Reaper values
for the final snapshot and event stream. The report contains no
wall-clock timestamp or host-specific random value.

## Logical multithreading and processor pulse

The scheduler divides a bounded job count across 1–64 logical lanes using
stable quotient/remainder distribution. Each round visits lanes in ascending
identifier order. The exported dispatch order, per-lane assignments,
completion counts and action pulse are reproducible. The tray feed exports the
kernel version, generation, health indicator, action counters and menu model
without pretending that a native Windows notification-area icon was rendered.

This is a scheduler model, not operating-system concurrency: it starts zero
native threads and does not change CPU affinity, priorities, process state or
the OS scheduler. A future native scheduler adapter needs its own capability
contract and target-platform tests.

## Memory and hardware boundaries

The memory module validates a requested size against a caller-supplied budget,
returns the proposed next size and generation, and marks the operation as
`plan-only`. It never allocates, frees, remaps or resizes process memory. A
host adapter must explicitly validate and apply a proposal.

The hardware module validates a versioned evidence record. The bundled profile
is labelled fixture data; this Reaper program does not probe live hardware.
There are no reads or writes to physical CPU registers, MSRs, MMIO, DMA,
interrupt controllers, firmware or driver interfaces. No clock, voltage,
frequency or microcode settings are changed.

## Acceptance checks

The Reaper VM runner checks:

- six successful transitions and one rejected transition;
- action, generation and state-version reconciliation;
- deterministic replay and a compact five-node transition graph;
- conservation of 100 jobs across six lanes and stable three-round dispatch;
- rejection of zero-worker plans and over-budget memory requests;
- acceptance of read-only evidence and rejection of a writable profile;
- a non-applied memory plan and a versioned JSON report;
- a valid tray-status feed with a separate system-tray adapter requirement.

The Reaper-language GitHub workflow executes the runner. The workflow result,
not the existence of these source files alone, is the evidence that the VM
accepted and executed the complete module set.
