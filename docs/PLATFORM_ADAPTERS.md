# PTFR platform adapters and evidence ledger v1

## Scope

This increment connects PTFR to real operating-system hardware-error reporting surfaces while keeping collection read-only. It does not install drivers, enable error sources, inject errors, change firmware, modify processor registers or offline CPUs.

## Windows WHEA event-log adapter

Source: \`src/platform_windows_whea.c\`.

The adapter queries the local \`System\` event log for records from \`Microsoft-Windows-WHEA-Logger\` with the Windows Event Log API (\`EvtQuery\`, \`EvtNext\`, \`EvtRender\`). WHEA publishes hardware-error notifications as ETW events and records them in the system event log. This is a bounded pull reader, not a persistent ETW session or a subscription callback.

- Maximum batch: 128 events.
- Resume watermark: caller-provided WHEA event-record ID.
- Payload evidence: SHA-256 of the rendered UTF-16 event XML.
- Captured fields: event ID, event-record ID, event time, event level and provider identity.
- CPU attribution: null / \`PL_TR_UNKNOWN_CPU\` until a CPER processor section is decoded and validated.
- Fault classification: unknown unless the event data has been explicitly decoded; an event level is not treated as corrected/uncorrected severity.

The adapter does not call WHEA management APIs and does not enable, disable or inject error sources. Reading the System log may be restricted by local Windows event-log ACLs.

## Linux RAS / EDAC adapter

Source: \`adapters/linux-edac\` (Rust static library) and \`src/platform_linux_edac.c\` (C ABI bridge).

The Rust collector reads only the caller-selected sysfs root, normally \`/sys/devices/system/edac\`. It visits the \`mc\` and \`devices\` branches, within bounded depth and item limits, and reads known cumulative counters:

- \`ce_count\`, \`dimm_ce_count\`, \`edac_ce_count\`;
- \`ue_count\`, \`dimm_ue_count\`, \`edac_ue_count\`.

These are counter snapshots, not one record per corrected or uncorrected fault. \`event_code=1\` identifies a corrected-error counter; \`event_code=2\` identifies an uncorrected-error counter. \`event_kind=counter_snapshot\` makes the sampling semantics explicit. The hardware location is retained as a bounded relative sysfs identity. CPU attribution remains null because a memory-controller or DIMM counter does not identify the application thread or necessarily one processor.

The adapter does not load EDAC modules, enable tracepoints, write sysfs, read raw machine-check MSRs or access \`/dev/mcelog\`. The Rust build is optional and enabled with \`-DPYTYLIKI_ENABLE_RUST_EDAC=ON\`.

## Evidence ledger

Source: \`src/processor_evidence.c\` and \`src/sha256.c\`.

Each normalized event is appended to a fixed-capacity, 256-entry ledger. The ledger assigns a contiguous sequence, calculates an event SHA-256, and calculates a hash-chain digest over the preceding chain head and current event digest. The current process can export the verified chain as JSON Lines with \`pl_tr_evidence_export_jsonl\`.

The chain is tamper-evident, not signed. It does not prove that the OS, firmware, device or event provider is authentic. The payload digest identifies the captured payload; the event and chain digests bind normalized metadata and order. Exported JSON stores u64 values as decimal strings so JavaScript does not lose nanosecond precision.

The ledger is in-memory and append-only for one capture. Export the JSONL file to preserve evidence after process exit. The TypeScript verifier recalculates both SHA-256 layers and checks sequence continuity. The Swift reader checks schema, field bounds and digest shape for presentation; it is not a substitute for cryptographic verification.

## Verified evidence-to-PTFR bridge

Source: \`src/ptfr_evidence_link.c\` and \`.h\`.

The bridge links a platform evidence sequence to a PTFR worker only when the caller supplies all of the following:

1. a CPU ID present in the platform event (unknown CPU is rejected);
2. a matching CPU ID for the application worker;
3. a caller assertion that the CPU-to-worker attribution was independently verified;
4. a mapped monotonic timestamp for the platform event and the worker-fault monotonic timestamp;
5. a caller-specified maximum clock-mapping uncertainty, which must contain the observed delta;
6. a decoded event kind of corrected, uncorrected or fatal, not a cumulative counter snapshot.

A successful bridge operation appends a second SHA-256 chain entry referencing the source event digest and records a normalized PTFR worker fault. That link journal is exported separately as \`pytyliki.ptfr-evidence-link.v1\` JSONL.

Current WHEA entries retain unknown CPU/fault classification until CPER processor sections are decoded. Current EDAC entries are controller/DIMM counter snapshots with no CPU ID. Therefore both current adapters preserve evidence but are intentionally rejected by the worker bridge; they cannot yet prove a particular thread ran on a faulty core. Synthetic bridge tests exercise the accepted path without claiming live hardware attribution.

Callers must serialize mutations to the evidence ledger, link ledger and PTFR monitor. The bridge does not infer a worker from time proximity alone.

## Explicit current-thread affinity lease

Source: \`src/processor_affinity.c\`.

The affinity API applies only to the calling Pytyliki worker thread. It first checks that the requested logical processor is already in that thread's allowed set. It saves the prior mask in a caller-owned lease; \`pl_tr_affinity_restore_current\` restores it and must be called from the same thread.

- Windows: \`GetThreadGroupAffinity\` and \`SetThreadGroupAffinity\`, restricted to the current processor group and its existing mask.
- Linux: \`pthread_getaffinity_np\` and \`pthread_setaffinity_np\`, restricted to the current thread's existing cpuset.
- No admin/root privilege is requested.
- No process-wide affinity mask, unrelated thread, system scheduler policy or CPU online/offline state is changed.
- The API is opt-in. Pinning a worker can reduce scheduler freedom and may reduce performance; default behavior remains unpinned.

This is OS-mediated placement of an application thread, not physical core control. CPU hotplug/offline, interrupt routing, voltage/frequency, firmware, MSR, MMIO and microcode remain outside this application API.

## Assembly, Rust, Swift and TypeScript

| Module | Role | Implemented behavior |
|---|---|---|
| x86-64 Assembly | Read-only processor identity probe | CPUID leaf 1 signature and low 8-bit initial APIC ID; no privileged instruction |
| Rust | Linux EDAC sysfs collector | Bounded, read-only counter snapshots through a C ABI |
| Swift | Native evidence reader/summary CLI | Decodes JSONL, validates envelope and digest shape, produces counters |
| TypeScript | Evidence verifier | Validates u64 strings and recomputes event SHA-256 and the chain |

Reaper owns the shared \`pytyliki.platform-evidence.v1\` envelope validator in \`stdlib/platform_evidence.rpr\`. The Pytyliki workflow pins the Reaper source revision used for conformance.

## Driver and privilege boundary

A custom privileged driver is not required to read the Windows WHEA event log or Linux EDAC sysfs counters. Those surfaces are already published by the operating system and its platform drivers. Adding a generic Pytyliki driver that writes MSRs, MMIO, firmware state, interrupt routing or CPU hotplug controls would duplicate trusted kernel responsibilities and can destabilize the machine.

No KMDF/WDK driver, Linux kernel module, PSHED plug-in, driver-signing package or privileged service is included. A future kernel adapter must have a specific vendor/OS scope, signed driver package, narrow IOCTL ABI, threat model, rollback path, hardware test rig and separate review. It must not expose arbitrary register or physical-memory writes.

## Build and run

Default build remains C11-only and does not require Rust:

\`\`\`sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
\`\`\`

Enable Linux EDAC integration when Rust stable is installed:

\`\`\`sh
cmake -S . -B build-edac -DBUILD_TESTING=ON -DPYTYLIKI_ENABLE_RUST_EDAC=ON
cmake --build build-edac --config Release --parallel
./build-edac/pytyliki-ptfr --edac-root /sys/devices/system/edac --jsonl build-edac/platform-evidence.jsonl
node --experimental-strip-types adapters/typescript-evidence/src/verify.ts build-edac/platform-evidence.jsonl
swift run --package-path adapters/swift-evidence pytyliki-evidence build-edac/platform-evidence.jsonl
\`\`\`

On Windows, the SDK build includes \`pytyliki-ptfr\` and its WHEA pull mode:

\`\`\`powershell
.\\build\\Release\\pytyliki-ptfr.exe --whea 0 --jsonl whea-evidence.jsonl --max-events 64
\`\`\`

## Verification limits

CI can compile the platform adapter and exercise it against a synthetic sysfs tree. A generic CI runner may not expose EDAC hardware counters or meaningful WHEA events. Such tests prove the adapter contract and collector behavior, not physical CPU health, silicon lifetime, CPER decoding completeness, hardware correction success or real-time guarantees.
