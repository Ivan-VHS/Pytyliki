# Processor Thread Fault Recovery (PTFR) v1

## Purpose

PTFR adds a bounded event-correlation and recovery-planning layer to Pytyliki. It observes repeated fault records associated with an application worker, estimates their cadence, and returns a deterministic recovery recommendation. The event monitor uses caller-owned fixed storage and does not allocate or perform I/O.

PTFR does not repair a CPU core, cache cell, transistor, die, package, via or copper interconnect. It cannot clear a physical defect, undo electromigration or make an uncorrectable machine check recoverable. It changes application task behavior only where the task contract makes that recovery safe.

## Current implementation

| Component | Implementation | Boundary |
|---|---|---|
| Worker-fault monitor | \`src/processor_thread_recovery.c\` and \`.h\` | Fixed 16-stream table; 8 retained inter-arrival intervals |
| Cadence analysis | Median interval and maximum absolute deviation in ppm | Bounded recent window; not a lifetime predictor |
| Recovery planner | Retry, redistribute, quarantine application worker, checkpoint-and-stop, safe-stop | Plans only; task-safety flags are mandatory |
| WHEA adapter | \`src/platform_windows_whea.c\` | Pulls WHEA Logger records from the Windows System event log; no source-control operations |
| EDAC adapter | \`adapters/linux-edac\` and \`src/platform_linux_edac.c\` | Reads cumulative EDAC sysfs counter snapshots; no writes or module loading |
| Evidence ledger | \`src/processor_evidence.c\`, \`src/sha256.c\` | Fixed 256-entry append-only SHA-256 chain per capture |
| Evidence bridge | \`src/ptfr_evidence_link.c\` | Requires verified CPU/worker mapping and bounded monotonic clock mapping |
| Reaper policy | \`automation/reaper/processor_thread_recovery.rpr\` | \`RULE\`, \`ROOT\`, \`BOOT\`, \`START\`, \`SERVICES\`, \`SCENARIO\` |
| Cross-language consumers | Rust, Swift, TypeScript, x86-64 Assembly | Platform collector, evidence summary/verifier and read-only CPUID helper |

## Worker event identity and cadence

A worker-fault series is keyed by \`(worker_id, fault_code, fault_kind)\`. Each application event records a monotonic timestamp and the logical CPU identifier supplied by the application runtime. The analyzer sorts a bounded local copy of the latest intervals, computes the median, then calculates:

\`jitter_ppm = max(abs(interval - median)) * 1,000,000 / median\`

A series is periodic only when its interval count reaches the configured minimum and its jitter is within the configured parts-per-million tolerance. Otherwise it is \`unknown\` or \`irregular\`. This is a recurrence indicator, not a statistical proof of root cause.

Default policy: four intervals (five observations), 100,000 ppm tolerance (10%), and one retry. Bounds are two to eight intervals, up to 500,000 ppm tolerance and at most three retries.

## Recovery policy

| Evidence | Recommendation | Preconditions |
|---|---|---|
| Transient application fault | \`retry_idempotent\` | Task explicitly declared idempotent; retry count bounded |
| Periodic transient fault | \`redistribute_task\` | Idempotent task and application-managed worker pool |
| Periodic corrected hardware report correlated to a worker | \`quarantine_application_worker\` | Correlation must be independently established; operator review |
| Watchdog recurrence | \`checkpoint_and_stop\` or \`safe_stop\` | A valid checkpoint must exist for checkpoint recovery |
| Uncorrected or fatal hardware error | \`safe_stop\` | Escalate to platform/OS recovery; preserve evidence |
| Insufficient or irregular evidence | \`observe\` | Do not infer a periodic defect from too few samples |

Every C recovery decision has \`applied = 0\`, \`host_cpu_state_change = 0\` and \`requires_privileged_adapter = 0\`. \`quarantine_application_worker\` means removing a worker from a Pytyliki-owned pool. It never means offlining a CPU or changing another process's scheduler affinity.

Retries must be idempotent. Redistribution is limited to the application's own pool. The operating system remains responsible for actual processor placement.

## Reaper command model

The Reaper source defines uppercase typed policy functions:

- \`RULE(...)\` constructs the bounded cadence/retry policy.
- \`ROOT(...)\` supplies a policy trust anchor and allowed application scope. It is not OS root privilege.
- \`BOOT(...)\` creates a validated \`ready\` state.
- \`START(...)\` transitions the policy to \`running\`.
- \`SERVICES()\` lists service contracts.
- \`SCENARIO(...)\` evaluates timestamps and task-safety flags.

The fixture values are synthetic. A Reaper VM pass validates policy behavior; it does not prove a live sensor or driver is present.

The shared Reaper module \`stdlib/platform_evidence.rpr\` validates \`pytyliki.platform-evidence.v1\` records. Reaper checks envelope shape and sequence continuity. The TypeScript verifier recalculates SHA-256 digests.

## Platform evidence adapters

The Windows adapter queries \`Microsoft-Windows-WHEA-Logger\` records from the System event log using the Windows Event Log API. WHEA publishes hardware error notifications through ETW and stores the events in the system log. The adapter is a bounded pull reader (up to 128 records per call), not a long-running ETW session. It retains event ID, event record ID, event time, event level and a SHA-256 digest of the rendered event XML.

The adapter does not yet decode all CPER processor sections. Therefore CPU attribution and corrected/uncorrected fault classification remain unknown unless separately validated; the Windows event level is not used as a hardware-fault classification.

The Linux Rust adapter reads \`ce_count\`, \`ue_count\`, DIMM and EDAC-device counters from the caller-selected \`/sys/devices/system/edac\` root. It emits bounded cumulative counter snapshots, not one record per physical error. The collector does not load modules, enable tracepoints or write sysfs. A memory-controller/DIMM counter is not attributed to a CPU or application thread.

The fixed-capacity C ledger appends normalized source records, assigns contiguous sequence numbers, computes event SHA-256 digests and links them with a chain digest. It exports JSONL. The chain is tamper-evident but not digitally signed and does not establish the authenticity of the kernel, firmware or device.

The evidence-to-PTFR bridge accepts a platform record only when its CPU ID is known and matches a caller-supplied worker CPU ID, the caller explicitly marks attribution as verified, the platform event has a decoded corrected/uncorrected/fatal classification, and a caller-supplied monotonic clock mapping is within the configured uncertainty. A successful join creates a second hash-chained link record and feeds a normalized fault into the PTFR monitor.

Current WHEA records have unknown CPU and fault class; current EDAC records are cumulative counter snapshots with no CPU ID. Therefore the bridge deliberately rejects current adapter records as worker faults. The evidence is still retained and exported. This prevents time proximity or a controller-level counter from being misrepresented as proof about one thread or core.

## Application worker affinity

The explicit affinity lease changes only the calling application thread. The target logical processor must already be in the thread's allowed processor set. The previous mask is saved and can be restored from the same thread. Windows uses group-aware thread affinity APIs; Linux uses pthread affinity APIs.

This is OS-mediated placement for a Pytyliki worker, not physical core hotplug or hardware control. It does not change system-wide scheduling policy, other processes, firmware, frequency, voltage, interrupts or processor registers. It is opt-in; pinning can reduce scheduler freedom and performance.

## Silicon and copper interconnect research

Copper interconnects can degrade through electromigration and stress-driven void/hillock formation. Software can retain evidence, trend counters and reduce application exposure, but it cannot reverse material damage at the transistor, via or interconnect level. Lifetime models require process-specific current density, temperature, geometry, microstructure and foundry calibration. Physical validation belongs to foundry-qualified EDA/TCAD, timing, IR-drop, thermal and reliability flows.

## Language and toolchain ownership

| Technology | PTFR role | Status |
|---|---|---|
| Reaper | Typed policy operands and platform-evidence schema validator | Reaper source and VM conformance fixture |
| C11 | Event ingestion, fixed-capacity evidence ledger, verified PTFR bridge, recovery plans, affinity lease API | Native implementation |
| x86-64 Assembly | CPUID leaf 1 signature and low 8-bit initial APIC ID | Read-only helper; not a unique core identifier |
| Rust | Linux EDAC sysfs adapter | Optional static library with C ABI |
| Swift | Evidence JSONL reader and summary CLI | Implemented consumer; digest shape only |
| TypeScript | Evidence schema and SHA-256 chain verifier | Implemented cryptographic verification |
| Privileged driver | Vendor/platform-specific kernel service | Not implemented; no generic MSR/MMIO/CPU-hotplug write interface |

A custom driver is not needed for the user-mode WHEA event-log or EDAC sysfs read paths. A future kernel adapter needs a specific vendor/OS scope, signed driver package, narrow IOCTL ABI, threat model, rollback, hardware test rig and separate review. It must not expose arbitrary register or physical-memory writes.

## Validation

The native tests cover periodic and irregular worker-fault sequences, jitter, timestamp order, bounded capacities, recovery preconditions, SHA-256 known vectors, evidence-chain integrity/tamper detection, verified and rejected worker joins, and invalid affinity requests. Rust tests use a synthetic sysfs tree. Swift and TypeScript tests validate the consumer and digest layers. Windows CI compiles the WHEA adapter; Linux CI compiles the Rust adapter and Assembly helper.

Passing tests establish software behavior against the tested inputs only. They are not silicon certification, a guarantee of hardware recovery, a measurement of interconnect lifetime or a proof of live hardware availability.
