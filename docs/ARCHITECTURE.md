# Processor workstation architecture and safety contract

## Product boundary

Every module must inspect, measure, validate or simulate a CPU, microarchitecture, processor implementation, execution path, memory subsystem, or DSP datapath. General-purpose functionality is accepted only when it directly supports those tasks.

## Layers

1. **Core** — processor identity, platform topology, bounded baseline execution benchmark, processor-source review and structured report output.
2. **Processor inventory** — architecture classification, vendor/family/model, selected ISA flags, OS memory inventory and logical processor topology.
3. **Execution lab** — bounded integer dependency, memory traversal and matrix workloads; timing and integrity checksum.
4. **Implementation review** — read-only inspection of CPUID, timestamp counters, SIMD intrinsics, vector types, prefetch hints, atomics, fences, volatile accesses, non-temporal stores, alignment and inline assembly.
5. **DSP datapath lab** — bounded numerical sample vectors processed by filter, compressor, saturation and gain kernels. PCM16 WAV is only an import/export test-vector envelope.
6. **Presentation** — native Win32 GUI and cross-platform CLI, both exposing the same local processor-engineering operations.

## Data flow

- The presentation layer requests a processor snapshot; the core obtains OS topology and CPUID data where supported.
- The execution lab runs bounded workloads and reports elapsed time and checksums.
- The implementation reviewer reads selected source files as text and emits heuristic prompts for architecture-specific review.
- The DSP lab validates vector dimensions, sample rate, channel count and payload bounds before allocating test data. Kernels operate only on in-memory vectors.
- Reports contain measurements and explicit evidence limitations; missing telemetry is never invented.

## HyperFusion semantics

- Concreo scores architecture/source-review readiness; Pyemont plans transformations without rewriting files; Bollo evaluates benchmark/memory evidence; JJ-HH cross-checks test and scan evidence; PenWhite records trace events; Serious Serious controls promotion.
- The 6×6 interaction matrix evaluates 36 directed pairs per round, including self-checks. A 1–64 round limit bounds work; diminishing returns permit early convergence.
- Scores are heuristic coordination metrics, not benchmark speedups, correctness proofs or physical processor modifications. The CLI cannot certify CI success and keeps promotion held.

## Safety boundaries

- No privileged elevation, kernel driver, BIOS/UEFI, firmware, microcode, voltage, frequency or power-limit writes.
- CPUID feature bits do not establish OSXSAVE/XCR0 readiness. AVX-family instructions are not dispatched by capability flags alone.
- Workloads retain hard iteration, memory, pass-count and sample-frame ceilings.
- Source review never compiles, loads, executes or modifies inspected files.
- DSP vectors are local test data. No hardware audio capture, device playback, background monitoring or network activity is performed.
- Findings are advisory and must be corroborated by trusted profiler, simulator, compiler, formal, timing or physical-design evidence.

## Extension rules

New modules must declare: processor subsystem, required telemetry, supported architectures, workload limits, failure behavior, evidence quality, and whether the operation is read-only or simulated. Hardware-affecting actions remain out of scope unless a separately reviewed, vendor-documented and explicitly approved interface is designed.


## APSIA implementation boundary

The APSIA router and promotion-evidence gate are advisory C11 policy primitives. They select a candidate language from workload semantics, FFI economics and declared constraints, but do not generate source, invoke compilers, grant capabilities, load native libraries, execute plugins or hot-swap runtime code. The `pytyliki-apsia` CLI exposes the planner only. Contract generation, hermetic polyglot builds, SBOM/signing, differential fuzzing, shadow execution, runtime canaries and artifact persistence remain staged capabilities described in [APSIA 2.0](APSIA_2.0.md).

## ISA capability semantics

- CPUID hardware advertisement and operating-system execution readiness are separate facts.
- `avx_hw` records the CPU AVX bit; `osxsave` records OSXSAVE advertisement; `avx_os_state` confirms XCR0 enables XMM and YMM state.
- `avx`, `avx2`, and `fma` are exposed as usable capabilities only when the required OS vector state is enabled. `avx512f` additionally requires XCR0 opmask, ZMM-hi256, and hi16-ZMM state.
- Capability reporting is not instruction dispatch. A future SIMD backend must still use a safe dispatcher, preserve a scalar fallback, and test the exact runtime environment.

## Blockchain and processor systems lab

The optional systems acceleration module exposes 33 C11 APIs in `systems_accel.h`. It groups native bounded algorithms, caller-owned VM/simulation facilities, planning helpers and callback-backed integrations. The complete per-function contract and execution mode are recorded in [BLOCKCHAIN_CPU_SYSTEMS_LAB.md](BLOCKCHAIN_CPU_SYSTEMS_LAB.md).

- Blockchain support is a proof/trace/simulation toolkit, not a full node, consensus client, EVM or wallet. Hashing, relay-proof verification and ZK aggregation are supplied by protocol-specific callbacks.
- CPU support computes profiles and local models. Clock scaling, governor output, affinity, prefetch, vector width, PMU summaries and CAT masks do not change host settings or execute unsupported ISA instructions.
- The lock-free queue is SPSC. The slab compactor owns only the buffer and descriptors supplied by its caller. Transaction atomicity depends on the caller's executor callbacks.
- FPGA synthesis and firmware overlay sealing require external, explicitly supplied vendor/cryptographic adapters. The core does not write microcode, firmware, MSRs, voltage or frequency controls.

## Build and verification profiles

- Default builds use C11 and compiler warnings; MSVC additionally enables SDL checks.
- GCC/Clang builds may enable `PYTYLIKI_ENABLE_SANITIZERS=ON` for AddressSanitizer and UndefinedBehaviorSanitizer.
- CI runs the normal Windows/Linux build and test matrix plus a Linux sanitizer pass. Sanitizer results are correctness diagnostics, not performance guarantees.
