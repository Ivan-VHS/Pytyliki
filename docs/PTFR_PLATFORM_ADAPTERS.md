# PTFR platform adapters and evidence journal v1

## Scope and explicit non-goals

This increment connects Processor Thread Fault Recovery (PTFR) to read-only operating-system reliability evidence and reversible placement of a thread owned by the Pytyliki process.

It does **not** install a custom kernel driver, offline physical CPU cores, change firmware, voltage, frequency, microcode or processor registers, or claim to heal a physical defect. Existing Windows and Linux platform facilities already expose the required read-side information. A generic privileged driver would add attack surface without improving these operations.

## Platform telemetry

| Platform | Provider | What is read | What is not inferred |
|---|---|---|---|
| Windows | WHEA-Logger through the System Event Log / Windows Event Log ETW subscription | EventRecordID, event ID, level and event timestamp | CPER severity, affected worker, physical root cause |
| Linux | EDAC sysfs at `/sys/devices/system/edac/mc/mc*/ce_count` and `ue_count` | Per-controller corrected and uncorrected cumulative counters | CPU/thread attribution, event timestamp inside hardware, cache or interconnect root cause |

The Windows provider subscribes to future WHEA-Logger events and also queries the System channel after its saved EventRecordID to close the startup gap. It uses the Event Log pull-subscription model and a bounded batch of 32 events. Raw event IDs and levels are preserved; severity remains `unknown` until a trusted decoder for the exact event schema is available. Event timestamps are converted from FILETIME to Unix nanoseconds.

The Linux Rust provider reads EDAC counters without writing sysfs. The first sample is recorded with the `COUNTER_BASELINE` flag, so the pre-existing cumulative count is not mistaken for a newly observed fault. Later samples emit counter deltas. Their timestamp is the adapter's process-monotonic poll time, not the physical occurrence timestamp. A counter decrease is flagged as a reset rather than converted to a negative delta.

Neither provider attributes a system-wide machine-check or memory-controller count to a particular application thread. The RAS and PTFR worker streams remain distinct until a platform source supplies a defensible causal link.

## Versioned report contract

The CLI emits `pytyliki.ras-provider-report.v1`, validated by `schemas/ras-provider-report.v1.schema.json`. Every event includes source, clock domain, provider record ID, event ID, severity, count delta, controller ID, flags and `attribution = system-wide`.

- `windows-whea` records use `unix-realtime` and currently retain `severity = unknown`.
- `linux-edac` records use `process-monotonic` and identify corrected or uncorrected counter deltas.
- Baseline and counter-reset flags are retained across language adapters.
- A provider that is unavailable returns `status = unsupported`; it does not fabricate an empty successful measurement.

## Persistent evidence journal

`ras_evidence.h` and `ras_evidence.c` provide a fixed-capacity 128-entry journal. Each record has a local monotonic sequence number plus the provider's record identity and provenance. Repeated provider records are deduplicated. When the bounded ring fills, the oldest record is overwritten and the dropped count is incremented.

The journal stores the logical provider cursor—WHEA EventRecordID or EDAC controller counter baselines—so the next poll resumes instead of replaying the same history. OS subscription handles are never serialized. Checkpoints use the existing atomic snapshot store with schema version and CRC validation.

Run one read-only provider poll and atomically update a journal file:

```sh
./build/pytyliki --ras-evidence ./pytyliki-ras-evidence.bin
```

The caller chooses the path. No background service is installed, and no privileged service runs at boot. The journal records system-wide hardware evidence and must not be used as proof that a particular Pytyliki worker caused an event.

## Reversible application-thread placement

`processor_placement.h` exposes three operations:

- `pl_cpu_placement_first_allowed` finds one currently allowed logical processor.
- `pl_cpu_placement_apply_current_thread` changes only the calling application thread after explicit consent and captures the previous placement.
- `pl_cpu_placement_restore_current_thread` restores that thread's saved placement.

Linux uses `sched_getaffinity` and `sched_setaffinity`; the allowed mask is checked before applying a singleton CPU mask. Windows uses the CPU Sets APIs to select one CPU Set for the calling thread. Windows CPU Sets are a soft placement preference subject to OS policy, not a guarantee that a thread will remain on one physical core. The CTest case checks consent, placement, and restoration.

This API does not change global processor availability or the placement of unrelated processes. No CPU hotplug/offline operation is present. Physical-core offlining is a platform administration operation with system-wide consequences; it is not an automatic recovery action for an application.

## Language responsibilities

| Language | Implemented responsibility | Privilege |
|---|---|---|
| Reaper | Validates versioned RAS JSON, source attribution, clock domain and event bounds in the VM | None |
| C11 | Provider ABI, thread-placement API, bounded evidence journal and snapshot checkpointing | Current-process operations only |
| x86-64 Assembly | `PAUSE` spin hint, assembled by GNU assembler or MASM | Unprivileged instruction only |
| Rust | Linux EDAC sysfs provider, static library linked by CMake when Cargo is present | Read-only sysfs |
| Swift | Swift Package to decode, validate and summarize the versioned RAS report | None |
| TypeScript | Strictly typed JSON report validation, aggregation and Node tests | None |
| Bytecode | Existing Reaper VM executes the policy/conformance program | Existing VM boundary |

Swift and TypeScript are report consumers; they do not load kernel modules or open hardware registers. Rust is only the Linux EDAC provider in this increment. No hand-written privileged assembly is included.

## Driver and silicon boundary

No custom privileged driver is included. WHEA/ETW and EDAC are operating-system facilities; application-thread placement is available through normal user-mode APIs. A future driver would need a narrowly scoped IOCTL contract, a signed build, threat analysis, hardware-specific allowlists, timeout and rollback rules, Windows Driver Verifier or the applicable Linux driver test process, and independent review. It must not expose arbitrary MSR/MMIO writes to the application.

Corrected hardware records can justify observation and service review; they do not mean software can reverse electromigration, repair copper voids or restore a damaged transistor. Physical reliability requires device/process evidence and engineering qualification, not a runtime thread command.

## Validation

The platform workflow builds C11 on Linux and Windows, runs the C tests, builds/tests the Rust provider, runs Swift Package tests on macOS and TypeScript typecheck/Node tests, and executes the Reaper report-contract fixture. CI results are the evidence for platform compilation; they do not substitute for hardware qualification on every CPU, BIOS and memory-controller combination.
