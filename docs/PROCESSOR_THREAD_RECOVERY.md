# Processor Thread Fault Recovery (PTFR) v1

## Purpose

PTFR adds a bounded event-correlation and recovery-planning layer to Pytyliki. It observes repeated fault records associated with an application worker, estimates their cadence, and returns a deterministic recovery recommendation. It is designed for low-overhead ingestion: fixed storage, no heap allocation, no I/O, no locks and no unbounded loops in the C11 event path.

PTFR does not repair a CPU core, cache cell, transistor, die, package, via or copper interconnect. It cannot clear a physical defect, undo electromigration or make an uncorrectable machine check recoverable. Hardware recovery belongs to the processor, firmware, operating-system kernel and a vendor-qualified driver. PTFR acts on the application workload after it receives evidence from an authorized provider.

## Current implementation

| Component | Implementation | Boundary |
|---|---|---|
| Event monitor | `src/processor_thread_recovery.c` and `.h` | Fixed 16-stream table; 8 retained inter-arrival intervals per stream |
| Cadence analysis | Median interval and maximum absolute deviation in parts per million | Requires a configurable minimum number of intervals; not a lifetime predictor |
| Recovery planner | Retry, redistribute, quarantine application worker, checkpoint-and-stop, safe-stop | Produces a plan; never applies host or kernel changes |
| Reaper policy | `automation/reaper/processor_thread_recovery.rpr` | Executable policy fixture for `RULE`, `ROOT`, `BOOT`, `START`, `SERVICES`, `SCENARIO` |
| Verification | `tests/test_processor_thread_recovery.c` and Reaper CI | Deterministic fixtures; not a live hardware probe |

The C event path uses caller-owned storage. A monitor is single-writer: callers must serialize writes and coordinate reads, or give each producer its own monitor and merge snapshots outside the event path. The implementation does not create OS threads, install interrupt handlers or make thread-affinity changes. Its bounded work is suitable as an ingestion primitive, but this library alone does not establish hard real-time scheduling guarantees.

## Event identity and cadence

A series is keyed by `(worker_id, fault_code, fault_kind)`. Each event records a monotonic timestamp in nanoseconds and the logical CPU identifier supplied by the caller. The monitor retains up to eight latest intervals. The analyzer sorts a bounded local copy, computes the median interval, then calculates the largest absolute interval deviation relative to that median:

`jitter_ppm = max(abs(interval - median)) * 1,000,000 / median`

A series is periodic only when its interval count reaches the configured minimum and its jitter is within the configured parts-per-million tolerance. Otherwise it is `unknown` or `irregular`. The bounded window intentionally favors recent behavior; it is not a statistical proof of root cause. The source provider, error record, machine identity, timestamp quality and worker attribution must be retained by the host telemetry layer.

Default policy: four intervals (five observations), 100,000 ppm tolerance (10%), and one retry. The policy accepts two to eight minimum intervals, tolerances up to 500,000 ppm (50%), and at most three retries. These are conservative configuration bounds, not universal CPU limits.

## Recovery policy

| Evidence | Recommendation | Preconditions |
|---|---|---|
| Transient application fault | `retry_idempotent` | Task explicitly declared idempotent; retry count is bounded |
| Periodic transient fault | `redistribute_task` | Idempotent task and application-managed worker pool |
| Periodic corrected hardware report | `quarantine_application_worker` | Only a worker managed by this application; operator review required |
| Periodic corrected hardware report without a managed pool | Observe, or checkpoint-and-stop | Never offline a physical CPU or change OS affinity |
| Watchdog recurrence | `checkpoint_and_stop` or `safe_stop` | A valid checkpoint must exist for checkpoint recovery |
| Uncorrected or fatal hardware error | `safe_stop` | Escalate to platform/OS recovery; preserve evidence |
| Insufficient or irregular evidence | `observe` | Do not infer a periodic defect from too few samples |

Every C decision has `applied = 0`, `host_cpu_state_change = 0` and `requires_privileged_adapter = 0`. The plan is advisory. In particular, `quarantine_application_worker` means removing a worker from a Pytyliki-owned pool; it never means offlining a CPU core, changing scheduler affinity or writing a processor register.

Retries must be idempotent. A non-idempotent task is never automatically replayed because a retry could duplicate external side effects. Redistribution is limited to a pool owned by the application. The host operating system remains responsible for actual thread placement.

## Reaper command model

The Reaper source defines the requested uppercase typed operands as ordinary validated policy functions:

- `RULE(...)` constructs the bounded cadence/retry policy.
- `ROOT(...)` supplies a named policy trust anchor and allowed application scope. It is **not** the operating-system `root` account, privilege elevation or a kernel capability.
- `BOOT(...)` validates the policy and creates a `ready` PTFR state.
- `START(...)` transitions the policy state from `ready` to `running`.
- `SERVICES()` enumerates the policy service contracts.
- `SCENARIO(...)` evaluates ordered timestamps and task-safety flags and emits a versioned decision record.

`SCENARIO` accepts explicit caller-supplied timestamps and fault classifications. The checked-in fixture data is synthetic. A CI pass proves that the Reaper policy behaves as specified for those inputs; it does not prove that a sensor backend or a hardware correction path is installed.

## Command-line use

The native CLI exposes the same bounded planner for caller-exported evidence:

```sh
pytyliki --thread-recovery path/to/processor-faults.csv
pytyliki --thread-recovery-demo
```

The CSV reader is deliberately offline and read-only. It accepts at most 4,096 records and 16 independent event series. Each row has six unquoted decimal/text fields:

```text
worker_id,timestamp_ns,cpu_id,fault_code,fault_kind,task_flags
7,1000000000,2,65,corrected_hardware,4
```

Supported fault kinds are `transient`, `corrected_hardware`, `uncorrected_hardware`, `watchdog` and `fatal`. The final bitmask is the OR of `IDEMPOTENT=1`, `CHECKPOINTED=2` and `MANAGED_POOL=4`. Timestamps must be monotonic within each worker/fault/kind series. The CLI rejects malformed rows, zero identifiers, overflow, inconsistent task flags and out-of-order timestamps.

The demo and checked-in CSV fixture are synthetic conformance inputs. An imported CSV does not become a hardware event merely because it uses a hardware fault-kind label; retain the exporting provider, original record identifier, machine identity, clock domain and attribution quality in the upstream evidence system. The current CLI reports the recovery plan only and cannot apply it to the host scheduler.

## Hardware evidence and platform integration

The event provider is deliberately separate from the monitor. A future Windows provider can consume WHEA event records and ETW notifications. WHEA distinguishes corrected from uncorrected errors and supports user-mode monitoring through event records and ETW; low-level capture and recovery are coordinated by the operating system and platform-specific components. A future Linux provider can consume RAS/EDAC reports and the corresponding kernel interfaces. EDAC exposes corrected and uncorrected error observations; a rise in corrected errors can justify preventive service, but is not by itself proof that an uncorrected error will occur.

This increment does not implement either provider. No Windows event subscription, ETW session, Linux sysfs reader, `/dev/mcelog` reader, kernel driver, PSHED plug-in or privileged service is included. Provider identity and source provenance must be part of every event in a future adapter. Events from synthetic tests and live hardware must never be mixed in the same evidence series.

## Nanometer-scale devices and copper interconnects

The useful software role at the nano-interconnect level is observation, correlation and serviceability—not self-healing. Copper back-end-of-line wiring is subject to electromigration: current-driven atomic transport can create voids and hillocks, with resistance increase, open circuits or shorts. Scaling changes the contribution of grain boundaries, interfaces and barriers; current density, temperature, geometry, microstructure and mechanical stress all matter. These are physical-design and process-reliability concerns, not operating-system thread-scheduling controls.

A credible silicon-reliability research track should therefore separate:

1. **System telemetry:** corrected/uncorrected machine checks, ECC counts, cache/bus error records, temperature, power and source location with provenance.
2. **Statistical degradation analysis:** trend rates, exposure time, workload, temperature/current conditions, confidence bounds and false-positive controls.
3. **Physical modelling:** electromigration lifetime and stress/void-growth models (for example, Black-type lifetime models, Blech-length effects, atomic-flux-divergence and stress-diffusion models), calibrated to foundry/process data.
4. **Design qualification:** layout/DRC, extracted parasitics, IR-drop, current-density, thermal, via and lifetime checks in qualified EDA/TCAD flows.
5. **Field response:** reduce application exposure, checkpoint/stop affected work, report the exact source and recommend platform service or replacement when evidence warrants it.

A user-mode program cannot inspect individual copper atoms or reverse a void in a nanowire. Such a claim would require physical intervention or a redesigned chip/process, not a Reaper command.

## Language and toolchain ownership

| Technology | PTFR role | Status in this increment |
|---|---|---|
| Reaper `.rpr` | Policy operands, state transitions, typed reports and scenario conformance | Implemented as a Reaper reference-runtime program and CI fixture |
| C11 | Bounded event ingestion, cadence analysis and recovery decision API | Implemented and unit-tested |
| Assembly | Inspect compiler output or provide separately qualified, target-specific probes | No hand-written privileged assembly or CPU register writes |
| Bytecode | Reaper VM execution of the policy program | Uses the existing bounded Reaper VM; not a new native hardware bytecode backend |
| Rust | Candidate for a future memory-safe OS telemetry adapter | Not added: no Rust adapter or Cargo build is part of this repository increment |
| Swift | Candidate for a native macOS/desktop presentation adapter | Not added: no Swift UI or platform integration is part of this increment |
| TypeScript | Candidate for a web/desktop report consumer and generated report types | Not added: no TypeScript application or package pipeline is currently part of this increment |

These language boundaries are intentional. A polyglot wrapper does not grant kernel privilege. Each future adapter must have a versioned contract, explicit platform support, read-only default, source provenance, tests and a separately reviewed capability boundary before it can consume privileged OS interfaces.

## Validation

The native unit tests cover periodic and irregular sequences, jitter, timestamp ordering, bounded capacity, safe retries, application-managed redistribution, corrected-error quarantine gating, watchdog checkpoints, uncorrected/fatal stop decisions, malformed policies and malformed analysis records. The CI workflow checks and executes the Reaper scenario program against an immutable Reaper revision and runs the focused native CTest target.

A successful test result is evidence for the policy and C implementation only. It is not hardware qualification, silicon certification, a guarantee of hard real-time behavior, or proof of live WHEA/EDAC ingestion.
